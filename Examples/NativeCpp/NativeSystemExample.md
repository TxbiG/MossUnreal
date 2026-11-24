# Native C++ system example

`NativeSystemExample.cpp` shows a normal C++ class implementing `IMossSystem` and using `FMossSystemRegistry`. Neither class derives from `UObject`, `AActor` or `UActorComponent`. The example can be adapted to an Unreal module, subsystem or other application-owned loop. The caller controls when the registry initializes, ticks and shuts down.
