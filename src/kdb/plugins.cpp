#include <plugins.hpp>

#include <iostream>
#include <algorithm>

using namespace std;
using namespace kdb;

void Plugins::addProvided (Plugin &plugin)
{
	std::string provide;
	std::stringstream ss(plugin.lookupInfo("provides"));
	while (ss >> provide)
	{
		alreadyProvided.push_back(provide);
		cout << "add provide: " << provide << endl;
	}
}

void Plugins::checkProvided(Plugin &plugin)
{
	std::string need;
	std::stringstream nss(plugin.lookupInfo("needs"));
	while (nss >> need)
	{
		cout << "check for need " << need << endl;;
		if (std::find(alreadyProvided.begin(), alreadyProvided.end(), need) == alreadyProvided.end())
		{
			throw MissingNeeded(need);
		}
	}

}

void Plugins::checkStorage (Plugin &plugin)
{
	if (std::string(plugin.lookupInfo("provides")).find("storage") != string::npos)
	{
		cout << "This is a storage plugin" << endl;
		++ nrStoragePlugins;
	}

	if (nrStoragePlugins>1)
	{
		-- nrStoragePlugins;
		throw StoragePlugin();
	}
}

void Plugins::checkResolver (Plugin &plugin)
{
	if (std::string(plugin.lookupInfo("provides")).find("resolver") != string::npos)
	{
		cout << "This is a resolver plugin" << endl;
		++ nrResolverPlugins;
	}


	if (nrResolverPlugins>1)
	{
		-- nrResolverPlugins;
		throw ResolverPlugin();
	}

}

void Plugins::checkInfo (Plugin &plugin)
{
	if (std::string(plugin.lookupInfo("licence")).find("BSD") == string::npos)
	{
		cout << "Warning this plugin is not BSD licenced" << endl;
		cout << "It might taint the licence of the overall product" << endl;
		cout << "Its licence is: " << plugin.lookupInfo("licence") << endl;
	}
}






void ErrorPlugins::tryPlugin (Plugin &plugin)
{
	if (std::string(plugin.lookupInfo("provides")).find("storage") != string::npos)
	{
		cout << "Ignore storage plugin in ErrorPlugins" << endl;
		return;
	}

	if (!plugin.getSymbol("error"))
	{
		throw MissingSymbol("error");
	}

	checkResolver (plugin);
}


void GetPlugins::tryPlugin (Plugin &plugin)
{
	if (!plugin.getSymbol("get"))
	{
		throw MissingSymbol("get");
	}

	checkStorage (plugin);
	checkResolver (plugin);
	checkInfo (plugin);
}

void SetPlugins::tryPlugin (Plugin &plugin)
{
	if (!plugin.getSymbol("set"))
	{
		throw MissingSymbol("set");
	}


	checkStorage (plugin);
	checkResolver (plugin);
	checkInfo (plugin);
}






void ErrorPlugins::addPlugin (Plugin &plugin)
{
	if (std::string(plugin.lookupInfo("provides")).find("resolver") != string::npos)
	{
		// hack, do with proper placement
		plugins[0] = &plugin;
	}
}

void GetPlugins::addPlugin (Plugin &plugin)
{
	if (std::string(plugin.lookupInfo("provides")).find("storage") != string::npos)
	{
		// hack, do with proper placement
		plugins[3] = &plugin;
	}

	if (std::string(plugin.lookupInfo("provides")).find("resolver") != string::npos)
	{
		// hack, do with proper placement
		plugins[0] = &plugin;
	}
}

void SetPlugins::addPlugin (Plugin &plugin)
{
	if (std::string(plugin.lookupInfo("provides")).find("storage") != string::npos)
	{
		// hack, do with proper placement
		plugins[3] = &plugin;
	}

	if (std::string(plugin.lookupInfo("provides")).find("resolver") != string::npos)
	{
		// hack, do with proper placement
		plugins[0] = &plugin;
		plugins[7] = &plugin;
	}
}




bool ErrorPlugins::validated ()
{
	return nrResolverPlugins == 1;
}

bool GetPlugins::validated ()
{
	return nrStoragePlugins == 1 && nrResolverPlugins == 1;
}

bool SetPlugins::validated ()
{
	return nrStoragePlugins == 1 && nrResolverPlugins == 1;
}





void ErrorPlugins::serialize (Key &baseKey, KeySet &ret)
{
	ret.append (*Key (baseKey.getName() + "/errorplugins",
		KEY_COMMENT, "List of plugins to use",
		KEY_END));

	for (int i=0; i< 10; ++i)
	{
		if (plugins[i] == 0) continue;

		std::ostringstream pluginNumber;
		pluginNumber << i;
		ret.append (*Key (baseKey.getName() + "/errorplugins/#" + pluginNumber.str() + plugins[i]->refname(),
			KEY_COMMENT, "A plugin",
			KEY_END));
	}
}

void GetPlugins::serialize (Key &baseKey, KeySet &ret)
{
	ret.append (*Key (baseKey.getName() + "/getplugins",
		KEY_COMMENT, "List of plugins to use",
		KEY_END));

	for (int i=0; i< 10; ++i)
	{
		if (plugins[i] == 0) continue;

		std::ostringstream pluginNumber;
		pluginNumber << i;
		ret.append (*Key (baseKey.getName() + "/getplugins/#" + pluginNumber.str() + plugins[i]->refname(),
			KEY_COMMENT, "A plugin",
			KEY_END));
	}
}


void SetPlugins::serialize (Key &baseKey, KeySet &ret)
{
	ret.append (*Key (baseKey.getName() + "/setplugins",
		KEY_COMMENT, "List of plugins to use",
		KEY_END));

	for (int i=0; i< 10; ++i)
	{
		if (plugins[i] == 0) continue;

		std::ostringstream pluginNumber;
		pluginNumber << i;
		ret.append (*Key (baseKey.getName() + "/setplugins/#" + pluginNumber.str() + plugins[i]->refname(),
			KEY_COMMENT, "A plugin",
			KEY_END));
	}
}
