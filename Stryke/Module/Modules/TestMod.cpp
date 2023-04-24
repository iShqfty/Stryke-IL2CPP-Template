#include "TestMod.h"
#include "Module.h"

TestMod::TestMod() : SModule("TestMod", Category::PLAYER, "A Test Mod As A Template") {
	this->registerSetting(SettingType::FLOAT, "Test Float", this->testFloat));
}

//SModule Class Methods (Inherited).

void TestMod::onEnable() {
	//Initial Call Prior To Being Enabled. You Should Make Sure The Hooks You Use Are Valid, Check They Aren't == nullptr, And If They Are, Disable Them.
	//You Might Not Need To Do Anything Here Besides Disabling Used Hooks That Are == nullptr, Like I Did Here.
	
	//if (this->getBase().getLocalPlayer() == nullptr) {
	//	this->setEnabled(false);
	//	return;
	//}
}

void TestMod::onFrame() {
	//Called Every Frame If The Method Is Enabled. Always Make Sure Hook Is A Valid Pointer.

	//if (this->getBase().getLocalPlayer() != nullptr) {
	//	this->getBase().getLocalPlayer()->SetPlayerSpeed(1.6f);
	//}
}

void TestMod::onDisable() {
	//Final Call After Being Disabled. You Should Make Sure The Hooks You Use Are Valid, And Do The Disabling If So.

	//if (this->getBase().getLocalPlayer() != nullptr) {
	//	this->getBase().getLocalPlayer()->SetPlayerSpeed(this->testFloat);
	//}
}
