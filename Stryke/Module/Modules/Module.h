#pragma once

#include "../../../Utils/Hooks/Hooks.h"

enum Category {
	LOBBY = 0,
	PLAYER = 1,
	MAP = 2,
	VISUAL = 3,
	MISC = 4
};

enum SettingType {
	BOOL = 0,
	INT = 1,
	FLOAT = 2
};

class Setting {
private:
	SettingType type;
	const char* desc;
	auto* var;
public:
	Setting(SettingType type, const char* desc, auto var);
	
}

class SModule {
public:
	bool prev = false;
	bool enabled = false;
private:
	const char* name;
	Category cat;
	const char* desc;
	std::vector<Setting> settings;
public:
	SModule(const char* name, Category c, const char* desc);

	const char* getName() { return this->name; }
	const char* getDesc() { return this->desc; }
	Category getCateg() { return this->cat; }
	bool isEnabled() { return this->enabled; }
	void checkEnabled();
	void setEnabled(bool enb);
	void registerSetting(SettingType type, const char* desc, auto* var);

	virtual void onEnable();
	virtual void onDisable();
	virtual void onFrame();

	Hooks getBase() { return hook; }
};
