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
	FLOAT_ST = 1
};

struct SettingVar {
	union {
		bool _boolst;
		float _floatst;
	};
};

class Setting {
public:
	SettingType type;
	SettingVar* val;
	SettingVar* minVal;
	SettingVar* maxVal;
	const char* desc;

	Setting(SettingType type, const char* desc);
};

class SModule {
public:
	bool prev = false;
	bool enabled = false;
	bool extended = false;
private:
	const char* name;
	Category cat;
	const char* desc;
	std::vector<Setting*> settings;
public:
	SModule(const char* name, Category c, const char* desc);

	const char* getName() { return this->name; }
	const char* getDesc() { return this->desc; }
	Category getCateg() { return this->cat; }
	std::vector<Setting*> getSettings() { return this->settings; }
	bool isEnabled() { return this->enabled; }
	void checkEnabled();
	void setEnabled(bool enb);
	bool isExtended() { return this->extended; }
	void addBoolSetting(const char* desc, bool* var);
	void addNumSetting(const char* desc, float* var, float minVal, float maxVal);
	

	virtual void onEnable();
	virtual void onDisable();
	virtual void onFrame();

	Hooks getBase() { return hook; }
};
