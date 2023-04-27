#include "Module.h"

Setting::Setting(SettingType type, const char* desc) {
	this->type = type;
	this->desc = desc;
}

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

void SModule::addBoolSetting(const char* desc, bool* var) {
	Setting* setting = new Setting(SettingType::BOOL_ST, desc);
	setting->val = reinterpret_cast<SettingVar*>(var);

	this->settings.push_back(setting);
}

void SModule::addNumSetting(const char* desc, float* var , float minVal, float maxVal) {
	Setting* setting = new Setting(SettingType::FLOAT_ST, desc);
	setting->val = reinterpret_cast<SettingVar*>(var);

	SettingVar* se = new SettingVar();
	se->_floatst = minVal;

	SettingVar* st = new SettingVar();
	st->_floatst = maxVal;

	setting->minVal = se;
	setting->maxVal = st;

	this->settings.push_back(setting);
}