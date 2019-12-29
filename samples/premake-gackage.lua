local packageName, customArgs = ...;

local g = {};

local supported = {
	application_sample = true,
	uiappfilter_sample = true,
};

function g.submodule(name)
	if ( supported[name] ) then
		return name;
	else
		error(string.format("Submodule '%s' not supported by %s", name, packageName));
	end
end

return g;
