// Build in a C++ module that depends on MossUnreal and has the matching
// Moss include root, library and Vulkan headers configured in the build
// environment. This intentionally uses the plugin's selective public-header
// boundary rather than Moss/Moss.h, whose global containers conflict with UE.
#include "Core/MossNative.h"

#include <iostream>

int main()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    const Vec3 A(3.0f, 4.0f, 0.0f);
    const Vec3 B(1.0f, 0.0f, 0.0f);
    const Vec3 Normal = A.Normalized();

    std::cout << "Moss Vec3 length: " << A.Length() << "\n";
    std::cout << "Moss Vec3 dot: " << A.Dot(B) << "\n";
    std::cout << "Moss CPU cores: " << Moss_GetAvailableCPUCores() << "\n";

    const std::shared_ptr<Json> Document = Json::parse(R"({"framework":"Moss"})");
    if (!Document)
        return 1;

    std::cout << "Moss JSON: " << Document->stringify() << "\n";
    (void)Normal;
    return 0;
#else
    std::cerr << "MossUnreal was built without a matching Moss include root and library.\n";
    return 2;
#endif
}
