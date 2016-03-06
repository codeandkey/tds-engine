-- tds build script, uses premake5

solution "tds"
	configurations { "debug", "release" }

	project "tds"
		kind "SharedLib"
		language "C"
		targetdir ""

		files { "**.h", "**.c" }

		configuration "linux"
			includedirs { "/usr/include/glib-2.0", "/usr/lib/glib-2.0/include", "/usr/include/pango-1.0", "/usr/include/cairo" }
			links { "m", "GL", "dl", "glfw", "cairo", "pango-1.0", "pangocairo-1.0", "openal", "lua" }
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
			links { "m", "GL", "dl", "glfw", "cairo", "pango-1.0", "pangocairo-1.0" }
			flags { "Symbols" }
			targetname "tds_debug"

		configuration "release"
			flags { "Optimize", "EnableSSE", "EnableSSE2", "ExtraWarnings" }
			targetname "tds"
