"""
PlatformIO extra script for the native (PC) environment.

build_src_filter only works inside src_dir, so the PC target and the ports that live
outside src/ are added here as extra source folders. The include paths are set with
build_flags in platformio.ini, so they also apply when the libraries are compiled.

Optional option in the env:
  custom_toolchain_bin = C:/msys64/mingw64/bin
    prepended to PATH, for when the host gcc/g++ is not on PATH (MinGW on Windows)
"""
import os

Import("env")  # noqa: F821  (provided by PlatformIO/SCons)

toolchain_bin = env.GetProjectOption("custom_toolchain_bin", "").strip()
if toolchain_bin:
    if os.path.isdir(toolchain_bin):
        env.PrependENVPath("PATH", toolchain_bin)
    else:
        print("pio_native.py: custom_toolchain_bin does not exist: " + toolchain_bin)

if os.name == "nt":
    # winsock + the Windows networking used by the PC ports
    env.Append(LIBS=["ws2_32", "winhttp", "iphlpapi", "bcrypt"])

project_dir = env.subst("$PROJECT_DIR")

# BuildSources compiles with the env as it is now, before the library dependency finder
# has added the library include paths, so the libraries used by the PC target are added here
libdeps_dir = env.subst("$PROJECT_LIBDEPS_DIR/$PIOENV")
env.Append(CPPPATH=[
    os.path.join(libdeps_dir, "ArduinoJson", "src"),
    os.path.join(libdeps_dir, "Time"),
    os.path.join(libdeps_dir, "TimeAlarms"),
    os.path.join(project_dir, "lib", "PubSubClient", "src"),
])

def add_sources(name, folder, src_filter="+<*>"):
    env.BuildSources(os.path.join("$BUILD_DIR", name), os.path.join(project_dir, folder), src_filter)

add_sources("ports_common", "ports/common")
add_sources("ports_pc", "ports/pc")
add_sources("pc_simulation", "pc_simulation", "-<*> +<main_pc.cpp> +<commandLoop.cpp>")
