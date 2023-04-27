#pragma once

#include "Modules/Module.h"

#include <typeinfo>
#include <vector>
#include <optional>
#include <memory>
#include <mutex>
#include <shared_mutex>

class ModManager {
public:
	std::vector<std::shared_ptr<SModule>> mods;

	void initMods();
	void deInitMods();
};

inline ModManager mmgr;