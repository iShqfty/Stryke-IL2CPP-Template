#pragma once

//a pch for developing all custom SDK Files
//include il2cpp.h to give the clases usable types.

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