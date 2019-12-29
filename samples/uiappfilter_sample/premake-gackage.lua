local packageName, customArgs = ...;
local g = {};

function g.project()
	project(packageName)
		kind "WindowedApp"
		files {
			"*.cpp",
			"*.h",
		}

		gackage.use {
			"guill.application",
			"guill.uiappfilter",
			"imgui",
		}

		 gackage.call("gackage-android", "generateAndroidSource", {
			 apkName = "UiAppFilterSample",
			 org = "guill",
			 libraries = packageName,
			 appName = "UiAppFilterSample",
		 });
end

return g;
