local packageName, customArgs = ...;
local g = {};

function g.include()
	includedirs {
		"include/",
	}
	gackage.include {
		"abseil.memory",
		"abseil.strings",
		"sdl2",
	}
end

function g.project()
	project(packageName)
		kind "StaticLib"
		files {
			"include/**.h",
			"src/**.h",
			"src/**.cpp",
		}

		g.include()
		filter { "system:android or emscripten" }
			defines {
				"USING_OPENGL_ES",
			}
		filter {}
		gackage.include {
			"glad",
		}
end

function g.link()
	links(packageName)
	gackage.link {
		"abseil.memory",
		"abseil.strings",
		"glad",
		"sdl2",
	}
	filter { "system:emscripten" }
		linkoptions {
			-- Awful bug, see https://github.com/emscripten-ports/SDL2/issues/59
			-- We should remove places we force USE_WEBGL2=1
			"-s USE_WEBGL2=1",
		}
end

return g;
