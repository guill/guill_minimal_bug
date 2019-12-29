local packageName, customArgs = ...;
local g = {};

function g.include()
	includedirs {
		"include/",
	}
	gackage.include {
		"guill.application",
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
		gackage.include {
			"glad",
			"imgui",
			"sdl2",
		}
end

function g.link()
	links(packageName)
	gackage.link {
		"guill.application",
		"imgui",
		"glad",
		"sdl2",
	}
end

return g;
