#pragma once

#include "../../../Utils/Hooks/Hooks.h"

enum Category {
	LOBBY = 0,
	PLAYER = 1,
	MAP = 2,
	VISUAL = 3,
	MISC = 4
};

class SModule {
public:
	bool prev = false;
	bool enabled = false;
private:
	const char* name;
	Category cat;
	const char* desc;
public:
	SModule(const char* name, Category c, const char* desc);

	const char* getName() { return this->name; }
	const char* getDesc() { return this->desc; }
	Category getCateg() { return this->cat; }
	bool isEnabled() { return this->enabled; }
	void checkEnabled();
	void setEnabled(bool enb);

	virtual void onEnable();
	virtual void onDisable();
	virtual void onFrame();

	Hooks getBase() { return hook; }
};