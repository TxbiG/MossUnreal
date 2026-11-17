#pragma once
#include "CoreMinimal.h"

struct ENetHost;
struct ENetPeer;

UENUM(BlueprintType)
enum class EMossNetworkEventType : uint8 { None, Connected, Disconnected, Received };

struct MOSSUNREAL_API FMossNetworkEvent
{
    EMossNetworkEventType Type = EMossNetworkEventType::None;
    uint8 Channel = 0;
    uint32 Data = 0;
    UPTRINT PeerHandle = 0;
    TArray<uint8> Payload;
};

/** Non-blocking host/peer owner over the ENet-style transport in Moss/Moss_Network.h. */
class MOSSUNREAL_API FMossNetworkHost final
{
public:
    FMossNetworkHost() = default;
    ~FMossNetworkHost();
    FMossNetworkHost(const FMossNetworkHost&) = delete;
    FMossNetworkHost& operator=(const FMossNetworkHost&) = delete;
    FMossNetworkHost(FMossNetworkHost&& Other) noexcept;
    FMossNetworkHost& operator=(FMossNetworkHost&& Other) noexcept;

    bool StartServer(uint16 Port, int32 MaxPeers = 32, int32 Channels = 2);
    bool StartClient(int32 MaxPeers = 1, int32 Channels = 2);
    UPTRINT Connect(const FString& HostName, uint16 Port, int32 Channels = 2, uint32 ConnectData = 0);
    bool Send(UPTRINT PeerHandle, uint8 Channel, const TArray<uint8>& Payload, bool bReliable = true);
    void Disconnect(UPTRINT PeerHandle, uint32 Data = 0);
    int32 Poll(TArray<FMossNetworkEvent>& OutEvents, uint32 TimeoutMilliseconds = 0, int32 MaxEvents = 64);
    void Flush();
    void Close();
    bool IsOpen() const { return NativeHost != nullptr; }
    static bool IsNativeAvailable();

private:
    ENetHost* NativeHost = nullptr;
    bool bNetworkInitialized = false;
};
