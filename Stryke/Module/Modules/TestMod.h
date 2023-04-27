#pragma once
#include "Module.h"

class TestMod : public SModule {
private:
	float testFloat = 1.f;
public:
	TestMod();

	virtual void onEnable();
	virtual void onDisable();
	virtual void onFrame();
};