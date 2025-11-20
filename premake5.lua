workspace "BlueLights"
   characterset "Unicode"
   architecture "x86_64"
   configurations { "Debug", "Release" }
   startproject "bluelights"

project "bluelights"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++23"
   -- buildstlmodules "On"

   targetdir ("build/%{cfg.buildcfg}/bin/")
   objdir ("build/obj/%{cfg.buildcfg}/")

   warnings "Extra"

   files { "src/**.hpp", "src/**.cpp", _MAIN_SCRIPT,}

--[[   vpaths {
      ["Headers/*"] = "**.hpp",
      ["Modules/*"] = "**.ixx",
      ["Sources/*"] = "**.cpp",
      --["Resources/*"] = "**.rc",
      ["*"] = {"tramdepot.lua",_MAIN_SCRIPT},
   }]]

      -- Global includes
   includedirs {"."}

--   links {"d3d11"}

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
