#include "Networking/Network.h"
#include "Core/MossNative.h"
#include "Misc/ScopeLock.h"

#if MOSSUNREAL_NATIVE_API_AVAILABLE
namespace
{
    FCriticalSection GMossNetworkMutex;
    int32 GMossNetworkUsers = 0;

    bool AcquireMossNetwork()
    {
        FScopeLock Lock(&GMossNetworkMutex);
        if (GMossNetworkUsers == 0 && Moss_Init_Network() != 0)
            return false;
        ++GMossNetworkUsers;
        return true;
    }

    void ReleaseMossNetwork()
    {
        FScopeLock Lock(&GMossNetworkMutex);
        if (GMossNetworkUsers <= 0)
            return;
        --GMossNetworkUsers;
        if (GMossNetworkUsers == 0)
            Moss_TerminateNetwork();
    }
}
#endif

FMossNetworkHost::~FMossNetworkHost() { Close(); }
FMossNetworkHost::FMossNetworkHost(FMossNetworkHost&& Other) noexcept
    : NativeHost(Other.NativeHost), bNetworkInitialized(Other.bNetworkInitialized)
{
    Other.NativeHost = nullptr;
    Other.bNetworkInitialized = false;
}
FMossNetworkHost& FMossNetworkHost::operator=(FMossNetworkHost&& Other) noexcept
{
    if (this != &Other)
    {
        Close();
        NativeHost = Other.NativeHost;
        bNetworkInitialized = Other.bNetworkInitialized;
        Other.NativeHost = nullptr;
        Other.bNetworkInitialized = false;
    }
    return *this;
}
bool FMossNetworkHost::IsNativeAvailable() { return MOSSUNREAL_NATIVE_API_AVAILABLE != 0; }

bool FMossNetworkHost::StartServer(uint16 Port, int32 MaxPeers, int32 Channels)
{
    Close();
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (!AcquireMossNetwork()) return false;
    bNetworkInitialized = true;
    ENetAddress Address{};
    enet_address_build_any(&Address, ENetAddressType::IPV4);
    Address.port = Port;
    NativeHost = enet_host_create(ENetAddressType::IPV4, &Address,
        static_cast<size_t>(FMath::Max(1, MaxPeers)),
        static_cast<size_t>(FMath::Max(1, Channels)), 0, 0);
    if (!NativeHost) Close();
#endif
    return NativeHost != nullptr;
}

bool FMossNetworkHost::StartClient(int32 MaxPeers, int32 Channels)
{
    Close();
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (!AcquireMossNetwork()) return false;
    bNetworkInitialized = true;
    NativeHost = enet_host_create(ENetAddressType::IPV4, nullptr,
        static_cast<size_t>(FMath::Max(1, MaxPeers)),
        static_cast<size_t>(FMath::Max(1, Channels)), 0, 0);
    if (!NativeHost) Close();
#endif
    return NativeHost != nullptr;
}

UPTRINT FMossNetworkHost::Connect(const FString& HostName, uint16 Port, int32 Channels, uint32 ConnectData)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (!NativeHost || HostName.IsEmpty()) return 0;
    FTCHARToUTF8 UTF8(*HostName);
    ENetAddress Address{};
    Address.type = ENetAddressType::IPV4;
    Address.port = Port;
    if (enet_address_set_host(&Address, ENetAddressType::IPV4, UTF8.Get()) != 0) return 0;
    ENetPeer* Peer = enet_host_connect(NativeHost, &Address,
        static_cast<size_t>(FMath::Max(1, Channels)), ConnectData);
    return reinterpret_cast<UPTRINT>(Peer);
#else
    return 0;
#endif
}

bool FMossNetworkHost::Send(UPTRINT PeerHandle, uint8 Channel, const TArray<uint8>& Payload, bool bReliable)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (!NativeHost || PeerHandle == 0 || Payload.IsEmpty()) return false;
    const uint32 Flags = bReliable ? static_cast<uint32>(ENetPacketFlag::RELIABLE) : 0u;
    ENetPacket* Packet = enet_packet_create(Payload.GetData(), static_cast<size_t>(Payload.Num()), Flags);
    if (!Packet) return false;
    if (enet_peer_send(reinterpret_cast<ENetPeer*>(PeerHandle), Channel, Packet) != 0)
    {
        enet_packet_destroy(Packet);
        return false;
    }
    return true;
#else
    return false;
#endif
}

void FMossNetworkHost::Disconnect(UPTRINT PeerHandle, uint32 Data)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (PeerHandle) enet_peer_disconnect(reinterpret_cast<ENetPeer*>(PeerHandle), Data);
#endif
}

int32 FMossNetworkHost::Poll(TArray<FMossNetworkEvent>& OutEvents, uint32 TimeoutMilliseconds, int32 MaxEvents)
{
    OutEvents.Reset();
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (!NativeHost || MaxEvents <= 0) return 0;
    for (int32 Index = 0; Index < MaxEvents; ++Index)
    {
        ENetEvent NativeEvent{};
        const int Result = enet_host_service(NativeHost, &NativeEvent, Index == 0 ? TimeoutMilliseconds : 0);
        if (Result <= 0) break;
        FMossNetworkEvent Event;
        Event.PeerHandle = reinterpret_cast<UPTRINT>(NativeEvent.peer);
        Event.Channel = NativeEvent.channelID;
        Event.Data = NativeEvent.data;
        switch (NativeEvent.type)
        {
            case ENetEventType::ENET_EVENT_TYPE_CONNECT:
                Event.Type = EMossNetworkEventType::Connected;
                break;
            case ENetEventType::ENET_EVENT_TYPE_DISCONNECT:
                Event.Type = EMossNetworkEventType::Disconnected;
                break;
            case ENetEventType::ENET_EVENT_TYPE_RECEIVE:
                Event.Type = EMossNetworkEventType::Received;
                if (NativeEvent.packet)
                {
                    const size_t PayloadBytes = NativeEvent.packet->dataLength;
                    if (PayloadBytes <= static_cast<size_t>(MAX_int32))
                        Event.Payload.Append(NativeEvent.packet->data, static_cast<int32>(PayloadBytes));
                    enet_packet_destroy(NativeEvent.packet);
                }
                break;
            default:
                Event.Type = EMossNetworkEventType::None;
                if (NativeEvent.packet) enet_packet_destroy(NativeEvent.packet);
                break;
        }
        OutEvents.Add(MoveTemp(Event));
    }
#endif
    return OutEvents.Num();
}

void FMossNetworkHost::Flush()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeHost) enet_host_flush(NativeHost);
#endif
}

void FMossNetworkHost::Close()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeHost) enet_host_destroy(NativeHost);
    if (bNetworkInitialized) ReleaseMossNetwork();
#endif
    NativeHost = nullptr;
    bNetworkInitialized = false;
}
