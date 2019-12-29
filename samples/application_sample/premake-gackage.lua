local packageName, customArgs = ...;
local g = {};

function g.project()
	project(packageName)
		kind "WindowedApp"
		files {
			"*.cpp",
			"*.h"
		}

		gackage.use {
			"guill.application"
		}
end

return g;
