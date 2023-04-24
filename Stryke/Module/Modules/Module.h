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
	BOOL_ST = 0,
	INT_ST = 1,
	FLOAT_ST = 2
};

class SettingVar {
	union {
		bool _boolst;
		int _intst;
		float _floatst;
	};
};

class Setting {
private:
	SettingType type;
	SettingVar* val;
	SettingVar* minVal;
	SettingVar* maxVal;
	const char* desc;
public:
	Setting(SettingType type, const char* desc, SettingVar* val, SettingVar* minVal, SettingVar* maxVal);
	Setting(SettingType type, const char* desc, SettingVar* val);
};

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
	void addBoolSetting(SettingType type, const char* desc, SettingVar* val);
	void addIntSetting(SettingType type, const char* desc, SettingVar* val, SettingVar* minVal, SettingVar* maxVal);
	void addFloatSetting(SettingType type, const char* desc, SettingVar* val, SettingVar* minVal, SettingVar* maxVal);
	

	virtual void onEnable();
	virtual void onDisable();
	virtual void onFrame();

	Hooks getBase() { return hook; }
};
