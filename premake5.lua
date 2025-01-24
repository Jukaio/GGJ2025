workspace "Engine"
    location "build"
    configurations { "Debug", "Release" }
    startproject "Engine"
    platforms { "x86_64", "ARM64" }

    project "Engine"
    location "build/Engine"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir ("build/bin/%{cfg.buildcfg}/%{cfg.architecture}/%{prj.name}")
    objdir ("build/obj/%{cfg.buildcfg}/%{cfg.architecture}/%{prj.name}")

    vpaths {
        ["Headers"] = { "**.h", "**.hpp" },
        ["Source"] = {"**.c", "**.cpp"},
    }

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "src",
        "dependencies/sdl3/include"
    }

    libdirs
    {
        "dependencies/sdl3/lib/%{cfg.system}/%{cfg.architecture}"
    }
    
    postbuildcommands
    {
        "{COPY} %{wks.location}/../dependencies/sdl3/lib/%{cfg.system}/%{cfg.architecture}/SDL3.dll %{cfg.buildtarget.directory}"
    }

    defines
    {
        "SDL_DYNAMIC_API"
    }

    links
    {
        "SDL3.lib"
    }

    flags { "MultiProcessorCompile" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "On"
