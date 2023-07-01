#pragma once

#include "../../MinHook/include/MinHook.h"

#include "../../SDK/SDK.h"

class Hooks {
public:
	CExample* example;
private:
	LPVOID EXAMPLE_EXAMPLEHOOK_ADDR = reinterpret_cast<LPVOID*>(gAssm + (uintptr_t)0xD34DB33F); // Fake Address (RVA For Your Hook)
public:
	CExample* getExample() { if (this->example) /*checking if example is valid ptr*/ return this->example; } //Getting Your Hooks Result (If It != nullptr)
public:
	void AllHooks(bool enable); //Function To Enable All Hooks (Should Be Called During DllMain)
};

inline Hooks hook = Hooks(); //Accessable Hooks Instance
