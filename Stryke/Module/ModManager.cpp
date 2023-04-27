#include "ModManager.h"

#include "Modules/TestMod.h"

void ModManager::initMods() {
	this->mods.push_back(std::shared_ptr<SModule>(new TestMod()));
}

void ModManager::deInitMods() {

	for (auto& mod : this->mods)
		mod.get()->setEnabled(false);
	this->mods.clear();
}