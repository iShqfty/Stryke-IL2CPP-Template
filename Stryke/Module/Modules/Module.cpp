#include "Module.h"

SModule::SModule(const char* name, Category c, const char* desc) {
	this->name = name;
	this->cat = c;
	this->desc = desc;
}

void SModule::onEnable() {}
void SModule::onDisable() {}
void SModule::onFrame() {}

void SModule::checkEnabled() {
	if (this->enabled != this->prev) {
		if (this->enabled)
			this->onEnable();
		else
			this->onDisable();
		this->prev = this->enabled;
	}
}

void SModule::setEnabled(bool enb) {
	if (this->enabled != enb) {
		this->enabled = enb;

		if (enb)
			this->onEnable();
		else
			this->onDisable();
	}
}

void SModule::registerSetting(SettingType type, const char* desc, auto var) {
	this->settings.push_back(Setting(type, desc, var));
}
