-- premake5.lua

workspace "faction"
   configurations { "Debug", "Release" }

   filter "configurations:Debug"
      defines { "FACTION_DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "FACTION_NDEBUG" }
      optimize "On"

   filter "system:windows"
      location "build/windows"
      defines { "FACTION_GLEW", "FACTION_WINDOWS" }
      links { "../libs/glew/lib/%{cfg.buildcfg}/glew32sd" }
      includedirs {"../libs/glew/include"}
      links { "user32", "opengl32", "glu32", "winmm" }

   filter "system:macosx"
      location "build/macosx"
      defines "FACTION_OSX"
      linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo", "-framework SDL2" }

   project "faction"
      kind "WindowedApp"
      files { "code/faction.cpp" }
      defines { "_CRT_SECURE_NO_WARNINGS" }
      macosxversion "10.7"


