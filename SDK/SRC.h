#pragma once


#include "src/il2cpp.h"

#include <inttypes.h>
#include <cstdint>
#include <stdio.h>
#include <Windows.h>
#include <list>
#include <functional>
#include <string>
#include <cstring>

inline uintptr_t gAssm = (uintptr_t)GetModuleHandle("GameAssembly.dll"); //base for il2cpp, change if different .dll base