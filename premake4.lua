-- tds build script, uses premake5

solution "tds"
	configurations { "release", "debug" }

	project "tds"
		kind "SharedLib"
		language "C"
		targetdir ""

		files { "src/**.h", "src/**.c" }

		configuration "linux"
			includedirs { "/usr/include/freetype2" }
			links { "m", "GL", "dl", "glfw", "openal", "lua", "freetype" }
			newaction {
				trigger = "install",
				description = "Install libtds",
				execute = function()
					os.execute("rm -rf /usr/include/tds")
					os.execute("mkdir -p /usr/include/tds")
					os.execute("mkdir -p /usr/include/tds/libs")
					os.execute("mkdir -p /usr/include/tds/objects")
					os.execute("cp src/*.h /usr/include/tds")
					os.execute("cp src/libs/*.h /usr/include/tds/libs")
					os.execute("cp src/objects/*.h /usr/include/tds/objects")
					os.execute("cp *.so /usr/lib")
				end
			}

		configuration "debug"
			defines { "TDS_MEMORY_DEBUG", "TDS_PROFILE_ENABLE" }
			links { "m", "GL", "dl", "glfw", "openal", "lua", "freetype" }
			flags { "Symbols" }
			targetname "tds_debug"

		configuration "release"
			defines { "TDS_IGNORE_SIGFPE" }
			flags { "Optimize", "EnableSSE", "EnableSSE2", "ExtraWarnings" }
			targetname "tds"
