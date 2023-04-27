// dear imgui, v1.75 WIP
// (drawing and font code)

/*

Index of this file:

// [SECTION] STB libraries implementation
// [SECTION] Style functions
// [SECTION] ImDrawList
// [SECTION] ImDrawListSplitter
// [SECTION] ImDrawData
// [SECTION] Helpers ShadeVertsXXX functions
// [SECTION] ImFontConfig
// [SECTION] ImFontAtlas
// [SECTION] ImFontAtlas glyph ranges helpers
// [SECTION] ImFontGlyphRangesBuilder
// [SECTION] ImFont
// [SECTION] Internal Render Helpers
// [SECTION] Decompression code
// [SECTION] Default font data (ProggyClean.ttf)

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#if defined(__GLIBC__) || defined(__sun) || defined(__CYGWIN__) || defined(__APPLE__) || defined(__SWITCH__)
#include <alloca.h>     // alloca (glibc uses <alloca.h>. Note that Cygwin may have _WIN32 defined, so the order matters here)
#elif defined(_WIN32)
#include <malloc.h>     // alloca
#if !defined(alloca)
#define alloca _alloca  // for clang with MS Codegen
#endif
#else
#include <stdlib.h>     // alloca
#endif
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127) // condition expression is constant
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference is.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning : zero as null pointer constant              // some standard header variations use #define NULL 0
#endif
#if __has_warning("-Wcomma")
#pragma clang diagnostic ignored "-Wcomma"                  // warning : possible misuse of comma operator here             //
#endif
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"      // warning : macro name is a reserved identifier                //
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                  // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wstack-protector"          // warning: stack protector not protecting local variables: variable length buffer
#pragma GCC diagnostic ignored "-Wclass-memaccess"          // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

//-------------------------------------------------------------------------
// [SECTION] STB libraries implementation
//-------------------------------------------------------------------------

// Compile time options:
//#define IMGUI_STB_NAMESPACE           ImStb
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wcast-qual"              // warning : cast from 'const xxxx *' to 'xxx *' drops const qualifier //
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

#ifndef STB_RECT_PACK_IMPLEMENTATION                        // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STBRP_ASSERT(x)     IM_ASSERT(x)
#define STBRP_SORT          ImQsort
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_RECT_PACK_FILENAME
#include IMGUI_STB_RECT_PACK_FILENAME
#else
#include "imstb_rectpack.h"
#endif
#endif

#ifndef STB_TRUETYPE_IMPLEMENTATION                         // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)   ((void)(u), IM_ALLOC(x))
#define STBTT_free(x,u)     ((void)(u), IM_FREE(x))
#define STBTT_assert(x)     IM_ASSERT(x)
#define STBTT_fmod(x,y)     ImFmod(x,y)
#define STBTT_sqrt(x)       ImSqrt(x)
#define STBTT_pow(x,y)      ImPow(x,y)
#define STBTT_fabs(x)       ImFabs(x)
#define STBTT_ifloor(x)     ((int)ImFloorStd(x))
#define STBTT_iceil(x)      ((int)ImCeil(x))
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF extern
#endif
#ifdef IMGUI_STB_TRUETYPE_FILENAME
#include IMGUI_STB_TRUETYPE_FILENAME
#else
#include "imstb_truetype.h"
#endif
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// [SECTION] Style functions
//-----------------------------------------------------------------------------

void ImGui::StyleColorsDark(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImGui::StyleColorsClassic(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImGuiCol_Border]                 = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
    colors[ImGuiCol_Separator]              = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

// Those light colors are better suited with a thicker font than the default one + FrameBorder
void ImGui::StyleColorsLight(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.90f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

//-----------------------------------------------------------------------------
// ImDrawList
//-----------------------------------------------------------------------------

ImDrawListSharedData::ImDrawListSharedData()
{
    Font = NULL;
    FontSize = 0.0f;
    CurveTessellationTol = 0.0f;
    ClipRectFullscreen = ImVec4(-8192.0f, -8192.0f, +8192.0f, +8192.0f);
    InitialFlags = ImDrawListFlags_None;

    // Const data
    for (int i = 0; i < IM_ARRAYSIZE(CircleVtx12); i++)
    {
        const float a = ((float)i * 2 * IM_PI) / (float)IM_ARRAYSIZE(CircleVtx12);
        CircleVtx12[i] = ImVec2(ImCos(a), ImSin(a));
    }
}

void ImDrawList::Clear()
{
    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    Flags = _Data ? _Data->InitialFlags : ImDrawListFlags_None;
    _VtxCurrentOffset = 0;
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.resize(0);
    _TextureIdStack.resize(0);
    _Path.resize(0);
    _Splitter.Clear();
}

void ImDrawList::ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.clear();
    _TextureIdStack.clear();
    _Path.clear();
    _Splitter.ClearFreeMemory();
}

ImDrawList* ImDrawList::CloneOutput() const
{
    ImDrawList* dst = IM_NEW(ImDrawList(_Data));
    dst->CmdBuffer = CmdBuffer;
    dst->IdxBuffer = IdxBuffer;
    dst->VtxBuffer = VtxBuffer;
    dst->Flags = Flags;
    return dst;
}

// Using macros because C++ is a terrible language, we want guaranteed inline, no code in header, and no overhead in Debug builds
#define GetCurrentClipRect()    (_ClipRectStack.Size ? _ClipRectStack.Data[_ClipRectStack.Size-1]  : _Data->ClipRectFullscreen)
#define GetCurrentTextureId()   (_TextureIdStack.Size ? _TextureIdStack.Data[_TextureIdStack.Size-1] : (ImTextureID)NULL)

void ImDrawList::AddDrawCmd()
{
    ImDrawCmd draw_cmd;
    draw_cmd.ClipRect = GetCurrentClipRect();
    draw_cmd.TextureId = GetCurrentTextureId();
    draw_cmd.VtxOffset = _VtxCurrentOffset;
    draw_cmd.IdxOffset = IdxBuffer.Size;

    IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* callback_data)
{
    ImDrawCmd* current_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!current_cmd || current_cmd->ElemCount != 0 || current_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        current_cmd = &CmdBuffer.back();
    }
    current_cmd->UserCallback = callback;
    current_cmd->UserCallbackData = callback_data;

    AddDrawCmd(); // Force a new command after us (see comment below)
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::UpdateClipRect()
{
    // If current command is used with different settings we need to add a new command
    const ImVec4 curr_clip_rect = GetCurrentClipRect();
    ImDrawCmd* curr_cmd = CmdBuffer.Size > 0 ? &CmdBuffer.Data[CmdBuffer.Size-1] : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) != 0) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && memcmp(&prev_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) == 0 && prev_cmd->TextureId == GetCurrentTextureId() && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->ClipRect = curr_clip_rect;
}

void ImDrawList::UpdateTextureID()
{
    // If current command is used with different settings we need to add a new command
    const ImTextureID curr_texture_id = GetCurrentTextureId();
    ImDrawCmd* curr_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != curr_texture_id) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && prev_cmd->TextureId == curr_texture_id && memcmp(&prev_cmd->ClipRect, &GetCurrentClipRect(), sizeof(ImVec4)) == 0 && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->TextureId = curr_texture_id;
}

#undef GetCurrentClipRect
#undef GetCurrentTextureId

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(ImVec2 cr_min, ImVec2 cr_max, bool intersect_with_current_clip_rect)
{
    ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect && _ClipRectStack.Size)
    {
        ImVec4 current = _ClipRectStack.Data[_ClipRectStack.Size-1];
        if (cr.x < current.x) cr.x = current.x;
        if (cr.y < current.y) cr.y = current.y;
        if (cr.z > current.z) cr.z = current.z;
        if (cr.w > current.w) cr.w = current.w;
    }
    cr.z = ImMax(cr.x, cr.z);
    cr.w = ImMax(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    UpdateClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
    PushClipRect(ImVec2(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), ImVec2(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
}

void ImDrawList::PopClipRect()
{
    IM_ASSERT(_ClipRectStack.Size > 0);
    _ClipRectStack.pop_back();
    UpdateClipRect();
}

void ImDrawList::PushTextureID(ImTextureID texture_id)
{
    _TextureIdStack.push_back(texture_id);
    UpdateTextureID();
}

void ImDrawList::PopTextureID()
{
    IM_ASSERT(_TextureIdStack.Size > 0);
    _TextureIdStack.pop_back();
    UpdateTextureID();
}

// Reserve space for a number of vertices and indices.
// You must finish filling your reserved data before calling PrimReserve() again, as it may reallocate or 
// submit the intermediate results. PrimUnreserve() can be used to release unused allocations.
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
    // Large mesh support (when enabled)
    IM_ASSERT_PARANOID(idx_count >= 0 && vtx_count >= 0);
    if (sizeof(ImDrawIdx) == 2 && (_VtxCurrentIdx + vtx_count >= (1 << 16)) && (Flags & ImDrawListFlags_AllowVtxOffset))
    {
        _VtxCurrentOffset = VtxBuffer.Size;
        _VtxCurrentIdx = 0;
        AddDrawCmd();
    }

    ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size - 1];
    draw_cmd.ElemCount += idx_count;

    int vtx_buffer_old_size = VtxBuffer.Size;
    VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
    _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

    int idx_buffer_old_size = IdxBuffer.Size;
    IdxBuffer.resize(idx_buffer_old_size + idx_count);
    _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Release the a number of reserved vertices/indices from the end of the last reservation made with PrimReserve().
void ImDrawList::PrimUnreserve(int idx_count, int vtx_count)
{
    IM_ASSERT_PARANOID(idx_count >= 0 && vtx_count >= 0);

    ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size - 1];
    draw_cmd.ElemCount -= idx_count;
    VtxBuffer.shrink(VtxBuffer.Size - vtx_count);
    IdxBuffer.shrink(IdxBuffer.Size - idx_count);
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv(_Data->TexUvWhitePixel);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

// On AddPolyline() and AddConvexPolyFilled() we intentionally avoid using ImVec2 and superflous function calls to optimize debug/non-inlined builds.
// Those macros expects l-values.
#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)     { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = 1.0f / ImSqrt(d2); VX *= inv_len; VY *= inv_len; } }
#define IM_FIXNORMAL2F(VX,VY)               { float d2 = VX*VX + VY*VY; if (d2 < 0.5f) d2 = 0.5f; float inv_lensq = 1.0f / d2; VX *= inv_lensq; VY *= inv_lensq; }

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
// We avoid using the ImVec2 math operators here to reduce cost to a minimum for debug/non-inlined builds.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, bool closed, float thickness)
{
    if (points_count < 2)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    int count = points_count;
    if (!closed)
        count = points_count-1;

    const bool thick_line = thickness > 1.0f;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
    {
        // Anti-aliased stroke
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;

        const int idx_count = thick_line ? count*18 : count*12;
        const int vtx_count = thick_line ? points_count*4 : points_count*3;
        PrimReserve(idx_count, vtx_count);

        // Temporary buffer
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(ImVec2)); //-V630
        ImVec2* temp_points = temp_normals + points_count;

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            float dx = points[i2].x - points[i1].x;
            float dy = points[i2].y - points[i1].y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i1].x = dy;
            temp_normals[i1].y = -dx;
        }
        if (!closed)
            temp_normals[points_count-1] = temp_normals[points_count-2];

        if (!thick_line)
        {
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
                temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
                temp_points[(points_count-1)*2+0] = points[points_count-1] + temp_normals[points_count-1] * AA_SIZE;
                temp_points[(points_count-1)*2+1] = points[points_count-1] - temp_normals[points_count-1] * AA_SIZE;
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+3;

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y)
                dm_x *= AA_SIZE;
                dm_y *= AA_SIZE;

                // Add temporary vertexes
                ImVec2* out_vtx = &temp_points[i2*2];
                out_vtx[0].x = points[i2].x + dm_x;
                out_vtx[0].y = points[i2].y + dm_y;
                out_vtx[1].x = points[i2].x - dm_x;
                out_vtx[1].y = points[i2].y - dm_y;

                // Add indexes
                _IdxWritePtr[0] = (ImDrawIdx)(idx2+0); _IdxWritePtr[1] = (ImDrawIdx)(idx1+0); _IdxWritePtr[2] = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3] = (ImDrawIdx)(idx1+2); _IdxWritePtr[4] = (ImDrawIdx)(idx2+2); _IdxWritePtr[5] = (ImDrawIdx)(idx2+0);
                _IdxWritePtr[6] = (ImDrawIdx)(idx2+1); _IdxWritePtr[7] = (ImDrawIdx)(idx1+1); _IdxWritePtr[8] = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9] = (ImDrawIdx)(idx1+0); _IdxWritePtr[10]= (ImDrawIdx)(idx2+0); _IdxWritePtr[11]= (ImDrawIdx)(idx2+1);
                _IdxWritePtr += 12;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = points[i];          _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
                _VtxWritePtr[1].pos = temp_points[i*2+0]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;
                _VtxWritePtr[2].pos = temp_points[i*2+1]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col_trans;
                _VtxWritePtr += 3;
            }
        }
        else
        {
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+0] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+1] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+2] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+3] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+4;

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
                float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
                float dm_in_x = dm_x * half_inner_thickness;
                float dm_in_y = dm_y * half_inner_thickness;

                // Add temporary vertexes
                ImVec2* out_vtx = &temp_points[i2*4];
                out_vtx[0].x = points[i2].x + dm_out_x;
                out_vtx[0].y = points[i2].y + dm_out_y;
                out_vtx[1].x = points[i2].x + dm_in_x;
                out_vtx[1].y = points[i2].y + dm_in_y;
                out_vtx[2].x = points[i2].x - dm_in_x;
                out_vtx[2].y = points[i2].y - dm_in_y;
                out_vtx[3].x = points[i2].x - dm_out_x;
                out_vtx[3].y = points[i2].y - dm_out_y;

                // Add indexes
                _IdxWritePtr[0]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[1]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[2]  = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3]  = (ImDrawIdx)(idx1+2); _IdxWritePtr[4]  = (ImDrawIdx)(idx2+2); _IdxWritePtr[5]  = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[6]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[7]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[8]  = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9]  = (ImDrawIdx)(idx1+0); _IdxWritePtr[10] = (ImDrawIdx)(idx2+0); _IdxWritePtr[11] = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[12] = (ImDrawIdx)(idx2+2); _IdxWritePtr[13] = (ImDrawIdx)(idx1+2); _IdxWritePtr[14] = (ImDrawIdx)(idx1+3);
                _IdxWritePtr[15] = (ImDrawIdx)(idx1+3); _IdxWritePtr[16] = (ImDrawIdx)(idx2+3); _IdxWritePtr[17] = (ImDrawIdx)(idx2+2);
                _IdxWritePtr += 18;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = temp_points[i*4+0]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col_trans;
                _VtxWritePtr[1].pos = temp_points[i*4+1]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos = temp_points[i*4+2]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos = temp_points[i*4+3]; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col_trans;
                _VtxWritePtr += 4;
            }
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Stroke
        const int idx_count = count*6;
        const int vtx_count = count*4;      // FIXME-OPT: Not sharing edges
        PrimReserve(idx_count, vtx_count);

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            const ImVec2& p1 = points[i1];
            const ImVec2& p2 = points[i2];

            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            dx *= (thickness * 0.5f);
            dy *= (thickness * 0.5f);

            _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
            _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
            _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
            _VtxWritePtr += 4;

            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+2);
            _IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx+2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx+3);
            _IdxWritePtr += 6;
            _VtxCurrentIdx += 4;
        }
    }
}

// We intentionally avoid using ImVec2 and its math operators here to reduce cost to a minimum for debug/non-inlined builds.
void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col)
{
    if (points_count < 3)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    if (Flags & ImDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2)); //-V630
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i0].x = dy;
            temp_normals[i0].y = -dx;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            float dm_x = (n0.x + n1.x) * 0.5f;
            float dm_y = (n0.y + n1.y) * 0.5f;
            IM_FIXNORMAL2F(dm_x, dm_y);
            dm_x *= AA_SIZE * 0.5f;
            dm_y *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos.x = (points[i1].x - dm_x); _VtxWritePtr[0].pos.y = (points[i1].y - dm_y); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos.x = (points[i1].x + dm_x); _VtxWritePtr[1].pos.y = (points[i1].y + dm_y); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+i-1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+i);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}

void ImDrawList::PathArcToFast(const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12)
{
    if (radius == 0.0f || a_min_of_12 > a_max_of_12)
    {
        _Path.push_back(center);
        return;
    }
    _Path.reserve(_Path.Size + (a_max_of_12 - a_min_of_12 + 1));
    for (int a = a_min_of_12; a <= a_max_of_12; a++)
    {
        const ImVec2& c = _Data->CircleVtx12[a % IM_ARRAYSIZE(_Data->CircleVtx12)];
        _Path.push_back(ImVec2(center.x + c.x * radius, center.y + c.y * radius));
    }
}

void ImDrawList::PathArcTo(const ImVec2& center, float radius, float a_min, float a_max, int num_segments)
{
    if (radius == 0.0f)
    {
        _Path.push_back(center);
        return;
    }

    // Note that we are adding a point at both a_min and a_max.
    // If you are trying to draw a full closed circle you don't want the overlapping points!
    _Path.reserve(_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(ImVec2(center.x + ImCos(a) * radius, center.y + ImSin(a) * radius));
    }
}

static void PathBezierToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2+d3) * (d2+d3) < tess_tol * (dx*dx + dy*dy))
    {
        path->push_back(ImVec2(x4, y4));
    }
    else if (level < 10)
    {
        float x12 = (x1+x2)*0.5f,       y12 = (y1+y2)*0.5f;
        float x23 = (x2+x3)*0.5f,       y23 = (y2+y3)*0.5f;
        float x34 = (x3+x4)*0.5f,       y34 = (y3+y4)*0.5f;
        float x123 = (x12+x23)*0.5f,    y123 = (y12+y23)*0.5f;
        float x234 = (x23+x34)*0.5f,    y234 = (y23+y34)*0.5f;
        float x1234 = (x123+x234)*0.5f, y1234 = (y123+y234)*0.5f;

        PathBezierToCasteljau(path, x1,y1,        x12,y12,    x123,y123,  x1234,y1234, tess_tol, level+1);
        PathBezierToCasteljau(path, x1234,y1234,  x234,y234,  x34,y34,    x4,y4,       tess_tol, level+1);
    }
}

void ImDrawList::PathBezierCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        // Auto-tessellated
        PathBezierToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, _Data->CurveTessellationTol, 0);
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
        {
            float t = t_step * i_step;
            float u = 1.0f - t;
            float w1 = u*u*u;
            float w2 = 3*u*u*t;
            float w3 = 3*u*t*t;
            float w4 = t*t*t;
            _Path.push_back(ImVec2(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
        }
    }
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, ImDrawCornerFlags rounding_corners)
{
    rounding = ImMin(rounding, ImFabs(b.x - a.x) * ( ((rounding_corners & ImDrawCornerFlags_Top)  == ImDrawCornerFlags_Top)  || ((rounding_corners & ImDrawCornerFlags_Bot)   == ImDrawCornerFlags_Bot)   ? 0.5f : 1.0f ) - 1.0f);
    rounding = ImMin(rounding, ImFabs(b.y - a.y) * ( ((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f ) - 1.0f);

    if (rounding <= 0.0f || rounding_corners == 0)
    {
        PathLineTo(a);
        PathLineTo(ImVec2(b.x, a.y));
        PathLineTo(b);
        PathLineTo(ImVec2(a.x, b.y));
    }
    else
    {
        const float rounding_tl = (rounding_corners & ImDrawCornerFlags_TopLeft) ? rounding : 0.0f;
        const float rounding_tr = (rounding_corners & ImDrawCornerFlags_TopRight) ? rounding : 0.0f;
        const float rounding_br = (rounding_corners & ImDrawCornerFlags_BotRight) ? rounding : 0.0f;
        const float rounding_bl = (rounding_corners & ImDrawCornerFlags_BotLeft) ? rounding : 0.0f;
        PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
        PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
        PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
        PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
    }
}

void ImDrawList::AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    PathLineTo(p1 + ImVec2(0.5f, 0.5f));
    PathLineTo(p2 + ImVec2(0.5f, 0.5f));
    PathStroke(col, false, thickness);
}

// p_min = upper-left, p_max = lower-right
// Note we don't render 1 pixels sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawCornerFlags rounding_corners, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
        PathRect(p_min + ImVec2(0.50f,0.50f), p_max - ImVec2(0.50f,0.50f), rounding, rounding_corners);
    else
        PathRect(p_min + ImVec2(0.50f,0.50f), p_max - ImVec2(0.49f,0.49f), rounding, rounding_corners); // Better looking lower-right corner and rounded non-AA shapes.
    PathStroke(col, true, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawCornerFlags rounding_corners)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (rounding > 0.0f)
    {
        PathRect(p_min, p_max, rounding, rounding_corners);
        PathFillConvex(col);
    }
    else
    {
        PrimReserve(6, 4);
        PrimRect(p_min, p_max, col);
    }
}

// p_min = upper-left, p_max = lower-right
void ImDrawList::AddRectFilledMultiColor(const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;
    PrimReserve(6, 4);
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2));
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+3));
    PrimWriteVtx(p_min, uv, col_upr_left);
    PrimWriteVtx(ImVec2(p_max.x, p_min.y), uv, col_upr_right);
    PrimWriteVtx(p_max, uv, col_bot_right);
    PrimWriteVtx(ImVec2(p_min.x, p_max.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathLineTo(p4);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathLineTo(p4);
    PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& center, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
    PathFillConvex(col);
}

// Guaranteed to honor 'num_segments'
void ImDrawList::AddNgon(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
    PathStroke(col, true, thickness);
}

// Guaranteed to honor 'num_segments'
void ImDrawList::AddNgonFilled(const ImVec2& center, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
    PathFillConvex(col);
}

void ImDrawList::AddBezierCurve(const ImVec2& pos0, const ImVec2& cp0, const ImVec2& cp1, const ImVec2& pos1, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(pos0);
    PathBezierCurveTo(cp0, cp1, pos1, num_segments);
    PathStroke(col, false, thickness);
}

void ImDrawList::AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (text_end == NULL)
        text_end = text_begin + strlen(text_begin);
    if (text_begin == text_end)
        return;

    // Pull default font/size from the shared ImDrawListSharedData instance
    if (font == NULL)
        font = _Data->Font;
    if (font_size == 0.0f)
        font_size = _Data->FontSize;

    IM_ASSERT(font->ContainerAtlas->TexID == _TextureIdStack.back());  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

    ImVec4 clip_rect = _ClipRectStack.back();
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }
    font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
    AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimRectUV(p_min, p_max, uv_min, uv_max, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& uv1, const ImVec2& uv2, const ImVec2& uv3, const ImVec2& uv4, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimQuadUV(p1, p2, p3, p4, uv1, uv2, uv3, uv4, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageRounded(ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col, float rounding, ImDrawCornerFlags rounding_corners)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (rounding <= 0.0f || (rounding_corners & ImDrawCornerFlags_All) == 0)
    {
        AddImage(user_texture_id, p_min, p_max, uv_min, uv_max, col);
        return;
    }

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    int vert_start_idx = VtxBuffer.Size;
    PathRect(p_min, p_max, rounding, rounding_corners);
    PathFillConvex(col);
    int vert_end_idx = VtxBuffer.Size;
    ImGui::ShadeVertsLinearUV(this, vert_start_idx, vert_end_idx, p_min, p_max, uv_min, uv_max, true);

    if (push_texture_id)
        PopTextureID();
}


//-----------------------------------------------------------------------------
// ImDrawListSplitter
//-----------------------------------------------------------------------------
// FIXME: This may be a little confusing, trying to be a little too low-level/optimal instead of just doing vector swap..
//-----------------------------------------------------------------------------

void ImDrawListSplitter::ClearFreeMemory()
{
    for (int i = 0; i < _Channels.Size; i++)
    {
        if (i == _Current)
            memset(&_Channels[i], 0, sizeof(_Channels[i]));  // Current channel is a copy of CmdBuffer/IdxBuffer, don't destruct again
        _Channels[i]._CmdBuffer.clear();
        _Channels[i]._IdxBuffer.clear();
    }
    _Current = 0;
    _Count = 1;
    _Channels.clear();
}

void ImDrawListSplitter::Split(ImDrawList* draw_list, int channels_count)
{
    IM_ASSERT(_Current == 0 && _Count <= 1);
    int old_channels_count = _Channels.Size;
    if (old_channels_count < channels_count)
        _Channels.resize(channels_count);
    _Count = channels_count;

    // Channels[] (24/32 bytes each) hold storage that we'll swap with draw_list->_CmdBuffer/_IdxBuffer
    // The content of Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
    // When we switch to the next channel, we'll copy draw_list->_CmdBuffer/_IdxBuffer into Channels[0] and then Channels[1] into draw_list->CmdBuffer/_IdxBuffer
    memset(&_Channels[0], 0, sizeof(ImDrawChannel));
    for (int i = 1; i < channels_count; i++)
    {
        if (i >= old_channels_count)
        {
            IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
        }
        else
        {
            _Channels[i]._CmdBuffer.resize(0);
            _Channels[i]._IdxBuffer.resize(0);
        }
        if (_Channels[i]._CmdBuffer.Size == 0)
        {
            ImDrawCmd draw_cmd;
            draw_cmd.ClipRect = draw_list->_ClipRectStack.back();
            draw_cmd.TextureId = draw_list->_TextureIdStack.back();
            _Channels[i]._CmdBuffer.push_back(draw_cmd);
        }
    }
}

static inline bool CanMergeDrawCommands(ImDrawCmd* a, ImDrawCmd* b)
{
    return memcmp(&a->ClipRect, &b->ClipRect, sizeof(a->ClipRect)) == 0 && a->TextureId == b->TextureId && a->VtxOffset == b->VtxOffset && !a->UserCallback && !b->UserCallback;
}

void ImDrawListSplitter::Merge(ImDrawList* draw_list)
{
    // Note that we never use or rely on channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
    if (_Count <= 1)
        return;

    SetCurrentChannel(draw_list, 0);
    if (draw_list->CmdBuffer.Size != 0 && draw_list->CmdBuffer.back().ElemCount == 0)
        draw_list->CmdBuffer.pop_back();

    // Calculate our final buffer sizes. Also fix the incorrect IdxOffset values in each command.
    int new_cmd_buffer_count = 0;
    int new_idx_buffer_count = 0;
    ImDrawCmd* last_cmd = (_Count > 0 && draw_list->CmdBuffer.Size > 0) ? &draw_list->CmdBuffer.back() : NULL;
    int idx_offset = last_cmd ? last_cmd->IdxOffset + last_cmd->ElemCount : 0;
    for (int i = 1; i < _Count; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (ch._CmdBuffer.Size > 0 && ch._CmdBuffer.back().ElemCount == 0)
            ch._CmdBuffer.pop_back();
        if (ch._CmdBuffer.Size > 0 && last_cmd != NULL && CanMergeDrawCommands(last_cmd, &ch._CmdBuffer[0]))
        {
            // Merge previous channel last draw command with current channel first draw command if matching.
            last_cmd->ElemCount += ch._CmdBuffer[0].ElemCount;
            idx_offset += ch._CmdBuffer[0].ElemCount;
            ch._CmdBuffer.erase(ch._CmdBuffer.Data);
        }
        if (ch._CmdBuffer.Size > 0)
            last_cmd = &ch._CmdBuffer.back();
        new_cmd_buffer_count += ch._CmdBuffer.Size;
        new_idx_buffer_count += ch._IdxBuffer.Size;
        for (int cmd_n = 0; cmd_n < ch._CmdBuffer.Size; cmd_n++)
        {
            ch._CmdBuffer.Data[cmd_n].IdxOffset = idx_offset;
            idx_offset += ch._CmdBuffer.Data[cmd_n].ElemCount;
        }
    }
    draw_list->CmdBuffer.resize(draw_list->CmdBuffer.Size + new_cmd_buffer_count);
    draw_list->IdxBuffer.resize(draw_list->IdxBuffer.Size + new_idx_buffer_count);

    // Write commands and indices in order (they are fairly small structures, we don't copy vertices only indices)
    ImDrawCmd* cmd_write = draw_list->CmdBuffer.Data + draw_list->CmdBuffer.Size - new_cmd_buffer_count;
    ImDrawIdx* idx_write = draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size - new_idx_buffer_count;
    for (int i = 1; i < _Count; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (int sz = ch._CmdBuffer.Size) { memcpy(cmd_write, ch._CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
        if (int sz = ch._IdxBuffer.Size) { memcpy(idx_write, ch._IdxBuffer.Data, sz * sizeof(ImDrawIdx)); idx_write += sz; }
    }
    draw_list->_IdxWritePtr = idx_write;
    draw_list->UpdateClipRect(); // We call this instead of AddDrawCmd(), so that empty channels won't produce an extra draw call.
    draw_list->UpdateTextureID();
    _Count = 1;
}

void ImDrawListSplitter::SetCurrentChannel(ImDrawList* draw_list, int idx)
{
    IM_ASSERT(idx >= 0 && idx < _Count);
    if (_Current == idx)
        return;
    // Overwrite ImVector (12/16 bytes), four times. This is merely a silly optimization instead of doing .swap()
    memcpy(&_Channels.Data[_Current]._CmdBuffer, &draw_list->CmdBuffer, sizeof(draw_list->CmdBuffer));
    memcpy(&_Channels.Data[_Current]._IdxBuffer, &draw_list->IdxBuffer, sizeof(draw_list->IdxBuffer));
    _Current = idx;
    memcpy(&draw_list->CmdBuffer, &_Channels.Data[idx]._CmdBuffer, sizeof(draw_list->CmdBuffer));
    memcpy(&draw_list->IdxBuffer, &_Channels.Data[idx]._IdxBuffer, sizeof(draw_list->IdxBuffer));
    draw_list->_IdxWritePtr = draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size;
}

//-----------------------------------------------------------------------------
// [SECTION] ImDrawData
//-----------------------------------------------------------------------------

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
    ImVector<ImDrawVert> new_vtx_buffer;
    TotalVtxCount = TotalIdxCount = 0;
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        if (cmd_list->IdxBuffer.empty())
            continue;
        new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
        for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
            new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
        cmd_list->VtxBuffer.swap(new_vtx_buffer);
        cmd_list->IdxBuffer.resize(0);
        TotalVtxCount += cmd_list->VtxBuffer.Size;
    }
}

// Helper to scale the ClipRect field of each ImDrawCmd.
// Use if your final output buffer is at a different scale than draw_data->DisplaySize,
// or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& fb_scale)
{
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
            cmd->ClipRect = ImVec4(cmd->ClipRect.x * fb_scale.x, cmd->ClipRect.y * fb_scale.y, cmd->ClipRect.z * fb_scale.x, cmd->ClipRect.w * fb_scale.y);
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Helpers ShadeVertsXXX functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = ImLerp((int)(col0 >> IM_COL32_R_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_R_SHIFT) & 0xFF, t);
        int g = ImLerp((int)(col0 >> IM_COL32_G_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_G_SHIFT) & 0xFF, t);
        int b = ImLerp((int)(col0 >> IM_COL32_B_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_B_SHIFT) & 0xFF, t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
    }
}

// Distribute UV over (a, b) rectangle
void ImGui::ShadeVertsLinearUV(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, bool clamp)
{
    const ImVec2 size = b - a;
    const ImVec2 uv_size = uv_b - uv_a;
    const ImVec2 scale = ImVec2(
        size.x != 0.0f ? (uv_size.x / size.x) : 0.0f,
        size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    if (clamp)
    {
        const ImVec2 min = ImMin(uv_a, uv_b);
        const ImVec2 max = ImMax(uv_a, uv_b);
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = ImClamp(uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale), min, max);
    }
    else
    {
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale);
    }
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
    FontData = NULL;
    FontDataSize = 0;
    FontDataOwnedByAtlas = true;
    FontNo = 0;
    SizePixels = 0.0f;
    OversampleH = 3; // FIXME: 2 may be a better default?
    OversampleV = 1;
    PixelSnapH = false;
    GlyphExtraSpacing = ImVec2(0.0f, 0.0f);
    GlyphOffset = ImVec2(0.0f, 0.0f);
    GlyphRanges = NULL;
    GlyphMinAdvanceX = 0.0f;
    GlyphMaxAdvanceX = FLT_MAX;
    MergeMode = false;
    RasterizerFlags = 0x00;
    RasterizerMultiply = 1.0f;
    EllipsisChar = (ImWchar)-1;
    memset(Name, 0, sizeof(Name));
    DstFont = NULL;
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The white texels on the top left are the ones we'll use everywhere in Dear ImGui to render filled shapes.
const int FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF = 108;
const int FONT_ATLAS_DEFAULT_TEX_DATA_H      = 27;
const unsigned int FONT_ATLAS_DEFAULT_TEX_DATA_ID = 0x80000000;
static const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX-     XX          "
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X-    X..X         "
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X-    X..X         "
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X-    X..X         "
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X-    X..X         "
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X-    X..XXX       "
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX-    X..X..XXX    "
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      -    X..X..X..XX  "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       -    X..X..X..X.X "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        -XXX X..X..X..X..X"
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         -X..XX........X..X"
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          -X...X...........X"
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           - X..............X"
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            -  X.............X"
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           -  X.............X"
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          -   X............X"
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          -   X...........X "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       -------------------------------------    X..........X "
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           -    X..........X "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           -     X........X  "
    "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           -     X........X  "
    "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           -     XXXXXXXXXX  "
    "------------        -    X    -           X           -X.....................X-           ------------------"
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -                             "
    "                                                      -  X..X           X..X  -                             "
    "                                                      -   X.X           X.X   -                             "
    "                                                      -    XX           XX    -                             "
};

static const ImVec2 FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[ImGuiMouseCursor_COUNT][3] =
{
    // Pos ........ Size ......... Offset ......
    { ImVec2( 0,3), ImVec2(12,19), ImVec2( 0, 0) }, // ImGuiMouseCursor_Arrow
    { ImVec2(13,0), ImVec2( 7,16), ImVec2( 1, 8) }, // ImGuiMouseCursor_TextInput
    { ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_ResizeAll
    { ImVec2(21,0), ImVec2( 9,23), ImVec2( 4,11) }, // ImGuiMouseCursor_ResizeNS
    { ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 4) }, // ImGuiMouseCursor_ResizeEW
    { ImVec2(73,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNESW
    { ImVec2(55,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNWSE
    { ImVec2(91,0), ImVec2(17,22), ImVec2( 5, 0) }, // ImGuiMouseCursor_Hand
};

ImFontAtlas::ImFontAtlas()
{
    Locked = false;
    Flags = ImFontAtlasFlags_None;
    TexID = (ImTextureID)NULL;
    TexDesiredWidth = 0;
    TexGlyphPadding = 1;

    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
    TexWidth = TexHeight = 0;
    TexUvScale = ImVec2(0.0f, 0.0f);
    TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

ImFontAtlas::~ImFontAtlas()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    Clear();
}

void    ImFontAtlas::ClearInputData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < ConfigData.Size; i++)
        if (ConfigData[i].FontData && ConfigData[i].FontDataOwnedByAtlas)
        {
            IM_FREE(ConfigData[i].FontData);
            ConfigData[i].FontData = NULL;
        }

    // When clearing this we lose access to the font name and other information used to build the font.
    for (int i = 0; i < Fonts.Size; i++)
        if (Fonts[i]->ConfigData >= ConfigData.Data && Fonts[i]->ConfigData < ConfigData.Data + ConfigData.Size)
        {
            Fonts[i]->ConfigData = NULL;
            Fonts[i]->ConfigDataCount = 0;
        }
    ConfigData.clear();
    CustomRects.clear();
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

void    ImFontAtlas::ClearTexData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    if (TexPixelsAlpha8)
        IM_FREE(TexPixelsAlpha8);
    if (TexPixelsRGBA32)
        IM_FREE(TexPixelsRGBA32);
    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
}

void    ImFontAtlas::ClearFonts()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < Fonts.Size; i++)
        IM_DELETE(Fonts[i]);
    Fonts.clear();
}

void    ImFontAtlas::Clear()
{
    ClearInputData();
    ClearTexData();
    ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Build atlas on demand
    if (TexPixelsAlpha8 == NULL)
    {
        if (ConfigData.empty())
            AddFontDefault();
        Build();
    }

    *out_pixels = TexPixelsAlpha8;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Convert to RGBA32 format on demand
    // Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
    if (!TexPixelsRGBA32)
    {
        unsigned char* pixels = NULL;
        GetTexDataAsAlpha8(&pixels, NULL, NULL);
        if (pixels)
        {
            TexPixelsRGBA32 = (unsigned int*)IM_ALLOC((size_t)TexWidth * (size_t)TexHeight * 4);
            const unsigned char* src = pixels;
            unsigned int* dst = TexPixelsRGBA32;
            for (int n = TexWidth * TexHeight; n > 0; n--)
                *dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
        }
    }

    *out_pixels = (unsigned char*)TexPixelsRGBA32;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
    IM_ASSERT(font_cfg->SizePixels > 0.0f);

    // Create new font
    if (!font_cfg->MergeMode)
        Fonts.push_back(IM_NEW(ImFont));
    else
        IM_ASSERT(!Fonts.empty() && "Cannot use MergeMode for the first font"); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.

    ConfigData.push_back(*font_cfg);
    ImFontConfig& new_font_cfg = ConfigData.back();
    if (new_font_cfg.DstFont == NULL)
        new_font_cfg.DstFont = Fonts.back();
    if (!new_font_cfg.FontDataOwnedByAtlas)
    {
        new_font_cfg.FontData = IM_ALLOC(new_font_cfg.FontDataSize);
        new_font_cfg.FontDataOwnedByAtlas = true;
        memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
    }

    if (new_font_cfg.DstFont->EllipsisChar == (ImWchar)-1)
        new_font_cfg.DstFont->EllipsisChar = font_cfg->EllipsisChar;

    // Invalidate texture
    ClearTexData();
    return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see misc/fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(const unsigned char *input);
static unsigned int stb_decompress(unsigned char *output, const unsigned char *input, unsigned int length);
static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c)                                    { return c >= '\\' ? c-36 : c-35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85*(Decode85Byte(src[1]) + 85*(Decode85Byte(src[2]) + 85*(Decode85Byte(src[3]) + 85*Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (!font_cfg_template)
    {
        font_cfg.OversampleH = font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH = true;
    }
    if (font_cfg.SizePixels <= 0.0f)
        font_cfg.SizePixels = 20.0f * 1.0f;
    if (font_cfg.Name[0] == '\0')
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "ProggyClean.ttf, %dpx", (int)font_cfg.SizePixels);
    font_cfg.EllipsisChar = (ImWchar)0x0085;

    const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
    const ImWchar* glyph_ranges = font_cfg.GlyphRanges != NULL ? font_cfg.GlyphRanges : GetGlyphRangesDefault();
    ImFont* font = AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, glyph_ranges);
    font->DisplayOffset.y = 1.0f;
    return font;
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    size_t data_size = 0;
    void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
    if (!data)
    {
        IM_ASSERT_USER_ERROR(0, "Could not load font file!");
        return NULL;
    }
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (font_cfg.Name[0] == '\0')
    {
        // Store a short copy of filename into into the font name for convenience
        const char* p;
        for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
    }
    return AddFontFromMemoryTTF(data, (int)data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* ttf_data, int ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontData = ttf_data;
    font_cfg.FontDataSize = ttf_size;
    font_cfg.SizePixels = size_pixels;
    if (glyph_ranges)
        font_cfg.GlyphRanges = glyph_ranges;
    return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char *)IM_ALLOC(buf_decompressed_size);
    stb_decompress(buf_decompressed_data, (const unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontDataOwnedByAtlas = true;
    return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
    int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf = IM_ALLOC((size_t)compressed_ttf_size);
    Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
    ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
    IM_FREE(compressed_ttf);
    return font;
}

int ImFontAtlas::AddCustomRectRegular(unsigned int id, int width, int height)
{
    // Breaking change on 2019/11/21 (1.74): ImFontAtlas::AddCustomRectRegular() now requires an ID >= 0x110000 (instead of >= 0x10000)
    IM_ASSERT(id >= 0x110000);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    ImFontAtlasCustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
    IM_ASSERT(font != NULL);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    ImFontAtlasCustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    r.GlyphAdvanceX = advance_x;
    r.GlyphOffset = offset;
    r.Font = font;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const ImFontAtlasCustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max) const
{
    IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
    IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
    *out_uv_min = ImVec2((float)rect->X * TexUvScale.x, (float)rect->Y * TexUvScale.y);
    *out_uv_max = ImVec2((float)(rect->X + rect->Width) * TexUvScale.x, (float)(rect->Y + rect->Height) * TexUvScale.y);
}

bool ImFontAtlas::GetMouseCursorTexData(ImGuiMouseCursor cursor_type, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2])
{
    if (cursor_type <= ImGuiMouseCursor_None || cursor_type >= ImGuiMouseCursor_COUNT)
        return false;
    if (Flags & ImFontAtlasFlags_NoMouseCursors)
        return false;

    IM_ASSERT(CustomRectIds[0] != -1);
    ImFontAtlasCustomRect& r = CustomRects[CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    ImVec2 pos = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][0] + ImVec2((float)r.X, (float)r.Y);
    ImVec2 size = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][1];
    *out_size = size;
    *out_offset = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][2];
    out_uv_border[0] = (pos) * TexUvScale;
    out_uv_border[1] = (pos + size) * TexUvScale;
    pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
    out_uv_fill[0] = (pos) * TexUvScale;
    out_uv_fill[1] = (pos + size) * TexUvScale;
    return true;
}

bool    ImFontAtlas::Build()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    return ImFontAtlasBuildWithStbTruetype(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
    for (unsigned int i = 0; i < 256; i++)
    {
        unsigned int value = (unsigned int)(i * in_brighten_factor);
        out_table[i] = value > 255 ? 255 : (value & 0xFF);
    }
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
    unsigned char* data = pixels + x + y * stride;
    for (int j = h; j > 0; j--, data += stride)
        for (int i = 0; i < w; i++)
            data[i] = table[data[i]];
}

// Temporary data for one source font (multiple source fonts can be merged into one destination ImFont)
// (C++03 doesn't allow instancing ImVector<> with function-local types so we declare the type here.)
struct ImFontBuildSrcData
{
    stbtt_fontinfo      FontInfo;
    stbtt_pack_range    PackRange;          // Hold the list of codepoints to pack (essentially points to Codepoints.Data)
    stbrp_rect*         Rects;              // Rectangle to pack. We first fill in their size and the packer will give us their position.
    stbtt_packedchar*   PackedChars;        // Output glyphs
    const ImWchar*      SrcRanges;          // Ranges as requested by user (user is allowed to request too much, e.g. 0x0020..0xFFFF)
    int                 DstIndex;           // Index into atlas->Fonts[] and dst_tmp_array[]
    int                 GlyphsHighest;      // Highest requested codepoint
    int                 GlyphsCount;        // Glyph count (excluding missing glyphs and glyphs already set by an earlier source font)
    ImBoolVector        GlyphsSet;          // Glyph bit map (random access, 1-bit per codepoint. This will be a maximum of 8KB)
    ImVector<int>       GlyphsList;         // Glyph codepoints list (flattened version of GlyphsMap)
};

// Temporary data for one destination ImFont* (multiple source fonts can be merged into one destination ImFont)
struct ImFontBuildDstData
{
    int                 SrcCount;           // Number of source fonts targeting this destination font.
    int                 GlyphsHighest;
    int                 GlyphsCount;
    ImBoolVector        GlyphsSet;          // This is used to resolve collision when multiple sources are merged into a same destination font.
};

static void UnpackBoolVectorToFlatIndexList(const ImBoolVector* in, ImVector<int>* out)
{
    IM_ASSERT(sizeof(in->Storage.Data[0]) == sizeof(int));
    const int* it_begin = in->Storage.begin();
    const int* it_end = in->Storage.end();
    for (const int* it = it_begin; it < it_end; it++)
        if (int entries_32 = *it)
            for (int bit_n = 0; bit_n < 32; bit_n++)
                if (entries_32 & (1u << bit_n))
                    out->push_back((int)((it - it_begin) << 5) + bit_n);
}

bool    ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->ConfigData.Size > 0);

    ImFontAtlasBuildRegisterDefaultCustomRects(atlas);

    // Clear atlas
    atlas->TexID = (ImTextureID)NULL;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvScale = ImVec2(0.0f, 0.0f);
    atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    atlas->ClearTexData();

    // Temporary storage for building
    ImVector<ImFontBuildSrcData> src_tmp_array;
    ImVector<ImFontBuildDstData> dst_tmp_array;
    src_tmp_array.resize(atlas->ConfigData.Size);
    dst_tmp_array.resize(atlas->Fonts.Size);
    memset(src_tmp_array.Data, 0, (size_t)src_tmp_array.size_in_bytes());
    memset(dst_tmp_array.Data, 0, (size_t)dst_tmp_array.size_in_bytes());

    // 1. Initialize font loading structure, check font data validity
    for (int src_i = 0; src_i < atlas->ConfigData.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

        // Find index from cfg.DstFont (we allow the user to set cfg.DstFont. Also it makes casual debugging nicer than when storing indices)
        src_tmp.DstIndex = -1;
        for (int output_i = 0; output_i < atlas->Fonts.Size && src_tmp.DstIndex == -1; output_i++)
            if (cfg.DstFont == atlas->Fonts[output_i])
                src_tmp.DstIndex = output_i;
        IM_ASSERT(src_tmp.DstIndex != -1); // cfg.DstFont not pointing within atlas->Fonts[] array?
        if (src_tmp.DstIndex == -1)
            return false;

        // Initialize helper structure for font loading and verify that the TTF/OTF data is correct
        const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
        IM_ASSERT(font_offset >= 0 && "FontData is incorrect, or FontNo cannot be found.");
        if (!stbtt_InitFont(&src_tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
            return false;

        // Measure highest codepoints
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.SrcRanges = cfg.GlyphRanges ? cfg.GlyphRanges : atlas->GetGlyphRangesDefault();
        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            src_tmp.GlyphsHighest = ImMax(src_tmp.GlyphsHighest, (int)src_range[1]);
        dst_tmp.SrcCount++;
        dst_tmp.GlyphsHighest = ImMax(dst_tmp.GlyphsHighest, src_tmp.GlyphsHighest);
    }

    // 2. For every requested codepoint, check for their presence in the font data, and handle redundancy or overlaps between source fonts to avoid unused glyphs.
    int total_glyphs_count = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.GlyphsSet.Resize(src_tmp.GlyphsHighest + 1);
        if (dst_tmp.GlyphsSet.Storage.empty())
            dst_tmp.GlyphsSet.Resize(dst_tmp.GlyphsHighest + 1);

        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            for (unsigned int codepoint = src_range[0]; codepoint <= src_range[1]; codepoint++)
            {
                if (dst_tmp.GlyphsSet.GetBit(codepoint))    // Don't overwrite existing glyphs. We could make this an option for MergeMode (e.g. MergeOverwrite==true)
                    continue;
                if (!stbtt_FindGlyphIndex(&src_tmp.FontInfo, codepoint))    // It is actually in the font?
                    continue;

                // Add to avail set/counters
                src_tmp.GlyphsCount++;
                dst_tmp.GlyphsCount++;
                src_tmp.GlyphsSet.SetBit(codepoint, true);
                dst_tmp.GlyphsSet.SetBit(codepoint, true);
                total_glyphs_count++;
            }
    }

    // 3. Unpack our bit map into a flat list (we now have all the Unicode points that we know are requested _and_ available _and_ not overlapping another)
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        src_tmp.GlyphsList.reserve(src_tmp.GlyphsCount);
        UnpackBoolVectorToFlatIndexList(&src_tmp.GlyphsSet, &src_tmp.GlyphsList);
        src_tmp.GlyphsSet.Clear();
        IM_ASSERT(src_tmp.GlyphsList.Size == src_tmp.GlyphsCount);
    }
    for (int dst_i = 0; dst_i < dst_tmp_array.Size; dst_i++)
        dst_tmp_array[dst_i].GlyphsSet.Clear();
    dst_tmp_array.clear();

    // Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
    // (We technically don't need to zero-clear buf_rects, but let's do it for the sake of sanity)
    ImVector<stbrp_rect> buf_rects;
    ImVector<stbtt_packedchar> buf_packedchars;
    buf_rects.resize(total_glyphs_count);
    buf_packedchars.resize(total_glyphs_count);
    memset(buf_rects.Data, 0, (size_t)buf_rects.size_in_bytes());
    memset(buf_packedchars.Data, 0, (size_t)buf_packedchars.size_in_bytes());

    // 4. Gather glyphs sizes so we can pack them in our virtual canvas.
    int total_surface = 0;
    int buf_rects_out_n = 0;
    int buf_packedchars_out_n = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        src_tmp.Rects = &buf_rects[buf_rects_out_n];
        src_tmp.PackedChars = &buf_packedchars[buf_packedchars_out_n];
        buf_rects_out_n += src_tmp.GlyphsCount;
        buf_packedchars_out_n += src_tmp.GlyphsCount;

        // Convert our ranges in the format stb_truetype wants
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        src_tmp.PackRange.font_size = cfg.SizePixels;
        src_tmp.PackRange.first_unicode_codepoint_in_range = 0;
        src_tmp.PackRange.array_of_unicode_codepoints = src_tmp.GlyphsList.Data;
        src_tmp.PackRange.num_chars = src_tmp.GlyphsList.Size;
        src_tmp.PackRange.chardata_for_range = src_tmp.PackedChars;
        src_tmp.PackRange.h_oversample = (unsigned char)cfg.OversampleH;
        src_tmp.PackRange.v_oversample = (unsigned char)cfg.OversampleV;

        // Gather the sizes of all rectangles we will need to pack (this loop is based on stbtt_PackFontRangesGatherRects)
        const float scale = (cfg.SizePixels > 0) ? stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels) : stbtt_ScaleForMappingEmToPixels(&src_tmp.FontInfo, -cfg.SizePixels);
        const int padding = atlas->TexGlyphPadding;
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsList.Size; glyph_i++)
        {
            int x0, y0, x1, y1;
            const int glyph_index_in_font = stbtt_FindGlyphIndex(&src_tmp.FontInfo, src_tmp.GlyphsList[glyph_i]);
            IM_ASSERT(glyph_index_in_font != 0);
            stbtt_GetGlyphBitmapBoxSubpixel(&src_tmp.FontInfo, glyph_index_in_font, scale * cfg.OversampleH, scale * cfg.OversampleV, 0, 0, &x0, &y0, &x1, &y1);
            src_tmp.Rects[glyph_i].w = (stbrp_coord)(x1 - x0 + padding + cfg.OversampleH - 1);
            src_tmp.Rects[glyph_i].h = (stbrp_coord)(y1 - y0 + padding + cfg.OversampleV - 1);
            total_surface += src_tmp.Rects[glyph_i].w * src_tmp.Rects[glyph_i].h;
        }
    }

    // We need a width for the skyline algorithm, any width!
    // The exact width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    // User can override TexDesiredWidth and TexGlyphPadding if they wish, otherwise we use a simple heuristic to select the width based on expected surface.
    const int surface_sqrt = (int)ImSqrt((float)total_surface) + 1;
    atlas->TexHeight = 0;
    if (atlas->TexDesiredWidth > 0)
        atlas->TexWidth = atlas->TexDesiredWidth;
    else
        atlas->TexWidth = (surface_sqrt >= 4096*0.7f) ? 4096 : (surface_sqrt >= 2048*0.7f) ? 2048 : (surface_sqrt >= 1024*0.7f) ? 1024 : 512;

    // 5. Start packing
    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    const int TEX_HEIGHT_MAX = 1024 * 32;
    stbtt_pack_context spc = {};
    stbtt_PackBegin(&spc, NULL, atlas->TexWidth, TEX_HEIGHT_MAX, 0, atlas->TexGlyphPadding, NULL);
    ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

    // 6. Pack each source font. No rendering yet, we are working with rectangles in an infinitely tall texture at this point.
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbrp_pack_rects((stbrp_context*)spc.pack_info, src_tmp.Rects, src_tmp.GlyphsCount);

        // Extend texture height and mark missing glyphs as non-packed so we won't render them.
        // FIXME: We are not handling packing failure here (would happen if we got off TEX_HEIGHT_MAX or if a single if larger than TexWidth?)
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
            if (src_tmp.Rects[glyph_i].was_packed)
                atlas->TexHeight = ImMax(atlas->TexHeight, src_tmp.Rects[glyph_i].y + src_tmp.Rects[glyph_i].h);
    }

    // 7. Allocate texture
    atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
    atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
    atlas->TexPixelsAlpha8 = (unsigned char*)IM_ALLOC(atlas->TexWidth * atlas->TexHeight);
    memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
    spc.pixels = atlas->TexPixelsAlpha8;
    spc.height = atlas->TexHeight;

    // 8. Render/rasterize font characters into the texture
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbtt_PackFontRangesRenderIntoRects(&spc, &src_tmp.FontInfo, &src_tmp.PackRange, 1, src_tmp.Rects);

        // Apply multiply operator
        if (cfg.RasterizerMultiply != 1.0f)
        {
            unsigned char multiply_table[256];
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
            stbrp_rect* r = &src_tmp.Rects[0];
            for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++, r++)
                if (r->was_packed)
                    ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, atlas->TexPixelsAlpha8, r->x, r->y, r->w, r->h, atlas->TexWidth * 1);
        }
        src_tmp.Rects = NULL;
    }

    // End packing
    stbtt_PackEnd(&spc);
    buf_rects.clear();

    // 9. Setup ImFont and glyphs for runtime
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFont* dst_font = cfg.DstFont; // We can have multiple input fonts writing into a same destination font (when using MergeMode=true)

        const float font_scale = stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels);
        int unscaled_ascent, unscaled_descent, unscaled_line_gap;
        stbtt_GetFontVMetrics(&src_tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        const float ascent = ImFloor(unscaled_ascent * font_scale + ((unscaled_ascent > 0.0f) ? +1 : -1));
        const float descent = ImFloor(unscaled_descent * font_scale + ((unscaled_descent > 0.0f) ? +1 : -1));
        ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        const float font_off_x = cfg.GlyphOffset.x;
        const float font_off_y = cfg.GlyphOffset.y + IM_ROUND(dst_font->Ascent);

        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
        {
            const int codepoint = src_tmp.GlyphsList[glyph_i];
            const stbtt_packedchar& pc = src_tmp.PackedChars[glyph_i];

            const float char_advance_x_org = pc.xadvance;
            const float char_advance_x_mod = ImClamp(char_advance_x_org, cfg.GlyphMinAdvanceX, cfg.GlyphMaxAdvanceX);
            float char_off_x = font_off_x;
            if (char_advance_x_org != char_advance_x_mod)
                char_off_x += cfg.PixelSnapH ? ImFloor((char_advance_x_mod - char_advance_x_org) * 0.5f) : (char_advance_x_mod - char_advance_x_org) * 0.5f;

            // Register glyph
            stbtt_aligned_quad q;
            float dummy_x = 0.0f, dummy_y = 0.0f;
            stbtt_GetPackedQuad(src_tmp.PackedChars, atlas->TexWidth, atlas->TexHeight, glyph_i, &dummy_x, &dummy_y, &q, 0);
            dst_font->AddGlyph((ImWchar)codepoint, q.x0 + char_off_x, q.y0 + font_off_y, q.x1 + char_off_x, q.y1 + font_off_y, q.s0, q.t0, q.s1, q.t1, char_advance_x_mod);
        }
    }

    // Cleanup temporary (ImVector doesn't honor destructor)
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
        src_tmp_array[src_i].~ImFontBuildSrcData();

    ImFontAtlasBuildFinish(atlas);
    return true;
}

void ImFontAtlasBuildRegisterDefaultCustomRects(ImFontAtlas* atlas)
{
    if (atlas->CustomRectIds[0] >= 0)
        return;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF*2+1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
    else
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, 2, 2);
}

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
    if (!font_config->MergeMode)
    {
        font->ClearOutputData();
        font->FontSize = font_config->SizePixels;
        font->ConfigData = font_config;
        font->ContainerAtlas = atlas;
        font->Ascent = ascent;
        font->Descent = descent;
    }
    font->ConfigDataCount++;
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* stbrp_context_opaque)
{
    stbrp_context* pack_context = (stbrp_context*)stbrp_context_opaque;
    IM_ASSERT(pack_context != NULL);

    ImVector<ImFontAtlasCustomRect>& user_rects = atlas->CustomRects;
    IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.

    ImVector<stbrp_rect> pack_rects;
    pack_rects.resize(user_rects.Size);
    memset(pack_rects.Data, 0, (size_t)pack_rects.size_in_bytes());
    for (int i = 0; i < user_rects.Size; i++)
    {
        pack_rects[i].w = user_rects[i].Width;
        pack_rects[i].h = user_rects[i].Height;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
    for (int i = 0; i < pack_rects.Size; i++)
        if (pack_rects[i].was_packed)
        {
            user_rects[i].X = pack_rects[i].x;
            user_rects[i].Y = pack_rects[i].y;
            IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
            atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
        }
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->CustomRectIds[0] >= 0);
    IM_ASSERT(atlas->TexPixelsAlpha8 != NULL);
    ImFontAtlasCustomRect& r = atlas->CustomRects[atlas->CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    IM_ASSERT(r.IsPacked());

    const int w = atlas->TexWidth;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
    {
        // Render/copy pixels
        IM_ASSERT(r.Width == FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1 && r.Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
        for (int y = 0, n = 0; y < FONT_ATLAS_DEFAULT_TEX_DATA_H; y++)
            for (int x = 0; x < FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF; x++, n++)
            {
                const int offset0 = (int)(r.X + x) + (int)(r.Y + y) * w;
                const int offset1 = offset0 + FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
                atlas->TexPixelsAlpha8[offset0] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == '.' ? 0xFF : 0x00;
                atlas->TexPixelsAlpha8[offset1] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == 'X' ? 0xFF : 0x00;
            }
    }
    else
    {
        IM_ASSERT(r.Width == 2 && r.Height == 2);
        const int offset = (int)(r.X) + (int)(r.Y) * w;
        atlas->TexPixelsAlpha8[offset] = atlas->TexPixelsAlpha8[offset + 1] = atlas->TexPixelsAlpha8[offset + w] = atlas->TexPixelsAlpha8[offset + w + 1] = 0xFF;
    }
    atlas->TexUvWhitePixel = ImVec2((r.X + 0.5f) * atlas->TexUvScale.x, (r.Y + 0.5f) * atlas->TexUvScale.y);
}

void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
    // Render into our custom data block
    ImFontAtlasBuildRenderDefaultTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->CustomRects.Size; i++)
    {
        const ImFontAtlasCustomRect& r = atlas->CustomRects[i];
        if (r.Font == NULL || r.ID >= 0x110000)
            continue;

        IM_ASSERT(r.Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(&r, &uv0, &uv1);
        r.Font->AddGlyph((ImWchar)r.ID, r.GlyphOffset.x, r.GlyphOffset.y, r.GlyphOffset.x + r.Width, r.GlyphOffset.y + r.Height, uv0.x, uv0.y, uv1.x, uv1.y, r.GlyphAdvanceX);
    }

    // Build all fonts lookup tables
    for (int i = 0; i < atlas->Fonts.Size; i++)
        if (atlas->Fonts[i]->DirtyLookupTables)
            atlas->Fonts[i]->BuildLookupTable();

    // Ellipsis character is required for rendering elided text. We prefer using U+2026 (horizontal ellipsis).
    // However some old fonts may contain ellipsis at U+0085. Here we auto-detect most suitable ellipsis character.
    // FIXME: Also note that 0x2026 is currently seldomly included in our font ranges. Because of this we are more likely to use three individual dots.
    for (int i = 0; i < atlas->Fonts.size(); i++)
    {
        ImFont* font = atlas->Fonts[i];
        if (font->EllipsisChar != (ImWchar)-1)
            continue;
        const ImWchar ellipsis_variants[] = { (ImWchar)0x2026, (ImWchar)0x0085 };
        for (int j = 0; j < IM_ARRAYSIZE(ellipsis_variants); j++)
            if (font->FindGlyphNoFallback(ellipsis_variants[j]) != NULL) // Verify glyph exists
            {
                font->EllipsisChar = ellipsis_variants[j];
                break;
            }
    }
}

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3131, 0x3163, // Korean alphabets
        0xAC00, 0xD79D, // Korean characters
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseFull()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

static void UnpackAccumulativeOffsetsIntoRanges(int base_codepoint, const short* accumulative_offsets, int accumulative_offsets_count, ImWchar* out_ranges)
{
    for (int n = 0; n < accumulative_offsets_count; n++, out_ranges += 2)
    {
        out_ranges[0] = out_ranges[1] = (ImWchar)(base_codepoint + accumulative_offsets[n]);
        base_codepoint += accumulative_offsets[n];
    }
    out_ranges[0] = 0;
}

//-------------------------------------------------------------------------
// [SECTION] ImFontAtlas glyph ranges helpers
//-------------------------------------------------------------------------

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseSimplifiedCommon()
{
    // Store 2500 regularly used characters for Simplified Chinese.
    // Sourced from https://zh.wiktionary.org/wiki/%E9%99%84%E5%BD%95:%E7%8E%B0%E4%BB%A3%E6%B1%89%E8%AF%AD%E5%B8%B8%E7%94%A8%E5%AD%97%E8%A1%A8
    // This table covers 97.97% of all characters used during the month in July, 1987.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,3,2,1,2,2,1,1,1,1,1,5,2,1,2,3,3,3,2,2,4,1,1,1,2,1,5,2,3,1,2,1,2,1,1,2,1,1,2,2,1,4,1,1,1,1,5,10,1,2,19,2,1,2,1,2,1,2,1,2,
        1,5,1,6,3,2,1,2,2,1,1,1,4,8,5,1,1,4,1,1,3,1,2,1,5,1,2,1,1,1,10,1,1,5,2,4,6,1,4,2,2,2,12,2,1,1,6,1,1,1,4,1,1,4,6,5,1,4,2,2,4,10,7,1,1,4,2,4,
        2,1,4,3,6,10,12,5,7,2,14,2,9,1,1,6,7,10,4,7,13,1,5,4,8,4,1,1,2,28,5,6,1,1,5,2,5,20,2,2,9,8,11,2,9,17,1,8,6,8,27,4,6,9,20,11,27,6,68,2,2,1,1,
        1,2,1,2,2,7,6,11,3,3,1,1,3,1,2,1,1,1,1,1,3,1,1,8,3,4,1,5,7,2,1,4,4,8,4,2,1,2,1,1,4,5,6,3,6,2,12,3,1,3,9,2,4,3,4,1,5,3,3,1,3,7,1,5,1,1,1,1,2,
        3,4,5,2,3,2,6,1,1,2,1,7,1,7,3,4,5,15,2,2,1,5,3,22,19,2,1,1,1,1,2,5,1,1,1,6,1,1,12,8,2,9,18,22,4,1,1,5,1,16,1,2,7,10,15,1,1,6,2,4,1,2,4,1,6,
        1,1,3,2,4,1,6,4,5,1,2,1,1,2,1,10,3,1,3,2,1,9,3,2,5,7,2,19,4,3,6,1,1,1,1,1,4,3,2,1,1,1,2,5,3,1,1,1,2,2,1,1,2,1,1,2,1,3,1,1,1,3,7,1,4,1,1,2,1,
        1,2,1,2,4,4,3,8,1,1,1,2,1,3,5,1,3,1,3,4,6,2,2,14,4,6,6,11,9,1,15,3,1,28,5,2,5,5,3,1,3,4,5,4,6,14,3,2,3,5,21,2,7,20,10,1,2,19,2,4,28,28,2,3,
        2,1,14,4,1,26,28,42,12,40,3,52,79,5,14,17,3,2,2,11,3,4,6,3,1,8,2,23,4,5,8,10,4,2,7,3,5,1,1,6,3,1,2,2,2,5,28,1,1,7,7,20,5,3,29,3,17,26,1,8,4,
        27,3,6,11,23,5,3,4,6,13,24,16,6,5,10,25,35,7,3,2,3,3,14,3,6,2,6,1,4,2,3,8,2,1,1,3,3,3,4,1,1,13,2,2,4,5,2,1,14,14,1,2,2,1,4,5,2,3,1,14,3,12,
        3,17,2,16,5,1,2,1,8,9,3,19,4,2,2,4,17,25,21,20,28,75,1,10,29,103,4,1,2,1,1,4,2,4,1,2,3,24,2,2,2,1,1,2,1,3,8,1,1,1,2,1,1,3,1,1,1,6,1,5,3,1,1,
        1,3,4,1,1,5,2,1,5,6,13,9,16,1,1,1,1,3,2,3,2,4,5,2,5,2,2,3,7,13,7,2,2,1,1,1,1,2,3,3,2,1,6,4,9,2,1,14,2,14,2,1,18,3,4,14,4,11,41,15,23,15,23,
        176,1,3,4,1,1,1,1,5,3,1,2,3,7,3,1,1,2,1,2,4,4,6,2,4,1,9,7,1,10,5,8,16,29,1,1,2,2,3,1,3,5,2,4,5,4,1,1,2,2,3,3,7,1,6,10,1,17,1,44,4,6,2,1,1,6,
        5,4,2,10,1,6,9,2,8,1,24,1,2,13,7,8,8,2,1,4,1,3,1,3,3,5,2,5,10,9,4,9,12,2,1,6,1,10,1,1,7,7,4,10,8,3,1,13,4,3,1,6,1,3,5,2,1,2,17,16,5,2,16,6,
        1,4,2,1,3,3,6,8,5,11,11,1,3,3,2,4,6,10,9,5,7,4,7,4,7,1,1,4,2,1,3,6,8,7,1,6,11,5,5,3,24,9,4,2,7,13,5,1,8,82,16,61,1,1,1,4,2,2,16,10,3,8,1,1,
        6,4,2,1,3,1,1,1,4,3,8,4,2,2,1,1,1,1,1,6,3,5,1,1,4,6,9,2,1,1,1,2,1,7,2,1,6,1,5,4,4,3,1,8,1,3,3,1,3,2,2,2,2,3,1,6,1,2,1,2,1,3,7,1,8,2,1,2,1,5,
        2,5,3,5,10,1,2,1,1,3,2,5,11,3,9,3,5,1,1,5,9,1,2,1,5,7,9,9,8,1,3,3,3,6,8,2,3,2,1,1,32,6,1,2,15,9,3,7,13,1,3,10,13,2,14,1,13,10,2,1,3,10,4,15,
        2,15,15,10,1,3,9,6,9,32,25,26,47,7,3,2,3,1,6,3,4,3,2,8,5,4,1,9,4,2,2,19,10,6,2,3,8,1,2,2,4,2,1,9,4,4,4,6,4,8,9,2,3,1,1,1,1,3,5,5,1,3,8,4,6,
        2,1,4,12,1,5,3,7,13,2,5,8,1,6,1,2,5,14,6,1,5,2,4,8,15,5,1,23,6,62,2,10,1,1,8,1,2,2,10,4,2,2,9,2,1,1,3,2,3,1,5,3,3,2,1,3,8,1,1,1,11,3,1,1,4,
        3,7,1,14,1,2,3,12,5,2,5,1,6,7,5,7,14,11,1,3,1,8,9,12,2,1,11,8,4,4,2,6,10,9,13,1,1,3,1,5,1,3,2,4,4,1,18,2,3,14,11,4,29,4,2,7,1,3,13,9,2,2,5,
        3,5,20,7,16,8,5,72,34,6,4,22,12,12,28,45,36,9,7,39,9,191,1,1,1,4,11,8,4,9,2,3,22,1,1,1,1,4,17,1,7,7,1,11,31,10,2,4,8,2,3,2,1,4,2,16,4,32,2,
        3,19,13,4,9,1,5,2,14,8,1,1,3,6,19,6,5,1,16,6,2,10,8,5,1,2,3,1,5,5,1,11,6,6,1,3,3,2,6,3,8,1,1,4,10,7,5,7,7,5,8,9,2,1,3,4,1,1,3,1,3,3,2,6,16,
        1,4,6,3,1,10,6,1,3,15,2,9,2,10,25,13,9,16,6,2,2,10,11,4,3,9,1,2,6,6,5,4,30,40,1,10,7,12,14,33,6,3,6,7,3,1,3,1,11,14,4,9,5,12,11,49,18,51,31,
        140,31,2,2,1,5,1,8,1,10,1,4,4,3,24,1,10,1,3,6,6,16,3,4,5,2,1,4,2,57,10,6,22,2,22,3,7,22,6,10,11,36,18,16,33,36,2,5,5,1,1,1,4,10,1,4,13,2,7,
        5,2,9,3,4,1,7,43,3,7,3,9,14,7,9,1,11,1,1,3,7,4,18,13,1,14,1,3,6,10,73,2,2,30,6,1,11,18,19,13,22,3,46,42,37,89,7,3,16,34,2,2,3,9,1,7,1,1,1,2,
        2,4,10,7,3,10,3,9,5,28,9,2,6,13,7,3,1,3,10,2,7,2,11,3,6,21,54,85,2,1,4,2,2,1,39,3,21,2,2,5,1,1,1,4,1,1,3,4,15,1,3,2,4,4,2,3,8,2,20,1,8,7,13,
        4,1,26,6,2,9,34,4,21,52,10,4,4,1,5,12,2,11,1,7,2,30,12,44,2,30,1,1,3,6,16,9,17,39,82,2,2,24,7,1,7,3,16,9,14,44,2,1,2,1,2,3,5,2,4,1,6,7,5,3,
        2,6,1,11,5,11,2,1,18,19,8,1,3,24,29,2,1,3,5,2,2,1,13,6,5,1,46,11,3,5,1,1,5,8,2,10,6,12,6,3,7,11,2,4,16,13,2,5,1,1,2,2,5,2,28,5,2,23,10,8,4,
        4,22,39,95,38,8,14,9,5,1,13,5,4,3,13,12,11,1,9,1,27,37,2,5,4,4,63,211,95,2,2,2,1,3,5,2,1,1,2,2,1,1,1,3,2,4,1,2,1,1,5,2,2,1,1,2,3,1,3,1,1,1,
        3,1,4,2,1,3,6,1,1,3,7,15,5,3,2,5,3,9,11,4,2,22,1,6,3,8,7,1,4,28,4,16,3,3,25,4,4,27,27,1,4,1,2,2,7,1,3,5,2,28,8,2,14,1,8,6,16,25,3,3,3,14,3,
        3,1,1,2,1,4,6,3,8,4,1,1,1,2,3,6,10,6,2,3,18,3,2,5,5,4,3,1,5,2,5,4,23,7,6,12,6,4,17,11,9,5,1,1,10,5,12,1,1,11,26,33,7,3,6,1,17,7,1,5,12,1,11,
        2,4,1,8,14,17,23,1,2,1,7,8,16,11,9,6,5,2,6,4,16,2,8,14,1,11,8,9,1,1,1,9,25,4,11,19,7,2,15,2,12,8,52,7,5,19,2,16,4,36,8,1,16,8,24,26,4,6,2,9,
        5,4,36,3,28,12,25,15,37,27,17,12,59,38,5,32,127,1,2,9,17,14,4,1,2,1,1,8,11,50,4,14,2,19,16,4,17,5,4,5,26,12,45,2,23,45,104,30,12,8,3,10,2,2,
        3,3,1,4,20,7,2,9,6,15,2,20,1,3,16,4,11,15,6,134,2,5,59,1,2,2,2,1,9,17,3,26,137,10,211,59,1,2,4,1,4,1,1,1,2,6,2,3,1,1,2,3,2,3,1,3,4,4,2,3,3,
        1,4,3,1,7,2,2,3,1,2,1,3,3,3,2,2,3,2,1,3,14,6,1,3,2,9,6,15,27,9,34,145,1,1,2,1,1,1,1,2,1,1,1,1,2,2,2,3,1,2,1,1,1,2,3,5,8,3,5,2,4,1,3,2,2,2,12,
        4,1,1,1,10,4,5,1,20,4,16,1,15,9,5,12,2,9,2,5,4,2,26,19,7,1,26,4,30,12,15,42,1,6,8,172,1,1,4,2,1,1,11,2,2,4,2,1,2,1,10,8,1,2,1,4,5,1,2,5,1,8,
        4,1,3,4,2,1,6,2,1,3,4,1,2,1,1,1,1,12,5,7,2,4,3,1,1,1,3,3,6,1,2,2,3,3,3,2,1,2,12,14,11,6,6,4,12,2,8,1,7,10,1,35,7,4,13,15,4,3,23,21,28,52,5,
        26,5,6,1,7,10,2,7,53,3,2,1,1,1,2,163,532,1,10,11,1,3,3,4,8,2,8,6,2,2,23,22,4,2,2,4,2,1,3,1,3,3,5,9,8,2,1,2,8,1,10,2,12,21,20,15,105,2,3,1,1,
        3,2,3,1,1,2,5,1,4,15,11,19,1,1,1,1,5,4,5,1,1,2,5,3,5,12,1,2,5,1,11,1,1,15,9,1,4,5,3,26,8,2,1,3,1,1,15,19,2,12,1,2,5,2,7,2,19,2,20,6,26,7,5,
        2,2,7,34,21,13,70,2,128,1,1,2,1,1,2,1,1,3,2,2,2,15,1,4,1,3,4,42,10,6,1,49,85,8,1,2,1,1,4,4,2,3,6,1,5,7,4,3,211,4,1,2,1,2,5,1,2,4,2,2,6,5,6,
        10,3,4,48,100,6,2,16,296,5,27,387,2,2,3,7,16,8,5,38,15,39,21,9,10,3,7,59,13,27,21,47,5,21,6
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF  // Half-width characters
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00) * 2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
    // 1946 common ideograms code points for Japanese
    // Sourced from http://theinstructionlimit.com/common-kanji-character-ranges-for-xna-spritefont-rendering
    // FIXME: Source a list of the revised 2136 Joyo Kanji list from 2010 and rebuild this.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,6,2,2,1,8,5,7,11,1,2,10,10,8,2,4,20,2,11,8,2,1,2,1,6,2,1,7,5,3,7,1,1,13,7,9,1,4,6,1,2,1,10,1,1,9,2,2,4,5,6,14,1,1,9,3,18,
        5,4,2,2,10,7,1,1,1,3,2,4,3,23,2,10,12,2,14,2,4,13,1,6,10,3,1,7,13,6,4,13,5,2,3,17,2,2,5,7,6,4,1,7,14,16,6,13,9,15,1,1,7,16,4,7,1,19,9,2,7,15,
        2,6,5,13,25,4,14,13,11,25,1,1,1,2,1,2,2,3,10,11,3,3,1,1,4,4,2,1,4,9,1,4,3,5,5,2,7,12,11,15,7,16,4,5,16,2,1,1,6,3,3,1,1,2,7,6,6,7,1,4,7,6,1,1,
        2,1,12,3,3,9,5,8,1,11,1,2,3,18,20,4,1,3,6,1,7,3,5,5,7,2,2,12,3,1,4,2,3,2,3,11,8,7,4,17,1,9,25,1,1,4,2,2,4,1,2,7,1,1,1,3,1,2,6,16,1,2,1,1,3,12,
        20,2,5,20,8,7,6,2,1,1,1,1,6,2,1,2,10,1,1,6,1,3,1,2,1,4,1,12,4,1,3,1,1,1,1,1,10,4,7,5,13,1,15,1,1,30,11,9,1,15,38,14,1,32,17,20,1,9,31,2,21,9,
        4,49,22,2,1,13,1,11,45,35,43,55,12,19,83,1,3,2,3,13,2,1,7,3,18,3,13,8,1,8,18,5,3,7,25,24,9,24,40,3,17,24,2,1,6,2,3,16,15,6,7,3,12,1,9,7,3,3,
        3,15,21,5,16,4,5,12,11,11,3,6,3,2,31,3,2,1,1,23,6,6,1,4,2,6,5,2,1,1,3,3,22,2,6,2,3,17,3,2,4,5,1,9,5,1,1,6,15,12,3,17,2,14,2,8,1,23,16,4,2,23,
        8,15,23,20,12,25,19,47,11,21,65,46,4,3,1,5,6,1,2,5,26,2,1,1,3,11,1,1,1,2,1,2,3,1,1,10,2,3,1,1,1,3,6,3,2,2,6,6,9,2,2,2,6,2,5,10,2,4,1,2,1,2,2,
        3,1,1,3,1,2,9,23,9,2,1,1,1,1,5,3,2,1,10,9,6,1,10,2,31,25,3,7,5,40,1,15,6,17,7,27,180,1,3,2,2,1,1,1,6,3,10,7,1,3,6,17,8,6,2,2,1,3,5,5,8,16,14,
        15,1,1,4,1,2,1,1,1,3,2,7,5,6,2,5,10,1,4,2,9,1,1,11,6,1,44,1,3,7,9,5,1,3,1,1,10,7,1,10,4,2,7,21,15,7,2,5,1,8,3,4,1,3,1,6,1,4,2,1,4,10,8,1,4,5,
        1,5,10,2,7,1,10,1,1,3,4,11,10,29,4,7,3,5,2,3,33,5,2,19,3,1,4,2,6,31,11,1,3,3,3,1,8,10,9,12,11,12,8,3,14,8,6,11,1,4,41,3,1,2,7,13,1,5,6,2,6,12,
        12,22,5,9,4,8,9,9,34,6,24,1,1,20,9,9,3,4,1,7,2,2,2,6,2,28,5,3,6,1,4,6,7,4,2,1,4,2,13,6,4,4,3,1,8,8,3,2,1,5,1,2,2,3,1,11,11,7,3,6,10,8,6,16,16,
        22,7,12,6,21,5,4,6,6,3,6,1,3,2,1,2,8,29,1,10,1,6,13,6,6,19,31,1,13,4,4,22,17,26,33,10,4,15,12,25,6,67,10,2,3,1,6,10,2,6,2,9,1,9,4,4,1,2,16,2,
        5,9,2,3,8,1,8,3,9,4,8,6,4,8,11,3,2,1,1,3,26,1,7,5,1,11,1,5,3,5,2,13,6,39,5,1,5,2,11,6,10,5,1,15,5,3,6,19,21,22,2,4,1,6,1,8,1,4,8,2,4,2,2,9,2,
        1,1,1,4,3,6,3,12,7,1,14,2,4,10,2,13,1,17,7,3,2,1,3,2,13,7,14,12,3,1,29,2,8,9,15,14,9,14,1,3,1,6,5,9,11,3,38,43,20,7,7,8,5,15,12,19,15,81,8,7,
        1,5,73,13,37,28,8,8,1,15,18,20,165,28,1,6,11,8,4,14,7,15,1,3,3,6,4,1,7,14,1,1,11,30,1,5,1,4,14,1,4,2,7,52,2,6,29,3,1,9,1,21,3,5,1,26,3,11,14,
        11,1,17,5,1,2,1,3,2,8,1,2,9,12,1,1,2,3,8,3,24,12,7,7,5,17,3,3,3,1,23,10,4,4,6,3,1,16,17,22,3,10,21,16,16,6,4,10,2,1,1,2,8,8,6,5,3,3,3,39,25,
        15,1,1,16,6,7,25,15,6,6,12,1,22,13,1,4,9,5,12,2,9,1,12,28,8,3,5,10,22,60,1,2,40,4,61,63,4,1,13,12,1,4,31,12,1,14,89,5,16,6,29,14,2,5,49,18,18,
        5,29,33,47,1,17,1,19,12,2,9,7,39,12,3,7,12,39,3,1,46,4,12,3,8,9,5,31,15,18,3,2,2,66,19,13,17,5,3,46,124,13,57,34,2,5,4,5,8,1,1,1,4,3,1,17,5,
        3,5,3,1,8,5,6,3,27,3,26,7,12,7,2,17,3,7,18,78,16,4,36,1,2,1,6,2,1,39,17,7,4,13,4,4,4,1,10,4,2,4,6,3,10,1,19,1,26,2,4,33,2,73,47,7,3,8,2,4,15,
        18,1,29,2,41,14,1,21,16,41,7,39,25,13,44,2,2,10,1,13,7,1,7,3,5,20,4,8,2,49,1,10,6,1,6,7,10,7,11,16,3,12,20,4,10,3,1,2,11,2,28,9,2,4,7,2,15,1,
        27,1,28,17,4,5,10,7,3,24,10,11,6,26,3,2,7,2,2,49,16,10,16,15,4,5,27,61,30,14,38,22,2,7,5,1,3,12,23,24,17,17,3,3,2,4,1,6,2,7,5,1,1,5,1,1,9,4,
        1,3,6,1,8,2,8,4,14,3,5,11,4,1,3,32,1,19,4,1,13,11,5,2,1,8,6,8,1,6,5,13,3,23,11,5,3,16,3,9,10,1,24,3,198,52,4,2,2,5,14,5,4,22,5,20,4,11,6,41,
        1,5,2,2,11,5,2,28,35,8,22,3,18,3,10,7,5,3,4,1,5,3,8,9,3,6,2,16,22,4,5,5,3,3,18,23,2,6,23,5,27,8,1,33,2,12,43,16,5,2,3,6,1,20,4,2,9,7,1,11,2,
        10,3,14,31,9,3,25,18,20,2,5,5,26,14,1,11,17,12,40,19,9,6,31,83,2,7,9,19,78,12,14,21,76,12,113,79,34,4,1,1,61,18,85,10,2,2,13,31,11,50,6,33,159,
        179,6,6,7,4,4,2,4,2,5,8,7,20,32,22,1,3,10,6,7,28,5,10,9,2,77,19,13,2,5,1,4,4,7,4,13,3,9,31,17,3,26,2,6,6,5,4,1,7,11,3,4,2,1,6,2,20,4,1,9,2,6,
        3,7,1,1,1,20,2,3,1,6,2,3,6,2,4,8,1,5,13,8,4,11,23,1,10,6,2,1,3,21,2,2,4,24,31,4,10,10,2,5,192,15,4,16,7,9,51,1,2,1,1,5,1,1,2,1,3,5,3,1,3,4,1,
        3,1,3,3,9,8,1,2,2,2,4,4,18,12,92,2,10,4,3,14,5,25,16,42,4,14,4,2,21,5,126,30,31,2,1,5,13,3,22,5,6,6,20,12,1,14,12,87,3,19,1,8,2,9,9,3,3,23,2,
        3,7,6,3,1,2,3,9,1,3,1,6,3,2,1,3,11,3,1,6,10,3,2,3,1,2,1,5,1,1,11,3,6,4,1,7,2,1,2,5,5,34,4,14,18,4,19,7,5,8,2,6,79,1,5,2,14,8,2,9,2,1,36,28,16,
        4,1,1,1,2,12,6,42,39,16,23,7,15,15,3,2,12,7,21,64,6,9,28,8,12,3,3,41,59,24,51,55,57,294,9,9,2,6,2,15,1,2,13,38,90,9,9,9,3,11,7,1,1,1,5,6,3,2,
        1,2,2,3,8,1,4,4,1,5,7,1,4,3,20,4,9,1,1,1,5,5,17,1,5,2,6,2,4,1,4,5,7,3,18,11,11,32,7,5,4,7,11,127,8,4,3,3,1,10,1,1,6,21,14,1,16,1,7,1,3,6,9,65,
        51,4,3,13,3,10,1,1,12,9,21,110,3,19,24,1,1,10,62,4,1,29,42,78,28,20,18,82,6,3,15,6,84,58,253,15,155,264,15,21,9,14,7,58,40,39,
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF  // Half-width characters
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00)*2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x2010, 0x205E, // Punctuations
        0x0E00, 0x0E7F, // Thai
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesVietnamese()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x0102, 0x0103,
        0x0110, 0x0111,
        0x0128, 0x0129,
        0x0168, 0x0169,
        0x01A0, 0x01A1,
        0x01AF, 0x01B0,
        0x1EA0, 0x1EF9,
        0,
    };
    return &ranges[0];
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontGlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontGlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
    while (text_end ? (text < text_end) : *text)
    {
        unsigned int c = 0;
        int c_len = ImTextCharFromUtf8(&c, text, text_end);
        text += c_len;
        if (c_len == 0)
            break;
        if (c <= IM_UNICODE_CODEPOINT_MAX)
            AddChar((ImWchar)c);
    }
}

void ImFontGlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
    for (; ranges[0]; ranges += 2)
        for (ImWchar c = ranges[0]; c <= ranges[1]; c++)
            AddChar(c);
}

void ImFontGlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
    const int max_codepoint = IM_UNICODE_CODEPOINT_MAX;
    for (int n = 0; n <= max_codepoint; n++)
        if (GetBit(n))
        {
            out_ranges->push_back((ImWchar)n);
            while (n < max_codepoint && GetBit(n + 1))
                n++;
            out_ranges->push_back((ImWchar)n);
        }
    out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// [SECTION] ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    FallbackChar = (ImWchar)'?';
    EllipsisChar = (ImWchar)-1;
    DisplayOffset = ImVec2(0.0f, 0.0f);
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    ConfigData = NULL;
    ConfigDataCount = 0;
    DirtyLookupTables = false;
    Scale = 1.0f;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
}

ImFont::~ImFont()
{
    ClearOutputData();
}

void    ImFont::ClearOutputData()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    Glyphs.clear();
    IndexAdvanceX.clear();
    IndexLookup.clear();
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    DirtyLookupTables = true;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
}

void ImFont::BuildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != Glyphs.Size; i++)
        max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

    IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
    IndexAdvanceX.clear();
    IndexLookup.clear();
    DirtyLookupTables = false;
    GrowIndex(max_codepoint + 1);
    for (int i = 0; i < Glyphs.Size; i++)
    {
        int codepoint = (int)Glyphs[i].Codepoint;
        IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
        IndexLookup[codepoint] = (ImWchar)i;
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (FindGlyph((ImWchar)' '))
    {
        if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times
            Glyphs.resize(Glyphs.Size + 1);
        ImFontGlyph& tab_glyph = Glyphs.back();
        tab_glyph = *FindGlyph((ImWchar)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.AdvanceX *= IM_TABSIZE;
        IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
        IndexLookup[(int)tab_glyph.Codepoint] = (ImWchar)(Glyphs.Size-1);
    }

    FallbackGlyph = FindGlyphNoFallback(FallbackChar);
    FallbackAdvanceX = FallbackGlyph ? FallbackGlyph->AdvanceX : 0.0f;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (IndexAdvanceX[i] < 0.0f)
            IndexAdvanceX[i] = FallbackAdvanceX;
}

void ImFont::SetFallbackChar(ImWchar c)
{
    FallbackChar = c;
    BuildLookupTable();
}

void ImFont::GrowIndex(int new_size)
{
    IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
    if (new_size <= IndexLookup.Size)
        return;
    IndexAdvanceX.resize(new_size, -1.0f);
    IndexLookup.resize(new_size, (ImWchar)-1);
}

// x0/y0/x1/y1 are offset from the character upper-left layout position, in pixels. Therefore x0/y0 are often fairly close to zero.
// Not to be mistaken with texture coordinates, which are held by u0/v0/u1/v1 in normalized format (0.0..1.0 on each texture axis).
void ImFont::AddGlyph(ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
    Glyphs.resize(Glyphs.Size + 1);
    ImFontGlyph& glyph = Glyphs.back();
    glyph.Codepoint = (ImWchar)codepoint;
    glyph.X0 = x0;
    glyph.Y0 = y0;
    glyph.X1 = x1;
    glyph.Y1 = y1;
    glyph.U0 = u0;
    glyph.V0 = v0;
    glyph.U1 = u1;
    glyph.V1 = v1;
    glyph.AdvanceX = advance_x + ConfigData->GlyphExtraSpacing.x;  // Bake spacing into AdvanceX

    if (ConfigData->PixelSnapH)
        glyph.AdvanceX = IM_ROUND(glyph.AdvanceX);

    // Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
    DirtyLookupTables = true;
    MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + 1.99f) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + 1.99f);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
    IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
    unsigned int index_size = (unsigned int)IndexLookup.Size;

    if (dst < index_size && IndexLookup.Data[dst] == (ImWchar)-1 && !overwrite_dst) // 'dst' already exists
        return;
    if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
        return;

    GrowIndex(dst + 1);
    IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (ImWchar)-1;
    IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

const ImFontGlyph* ImFont::FindGlyph(ImWchar c) const
{
    if (c >= IndexLookup.Size)
        return FallbackGlyph;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return FallbackGlyph;
    return &Glyphs.Data[i];
}

const ImFontGlyph* ImFont::FindGlyphNoFallback(ImWchar c) const
{
    if (c >= IndexLookup.Size)
        return NULL;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return NULL;
    return &Glyphs.Data[i];
}

const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
    // Simple word-wrapping for English, not full-featured. Please submit failing cases!
    // FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;
    wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + ImTextCharFromUtf8(&c, s, text_end);
        if (c == 0)
            break;

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX.Data[c] : FallbackAdvanceX);
        if (ImCharIsBlankW(c))
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
                word_end = s;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = !(c == '.' || c == ',' || c == ';' || c == '!' || c == '?' || c == '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width > wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

    const float line_height = size;
    const float scale = size / FontSize;

    ImVec2 text_size = ImVec2(0,0);
    float line_width = 0.0f;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    const char* s = text_begin;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                if (text_size.x < line_width)
                    text_size.x = line_width;
                text_size.y += line_height;
                line_width = 0.0f;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        const char* prev_s = s;
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = ImMax(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX.Data[c] : FallbackAdvanceX) * scale;
        if (line_width + char_width >= max_width)
        {
            s = prev_s;
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

void ImFont::RenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, ImWchar c) const
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') // Match behavior of RenderText(), those 4 codepoints are hard-coded.
        return;
    if (const ImFontGlyph* glyph = FindGlyph(c))
    {
        float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
        pos.x = IM_FLOOR(pos.x + DisplayOffset.x);
        pos.y = IM_FLOOR(pos.y + DisplayOffset.y);
        draw_list->PrimReserve(6, 4);
        draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
    }
}

void ImFont::RenderText(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // ImGui:: functions generally already provides a valid text_end, so this is merely to handle direct calls.

    // Align to be pixel perfect
    pos.x = IM_FLOOR(pos.x + DisplayOffset.x);
    pos.y = IM_FLOOR(pos.y + DisplayOffset.y);
    float x = pos.x;
    float y = pos.y;
    if (y > clip_rect.w)
        return;

    const float scale = size / FontSize;
    const float line_height = FontSize * scale;
    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    // Fast-forward to first visible line
    const char* s = text_begin;
    if (y + line_height < clip_rect.y && !word_wrap_enabled)
        while (y + line_height < clip_rect.y && s < text_end)
        {
            s = (const char*)memchr(s, '\n', text_end - s);
            s = s ? s + 1 : text_end;
            y += line_height;
        }

    // For large text, scan for the last visible line in order to avoid over-reserving in the call to PrimReserve()
    // Note that very large horizontal line will still be affected by the issue (e.g. a one megabyte string buffer without a newline will likely crash atm)
    if (text_end - s > 10000 && !word_wrap_enabled)
    {
        const char* s_end = s;
        float y_end = y;
        while (y_end < clip_rect.w && s_end < text_end)
        {
            s_end = (const char*)memchr(s_end, '\n', text_end - s_end);
            s_end = s_end ? s_end + 1 : text_end;
            y_end += line_height;
        }
        text_end = s_end;
    }
    if (s == text_end)
        return;

    // Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
    const int vtx_count_max = (int)(text_end - s) * 4;
    const int idx_count_max = (int)(text_end - s) * 6;
    const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);

    ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
    ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
    unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;

    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - pos.x));
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                x = pos.x;
                y += line_height;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                x = pos.x;
                y += line_height;
                if (y > clip_rect.w)
                    break; // break out of main loop
                continue;
            }
            if (c == '\r')
                continue;
        }

        float char_width = 0.0f;
        if (const ImFontGlyph* glyph = FindGlyph((ImWchar)c))
        {
            char_width = glyph->AdvanceX * scale;

            // Arbitrarily assume that both space and tabs are empty glyphs as an optimization
            if (c != ' ' && c != '\t')
            {
                // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
                float x1 = x + glyph->X0 * scale;
                float x2 = x + glyph->X1 * scale;
                float y1 = y + glyph->Y0 * scale;
                float y2 = y + glyph->Y1 * scale;
                if (x1 <= clip_rect.z && x2 >= clip_rect.x)
                {
                    // Render a character
                    float u1 = glyph->U0;
                    float v1 = glyph->V0;
                    float u2 = glyph->U1;
                    float v2 = glyph->V1;

                    // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                    if (cpu_fine_clip)
                    {
                        if (x1 < clip_rect.x)
                        {
                            u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                            x1 = clip_rect.x;
                        }
                        if (y1 < clip_rect.y)
                        {
                            v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                            y1 = clip_rect.y;
                        }
                        if (x2 > clip_rect.z)
                        {
                            u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                            x2 = clip_rect.z;
                        }
                        if (y2 > clip_rect.w)
                        {
                            v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                            y2 = clip_rect.w;
                        }
                        if (y1 >= y2)
                        {
                            x += char_width;
                            continue;
                        }
                    }

                    // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                    {
                        idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx+1); idx_write[2] = (ImDrawIdx)(vtx_current_idx+2);
                        idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx+2); idx_write[5] = (ImDrawIdx)(vtx_current_idx+3);
                        vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
                        vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
                        vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
                        vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
                        vtx_write += 4;
                        vtx_current_idx += 4;
                        idx_write += 6;
                    }
                }
            }
        }

        x += char_width;
    }

    // Give back unused vertices (clipped ones, blanks) ~ this is essentially a PrimUnreserve() action.
    draw_list->VtxBuffer.Size = (int)(vtx_write - draw_list->VtxBuffer.Data); // Same as calling shrink()
    draw_list->IdxBuffer.Size = (int)(idx_write - draw_list->IdxBuffer.Data);
    draw_list->CmdBuffer[draw_list->CmdBuffer.Size-1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
    draw_list->_VtxWritePtr = vtx_write;
    draw_list->_IdxWritePtr = idx_write;
    draw_list->_VtxCurrentIdx = vtx_current_idx;
}

//-----------------------------------------------------------------------------
// [SECTION] Internal Render Helpers
// (progressively moved from imgui.cpp to here when they are redesigned to stop accessing ImGui global state)
//-----------------------------------------------------------------------------
// - RenderMouseCursor()
// - RenderArrowPointingAt()
// - RenderRectFilledRangeH()
//-----------------------------------------------------------------------------

void ImGui::RenderMouseCursor(ImDrawList* draw_list, ImVec2 pos, float scale, ImGuiMouseCursor mouse_cursor, ImU32 col_fill, ImU32 col_border, ImU32 col_shadow)
{
    if (mouse_cursor == ImGuiMouseCursor_None)
        return;
    IM_ASSERT(mouse_cursor > ImGuiMouseCursor_None && mouse_cursor < ImGuiMouseCursor_COUNT);

    ImFontAtlas* font_atlas = draw_list->_Data->Font->ContainerAtlas;
    ImVec2 offset, size, uv[4];
    if (font_atlas->GetMouseCursorTexData(mouse_cursor, &offset, &size, &uv[0], &uv[2]))
    {
        pos -= offset;
        const ImTextureID tex_id = font_atlas->TexID;
        draw_list->PushTextureID(tex_id);
        draw_list->AddImage(tex_id, pos + ImVec2(1,0)*scale, pos + ImVec2(1,0)*scale + size*scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_id, pos + ImVec2(2,0)*scale, pos + ImVec2(2,0)*scale + size*scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_id, pos,                     pos + size*scale,                     uv[2], uv[3], col_border);
        draw_list->AddImage(tex_id, pos,                     pos + size*scale,                     uv[0], uv[1], col_fill);
        draw_list->PopTextureID();
    }
}

// Render an arrow. 'pos' is position of the arrow tip. half_sz.x is length from base to tip. half_sz.y is length on each side.
void ImGui::RenderArrowPointingAt(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col)
{
    switch (direction)
    {
    case ImGuiDir_Left:  draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Right: draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_Up:    draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Down:  draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_None: case ImGuiDir_COUNT: break; // Fix warnings
    }
}

static inline float ImAcos01(float x)
{
    if (x <= 0.0f) return IM_PI * 0.5f;
    if (x >= 1.0f) return 0.0f;
    return ImAcos(x);
    //return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
    if (x_end_norm == x_start_norm)
        return;
    if (x_start_norm > x_end_norm)
        ImSwap(x_start_norm, x_end_norm);

    ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
    ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
    if (rounding == 0.0f)
    {
        draw_list->AddRectFilled(p0, p1, col, 0.0f);
        return;
    }

    rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
    const float inv_rounding = 1.0f / rounding;
    const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
    const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
    const float half_pi = IM_PI * 0.5f; // We will == compare to this because we know this is the exact value ImAcos01 can return.
    const float x0 = ImMax(p0.x, rect.Min.x + rounding);
    if (arc0_b == arc0_e)
    {
        draw_list->PathLineTo(ImVec2(x0, p1.y));
        draw_list->PathLineTo(ImVec2(x0, p0.y));
    }
    else if (arc0_b == 0.0f && arc0_e == half_pi)
    {
        draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
        draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
    }
    else
    {
        draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b, 3); // BL
        draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e, 3); // TR
    }
    if (p1.x > rect.Min.x + rounding)
    {
        const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
        const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
        const float x1 = ImMin(p1.x, rect.Max.x - rounding);
        if (arc1_b == arc1_e)
        {
            draw_list->PathLineTo(ImVec2(x1, p0.y));
            draw_list->PathLineTo(ImVec2(x1, p1.y));
        }
        else if (arc1_b == 0.0f && arc1_e == half_pi)
        {
            draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
            draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
        }
        else
        {
            draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b, 3); // TR
            draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e, 3); // BR
        }
    }
    draw_list->PathFillConvex(col);
}

//-----------------------------------------------------------------------------
// [SECTION] Decompression code
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array and encoded as base85.
// Use the program in misc/fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(const unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier_out_e, *stb__barrier_out_b;
static const unsigned char *stb__barrier_in_b;
static unsigned char *stb__dout;
static void stb__match(const unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_out_b) { stb__dout = stb__barrier_out_e+1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(const unsigned char *data, unsigned int length)
{
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_in_b) { stb__dout = stb__barrier_out_e+1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static const unsigned char *stb_decompress_token(const unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
        else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen = buflen % 5552;

    unsigned long i;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int /*length*/)
{
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    const unsigned int olen = stb_decompress_length(i);
    stb__barrier_in_b = i;
    stb__barrier_out_e = output + olen;
    stb__barrier_out_b = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        const unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                IM_ASSERT(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2))
                    return 0;
                return olen;
            } else {
                IM_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        IM_ASSERT(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Default font data (ProggyClean.ttf)
//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using misc/fonts/binary_to_compressed_c.cpp (with compression + base85 string encoding).
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
//-----------------------------------------------------------------------------
static const char proggy_clean_ttf_compressed_data_base85[117880 + 1] =
"7])#######=sMRQ'/###I),##c'ChLr*pA$$B:t8HwHXVYA@>#,l&##cC0h<K8XF=nA?>#+b###;?/e=Fn+p`?#D>#S1###R*re=ke>pW)^?>#B?t?9W2^+>&3YW'>hE>#rZ]w'$dA0F"
";_0=NqiB>#a`FO1L6<'I7JS;8o3E>#j3f--.%`-GIN.>+s=A>#,t+##:iw<Br4cc)6uFc-idg--C=HkE95KGa`P&##Q[?>#a]AUCL,xLUi_D>#b>g--f'TqL,B2<dCJIwKU>`w'E3n0F"
":i$G:b[h--=j'##[-%iFNwoAS0%@>#7C*###JW=B.BW>ckXaw'PFs92;LNDFF6Nw8A[m4#q8c'#,Ra2G<dv1b0R=&#a)v7#Ppt2GFWEoM1nq;#SvmuL;n@4V_-;YPL<]lS7e%v#Hm$)*"
"pPG>#I?pQ&b=SYGVmQ-3iR792SO;8.;Pr`3*^B.*vkh8.t1JS@#:xVu.0e.<.*Ri;4T>m&gc:@7=/Wt8dKix.TU=AXv8#4=ct+/(akr.LPPPq;a@1reFX5@.ntlV-%rBB#CHgB/I12I$"
"Xn+Qu$(D?ud9kKP$f$##37g-6rGp%#giLY7xwq%,pK3#-#/xh(P[wS%OZYN'fxN.)=eWT%*OR=7&[)22@q3L#>FlKYc4QLM=x*F33.*v#@Qe<C3?'.$Z6V/)BPP8.8JT/)4^5g)hH+U%"
"orVn6^Z-lff?rbuGoqs;A2A>,R*&X#9VW6='hH:Twv]xtk*hX61waVH&@[iuSuq=M6iqM#Jhv7[OLv-$BuUiu%+*)#F=h+v.4Qv$YQk&#/dZ(#fQe_+).>31-Ru6'PAnh(jaFT%G),##"
"+[wkLrTGs-^XQlLmJeh2TU39/VeC+*&)R_#ND#G4-a0i)/LZ;%M]-O(m]WI)VP[L(N29f3S*)T/*Dw<(F=G+*uf@>gPA+%Epv.8C=c83F]nDp9MR#t_@ENVFiX@N<n[Ep1C[(#g'H(/`"
")4[>#,D$hO)mE2>''uL$0iZO,itMxFB$JRE[`[-i^PdZ>dPTj29jilAmxam1%+K(#/Q.)v.(Ub$Oh1$#Uqn%#(WH(#j`Un/GFg5'#_%h(`eL`+QHoYG^A<.3b<a>37_EXec:SC#Xx4gL"
"JY=u-iL0+*pQ]<_;+;8.>/Xm+b4CVQJm99W;3o;V@)]Q:X#]'$craTYvPc[TEqDbSA>=[:@Pb3;2K<8ehskBrhNS*q8ZX<VLT+[U+h>+;Tv-oegjbBrgNfEq8N=wUOQ`S%CS(##9DF*v"
"GtPn$lYt&#:8E)#Yfr:%&1@##'W1U%e-^Q&r=nR&xBGZG0,+,Mh,>n0$X)22Gb:F*%#Bc0ig'u$BTwF4l$-gLDlvS/#0Tv-r$M<hH_Wu7UW^/)@h@njV$6GM#R^C4f[l5JU1nOWT0'L#"
"*/jg*v(bk<DcO@tEcRfZnZoQH3fVQ2YeGaZkccN3ZxLGZr+>:==`1BHpVtoe:,&nMd<?oR>)r8LFm8xk%N-S1x]P6EG`%G4^ujQC6J]XlG9`<9Cm_e41.-J=i(&[79@]'?$),##Uv%P#"
"`kn#MLx0Q]V%]fLbH@&M.$-A%=$@,Mk#ebuDX,lL*Ia$#G^b<$#)(02wUXb3Xp9-mO?Zj0,wW:.JqAA=^9wW^_J.QL#i^3NAJ-CY2$<8.6hLc2+ktUdQc$bI'>su5$#7v64NvY-$imM("
"qSW^#u1x+2Nb*a%u(mY#@vNl(Ke4`#@>K.*k:cV-[*Iw]1/&<8x4=?DumMD38i$ouT?rK#tooLVZEaD#A.2guFSK@MCT4a=^39j'K3KM'92t5(<7%<$HwJ2'=*gQ&]?[0#:71re(YQlL"
"uMj%$x2V,)WIdP/BbK]$8SqD/pW&]uZXpJ[j9BsuTWWiKdR.[uKO$M.GncH#/)Z(BI=bDbI=dtLssT-#=`v?$Eh1$#N&$q%7VY##u5oO(5ES]%E0fX-4t@X-if*F3S5^oR+spjuoEDvU"
"7+[jumj`?U0BDe6B%6%tL9uLp9Xjl&^AwK#<81-3mljh2EUugL<W:a#?Qq?'%-U^-k$_Z-'hjL:aWKh<N+P3Mw+RF#>?5b#N6>##CH_k4o2=1)1jhe;En5Pf%uSe-h7wUmf4c=uuPk9&"
"CsJQ&HAP-3Vto79J>[^P.3n.<.'Ii;>c;^%`'(,)p#%Grdm^-?eH13$Ku_S%f')kLAE;8.o/aWu>NiV-$w@+;R/F`#qIN1p,mroeSu4;-^TfQ,hJmo)tG3XC$bJjB@L.s$?]d8/KJ=l("
"*^B.*aMnh(*)TF4Ke4`#?;Rv$<ERF4*T+OXIBl5ilI`$E%/9SKVWI05X114EcP`%k+#bYb8w#&=Ow%&=;owR[%XQ=u*'H;$W5/v#'IkXu#uo=Y`V_lpjG#7'-?%<&H_ws$P41/#:N9b%"
"7B<.3<Eeh2WuEb3xHhx$_n2Q/I;IipGR(Z#C@6aX(#6nX=$.]?<JOf2O6JL#iPu-42H:;$xP[8#BFmU7O0OX$$*0#,fA5U;hC+F3$2bj.F%xC#5#`AM1p3Q/eY0wg/DXI)Co(?5_Mh>$"
",TdC#D$-v:u*(nVVY@GO(A-.dbX1MH-1>PfeBhkF88@dGZW$0N7IeqA2Tx+d==M7n&iiEI85N1pE*kEILrpu,&>i;$<eM`+/r<[$KG$q7WrgN(&34l(LWeh24`xb-F=e-H_]d8/0dB49"
"*/M9/'EsI3LA^+4DAvd.8bCE4*G/e%pr?C<j=',]$&6lHw-A8hG.V.@o-#lOjKHLN)pLQEGb@@n4v?T$/Plc@[],7]mDp7GY=5$2oRAh>WLJ+8m)(IG((w`O+.18#OvK'#DC6c$?b($#"
";-q@1QBkp%,>cY#^WfYG1dSc$Nf(H3s3rkL%Z1N(XRG)4gBxT%5pEb3i]%>.7i(?#+J(Tcv+JYY(`=`E.Pc>#2d=F+.ccf:$Eff:Aen8u6tH:35$mE,pL[Au6+,##;1ST$W'G6#R9/W7"
"+@Z(+ov=&#e9i`+[t3U%`CPgL<7V-M*T)u7BN*9'&eC?%>76g)_ZwF4&P_ritiYn[Ad&n:_VFUUnBtj6w6q,5(,7uc1F`+$`')rO7EH#0f5S*41JFkNgTA?5@;2mu+51hL/C]qLjlT]$"
"fGX&#ZIWf1ClI60#rh8.#q=>'cDpPqGkfZ$u@71)wiVD3BY0f)`GUv-fv#E3i'fp.C]R_#xSG[T^KCd)l?`cD)#=Eo.mUC.;:'O_cn;Q&)2We?=3PeGY@9:_Dd9crF0FRN#xP[.PxvxF"
"xfw<X=ru>Y'&-juFdhe<j%x4Qeua2=%=`=c:(QXUd/Z>7OEw)##tu+#bF8]$5IY##/HTW$nC^P8u@kM(M>f]-FF3]-L$lf(fUBCsi9I._p%%NQQ)'v#B'TEn]W;8eqv]f1$o=A+`*8e*"
"f3I##+TcZG7hkg$_meO(j/UQ/PTk]#XeRs$aKf<-m`QI%g7WT/FZOZek1s5*Zu.&4t6NJ:*6:8;5kj]Rc3n3Rgl//>h@r^HU]IPZtO#ATe-m%.3v<E#-X%`89p&-Fhb>8e0vDG?^bUQ0"
"V3Y?JEUeA[ZK(TEb0U0O0PF6=f^p3NofSNDutnT.YVh`E?^H9BCall9?:?6D%/5##r*JT$S4G-&SeZq7_7?Z$@p^58mGEp^m+WO'uK2#&]7&>.5oms--,u+NC5Wb$]NT_#)^@Z?'Q=%1"
"@B>QB;+NN@Z<QV@B0HT`fCUQQs+N:;@4V#Tm#f/OMDX*GBD'ktcT;?&HGrN'cZW`W=BYhDIOMmk+j([qPmMqWcPq$I5P_QEvASv2B;dV#&Co-#EXI%#eR+l'FPEQ_99e'(bTj&dZ]p)("
"VmQv;9Pkn:TR[Y#]R5%t@q=e?v^$)*V_qZ)VFLJ#^7-$#tJ(T5e1QEG_E0%'L$;h(SH(sR6oj8&T[)D5.VZc4j_N1:bWKh<xIDb#w$hs02PSiB>=nJ1fY#s$7Qr/1.S(v#0PY##S&':f"
"+tDM2Z#%&4;rG>#2/.]4O5Ul&[9;W#ubp=ljwcju8>iQ4NR$O$4Um;#-Mc##9F2##[0x+2H9cR9.rdC#P,_bE$wrI:/ptc;suV%F@V@YPifm>#/S1v#.6s34J<X/)M,Ul3)C&XuL,r7u"
"DQ#N#u==*ZVXGeuKEd.5$LD8$IQ[8#n&U'#O?*w,](j50kpC-):9;m$m3H12fG%&4PS>c4K@<n%ds3RU[Ko8%^fse)H$lT%BJwf;0rcY/fIATU-.t@e%Vkq5dTm89<gTb5,$TI4Td@YY"
")6[uBB?Xc>UivN3LpJ_5iwqSLP9nI<*g;^ud6ra%`q6##;rq$%D#>6#_UG+#>A?L:::H.)QM_YG)EYE,eU+0Lam#,2pZa>3t/fO(+_EXeai/gLpBWO';]d8/$=Ss$=#[<_p]B.*I1Rs$"
"IJN)3pe4`#;)r>$T]WI)I'D.3)?&.MhJ=jLqK0C4[eX@$af/+*_TZ2Tuc6<.W:Rs$>trlMUPIY'N[ZVDY7UFdC:Ns3+VPM9hh$81]TftY)mVh^$.6,)WjxMA.0xTrNWvgIi&/^d`aq0a"
"*N#C@eMam&rN&kCaA)Dj#$E/?wmY9BRgh$gwC&[u5)K9&7kH*ZNFF8%4`(ou+f<ouNh<Y.mtl6^'(@^a04OaM7&lmu``5/YhYJQ&o>(##R3n0#te;j0AE=8%PMW]++`@w#<P1v#NnDD%"
"K@1/#hW%O(%*oO(4]e[-n*M8.YwH>#(q[P/=o_v#r8#Ou`vGq#Q/$j0hG_H%qa06&PxV/U$Kb(<$RH19;4u7[e'u)-eA-i*w5xu#=t)##XhZM^A:L(#Rw`V7Vpq,)ovx%#x;5ZG0J529"
"MU9j(xhv=0&m_>$XeRs$8T7w%*RU/)-pXo$688L(/1ST%tV8&c:JND*Hi8#I]^h4=':cNuL+c4];U3/n1Fk,S,k4e$FrES>r>s6&+d#2GuiC*$u0S+Hxscju(T+T;v(%##We4g1mR[8#"
"`P(k^]2^YGv<CdM4udh2VE/[#8XHa$RJ.U%O0_:%Q)r`3]-AB8d*o'B2%*Y]S`dFiqv@Q#frI=gdS)eX)OUXS.3tluCHa&,#)Yi_W[EYXoH5;FI*gi-W/5##BRUV$9(G6#o3UhL-+Yt$"
"XeIX$Pdf;-vwHa.a(U8.-?xf0uY?a3aPq;.`loiL8F=Q^k:>tu37`6lJmT>in>+PlS)4A#wbX]s/gLi$>6a@b>S(RI6o+#,`p8_AoM%+MgF3$#9d5t$;r_V$N3AA&3`X?/%d<I3(mxiL"
"Akw*NYOc%t$(=&4T[YP8IpIGD9jllu*F#2'i<'C-bW>[..FmU7.tsNM?OQ-36Zi;.x+lw9PlFcew7T$9DCWCapCZ2(%ava$1D/]$>8</Mq9xiLa.<4'eQXY$V/+dMSfYV-c2RY%'3E.3"
"/+5gLc7Rv$HoIT%D9L#$xDCwG=?2ruCGCQ#jGC2k,3Xk`O^rdL2WGK@@adF``<xF2(9QQ#EPYV#24(70_fO_P+p0jZq?lV)U:C9NBPP,2QFNm/6lqV$SHSYG8DlD%p2hP(F$nO(8c&]$"
"M/)G2=Ut1g?I)Ou:o+QuW633NtWAVQu1#%$;?YQ#JoUS%AW<n0:.rkLn3=.f$w@+;./NlA#uK]OZ+%O(FPAE'IPg##]whkLm2Gh:8AcD4HQ'=7#9Q&4O,%j:n5(K4Zw%mu_X@WZ&x.eE"
"sN'QtIh=.A=c0Z$q>Rq06X2V7>+eS%Gn]2'K7*k(*Weh2p'A8%$<LG4pGDD3pT>ipF-D)fmehk3]5jVux&MAL6x2na3]'#I;G0UuPFm`-ckW-?Y[JM'Bu:P-JSu@M7[*//A@<.fD)[Ee"
"%bj;NOUcGMoHTI*9,Xp'P$S@#G?^6&jAV1(#,#S.:-R-3SC5B#&Hrc)oRxY-kFxY-V&#d3oql)36/'p7fpjv#BTCR#Y<-K%]Zwo.Mw_v#]V2@$OWs^Sflnh$3YnD$iGu'$7/GYLO[i78"
"o'-Z$QCJV18FmU7En/q%aWSYGu4l3Fl/YA56@b3FBa*XC,reC#gD.&4$RhMB+XjT%4cl>#uJLfq7%7W$v3gS4xi+*uxlPE6g..eQ:t9'#LhQ,v7ijl$W6i$#aP(k^og]YGg-FG%R:SC#"
"S&Hm/`?7f3j2E.33K/$&0pB[#qk;Z#E'0GYx5c1v`P/N#ou+V#qwL=P@UDtVr5FepNVDe#@p,lo7_@FdqTgOlBl<$`k<:M];r,MP0h=Q#D7RbOf1KP8Mvg>$ux^4.tH-.MEJLq%QZ>n&"
"&`x,MQ<QA%qa^>$]q.[#%N:-m/h`8.%)TF4>U^:/C$d]+BbIJNl79+fEo`jQRA]cY=LO.gM)d9;W$[M^1%qT5nL;7NU/5pJ5oI'oB`ho.3^[s&#w]YGe$M`+X)>ZGNA'Z3K>w.<VjM<%"
"ikur-?]d8/>]@t@nOmr?GlM4oAuDNTQmgluUQ=_uUMPb#lL->6?mq7#pGop$>*V$#CkMV7^J[h(Z5rK([3AA&';&G&7Z]_m_Ko8%jhYV-,oET%/U^:/(aX^qiCJU;Sv.su+tE3f,s83d"
"w7hi$(D(nPWBI-,CE^Y>eK+U%DJK@t,T@.H$),##%1ST$PP[8#YeDV7W^CG)AjUu$lY(#$?s,m8EK9j(&[.8;Z?,r'S8sm$iK</:h$;NV:5F&%#VB+*_XM;$83B&WePeQ3+Z]VCvo0?R"
"/)-xB5=lTPSJO#8,XxG>K?mGOrS(g@?[rW_$G)d*%7lc4tNgWAboef;6l%#J,c_q/O]Z-3e6T<A7CTeEqq%;3?G:;$<(G6#(.HU79f:v#6I1>9wtkkMZ^H)4'<1^#-Mn19[a.M9;J?,]"
"advP^@=@j0lb@0)SOU?)YWXY'S>`T%0['u$kMG&#X$nO(^^D.3+K+P((Qo,v(vdLp5-)a_Gv]t;*QkC<>VAM#G@qrZF7bZ`GC<,$P%=J-Sv<J-T2QD-qLeu1?CIw#L%iS%AW<n0F0Vf<"
"n0(<-xBk]#]1f]4^n^UuBwh[tPl+n(rTkp%sN/@#tR/TtvM0W0-q*lEY#thF/lLxL`rJ%#f%%qL%WD'#?,)U%q)-j''FA[#=&?q7&iA,3]=<m/w2gf1_^Jw#2F[m/5-1#$^7o]4S+/rL"
";Pe]4J2SduqY>(s1'`hLC*f5'UL2Zux5$JLJsud)NI7W$iM2o&E^/v#icik'b_1%,t^d3OqL*+/%c%n($Qb(?)H_pLTD]=1U:6>/Z4R&@m+l.u7e1^@B-@@HUmvrHs_,Z$5C<>d3M_U&"
"6s<T'=haT%8Cws$GPG;$.,6j(ul/gLo_CD3[UiS%Q+vIo##@iK:9QruOE.2B`iEquvV=Iu_k+u#T:wo#s:DmLotJfLaipvLX1n##7d5t$-YU;$KKSYGFf0m0A*@,M2$#:.kl)`#k;,lj"
"_CooupT%Atb9G+Mq7;,2xKJwTR%q?9[@q?9tvv%+%A%<$KESYG6u1?#'5vN(pmeO(.]e[-djeqT-Yi>$jT+u#><'#$XI:v#>QkYGH#.uPKIrJVND'&Fs7l<%Ajqj$Je))%+,KkL8hdh2"
"=IAv$m`Xeu$Knou$:rZu'A,.UZM6_JZ]###lZQw%9@C58#EpK26J:8.r5/s-YZX18UK1_AX)3_A*UU_A#;4gLNB2/#2oLkLxRrhLZ]-lLQOqkLAKbW#wB$(Y,:%6t4V.R3_nhY,9Eg`a"
"@(mERK8>>#bi?x-5an1MXPqkL_*CB#;HvsE[x?luxI_n#9;Z3uRWnSuT,GuuASN5$8/g*#5?<p[qA?-dwTJ*4g7(/u&d-guKH'F@84[ci_R1W-A?cf(?_G##e)v>#a2]t@(@@D5vLn29"
"pr/f)IweP/Z&_G+hOVGNGE7p:9G3j1>,)*+;aA/=T`###,Q.)vTprS$L(+&#Y9/W71gcs.sF+.)V8PZGflMP*[oA*[#C>n05dH&=YrGg)#CBH<tY>W.>59f3?gg8.BbA8%)B?T%HW(&="
"sMo`/AUJ6M1%)h?-f$s$hD/I>ABb/<nXNST@7-+?h`G2B]9F`*#'>u/lnT.YvHXs6ck,x-h)JCuYcf>GvssXS9mfe%1-Q1pp2)H)^ULS.Gu5N)l;mo)p>%.)&?-R&aqdD*kp(RWoW`I3"
"pC+F3/A(kL(F5I)DT[F4&9+l(PL.s$a%NT/,P0N(R9Ss$V<,W%Ub.;HeTFK1^M]%rgGbXL&`b6/nTB?F_Yt.Lu)ViPjIS+MNNY2:il,rupZ;>#*#'EP9FT=Yg+tw'S:PTcc;E>#(rN1p"
"2OcMBPa,/(Nm*9%cxNP&HAP-37Beh2`DSl`-)Zp%E7J'JZ34=Ag+*`#/f8JK6f--@k);)5*=[hOF`YXC,Hu`5:v(jt'*4PJriF:*1%@ZM/&Zv/4s^>-S%x+MSi-m8T/@D*K_kG2l,:;("
"UnE6&o9o8%*Zgb*)C$'8`PQZ$ivq0(]Ee*.[4qkLmg*F3DLIs-ILNu7GqG*R=Qxl/LFwQevEm)-^31Wda#QZ,#_<HW/FWHZ65UV)]f*&+ee:8.BC&%-cl+DW)=bW2W1KX`@G>(hRGTef"
"6q-T/N](_S3dw%+;wgu$B?(0(PbZ78pel-3&d<I3c,))3$Mte)HUTa#ArmX$0Z2C4jBo8%wwmY#L@-WF>WIS[)]VL?3+KSVJtmMk*3xA'')wx4w-^ZJl7]%]/-)BY>q`jtAm.v#)L6eR"
"WFC+#E?O&#TS2w$Z_R%#=pGt$AQHD*fZpn&N=R5&AW<n0omeO(<^DwKH*x9.ee-lLTcxF4/]KgL<@q^#=Crpd=hUlV$'_Y0R.v423Wri'$?O(<#]na6H0v:<EUta&f-[o;:6$6/OvLc)"
"#$%jB<@,87FGCJ1RJJp&u1Av$=:K60lwgF*K9,ZGO(Z68&xiEI@`d8.g&$@5qntv->;gF4eS6f$s@n-*xDL:%#DXI)D(NT/?FJZ$2(Sh)=(p,3N29f3aPq;.Wh%Y-.^q+,5?%t)2rMul"
"<%;eHoGD99_'F3aDd?a5xTb/ckom4e/FY315bqeaikN1g;d%eZxY-M=n#Xe5AexLF4JMIGKVp*WpG:5+Q*&]6sc,aXbH&M?Y>L8Id:o'7BcF8%fl>B,.kf,Rn$Ap3A:O>7Kp>`67'SJC"
"G[0)IG+fbH$),##(I:;$CY.%#C_;V7i#BYGs]GZGRP*.)W$O(&H3_#>?U9N(]=]:/Q8j'&vl,;H]_H9..MwQLoZ9PH&j'i5[T.[u.8#b-&Ew&>=5#D38fSqMX-FKuRC]'.Eo4wL]&re$"
"l7>##OP(k^:47s$jnFZGHE<9fc:SC#;x&#oYgtA#MH:R2e&fh2Qs>)<$pB)<=@.>7jb3>7Ex$##Zgshu]8p9;&Qsx+*0`v%c?+t$;b_;]qQTC-h+(l%KKvKjQJ=g&bDSC>wsUs/s<lf8"
",.fj2A1L_uG=FdV;3g6CrhW+)xm&A%.4+MMZl=w$8%+kb6&5;-3ur$#bue<$AUes$X2iK(ZC%t$lem,3A=Y@5l6x5/sq4D#P:[c;bwVm/Locv#WHgb,mbgru3X'epSY%Id/E`brlgk_,"
"w#DSI('@xO<%_fuWNO?/%+85#r*trLWX?##<R;##CrWd;BJ^Z-6>eC#ae^YNnTbYl[VKP/MC2K*PMZs.oNRX-PT+ZG-)JM+YR.X$p=Nu.3C>n0WpoguV(k5_INwq7vOcb%U-JW-.0+Ma"
"5JLK*s9C]$$jL3dkKJsMn#$9:]S$]db>`W-rGusJ8w<'4Z@BNa%Pd_#A$Z?^&*]l#6Dhf5Rq3Ku-hgtS*#pv?#VT>8M.tWU,WW]+14Ot$^7^6&mh^q%gqK&#<>UH31qqK2Hq@.*JRRqM"
"2vxYaP#b`ISdwEI_wtnMuO.Ch/3Y20&SeOfKn(k5wBcW8:_4rLlbj$#aP(k^t),ZG7&IW]fMMG)#0Tv-p;_5/`[C+*G,uh:2&,d3sVGn%iICu9R,V/G.6MtJ+kX,nZ?`/8^ZWkNB:nnE"
"+a@20c-?s;c_0wN:]%7Nejwv^uo('L5RVsg@kF#Zkw4,MkUSf*q?io.:Xkt$'_lZG)HKv-h]mo)Yv]49YU$-b@Z*l(BX5L%WgIIMIBhS-IkY1%9/Vm8UoC`q9_UL>+iL5'K+2;Xn3?;@"
"eJjtKqiGlLUZjd6i$JduS0M``uhe0;[$*=0IUO#5uSvO2>`)(K2R[dA&.k8`=FHL-)EHL-'Pn$:#;hR'#:#ZG1-4#-ZbHG$_;$V%d=-QL2B/_%QM>c4v@h8.7gDb3&jE.3f8xb4Ew')3"
"['&?&=C85JfaOK1[5&`qf>OXLn#1B+EiSbEiCZcMrW5lOeJ&Jht#_j:S:OA#O[2##E*tTTb*ooIf?JE.X4^wa;(vv7C1kY5I_bc)4gw8'ddXY'n,DL(O/o/3/)HX$C&.T%$nqc)Fl*F3"
"XE:a#G7VP9S-[:7KhJG?01%r*/PmsK@MCW$so0H2TF_FuLff'd,NglLdJ,*vn,?tL4^M'#DcwN'wAY=$hYq:%)^OGM-92M%DO/d;$]1*4v1NT/h?7f33YYU9l7=$%NQhfLV<$JHpt&ha"
"MUxr1V%e$S#%'CCZ-s5:5[f0FE2/UC<Ha`3:1-@.PN%E4$)M<7]'QE=qDI`S*PH0-K5SP1;i<9/ovMfLa*2#v;4Tm$.`R%#1iIB0kr@1(h/#ZGJt*@T^]tO('eDM2.FLu]:4x:/^gx9."
"O80J3:0fX-aKx[#G/WD-La&#@GoCWV'0887x>2)Q=q5$(=qHsM;>Sp:Q]>NuJV=pLg@O0vlJ.D$3YI%#N_lE*VU-,$SdKU%jxE$%gC+F3YQO/:X6^;.6Vt_$w^+KNjq^N(8</4,C':4N"
"P]uJ@KY=,H8[>)<M(BN5F(Gx[,_wSu)`0F@4;1&415o&#JxT8.CtR6'-?%<&<Ljl&Ciem-p9anL)i9j%OZCD3$f^F*OS>c4lwH>#N2uSh[nAX%%*/.*@?Po@8GGxFeE[I3<hKVQNq.R<"
"e%ZY#n.ZD<30Aw^6gQv$%KXB,*bRY0*-d%.T-UL;P*;t.)weP/_IR8%+N2_A*ZRD*p3hfL:L.IM&(^fLiHE;$tC5'+[n*T%kqG(+imdqugRw#(:M[w'77Bh(,UB<jlg#,*IpH)*_hc,e"
"HWsY#G#%?AM8N(=^xnAu6HpEu^N?NF]C?F>KVKMBZ?O*FB5*dEx1vo7U0PfL64oiL=(1kLTN3$#B;Mq%0YU;$?$BQ&(;A,MDx'kLe?Sh(/j/V-DcGd$kW'B#'9_s-;(memC3pi';j&M#"
"tb=xk;.ur#ske[u#L^Mu+P3T#PP4oLLA6Yu7QEjLvg,&#<pGt$n?]<$[YVk'R)rr$%)^GM7s<F3&]Md==S$X$ZC+T%XYGc4Q/h))h'x8%V/6%lF`CT/okJR211I:7wRJ#8i>>)<5)tt@"
"kT$xQ<>(jt6xw5=0+]h$HJdD6%^Kk.q>uu#DXtp;eXvv$RMET%JsVQ1<9@'(+ch/u4bsDu]+p=u`jV6L.mtjPKJ'(?.LPl$TYt&#T0@$,^$tp%SG3.)hZx&#Zd(o0VS(am/rR_#o=]:/"
"5SHx6h4%F4ZfaO'`(4I)[(9+*vt@T%Tf^F**sb[^wJ_1=%kfu@]#]TBQbrIGl1hW9#nQP8L>b`33(2HW:`<V9LdUkN,])p.3dbIh#upn&lJsduRbs[.g@nN7uQl%+l&d,).uQS%,15##"
"89eh26ax9.G/e<V:c(r$nARm9eb1Gi8,G9&,'8A,*(ChLxl=)P9`e*N]ef+4YEFt_k=7`,?=/-*Np+u-fCu^]msSs$Ejuj32>Z1F2o*-O1*k(Gpk0J5G>.c`9$&H<P5d0D$O@-)4Z.,2"
"0P<B#98b,OIm3d9eo]T/0b6rd1JemL=.+mBHShJ1Gt#&+7j[s&a?xw#wVpE%Vf&F*d]aI3Z@p1(J/Sq$Bd19/vBo8%T9bV$bOfT%AW?3:p1VoKPGls:&E8p:m%28KMG19;acX`N)2lg3"
"SK)G+r)Bo8n]gC5>5YY#<bLxOtO/p@5_fi'CWP6#>-p2'I29##lmeO(3lBL2X<N1)sXkt?HDGH3q9CPQTA'GDa[H_#[ekq$3kk>7@KQW8A=(x2n(89$4S[8#hiLk^'G'F*iT;w]])<I3"
"0Geh2,5M-%H<wt8oxId)ftIkC+(]v52NH(QQ0+2MD'+A9]K>b?N>huI6G2<_uYpdJE&H1$oibU*1CWsN($FA.*TTY-s.C&)QW3^_RvMwYAq(3:vQ<)<L%+XLwcTm(w@'+*u7G?,Uj$)*"
"]Me5A<e^j(G^aI3F6ex%`ivM0qH@X--fXI)dL4gLdXQ.4#IR+4k;oO(qtF`#DMY^:3nK8<H4x4SM^J98N/^]t=soM3$97R`E0'x-)AC$H4JEOSBJM@8P%G](QpL]eTM-.M/8&##HO$K#"
"IE'>$8b)d%$[Wf:qd+^,AIqb*Q,pa+2k[=-#<Hn&iQ8s$='$u%Oh[<$UiS^+LmwJ30/B.3+_le$F4tx'k2=1)At>V/`UBE#BM5J*sXS,2pH3Q/u;Tv-I$4Q/3nVa4#P&U%P9dH/hUFON"
"C#6^6c`ZREj^ZW.eFOkNA,vY7b>qqD/P9WRj[K6:?P:BHjmwx8Snt`Gu48RMX>CZ-s?Z:21g=&Hr(/RMO1fw'u<P_G_d[]8Y:G892-'##=e[%#M$3d$ET@%#Od9%#gv#7&NmL0(fB]N("
"Vahc)(]fgLJ8'W$gXieN'&eh2YcdC#fv</M'3x9.dhH1MkZFU%E:#F#c3a+DG@BU#;fqWb$/UM0@[qsui'Bv#/R%;$/FThuX_^`NRkt=uRNIvna'e##B9Fs$Uww+2PCjU8I:kM(io;T%"
"?Ot-%w^Msu8_D<#FCq+vWVc+%xid(#nX2V7aEhG3o2.'$=52X.ZKJYGd2@e6+ciu7,RY8/pjM/)ug'u$ZP_5/a%NT/*vRS@3,M9/]Ve]4%f^F*,O[q$=J.w$Q>MB7Oni#ahCfb5pU6dG"
"5rK]Ar:[C?3YQwnOt-G8/Std6dEDEF1pn5=34avBUb@I4U4@`>-S?<K/,Hi=$C>$-9U9w6NS:)5<0/&I/Y2F.=EZbPUUIJV&:#]>Ql-&6@0A]ITHrt8#r9Y?0&=r9^sV,#T$ff1c@ffL"
"i(VP&H<KM'g9te)Ms7t^bSfF4>d,<-q.^W6eG.p0I&8p0g<QcMH/Cw6HjF$6^H2J3ND^+4r.RF%RC$##5,x)v`Tl^$WB%%#>]&*#Xg8t-/''w#RFv0L#b=ZGEiYF+g?c3jv<Ds-O,.&4"
"?4ae$4;Xe)Crq;$p#M^>%3Hg)t$F4:OAm;%C18X1U9T_##F/[#m&l]#sGGkuhtY.GPKBeVL+N6f(:`ou0k3/f]sx'8KB'/3h2KNO$V:@HX$v6DJv6&CJ#bYOK<>GA)V4a)s88,)&i6,)"
"nhVn3g>*:WKUB8Gk/ILgf*oG-E%H`+diR0Z=(T6*XNCaZ_x6bnGX@aZb1['o9c]Y#pMP5&t$'vcg-IP/;,=T'.bUj(Jo-W$XAG%#Y*Lo-gB_<LE'ts-OCoD=Z=u`48]2T.irxF45@-x%"
"9iHVHZXq0+OO4=8CY^?R7,S@7$Gph2kc2dBS)PM2Xa%vcS'So7nW[P1k?9w@/]2m/&b3)+xQu]uP3X87n1QqA@;sd%d%`uG<w?Jh6U/2'D`Tm(KhnW$BLes$=i($#3JP>#[0x+2m4dI*"
"?g*$%l2=1)=xG>#+dAgMO78d<fBEiuaC$>Qrj(>YhE<iu`4UxPKvTG2pM`bub+HR=l.P(O`k?>#5p+v#t'Q;-6[JM'3RC;$Qi0'7:ax9.9@t@$fpneh(+E:mBY$0v?B2[]X0/s$3S###"
"=0-90svE$#dvK'#p9/W7WK>N'=[[@#DK^YG.WBw0cpZU&h`rJ1H&B.*Ynn8%*1M8.9x1?@;Dq;.*L[n(1&'32L:B;6nF8@Ua.YsY75kL=DemME:gUm(SpMT0/u'#,]#IUMd``[uOk@du"
"G4===I'6dt3qq2)t*hp0deHuu<*F=$+u`4#5?<p[b>+`mu/Bs$$7Am&%&U2#<OH`Evv[>#Xfmi'NZ,/(Ut6`avK^p9Xgdh2>5p$&qbPv$wWCT&oc^aBHIw[R)sa2>rN^a?FG[]@1Q:,;"
"nmCZA>%r_Qn)%p<fgc(?Ha,,>Ds0@/5H:;$Wr)'M$Skk3+ie<$Mk39%9PG>#/Xu-3uP<I31wHl$&(4]-c]p3acUP*$m^,4EXAL2Uav7.E[].J3X>w(Nn9CA+Dg1n##W:SIa$`/#Rah(M"
"+Lck-w*:w8=E@@%vBwKu]Rfe=S[;)5K3H%7,UC$TTSKUT^)1#>-W`tAnFE>7oJhk3D`BA88bD$R$),##C4#b#?0O9#R_;V7Yfx>,3o0J)waf&#iWW,3s.;X8<(sY$2;d;-8O*3&p77<."
"BP'32Ce4$.8bCE4B8sJOX+ja>/g3?;]Km,O;Hn2E25:SD?:A&6Nqp$7`N_dKVMsn)Na&l<iIsx,HGKH<in?\?I&c[]6Z98U1d(#%(Zn]m&2R,U%k'Jw#lRHx6qxvf(9S?&@>D[G<Zi1W-"
"T3&`?%kg7/[0b0MDk9luF)+E$9(+&#Q:9L(AxCv#9F_tSRM(.;]:Im0rxpkLPZ?L2B@r3VAY?<.RxX8@4*Ld2A^Sk,rg``HY)PA/3V(Z#P5t`IZ2FA$dVS)TY)od3fh%@@eH?0%&i%RL"
"4'#aK[Dm?K&tN=uV<(H)GQcf(Gjjp%<uP##_2@12(LOhD=>^;.@h1.&$qx[-/'$Ee3XRX/0RAhFT`h]4g;T5Swo*p7q5e2j*DYLA#)>>#HTsD#nLrc#;4i$#*HV,#/f:Zu7x%5#9R@%#"
"bI41(XU6%)9.lUeY4%gbr=S('a/aa4(Uk]#OPHATUele)a-Lp9axw4MWCdO2;U6eGX-*H$a2EI3pn`%C%KB3Mr$O($%H_7%GtC$#4)MQ&D@%<$xfC9I_g^]*DfV,lY>Qv$_n2Q/<m-(W"
"<0uf(#7@n;UU3&b#sr`*;h8H+hP5L:(81hYFro&$PVal$Mn:$#WGcN^OZ?)*2NN(RXpdL26skkLYO8f3qvpC6kXg[%xdBYcU83`]YD$*Q&U(?>7H>G>;rUD$P@ua^*QYT=qb9DILXXcH"
"C05##=J6,$KM-.%i&Xp'EE53'4lq;$@X[W$(rl29_RAqDspKB#bQaV$7t)%$8KSkP$.jUmg_.duU_+1QqoW$QDe9)*(7,G#5Av/=#bV(Ogc6(#eVIV601n8%7e%v#PSsx+)&7A4^[i[,"
"/SX.)<4q+*M6(g(f69n&B%`;$C_oi'^T2M/FH,Z*<lhkL_5*22`dKW$w[e[-[3*v#>$QT%xanO(ZjH)4FF+F3rjS@#K;gF4nCn8%Diwo@6i_hH=9:,)x6@n;&Sdj#=:_J#V7-wK*?Au#"
"+g4/*$w@+;un&HFjx#1:6N1oR_s:7G3p)oeNS:87Z:un2i[I&#N5O215mAs%XNgl(wW.5/b^lk0KLh/)hE^3'D(`;$Eh4/(M8ZVe23t+.=InhM.g9a#bpaGMLD(6*Td/9$trWCW2T:v#"
"mUpE[^MHN#9Pfe=U[2)5I'-`6,O1_SSG0:TB[2@^uviI3NWck0G@]jV<2u1#.,KkLNxM_$;:Fi%bPnx4Nw,87Cab_5JNR12W/k.304xf1F&vp/n`7L(n6iD+Kp,Z$HAP-3bPfTem$Ip7"
"h+.Hb7/ei$&*Oson=kW)v&v^ou_Vtiv:As8[DoZA4r7/GnvU_6LfSi;2P=KOV(ja>jv;&4>RWp./BCv#Wu-^ZT[ID,lG-T/?Hms8./Rq$teh'5k=ukDimsEE'F=MuPVs$)Bh5A4eQaYG"
"OWru,el%)*m>###DoW<%`f%%#f^KC6fVic)JCn8%YQKW-+9vq9gu=c4#IR+4bnC?5V>$g;0oP>/fISpU,xE`d&Sbq5cHHW8(OX_@M(`lS(0IYBCBk(?VxDk3KpJ_5Q$W_@7KtF5%(5M;"
"9S'4;TIR:v8)NV6:ZeAOp0MS.+i?8%++,&+Bf65&LK2Z#:p:9f:fdG.&Ec(a`k(B#&`f_MlU7>Qf>rLNYqt%=7o>F.q1QG.N?/.QA-2;6M#Tp&LOFfM/TbJ-V])60%_<D<O$=SITJN-Z"
"bebi$>*V$#LjxW7^9kp%M353'%`:5/MHSYGDrXfCB7?O(we#@5wgIO(xND.3r8Sn;r0M8.aEn$&eR0N(x1tM(IuM4u8B6TeN?+M$3'1n#$c5r#VVAE#L87P]1&F,D]h'B#Ow4wL:(8gu"
"d1Km$59po3I####QkZg#D&>uu=pq7#'JhR%hY`=-DFQa$i8.bMJ^S[#Z+VvP5[rJ#Mku;.'.`._E0D2:NIK,E5a&F.([bj-PC_@/S6S$v1_J(MEJ2XM1B-hMe:NuPrxb8&dr#d.37YY#"
"lwXv-:DL*Q+oExb)>trRIk(B#o3e+QlU;;$Ai:#?pxsrMpY>eu0J'g-;cYnEvP])#]om(#2)4a$=$M$#Iw`V7aG7L(`)$r%aaSYGE7:q:xg^L;0$q>$j@jwAG0u?.LQGX$816g)SVZ@-"
"9=]:/Y$=0l)B(a^j7>tu#0MN#IH@A=0vLQ#+x+SJpAEWWs]r]s,KPl#2+W]OJqG;[rk'#,D'oZr3[Hi$ll^B%Ie''#U,>>#(nuF#-^Im-B2Yj)x)S,vb:?]MSU)u7.XkA#?fXv-Ii^P8"
"],GG._:2,;id2p/Yk(B#_VNRM$Ub?Q3dEVMl-OJ-8Rx>-)wx>-&:8ZM)Li(E=%-F.=oGT4T,IW%)pjh2S%+GMC*9m&uNSP&Eip2)7`(v#=(iV$;fY##=_ws$1SY##qsIO(WcuS.asDM2"
"B:@q$uBRP/^%3D#2:_eu;Ra.F%QwR[<llbu@vsx=,(heu=ocoFNQ=W6A#9>>=m*fu&7>##ahZk$@o]=#cWt&#xiLY7Pgf],(Ste)URL+*uI',-6E_Q&x;Kv-DXEp%:p:9f-A7+Y8f*F3"
"fu'[KY*:T%G$4Q/c,.&4lU,H3e7#`4j`*i(V[NT/.cWF3n,cq.fNf@#R^?wfx$4BG`)+tK0Kt]RO1BAEWa@WK$5CQic?@xO#L&lV]liICL2QVH5t54DH,Ld2dTu1>A.]V-cd@A=l&[HN"
"kvNduTCwGd_LVE.1D-v#^t@`:Aa0W.X'>>#/B/M^%49w.nw`u7oW,<IBf65&<T$68S/GG./<3#,[C&a+Yk(B#.5DMMnbI>Q_Q?b#:d=REJ*0MTM#Tp&NXOfM@B^S%Af[H%vX4?-AH'g-"
"]jGeQ7:L)Nv/f.%1qWM^I&[`*<5Xp'[))S&x(B;-oH,[.P@1/#K^>p%T:SC#Z*uEZFZ7J3:M`>$k(TF4>U^:/;8`SIc@n$W3+(mdC4H>#O+KVM5i_=`An/6AAdtauJKj8tO+xZ5m@dUM"
"dZr(#k1ST$oG=&#`:ZU7AeFD#FZHZG24YgLe@(o0ki=XC&SqP2d0ut?5-t=&fM8f3t6xd>ei@d)rq/T%k^B.*o[Vb5(OGA-oSI3D?vsGJrlEY.6g`IEGvJo:;DC'5;mvCPN2QW:sp/wN"
"FnbG$g=>O<:vP^,CNrk1jsW+GCEm>-&)jd*Depn9%Q=%?N%@^?*L]e4TF3nB8dTF$DFMV<FpT:vm=N1p5,9W-:R_l8P+$=%0^-Vd-(35&cG88%5`QvP5[rJ#m3#?-5Og?.R]=YPv.?)4"
"vD/Z$/vt;.AYA+Gk/u]5*+Q`kI$;'#&F+##T(Y/3hFZ^$[)=J-vm+(.[u0hL,&ej^QvB'#Q4*h>b`/f3T++,M/Mdv$'+J-#<>N)#1ElX7g/i0(RZJp.?i^c->D8x$*o*S&AYE#7^B8-3"
"p/R-3,&+22+5h;-NYl)%.ir)dhd$d&b3=9.ae_F*Y=bT%*gxC#t4:W-RJ@?$nwn=ES?gcVku,4>;J]@AFT0h2U`BuBkL81k`OI;CB='[7#l&=K2r9]ZRZ&&^o?Kf='D8]k0gH?#idVF$"
"x&^LS0aA*0=D;<.UeGR:GGt^QCA=Q=JIh<JmOxYQ2T>]@3DdL^^t9VCAdUjM7p[0UD7Gr;#)>>#Qc3x#1=+e#:uJ*#*Mc##dpk.#9L:#vAS)w$SX`=-S:*7%bok--)'6H/p%?<-e-Yv-"
"R<GASd`KGN.1af:a>=F.F4rx9Z0P@NvnZ##._+##BKh70d^]W-iNCqMS@Z-Q^wuE7MJY##il1T.B9fZ#LIRb%)v'##st:f-L;7)#Gg+Z-rcJe-IV/gLE8^Z.v<5'vVq*J-#7t^-Z*.F."
"6%?kODbc&#sh3$M*2_m]H^]b.->h;-j)2o%2B8Z,&gU`3B`p2)v=$T.le5Z,,)x)*u[9R'K&'##mv*l(1@Z@5d:SC#15*j$1W)^=[$gG3j-4r.FYeZ$_]Le$+aQv$Z^CT%BH7l1(Z(F9"
"nlv]<wI>V;h:KeR1rXSi$VYD@=tUPKn7*D<[p`52WED(8L/)7VDaT'Hn/Um8?IF.H+?Krup4@nuT$</lZdUOIEo.$$4(@XSk;T`4tn3T<()*wS7Tln5d@K$v$.85#2,=a$5g''#v,>>#"
"'eY+#blv5.Ixus7^T`DFE)'F.4>=?%m@8'Nc6nQ8fCM)FJFl]O5[rJ#?4#?-5Og?.%r(M^n;fM1d%a-Q%1O-Q/K=F.)]QG.Fq%x9hIh(ED:jl&iG3iMq5=^2W5>##+eFU$V/aS=au0'#"
"Rj^5&FVmB=j<b2(S4Ph#k2=1):eS@#Y^jnUthXI)$iO_$n3T/1TE[ouJTPW8`NPW8e6g,288I?Ui@V;8t9#89c@Nc`D%IwBm3v*vUB$G$o`R%#s2h'#%3'A,F7C+*7cXI))IV]/pd96&"
"0]nQ0CbsP&W43l9*LPGG=Ze4(Q%(P%Q[2j9%,$Z$e]MT%5nOAFvP<X;TU/]IZ?iE#P09LN%Xg,?r#<4fU?VrZmHvkU2DBn)U6u(-FXUkNE:FQ^x28+.2(@uJqoUL#-m1t.*G(]IY1It:"
"(*wu><(HZaa9YBu`i`OB0+>>#a1ST$Je0^#8--#8C/UB#'####bkF5'kUVvP5[rJ#?Sx>-lvx>-5Og?.7jh:d&E/Z$+Vm50fXc[/9s]m]=#P[9`H+T.;eE=$'3OJ-aE0,.[;MMM[gu&#"
"Eo*h=Z>Qv$Sww%#Y-sV76HX>-r#:;(^T,O'B6VT/OI1/#pC+F3W>Ue$R:SC#I<f;-gLJ7&:-$E3twC.3=i[:/?/[O*_=6I46O8+d4$8b@X=7s$mv6m$T0?;@dm?`qVr-;V*ohNX>s3a#"
"8U(GM.go0)kvK[ua/,uufVAe,(10_ubflSd+7eK.*_^s^R(4GM0)_DMvM-T-mYHp-sS`0cTw*9.7C.&kU.Wm/))`?#J_/Q]DJ26&=>N1p2kW;HvS_m/-C17#6^/Q]O+>H&TIi;-wIK$."
"J&=cM2&D=Qb+l$O/[6+#X'[x-D94bOng_%4khMG`K/7K1=,(/##)>>#b,s@$sK:_A=$1R3JhW`<$;JR*^IO1p1DkAO5dGp.LdXrHR^'FI%2rI;p.2)F-^>)XUondFDNx>-t;D].ZjTg."
"st>x%s,*B#BXOOME=x^PGT.0%5Og?.Tot:Q^En8&.6%#:^j&.QBeub#Ypkf-=W8x9oj6GMpZ10Q5#$)</3Z>-i,rr[UHV/;'mfw9&x?.Qdt)##j=gAOaecT/+i?8%GD1/1%+8g)Wsqt%"
"NrN1p0s)H)0')m9DtC^#D1ChLPA6N^=T)hLQG3,#-o?uu@UA;%NMY##oViX-2ie34BbR]k]xou,$$FM0J/xh(lMDk'aY.-)rlD9&lQ*>-Nf1$#.D+F3Iw1o01X)22#G+F3X;;r(tS;]$"
"NT/i)9,0G*7V^S>-i1p&Ax=fC%/t(U4wIs%GRI$0$Cw.Lxp(MPSw*A#S'4A#>(642P]`aFV;o2QnVZW2QPR_^3Qnhe?Q[0eF,LW-n*A-m(dC-mDF,+#wL4.#>`dE<bWjJ2<FkCMCOeZ$"
"NQvt$E0&F.vn8'o%Xt/.`jYa<xa@5Bc+5g;tQiw9+;$#:bs8x9J.Di^3WrV.t-N>?d4P,<Gugw9>uYdMKKo#O1kZF#Fo4H(AGe%41K]M^k/w&#(q1c#is?W%n[d%F,<oP/I_bc)05r%4"
"K+6N)%Z7L(dw^g2um=x#&5QZG6B,ZGVW(o0n,-@5U^Bs$0Eeh2[]Sj%Cw]5/+?AS'OLsk:RSG)4]ifY%W]&K2`ZwO(iKXHEYbAm2@vgA.l-ee+LYx02bjoa%1OJd5xWEj(wnC`$-(bj*"
"k2v30&u(M^Bf65&B7W&HL<sJ:>.30;b4(##VE=N-j*(*3<:op$@6Bi#tvK'#,Y(?#v6vTMrh(7'kwIT.SkN=$N+kB-@m.N1%rq?@:)5N^7AC@#A')w%m-kB-wIK$.DSLXM@x[#Q4jX`s"
"@3lw995sSMm1g_QlfeiN1qvb#%fd;-COkB-ZW,[.%[YF#T%b?NI2DpNjQs:6-a;8erSN'#kFWD#Z&>uux7]'ftp^^#)Pvu#7s3kOZ^/Q].+&Ksm4s7#_78=%4'@A-:?&n.wB5F#-HrP-"
"RR4s%M:S5#4>tA%'Nx>-Cmi21GGVuc;BcQ#a+H>#5Sn*.RlIfLX7',rGJ.,)btHP/0oI<$@@i?#u1OI)3o1Z#M:0A=o[r9fv,fO(@Eeh2k_Y)4)a0i)Q->)3J%1N(4ctM(VF3]-?SB.f"
"G^F&M)?#G2$;)G24HvDonL_6T=a:DW+I/#>Y+,##KxC'S$Y.%#S9/W7=Kf],9hS2'v)68))mMP*S:1/#Y)5L#fuo#IbJic)M43j9,cYwfYqNq%oP<qMk@_J#)np'%vo`/D$Gra[c1=T%"
"L+O-%f,,,Rir'^5K=l=]b_Rd#ioTb@I`.%t=]u%^#,Y:v*7YY#,AMc#v,_'#,Y(?#(b@H#N*5N^*>G>#U]7;93Rkxu'(LW.IoMs#*D(p$,:o#6-(35&A+xw%r=a*M%`:'%JW^$.Dc$uM"
"k@,gLSrlSN@Fk'M`Wv^$5Y#@.Usg-OjGAv$KQn%la4U;I[T3x9wiOuPnYxV%5pEv[-tfN-elkv-c/k$O$`Q'v)@bJ-%0;X/f$b%4LLZM^l/n&#7V:o(doW&#o_.u-4ou)M'<TnLbLM=-"
"rQM=-*.p7.XqdiN_b?+#PDcG-E:cG-T_s'1[42VHpZ[M^v#'/1cJ4<%Tuvu>`w];/,d3x#Aou)M5?D].$[lb#_j5l0K[.VHH9iAO#-BJ10],<%15#AF(Su8&GAWN9go0#voYn[#P?f;-"
".Yx>-@E%m-*=x?7k+snujf_V$j:1j1A9*<%ic/S[F'Y#ZfkjV?uv1_A+E7R*6d7v5aPq&6$()##(`?)<PMW]+&>i;$GeEX$Jkes$T]s5&@T^5/asDM2$7W]F++s20t[Rs$u@#G4B9;#5"
"E>;Yc=BS0Qx%gV$x7jF3d$3H$.>%@uqH,8H=>-C&f;/=S&]3a,Xj2qr&pf9D]3S3klBO*IpdXrH,YU3kU&f`$58u1##3@@%PFHL-DKQp/&-78#C?oZ#X<4c%SVo>$j*+(McrJfLQpX7M"
"*_=hu&a4WM0(@A-f;Nq-^tT9rUQ9eHf1eX1ZcCZ#AK#/v2+5N^)7di9@ftxY_$4;edp;B#P1nR9@W=Z-8QoF.FqUFIJUw(<WNT1K.Tc5/?S*H)#@,GMkhx(<$=V7R80CG)&Lq92drC$#"
"=^K[#57@<$CbET%GESYGDCJv$'.:F*FTNI3Dk_a4ZkB_4rn?X-X2XN<E[U%=SRNPJ74r.N;$g=uBw@uLBp#6#Aa(*MJW$lLg(3$#hDjHMPR49%uY3'#hhFZG^Wo^Pm83K(0=wt])2KCd"
"uD'jMgRXAu/DYqM>iGYJ/2Ci#v:dd$pR6g#uJ4.#Q<:]>udtxY8ka;@E)'F.b4<dOR^v:Z-(35&Lr>3Dl(HH.F-WM^k/w&#kFWD#FWf_%]p`MB)i?R*PQMUM$2s7#<l^B%Gi+B#uIrc>"
"$X/FI?Rwe48M8O;9kG,*HiF#$`=Jgr[TNP/f<JiTeE`v$BK>)MZ&OxbTSX-Qu?O&#BG%R&[ZC,)uvSYGcYcQ<6X&eE@SbA#q6fs-F+GPA6Ki/:2:S_#X$nO(FuUv#Iu6G,URD[6joLxt"
"_HwS%1.dt#&.7s$R`aZ][EBn&_V]N06HxuYl@M'9v,]M^&XReNw9WoeY`'%^+DLw%2<OS.AVTm(#&&P'rS,ZGpDQS)SrUS%#nQ-3i^sG3FCVLP%Ek39'$^G3VE9W-eI?eQ[*_h;dhA>."
"BL&)I$DR]dfV@9.3V*RLM'`/Ne+FT%r^4oL-'5&L@iC)<oafX/`#L1uWm3Q=#)>>#<:op$&+?7?b:_B#b&dN#J_/Q].YUL&NrN1peQS;H8eAT.U4H;#m4%K;=H?Z$1RajN0eQF#F,kB-"
"@OkB-dKK$.x4&_MU7Kd<KSKY]R#:A=ftUw9.<Zd/Tx0[%PL$.tNW4?-5esNM]_fP=.fv5/@XbQ%NVe8#dj9'##pCr.,B_q%[%96&G6'U%KZuY#CfZ(+<H&5<qgUel'Aw.%Oe_F*&QGW-"
"_mW78a+mG*kp8^dOhRT(/K)g(F6h$$PRFm#$8Kq#'=jF1Ne.ruuOlM#Y*2)[QRP^e8o*]/1`XSe>eflu*F#2'Nt$H`=o5rl7g>-]dLF?Najl=P/,8v5DS###<Vb*vY*pA$RRk&#juclL"
"CuoX-Ukgx$tIR(#fY,ZGBXTt%e;mR&WlG,3X/Hs-N+CX@[iPPjUft/M'1`^#aKinD6k9a#3r_Xo0)DM4.S39/xBo8%$=jV$d:$WFA<H>Y'Svk>ss<JPLn)sfP%GcLO*FwLu4v#H<6gpT"
"1Q1UD6^d>7uZwII+V:,)$98G$w5>.T-W/*0=D;<.2D;.GcF/M^x9$1GkjcN37AXxMHq8v]?>`g@ef3H@FCtv@P>%2p2(D5/Xn$##LdXrHCT[cDVE=N-HEii%K+85#gk+#%`Kx>-IB6T/"
";####cwtl'aWO9.&']M^wx8(#j=<)#+(>uuP6wguJYJA$uf-2Mp>Vc)<PuAO`:&,;FA-FI_9^FIoRnEIosn;-_d2B#TlTh3RdK,*XOFjCqFd8%g_`D4)T3&GEX*a4%g3v%DYN1p@wEq`"
"9x7r`(Z'S8-pgx9<:p*#J,kB-g$F3MIDg?.V+He46,$a4n#w]uG1@W$Ng2#M2)88%42SS7ubANCgbffL#,8R*%iHR*LXGR*@OU/;UbIeH.#egH7mb;-u3[A/$V]A$jkn#MnfE;2ic7v5"
"f)P:v,Ms:6A;T/:9cq#?)Yqr$X'&AMw&T%#6J0B%._Ht.x9%,;Eo1LG<J0p7elPW.NoHVmR6LL5/qg`3HLY'8u%+GMLGUM^S%PV-.rws$;llY#4>>##@2kx?QVW/2nN/62*t@X-Udt-$"
"Mic87#.H+i=e-M9YNMl8$;#ci^$=P#Vl(muosFuG%/5##J_3eZOLvu#eqho.IG^'+x(x-)cYr.Lr;#ZGZ8PZGVW(o0`]a>3'-CT.LH7g)x3Zo$Rk=#Hlac)4K#Yw0X&Vq.t/bfQ6+[(n"
"[a2i%e,&qr5x$##(qs%=[EVcDJ.LZ):%Sr%-eP1pjxmonom^m/#>S5#6^/Q]/X<N&>NQ1pf*.dM/_HF#$,kB-1OkB-4JK$.BQRbM4/DxPIu^2BUZ7g;L-ew93S$#:bs8x9(QZ4o+eB&,"
"nNh%ue=lG<VFUw9hW1n#L0A@#i]wG#qH[m%UfL:;6j',;F)I_/lY$f;wxxc3I82:.jNo.U$SUp&Kq^gM)#$l$+eP1p.=<G5IW4?-?eG@./7F21WdLT%Q/5##h68]$`'G6#bpB'#f]_30"
"t&qr(8v>W.Ur]^+uUdi0?TE.3VF+F3'QnF#sce`*KNr'4Bj-o0P-3@,e^<1F(?l*O8+kB#l=wR8C/v-jL$B<$#lS^Fb;YTD`/7HFVF^j*eK#40p.IcVahJ8@wT[j)aK88%#5+;Z#)P:v"
":i###uXgAO&=O_5+i?8%v;kV$Bf65&a+f68AIDH*rquM9D&'F.F=DF.oOh/#6^/Q]iwUI&dZQ/XG0af:Kc^@n.0I&O:A6Yu9d&KM6%%N<Tlr8&@W$29s'4@9*'pYocbY*vY6T?.jE$j("
"o]k>R><U*#QgXOM$gO89$kUPKDnnM(_.36/q>uu#K@wiLqr[2NLqugLas0Q]0GC4Fi-N>?\?-cw9A[YF#K,kB-+U0_-<Q/x9a-:v-uLQ2(PGIDO`iJ/;XF:p$$#qx9b?^-QH7%,;X(35&"
"X7F@95I)^4>>Xv,A'KM':kC?#mwFZGVmQ-3RFvr-aeXT%3@n5/nLbA>Fx*jWnBF=0#QJ`$uB8itcLjnO;sBl'_JChLcH8`aPAI)*o,oo.AJt5(;]&T&iZ#R&[lj.)lZX,%AcTgL[N>$e"
"%%ue)6otM('k,T%jK^.**)TF4m_Y)4O;gF43`WS9aGww-Ail1<'gj[tLbEY0Ch*v6Znw$e8SCh$_BaNTjNMdG#na77BQP:'8N56&TXEM^xr#0Peo<;$#),##aDW;ZSiSY,/>R]4Gop2)"
"^?92'i/7L(SgYj'nr]b41id##=EI12Yhj>7fgXs-r0HZ?4@/=%ltlV-^>L#$3>XQ'2>mT2727&4#C.J3-tkU%GSA2'eDS<Ckixn0SC-bUFxu;iRx7F&pK=9%ig'u$w*0Z7[e<^6A4N^6"
"#JYq7Dl&?$koC&74DC8D'Zl_td$[gu/hdC#mA-+>678Z7..R`E_.r@6_7Ix62F2=-T6em'Q^B?7IGjPAStKd2F(>>,*sU`3]JTc;]-/H+1t$0)k6OP&ZIe5&JrOR:f&=*4PdK%#sUdi0"
"3&LXeC&;-muoP=7VESZ&k8]`.q/Qv$xf?p./]NT/hfA'fc]d8/oe+D#`*[`EX)8)=5FIg-L_jX7[+@=7>E6NDXDjTC(v>VZZ#_92NAs#7FAVv[rv^>$.KK+4_J9f3bgX7BhKtcHj#lJ#"
"h>$J;`rP$,&gZxu$rB'#0wBJ%+f''#rdud$bR@@#J_/Q]:3'4&Rl-W-E)'F.?)GF.[*Y6#6^/Q]^D)wev#S,v(pS+%2ZOF.,c)W-w%Mk)gtu*v@nw.%Q`GF.cf=WM&Xe4('d2jCv0aWf"
"b?s;$&)?uu2,fdu)-+K&&#Rfr%cc)#Vi,7$nKF&#7FmU7I*pM'j>T*%Kesl&8Z4N3:H3c*cYCT.Gc*w$:/>)%9Z8E+?wFaO;_%G4%TZC/Ucju$Ms3PE<a4mBmXl^.>m/.G1`,AOX6=V7"
"FhJM'=X3t$M;Pn*@ZCE+g.fO(HbY'A_q[P/c_C+*3S>c4&u.[#?Fse+8OAdN/QtkLlfvC/VM*>$ZfnOE@sFmBqelB.=ZahFb.Zp1oxdk$E6>##K22j'Uu[fL`?eL26%eX-l9lQuwW1>T"
"BNXg2C,Jp$,1`$#ZJ?U%nrn<$1L;qiqQnkL]oa]$E/m]#36d?KT<+Q%^TkWTODNd$&7eG4p-BW$<q)X:`[4D5fJ+,4v%gZ$ebo]$KUXgLungi'aR'dM7&_b*KF430OoC99aWPW8r'Dr7"
"O@q^#S3[ci2)P8RE2Im/^(>+'/Xu-3Mh^'AK5p[o:FlD4B4iWCJ[iwB^%CN2*tUA$BC]qC3H=nD$fI,4[x,g3Uh7eQ4gvC#H5=&#O)pm]Xk6%)Ab:I36hfG*-l/O05nPMp)@$d3=YB&?"
"_E0jCFvhc5$2&E-53%A8K5VIjelmonYu8>,>EM4'rtX0Lo2mj'jw4b@l16N3Z4p1(/Gw-3+[it6TOr_,l(TF4DnSs$C]R_#vBo8%3=EZ6%_;b=ZOBm09bx='(0S*=_hp21F#:Dj0)J?."
"&0sA5=B^c.@fQ]6%/5##Vi,7$DRO&#Sdk43<Bk?#ex8'+cARs$[w@@#=,KkLNg*F3eRDs-K^kxF$:T;.9eZ$$o)Od=*GYf,[9_kT'q_q8Mc1W-S0&`?'t,S/bjuEOQ@?>#m1$LG36+aE"
"J-JQ'g<J/LO*b(&gTU&$Lr7T%6vO$,xgqoe(amZ.C_38/v#^<0'xl0Pb>uu#>ha5&5M,m]Ej?D*^]@D*M4.],4xx/LG_r$#'.gX+E&nO(vww(9I7Z;%X1;<-_Z5-2G3VX6V[ss'WaH2p"
"?N`o09Ba`#pn-&6=>Gc4s@rk0DiVI*ULRr1DI,G4J1n`%I50J3-.<5&^vG&4T:?5/)P)C&O-T]=)s'PSrS@^PcqVlS2(^1OaNicD5XtJ14q[v#sWel/.F)##-#Uw0Gj^w0J$Z&#U(04+"
"oGJ5+lB+OM5>`'#m?[=.<ll5/^Em8/DZd)QK-6`M_RbD#Txxn0v[qr$/NG##s1/&AfVfCu_lY/vJoa5vxJ-0%VoBB#T->>#)qubu)W6M2*6tv#f'-4(+P+m>b`/f3q7oI%b^_n#Fi`,<"
"D-'vdq6bw'r_Nq2niMxb:<L,)9w[V$?T(n#7;Q'Zi>eh2lntM(opL,)sOOm$/b,=uBn:^#RI7X1NP:/__x13(3^[s&:MMq2h$P/([d4ZG=v*l('n%O(FM_lL&[R+4_PQ_#bCI8%MoNT#"
"&g/M#k@[oudg.a+oH[x,'N=q$hA;.J1$7Gtj%3f2=759/oX='u23^V%<67p/l).m94kvc<j:nY6@]-<-[.m<-^ZlO-5-UetPF?##wHb(M<pvY#bR9j(gj^r%-TZA']_G)46Oc$%aZVb*"
"CFpEuM1tp>aa._+8t+#[Juar*KAsc<ZXkA#X&<M-]<#O--.m<-X*#O-5QUA4m5Sp$=$M$#1Z59%RaSYGW,5ZGmt)u7^9^;.XVw#%wu(B#h(2p%uH6%)'t0q%wKsZunIP8%$w@+;ul`@H"
"`bY'G#?sR@-'t-HSQ2UM`uMeNH)SQM[C_kLgS?##oG:;$vsto2-Mc##2R)V7<o_;$6.VZ#ZPwP)asdh2mH7g)7Ku`3Ou^:uSYd;u+6%)]+8=I[=^T^[7BBK%soNp#5PGLM11s7#lNhhL"
"Wod##G$&?#sRwH?skH,3((3N(Pm3jLjU4P$0OEi93TEZ$O`<(Np*D8IE>cG3UM&j:xDcE[d:r$#Ks#x#GXw8%5M:;$tDk5'R?vr^?x*F3Od@k(tlh*%axpkL*&xC#3bQd2YYd8/-k>A4"
"nB7g)S1Hrus`wSuFHNG>hoNpug8Cs;4RAl#6xc'X,(p=usH4m#-ah?K.'3gL>-o(#C*G[$h%PWM:2nlL2uK#$w[N1pid:8e]@PV-owPV-A*uA#m'-c*qSv8&hA6n&ZqT4.--ikLCp.T%"
"N;SC#iDI)*]q.[#BkY)4iB7f3%Ue+4Ymqc)%f^F*249a#.S39/_AAW#.3-^QGV7(_)OsV8Na0#VOWH2a'aSs`<f^k$;:C8R,7.hb^O,>lb(C6V#lYLbuGdl]*1T5AZJCGZ30p=u6ofMB"
"YB3lW3:lg.,G:;$e)/07_####>lk.#FH0Y$L)+&#Tn:b*]-Op%ieGG*gBv4E[kJ,3CnO(&-sE-ZI0x9.]jo%@-LC+*V$nO(LA^+4;L?u6'xt/1sV2tP?DYP`v>&s`9xFmue@xOQexfE#"
"2sglb#&6ru`*TX;$J^O]<esK#,Ts#0rPUV$F`n8#w,PW1W#^YGE9ox,a6^Q&c+Yo1;f*F3Ihf20iL0+*1heX-%b([K`sFU.Iv2g(Tnc;&&B_:/][=5T-jY#RD&JP#1KE2>tfN3hv@&LV"
"hj<H#fgsFJ1[.[fB[dljGcSYblTg,8<RAjc3Ig<Va'h+T':f;]$D/.oCNlnH`PpCI;0B8%CfGF><_v*vEnY3%dnBB#L->>#RA8V06^/Q]P9q99QDN(vJa*ZMlA3qMx/-DMD2Kt@@(C#Z"
"@]I8@m?4/M5E@/;Ux)d*BqST.durs#n@T59/L4a=hdl+M3)s0(8%/6/_7]T$8c[#M8ETVMcVbu/<253^E0C-<D;,8f/s]GMj>C2Bdm9D3I%6N)#aMP*d9FT%GN;W..cUkL.m0d'+tqK2"
".Q5l95G]I*ivq0([]B.*/A#G4H$0[#bT:[.*ZBW$2M8Q'?SEMEcl9JLsTpOOBT]BsM,MU3ptP,*S$f1(Y?Oq.6xjj1a[AO*,/voL>Q?;NDfrcg#dsuW]f*&+d$RP/IQe&>O(KoW';:Y9"
"RZplS2qe,;5;H42%/5##;mN=$[G=&#lD-(#TViU%QhfQ0q#rk'<:bY-&EPZG,;gl8rP9N(xv@k(UT:6/ig'u$.3..MwEi8.U[s)FM7;H*gm?d)p):n)nkm.4t7WfUHn1WUO_jhI^pFlW"
"@8rgl)YFR<ZYrJXhL]vCN*0mO5Z'#?ATQG[sQc$cAkr@DIc-xk(Jc'gF[TcOlr8G/2)GuCr>whQ9ju/#+f68%eO$o074BD*-?%<&[5wD*Wq))%:aMY2w*-@5j:SC#Imqc)<D#G4A6lgN"
"6Zrc#GO/j0/jhM1]Hl.2Y2?9ggRm<.VxOv,5u^Q$;@[s=lVJ'6=$5D$Y:Mu3b1+?-M9r_#m5YY#&lN1p]g?a*%^:D3:5t5(AQ#s..oZ##9ZPZG2mZo@wabA#c?OT.%f^F*RaNZSW^an8"
"9>utpTk6g#iOol?-Kl%C5xq:JXZ/M?j(9*SeXUOaEF;D7S[ZIXl)FCA6=gKNX#[p0s3s+=-%ie=;/v]^B/v7HtXvrMfki21._HbPV%Si)R7^-?/4NXSkc35#%0GY>qK25]a.)/1<5Xp'"
"UXb31_<s6/qBAA&x[uB])SY0&=SmGESX:a4=qN)<OB(a4s6]@@C]Lk)-V*`#IQCLO3'vH?,FL<8=FvQnD990FK%=HHd*3t8e9`#K;/rxOvUpC6[gId39c[50YNwAJ6Oc4KTJP'8,SH0-"
"9w7b>v#Ok5_]Z,$HZ.%#R_;V7w0Z(+4oop&_&BYGFc?R9/IHZ$hFq8.&;sG2ZD.&4$2f]4x&9Z-?<1u$mIKn$YS'],.Djs<)vt>63rfTCp#w>A5>P<(($%`uM)j2gr*tH<i?I7MR.*2W"
"VVR?Ac=&:/sx4:_oaLXloLP;$V$`F[Rfxb_AHrJ5A()?@h[.'0;mN=$:*J-#-+F?1g,#ZG`tdc$mH97&R'1g%$[qK2>m97Bir?T.4#6x6x:qDLlQt4H'sTI6`n[S&<[NI$qX=mP_Z0,V"
"8df][.7%x.j<@##+1ST$QRO&#DF.%#BR)V7l%]h(HXM?#e0u*?UZQ-3%d<I3`3:T%Z*ZJ((n%J33$()3&r9j(m;:^#KOmY#u,&hk%&7crqPEQ##Ahr$QEC>uLG9xTPmdZ#j^,/$pP:fU"
"1_`O#QKfiB>]r,uJ?XA#%(HruUf]tu_>AX#nn'hL=B'1vv8LN$,/`$#OhOw>JIfBJW:AT%w@h8.rZv)4F$N4<sPs)-N,5;AZN?)<*6B^@<k5$(<wd8NJkRT(^=jV?(Q0s$si+##tJW@t"
"p36&G=<]O1=D9Q(`qml/i3r[,^ea2'AwT0%lJFvS)>)B#ec6<.2?Z&4twC.3hNf@#[nIs-wOx+M,6^Z/)weP/YCn8%^>=ci$4wF>N@#i(`u$cE/5bB#,HVp/%;Sp&M8BI*Q4./*K,2D+"
"UN1K(u.vU$H09E?/=+i$e7X<Sb9X03*``v#*gE9%,3dD#*->1<WvV>R&FC7#/'i]urYqPMOJL8Mn2[Y#EJxY,1SIJ(-?%<&A$GJ(Vo>$#,_9U9lKJG5vL=F3nYRD*[+^h(LHg[#QsB^#"
"fB.R/v)PvGSXRY<[apP$#'_%0K8KsI/[`f:O7o@Owvx4#9MFeuWL-0%t7E)#I#HX$/^bY-BYN4'e`Rh(:4Zi9)R9N(u3H12V2:T.,D#G4=eA3Mjm/[#kL3]-6.,Q'eL0+*3Ch2`8aAc$"
"O-YT%p:84MR/V04sEQD-8#('5<*%a>3wJ@RMvGcJbt8uS^s[0535+M.Z2YMC'fndA#AP<TnWrg(+v:@@YZHx#a/L*STulo;.B)W;)CwQDrQ&o8`P;MWVP7$Ac:8U/_w]-Y1x<J%_n^MB"
"(q/A=$,>>#)tK1pE^=)<Xr8>,;6`K([sh0(gdXY'7]FL<XZQ-3YNdL257H=&^TAX-60x9.>k.th3=5I)Ur(l,&mDC=I%Vgu^^9cUP,lX4eQ2NTVuOOO%v(Y>v31p:jw%w8_/5##)mN=$"
"K*J-#23*jL7vX**g^=>'q;*B+f2KS8IJ0N(E6RC>x)QK)q;Tv-,v,T8;aCE4*^B.*r.vP9s*AZ53ZRrh8/)k5[-C8eOv-0Q8Z1D*dRq=Ro,xqM]P)3vVLSR0.qqbu0'FM_sd.hu0b+ou"
"g`ewd_2x@MVtYeu,lw#MU6)'#FbCB+fKf<$_/SN2;vAI*GxQ.'#GUp7+dIg:LJ=l(22[0>)s[A6(A5cS9mK3F[GqR1j2^Y.j9bB@A9miMHiWB.ogsX-$]ISM9k/o8i7+,YViwZAgL]q/"
"Z(p^HtN=U#m_l(E.0dwd1SIJ(ZS@D*E&ol'rQ4t$DN'*>^2aI3iGlJ)emk5/:n-H3=X2d*a1JT%OU^:/aJUv-jST(I6?l,nBiti'715YusW(-u$?wnEpHv2:tXLuaAlNDo'QP3]Y6WSV"
"4uMJGHcW]V-.mh57+,##+w.9$G#iH-ba/$0`G*.)bq))%HMcD%[#:g)E64s'S?H)4_4AlufBo42:f4#HnZwEAKA-L$fC>xXWA&R&p>1x5VggfCSp3_A)elLp6n5_AtN,/(LWIs$&Tb(N"
"Whdh2lX(l2.c0j%I2%`,Jn`$#$)LdfbW5(SD4p>-0Iol'bN2FNV(<8@0kbx_hc:+n6F36XHbqnuM15##SujX$T$A-#jpB'#Vhl**o&7E*&gZR&A'###r#,ZG^A<.3FYLI$S:SC#bO.+%"
"+$fF4`[C+*R:M;$pn0N(h?7f3c5Qv$%6C-%2&OP8jgQ&@p^o&Ua&mWT'/Ipa`#xbIVsV)>ot]j3LT[5D*OAPO2%lZ#(SN=N(Menu^I-Zu0_nh&U?_RYq+lmBMo#,&GnYbI,>&##bBmru"
"W:qF$K_''#Or*4'm>6V%HJG##5xBG-0.[s$JG#02T11F*V7&/.pmMhLavO.)lo2,)X9Ep7quCT/KW'B#<AG)%c^%G4tpJ#8P?(&4^Ajeb+mwP1F'w_+a-<DHt,1iu$W)Gi_6>039[Z_+"
"O7/,5;0xUZiE@Y#kv52;B#2/UJ(1WJOl6Ku<%)G#v&C%%3Kcm&nfCfUJ`PJ_Zrou,AVTm(QWKu$(euZGi-A@#[I1/#lUdi0*eAp.53@L2kw#90N?HH3h,mT%f$nO(%f^F*.`MEIn>NZ-"
"lJB8S/lWqaa:?1c2S)(%g&(C&VFn1P&k67s%;3uux5q4/%La;Hb5+p.5#+/ZV;]JL0(``GXn42pPl?G2BS9Q(SDrH2jQxx&34tW%0q(hL2.BC/qv1o0$Sm1MbJ5h%'v,T8UR(E4@I<Z#"
"CW0W-T*b'S>01T%XBdZ$gvuF+-=#+IUDMW-bKG$^L%c703qOAHLScu/I.)A7@[&]6lbWXALG94WnuC$Akf:4tNwvd]_3s_s;,#g8xVAA4ei+AI>d8)K_oIZPe$9lorW`l]7$pLN%?j(E"
"5b)##`2?H/q)#ZGAs^Jf/iQ:vZdlLpSms'/1/(##?Ih(EaaXv-V0RAO37v*v3UZ(%ce''#C96T/;`_%#WNXj-Ql5HM-Cf>-f09$.t<Q/M-LU:@ux3^GXn42p)8trRj]+<%S0nCjaQw8&"
"mQ%f;EaI&cvVa%t)MT$,d),##.?xp$Q(G6#O-sV7wwY(+dCOI)j&qr(OPg##R:=R-0%OY$;d<I3V_Ds-Sg'8%A;T+4m>:Z-f@5*$Jd7/3x@XI)Vhnb2]D@j:aTt+5OMh=PA$]1$JCjD#"
"(5gLfUp*9[hjV8A4.@PETB/W$_/uFMKpP+eBpfx=CR2<%,oBB#TV>d0=Gi(WBf65&F_3p@#-.kXnIO1pB[#pe`ow%+'Y*T%]f8>,e)iH)Mr7n/]CI8%C9+^,w,q?fE`_0*8v19/*)<bZ"
"RbkZ##q=^$dYo/IK9Gd>Ks?suE2@s^aE`]L>)@1<)Eq=6hWa([%?(@93kxM-RG?A=o1RG)bw*o0&h7(#1<$jC`HI<%Qv4s$2hg+MDfC%M4xPl&J/###e,wt,]Wv%+#n7gLbAi'#fi4)#"
"p>=Q%&(G6#$E-(#eP1R/>HKS2e`8>,V<tGOBgeO(?k1o0#WTI*tXIQ/.ZOl(I*vM(bS2T%-x&Z-hTf<$`R$p.I5^+4rc6<.<p5Q9Y.*A-2<8k(K4/60>+s#NTCbeuZ<drupf,c&8bM`+"
"wc11NMULb[M1jCKfeT6&=O.w>$EM#eQUMM0$jaqEgBq.o@3.P`-)OF8?V&hbIjFjER/5##sql;%EbQa0S'+&#I5):%tsQiLVjv>#S/?C=;1.m0aF2H3tDeh206kM(3,dIZ1,41YL[Up#"
"G:6be-En[tQ^X.UH<jP/^PcP>cKub$#CVk#d3'c.$Pa4ASosB/<_R%#v?ToMx^=T'WDDO'L8e-)[B,0(T`:L*217OCcsGg)9L2d*^nr58a9CW-eYi(*ZGixi+OCgultHxX;v(murWDQu"
".k%uGfQ;[$N?L,kUbQ##2(D5/cZ2QU1<7V#J_/Q]fJJJ&LkK1pdIoJLPICJ`DtC^#wtRNMEC(N(T<OT.dvDd$<6-A1.:ZU7@%;Z#KQP>##CfF/+2R-3*)YJM6^+F3Df*F3##s:6UAFve"
"Unbou*:5f$-=_r$p^&,(_8g;-LcIJ2Gp?D*3^[s&VHJ%#G9SYGvBNf'NawiL31&Z$>^uS/no=W6TO+Iuml<^rk^Y=l'U$tWKVP*[9jllup@m*K&.;TBh8q-%RBW)NwGYX$8Xu>#.$###"
"_XbgLk5urLY#)hL7L4V7Qn7[#be-)*J:[s$83+ve]RLb*=OWs-cm3jLQY,<.cC)G+-h)W-<7I8%j$lB_x1IAcgs8<CtAg=up?(w5.Q<a#qfk<5qSn%O-dD;6&L?C#(meDZ/xT=l0plRn"
"8,Z5(K),##,Yqr$R<(.6A3&W7Buqv#JLRs$KZ#R&b3a`%H9SYG^A<.3lpw[>,v^N(F*CD3IW'B#$aoD#4YjDuO2nY,:^oCk;ND87K[qsb3x9s#0gURuexL5/n+.[#Pgd.MJv`m$pV'b$"
"s:q'#@dp<$wW2D+AGIm0bQTR&VHYa<e0H:Thu*a*ID6t-:FxR9&l(9/vBo8%YQWb$#FqS7Q=xpbe9Z[b#&T&7[./Ect$uZa7G/1@3pRt_]*eSTp.>9j7S?^RatEa*[x#kbCblm^j@)xP"
"N1Z/Y5[lkDJ0?cOq1us@t->>#?$*##UZWM^RM<A++l<T%LuG##iA5ZGL6SYGVW(o0b:Y@5vL=F3Xq.[#<;DD3GuEb3i]%>.w:Fx`1`1Z##UKpnD0Is$smCFrrgf5ufk,/GCFVVZ8K3s-"
"Kp_c?,:5+#d2NXC(UEh#>RsG80t&Dn<vnIMUn<^2+_nI-j`nI-[u_=11R)V7CXs5&8=RW$)4O3NoI&O4F*CD3rj1fq@q</fxq0VuK26Vu:t/-8&.0X:7%kOo:K8X:.9`c)8T8#,bZpn&"
"QoUS%wh`?g#]e[-1GxP'G1=x>7EL</lU/A$$?Rl8>&Cv-$g:1Nm14e+w8WA5Z#[n&RBeK)ea/W$CpFdh#'Z_CENHG&$7Y,@3[J;$2h%F8;4gfF2,@e%3F%eM?)S)*<64s$&D8)Q5vgo%"
"XmI?$oe<p#)^/KMGc&('q#u,*sU2T%%W$I*@aR1(XHJ[#>gT58+p>f=`%)Q92[.K:w'_Y,Xx5x6cQaV$=QAw6)hWM5G)2nBUs&7;Qot<ki/DH)NR*9%Q@2?#(]JptalKFBl-[@)jTVQ;"
"XRUXuRA0.*R#ok(U?M^mj7X&+i*qA#g0e0?An@&cpZqlfr/+T&ta^w1QESYG=l(?#L@1/#Out_>j]Ys.4t@X-jDZ/1%f$TcS<Up#V@?Pu_oIN%9ES=#Kva5TV;Xci-bo0)9&=T'R_.[#"
"@3(,)g_dD%'RJT.)*ct$92bd:AU?T.KG>c4E+Zcic7ggJp;eJJSjOkHrU]r0#grm#k),oHFZ%_#E1,d$gubSNXuF9%Lw``%Z+o+MIT-1M4F>d$OuAOX7'+5tWiRD3KW7:fY/c8.$]Uv#"
"dmd]u_,QlLGCW$#D]nN'n6<m'H6xG3L`1X:pskQsU4qCc=TQp#Hs^M0K.e*Mp>lO/0f1$#,)0kL@CE39spcG*m@9^#9N<C(Dn@X-22U1&o$.97/#I%X?TOmd+g/uuq`;fgq0qLW`C4W$"
"6jaqEg<q.oU?0Q/@oMxbFMo>,<CDT&QDw%+S-Bh)@Q?D*YJ25*'ol29tF'N(sQ<3VFefZ$`6qW/$sQJ(O4<.fiS7$Fct/lomtg4Y?&YA]ddHm$M#N:15v,(AhpDr?$w@+;]g'-%?XELG"
"RN@W&]hxNFc_1m0v'f'cKao=s3@AtuV.ATZLY-fIm-J3NsN-##U0ST$3E9&;&?'_,&v*E*YC1/#C)q8g4E%IM>?vr-8bCE4neX@$u7XI)=JGd$PZD+*tB:a#'G3]-DF#Mp'dXk;+JG,Z"
"(;]l>t5rFpgZ6#Ax96T`@M%>u+'3L%=]Ji$:.m]YUna-cP'6FoPS-4-3#Zr.&4[eJeXIfI`=Fs5_H_Jqc_FD*lnPV-9eg6&kk5K)R1)[#mQNa*;G)<-I9kV$80qJ1=J-W.)OuM(F$nO("
"V:[CF64;Z>MK5<.T+I8%Q'`<r&ilRnp]:rZlKWY>5K6,)8&E;-+`JM@(Oj5b.ZJeS/QPd#@p,loZ*aP#5=(PA%x.fUJat1Uo2GPlap:uPh.*5]esP]O0t*GMt$H&#32Fa$=$M$#CkMV7"
"c1gF*Bmb2(^DE-Zqr,E5YX;e$ugZB#xMQ_#'pCZ-(spM(j78T%iX+`#pp*`o<#h0N8YR@#MBn*fL9op<N#_p#$/fl#lj$NW:S>WttHrc#(*/c?1'U:vi`Z5/;51s$?3LfLgeb&#/1ST$"
"W@4&#xP?(#(Ysh1IlfL(tOB+*wOaS&^oET%^4o+MnbeO(qv*l(vRUd-h>eh2^k0C4SS>c4RIR8%nv#E3gC)W-T-wj<,a()3N29f3(CPf%4m_[,ttjvfkFU1K7l'7NN?_VKG;Y?,O,4,P"
"U^<Hms=f?Muph;?AQFmN$uxm)XAgg#LmDD<`3A21X%Xd)tx*e)ZXB^0]q<2k<*w0K*&)uu5.FXM1Fd6CH^q/R&?uu#$()##$S?)<NA<A+fOpu,$/Dv#:%C+*bZ=t$nbK6&Im>k9Wf2w$"
"Dn@X-/[=x#29r'&]1eb%);F.3Y'dwH8(D)AM=/A+?Pc8`GN&uL(u)p%`B:,E%QSdq[LUA4oCXU%+Ya]A[N?)<'89uYvg@uYJSWo9:=mb:;olER)WUpLOT5o$m_HG$>b5K18*x9.@d.du"
"V`C)<TWHw^qxgo.GiMGVlk.S'SVW]+8'2O'IZhc)o+8/Lr%+87>`Wj$@seL2Zw*L:uk`#5.(K]$PSlY#PRd>#(Nvtu]t0&IkCYuY$LW5C[<]T$l_cFu-'[guEcL5KRDu%=OF#iul_FfC"
"g#Y,%.(_MBtku.:n;>'ZDD4jL=(k$#AZ9@#Q6X9%DH(K(apxYGdXb,Me+<.3kBVG-T$WZ#5`Qs-1P51M1ckA#hxSl8>lxqu9Kl+2>-?ruIiUl]D;UN$X@dVuq7_BuMtL5/rI%@uTW6.M"
"RVDf5Y'89$3+J-#]w`V7dF_s-2NP&#tGPZGB`Gm8P/'e=<&B.*sG)a<6Ti3rSq]:/J`Wj$3#.T%,cCE4f1@MJqnVxA,`SjD)_4(QveDKahkOR<B@-.PO)G(K2Bw5=5UHP$SQ*_Avw^r%"
"3MC1=);eS(,kX/.bg:x7poNmTZ:Yn:*(JE/WeYl$H&)K$.;r$#?Ywj'S&Vg(aH50(+R4(0j:SC#Xq.[#wxQi$o+#G4:>0N(CSuV$m+to76W=s$0_/=sUM]=Pp[oU9^RCjtfBp$/f>D%u"
"R#DC?PHPXRj$@5/KE*##`dKD3;)35&)PIwBdfmN:FnB#ZmUG29^j/:&4gw8'7i1v#97es$:p[>Ka,%Y%B?gw#rf2G2Y*mruS6+&MVtwVun7_BuK[5<-*i3j.#N+U$+T*719:ZU7L60u$"
"c:BQ&.cXY$*-R-3:rd8.3.Rs$=9LT%O8wf(8tpM'nav[)D28b@M.'l'roC69i_>Q'8v7;ZcgW4S1Fnp%d^pP8%;$v5g2Y[$;i<rLpq/%#J:T-)F9G/(SVIm-Lvn/3R/jM%+uv9.+M>c4"
"v?3L#J,.T%w@H>#TE1k$$AeCWtJk.%:9uQ#2bcF#q:BetY+*74KLR=P:+WIEtV9F$j`@1$sd=RE=xWrLsF3$#46X?#AF@<$Wq))%j9WN&T:SC#u+gj2-DHVHA6/PdgsqP$W;aD#CWFQ#"
"'6'>$'Duw(H()##^i;e?dt+/(A<[s$$aB)Nl_gC-AHr=/aG.s?Tvx(M@tRO]$&`:?oX/$Zkw4,M<81s$e&EP8[nK#$(92X-GjfE[sO$##o8:L>Q0`v%41Is$s7jL+:f*F3@v)Z-A+2m2"
"@(O]b+GEou]320$$;.)$?h>X:^'1G)Im=)<&3gV@wW:N0;MFeumgJ*%LL7%#h^''#gv&x,evLG)q8?s%b)hQ&wNuZG@(1/#OT`B>@S1_A-.SZ7UIDE%@S>c4q`%e,53D($OrT_NfY5sa"
"&,dXL93+vT%54-I=0aW;AT.[uQCd:rE;%UQTN'B#?N)VrF2DtP>tFQu>v'bZ#8&#DUNF&uu#lEZnlB5vC<F&#-8gKM*UiPMC?9lu(kI+MYK3$#<)2q%T?AA&Zfi.LX&M3XEhvQU@'af:"
";4xJ36-c6u*%J8._*xILL*A)<$DouYss%##b>T2#H*8nLWt&%#@#d9%FT(k'X#%-)Eh^WA,U^3KV/CA7:i*F3.;Ef%So.8@K$?0XcS:5?h^4Va<iLuGJPli2]LYO]'d,V8cqu1T9$4W-"
"Ir]-Z[N@W&?.7s$S5.W$Ae:q:k[D:8l$tY-IPjZ-%W0QtuVVrd$x#?$Q_j;N,mVYMfo8fL.r1X:,sN-Zq-O9%T(d?#d+//LI.$+;FSbA#wVIR<ag;hL)v&<VHthINBk']=.Q_iK0)b^#"
"@%$##-*:e?#t?D*3^[s&@:WP&b##ZG*oaDue;.^$>39Z-6%xC#8rKd2J29f39lAE#buZcB==0ku3rR1$I1ZV#oNU=LaWm48s@>RDj?ouYhHMku1E<B@i+nWAHqk`W?F;p&NtsT.jvP4'"
"8x4G-Jr^@%n=xdd`q*.H:L3W-x/,kk$JtS#0Z4.A0ls/vjK$/Pmks9$NhbLGOQc:B+'Vi'$DTO]%/5##(I:;$O4NY0AlWK(8nSQ&FF?R8U%`?['XL`N6DuM(bXHHb.JHVHYqxlbC`_CU"
"Ml3p#C:)guBLR&Gs<]rMsi=)<a;/UD[jr+#h.BP8P*wrQSdru,3^[s&H:;$#qG,ZG3Z.duG;Gw&)qH)4XMqW$8pvG,*Woi)@Vu>#q:MG7c;woCd7A>u1$gm&qF#dOxs]cX0w-aM(B-x,"
",#DS/mr?>TD;`,*r`4BGW$#4v_gF-m9B46%Q)<34`^''#O%FO']=U8.gTxx&[nE6&B29,P)lo(']]d8/C83J%On6a*ke'U%nqN)<D9YmO'.6ZuVKlS#:DS#g?Hq0J,>krJLHeg5Km2tB"
"[U=20`BB`NuW_ku8U>bu9*39G7Y*?MN?,W^;,Vg@b1PfLR.g%#@=xK8qR[A,kQ'Q/PK,r%SxY?#>60t-W3(l:WUD^4jtuM((E_#$_g`I3(5Rv$@-(#5RP=Hk;4MpdjnIbI#o(THh(n`("
"E3,QV'WFQ#/Ne@@C9D<Bb8p;-33#4M&7@>#FZLc)Hh'H)&$+20WFM%,9n@A,4avN'5&RlJcoa/24G.r8[G-.OCblV&(EsI3;NX7873+H;FuUv#?hXe+$0<&5wNofGMp,)U_]^iKW@Zv#"
"=Io6MRi(K@PePc)?tTF&4qH*$UI'b@I`.%t=Vc%^.mg83.Ldi;4F]5P0lg?SFEfn$^J''%nmBB#C1@.%(l$##CD8E'8+6gLDZv*v*sHO$P:F&#F.B-)_Bgi'j(JM+a,Fb*vM$U.gh5g)"
"L`3f%9LFMKk@n3OWr5MWfs5,ODB5r;5JP>uGg'hNhk3a8(5JUIZH(puuk(MU.v6*P4j?YTZ#0`X9rJfL$*8qLP$1I$8NY##bmdw'c]jKYlHhipwmZJ;/.1#vb7YY#7(mi,I,Y:vnPR@#"
"K>xf;rN7a#us(fq=C@)<k?el/=>t5(X$lf(sV#ZGwMPZG.9$I%gLv;-M+Z)%x@h8.jSL2:&%lA#EZqV%@29f3`h7go0*.B@5%Rr?o_B=6)e.^.L@@t-4Rwa52XJw#3M(qGnF63;c,nuY"
"cgW4S0Fnp%cmGm8&>$v5boe/&L4PuY5a]M^;U+M)db18.2Ulf(f)Q3']KXt$ne^Q&hX6m%B#Y#PB#^Z$#[twTcZmUn2t_&@/Pu=>:QkieIQTfU(R^iO5-V]0G@%VD$I@G#PH$iL;XkD#"
"$77:23e4/(-sH8@EL.s$c8+##n6xP/>B3W-vfEigo<+Q/E$###PJQU%GECN0;eE=$i-4]#(<:pLrn;Z#ruTe-BFc(NmL?>#O_=fLA%dQjoW_c)w(DZ#Y_wp%wFNj$pdM)1^`.k0u2h^u"
"AR'm/0To6aC?b8.Rw=fLEOxM0;-^Vm:t+/(JZd%F]9TVeER*r$Wt@X-_gtAu]HNYY-B*;-,W]M^eq1hPK+85#B:-c$ec0M;5tBl4m]OeuQW8v#0.r10k=?)<`gRPjHFCP8&NWcDE3jJ2"
"Q<#=-6WjU-:&1%/okJE#Xp$kt(Wbr7:OvD.vcYrHJ%i$:?,afLJ=45&NuwD.p-kB-$T]F-pl=(.vp-qLN2_m]VL.IQYQ^5/Bf-s$,C1xNAa>.#3V^G.XFLg3Hv$(/Jq4jF?@CP8dA'FR"
"^Ai;?(]txu4#H+<R'#T8u<0O4uS[T%D),##L5B6$Og-0#5?<p[Sww+2u2=1)6AEC=VU,MgJhnNXD3[;%T3[;%_<b9D80Pe-]`:Z#%####wfxtuVI>'v<(Xr0?jAQ]1%7W$T,0:fx]wA#"
"T>GL#>.B*uUV44M.DWu50c:9M+qlx#F%=a$9b($#=L2##G@hKcgJ(T5BJ#W/#C.J3b_2Q/gU%;8kv940j(n%b',,oXqhN/[I5RA-q1RA-[2RA-eF3#.<bUaNB^ID*<rT>.G,3D#scf$5"
")+C#('K$V%>c_L:J;Jb55?WN4+)&u7HS@JLn.?8%R@HP/W*@>#Rm*J-`<'G.f8lB&KL`G37H.a,81Fj1x%C1r73eK)Sm$*5T/TM^puWv;YB(]:;8WFMfCWulv0xg)/E%<&Z?Xt$rh.g)"
"@:r,3o(B.3staB-^S0K-o.+R-W$t/1Fe*o/*=I)()B$$.(tjj0CRJ_4CmTM^p,2&:r2l`Xuxcs-RlIfLo<<YlA-K,*2DOB#Q0FX$K7EFNl;:c$sLB^#aMdb%-5N`#kcPa6/e2W)81xDa"
"iX.;8XC6[,bk@W$6YkHXiu#=]Qa[b%W[NQ/sHT1p6IE5&TLEm/Q?uu#(Z.%#RQS;12>G>#AC%w#^*x+2DO&'&;6%0)>ZH&4b%E%$5TpiuSek^2$9//:Y'YC#xJ<U,$WIcrm]LA#@iLH,"
"FZt7I^aNMU9Bch)F`9Q(2Yqr$YrNI)sUffLZ/?j'(Zi].wgIO(.>@^MWmdh2;,=FNJgDqLPe:58$^d@XVe+_uNL8;0LVGs-0hw0M4J`B#cg86,$)@MK-CqB#`Sf#,vK<MKDKViL(Js*M"
"tf[N#%l0+$x*V$#Bj>m&aAeL2:+kO:4$Ww$?]d8/4n'jDN7^b6-B_s-8+LdGZht%A_X%G4@*>>#F`%p$Jrx=#Z4i$#Jd0'#-'eiLUed##M=DMMUi/)vmxkf*;V#e2X=#+#<^oU7:vA8'"
"^;]i1ScH:R-`.m0]2.>5_2B.*Z(2uT/E=u-P(js-k`Aq*+>`&5?fjsV@3SvU=+Zt8F]=R:-on(ql_NEr(UR&0cgro*a-DU%Itfx=H_-s-7e%v#86cm#C.1/#(mjh2&U;8.F0(oup@XM^"
"libY#EwL#$<IHT/C253^2f-s$$2U,3'RVw0i?uY-)4hB#nZo>c.Xiu#=Ib05,x6Q'CPefLc3@>#<%`uG:r9Jh9q+/(=qUv#+Pl>#$u(XeJl(fD,(sc<7CV1&-cug&OY=,<;Os]5K7d05"
"'AxfLm1wi2bpKB#adlo@V%ZM#&ID8$u>R1CcRF&#9v'W-9+Z?^xe;;$*c68%l?^1p(x9B#NV'##;<X7/;mSm],LXrL95Ip0kD_5/J`VCWB8Tj*(TIT%X1MG;Bu*2pPAw%+;H@L(Gbe<$"
"C_`$#_2@12f1^I3s3rkL=voj%2oRv$(o0N(,0]F4V21-Kp3s0<+;d9D/XP8J_+3;Lkmk+V[[a,Hj1hoR=PS(#tTG+##K<jLf@k$#>Ht?#T6T2'D:;$#QQfYGo++87w*-@5CU^,9rGPH3"
"kDsI360fX-4W,g)8k_+tQi+j0S4bV6%_:87HGZM'+$HV[,QQPlwqC*Mk*9nLL#aG<rM020O=Qj)1FGQ'>te],80N)+-x>(+;fUV$HAP-3Gg3v%2&fA#v[h,M+lE>GTHZ9MTJu-$JD#G4"
"VF5]]-J4BQT;1K#PgWrL8w_(.CCsV$b(Y^?B,u0#k`J5`BqN[-viF^GtN%rVgpGS;Rr+A($BefOeQ1IQPuxu#apP1p#H4-v$[_c))EBJ1hj#s.Or*O1VQfE+Eh-c*OOJW-H>/<&F6Ete"
"2@75iqh>;-qYVO'IK&RjiY8f3A)NF3]b^w8xk#D.st4D#KX#bsO8laU>Sk`*o_+H]k',apZ8h>._'ESAgaMD7Su_^$BiTgAB^XEu^FUjZk+d$TDgg[jnSZR;qN4WQZeZPL5*8P>?FE>#"
"H&M1pokA;d/Kbi01IdL2Lkid*18*o&^Msx+]Fp%8kpG6qL,t>3E;V@G#*(R'ch5g)tA@d)ZAC.;W=v<.d1NT/*OT:/>;gF4B%JT%;[3v#HWdS=5Ltr;3wq#8jbgqu:8G>#4+Oo@S7IqE"
"3A7^6V)$CpYIT0QVpXs6kif6+x)F;d=7w0-m0_,PA+x6/5k)h)_JF6=0Fa5,Q95#XTlMd*Ov*'OG/<1#G=h+v-L[W$=7>##%_?iL=RCW7OD1;.w%L8.k2m4'?lG>#SL1/#7Ew-3dv/.N"
"C[fZ$a*'u$#&wP/NmuX$^^D.3F^t;-$F$D.%;j[$f$'qYBr6MTK^?C]KjZ_];*/%@q^X_?$0O7[,g[2WPX3=0CM=>/`w:9CcEG9/X>ie$be7.U5*v5Y3wl5YHO36E02MK#HX`DK.sgL("
"1W<UI34_S)&^ee).&w4Aj[XYYmp78.s3mr->3m3'>Tlk0r^.*+KS>v#8tx2'PI7N(]j%_>Gils.K50J3'9$hc3aS+4l>Z&4-W=P(F<+W-3M[w'*#8I)QC]:/.m@d)MtF`#O@&_%'.?*P"
".sAj>mFle3&n(HObm'?CD2f-XkRmK3Oo1[BUOHO>f@Q<h7-.*N`6s?#:f:S<]2DthCesbrj7pJ(7plg=C$SVf3H&M^L-hSeGJ.,)`bho.%e.5/l&U$-5Mke)<(no%L9SYGC:RS%mQikL"
"03B02+&+224&LXe?TNI3F;DD3DX)P(63(B#&Hrc)VF3]-,Kc8/>;gF40]Ve$L(gT%kHNr#eGnS%cd6Uu^MwS%Dqw[#9=+6@=AtIR]g-(ZQ%[L<s<3i<EH[CENIp44-@;+;4Tc@Q,7IIq"
"Y.qf(-C_.Y7kVNGvM*-H6GmLFx,>/#GK'^#ri;uLsj#K)kQai0U+6G*XHGn&Sj5N'F6_c)DPlY#Z.1/#8N<I3>N1dtDkY@56jg8.81,.)?uHN(=O'>.v1^V-<6gQa5Hx9.]BF:.,kj]^"
"8oN)KWg`Mg_nSZ#5VWJgn-(q#0`wSu$J$Cu_Ww9*.?2C&%$dA>8B%d#]wrHug*>>#rPUV$HrNp#J=L/#%/5##l+vR$P8^*#_EX&#Ua/t-&uJ^,b2qr(QH=W-oEC[8-v-&4Plk_$C:4#H"
"tYLa4^Z:,)vtUp$(ZK[#]bt2J8h+IA4,E1uiaPe@MhD6:%[8MZbeTUCT-fJVN0ocRQ5,HOmbrIU41RjFebiT;r5mc)f_:n1#21bR9c/;6=]p8.7e%v#Soou,J3`l8Uhi,+%60*+_h%@-"
"[`O;-iHpN']&B+EqQp,3I4dI*jv514%P#G4nCn8%TE6H3H<RF4/]NT/pmqc)RNjR8>;u`4a_E_&4S'f)F)@8@W:Ab.aqQ:$u%t_W1T(Z#>D>iCsG)7Ss40i<^xN11u@UI=(NjN:+5`aY"
"`kdO>p2eaZuEV992/XN;G]E%+aO9U%6_02:Ntoh*VD0@6Bed?ITxq(>SHjs@lbMMFc4-b-$mAcG;TGG*]Pc_5[La:8(AP##EZX6%9EI8#S2h'#^tu+#RjLY7C_&V8(%U8@/`o$$7.eZG"
"I@hb*^rdr7&<BZ-[3*v#$h8<-p+J/(uc[[Hh4_s/i@lo7(g,4uUd%`F3F1H2Shq/?v#%EZ<7lL<]r3l0vCUI=)WsN:+5`aY`qmO>p5naZt?M9938tj;$w@+;F+`qU9qJf1;bCX%VIn(<"
"de?g:7e%v#v;>G2mU.>>l)(B-H[]r8VF=%7(f;v#=ZQS)pc7L(-8WS&WZx@#.CH29ZYlc,8f*F3Swn8.*Weh2DJ7O0$l2Q/80fX--XNu7NxWs8u/p<7(g,4u212EPHE)l4^?dwAWC<M:"
"8*jsulih=69S7H]6%lL<]uE11wLqe=MQu5(iA6w7=0l;Hx+`=Qg*nb>/m2U%vC,DK<sIvP'0rjuaLv(5NN%-)Cj&'52gLLG]kwJ*c6hhEni-S;72N)F#4bi#%5:H4QJ0/3D@%##('D(v"
"O1a[$27>##E_R%#:8E)#ee#M([?6h2H5t)+Mv;&+]=rs$0GG>#L<l[$UE&Z3n^TN06ddC#(xF)4F`ko7^b?TK[AX8$trWCWRSrcutG4eu5%auLT6fh*,&^iLDX(+ABEe>%Z2eaZu92t8"
"1#43;@PYd$w#XnE,mMaGlR4MEl6_i+ia$8@R8<s6n/45/#4qu,jd4f*F+ue)qI8-)lDt1BqgW9^nVl]#@Cmi9;?%f3u$D.3hAxF4VLD8.2?Z&4?W_wGurS,3wQpdmp79AO.VIt0fGoce"
"sc.(9l-Da[HuW@0dJ7;6f]f]QN=cmDvgbN=DBwP;RVdp'vU]VdML?SG:c?_s1G/Y^2vLPY>aj=8>C4qIqIUU/#eti(];fvSP24tK?H_bB-bS:vAp86%Y-/s$0H&##9;qCanp8j0;3=8%"
"Bs60%GRHG$/Xu-37Beh2$ctM(PhA,2#dP[fmUbx+$cbeC$=#`*?Bq88&I/;mjCZU&8sw8'<U<T%+M(Z#SYjp%''j,dM-ZZ%,(I>#Z0fX-[U5g)%ExHut],+un1UBur=KiuBiFDNr=bD["
"8nG:Z%P%>/gkAI$qu7)=DhbmB#?DGM4?OfL_nkv.>b55f1nWS%/E%<&/]1Z#/PY##)Vdi0w/JO(&=dL2(v98.8XA@#F7^1pp6XQ#104Yu$Si`'lc#Nu?pv7tK(P'Ata`J#?$E@$Pke%#"
"&Q?(#ggW[,&q+K1[I`;$,wk=$60CH2#>+87sg;hLSs;I3.:?A%jeRs$a%NT/:,af4kuxF4NmuX$6+X,<(O>s.C:fT%m/B+*o-C]$-pIF@?JI^6Jr>/ON,fF@6,V&6IPUkXc;gkC`m4,>"
"uhde#S)V_6YLt-Ea7:d#6kfJ?q8bNEj?^a?2b]KF.(9pO4B9R;/5DD?1AI]@<@r-#*GcYuLU3t$T'+&#bR4l'SlL<fXgdh2=FP-&I?4W-oE4[T94Bd)Bo%gU*aOD+/D55;HoU[/;ba78"
"9BhB-T%%l'EDRK(juX,)KE#u&E>70(M^qGt-8v1v2/oH#0e)@$qj0'#/$###xKO,M1J&;6[]<N0+d@L$]cl##HQSW75&###PQXX$WBB6&NH,n&hNx+2#G+F3tvR4(0?ILM_vdh2dh:W-"
"5dt-$T__a4t+>/MSvB^#pegI#RVw.I0,@1pF?0eu6:c(Hml?/(PItckTecjuRMl'M`]c@0U64g1ElG5/8LR-H'lVM0H&Q1K^UtY-BpCW-7xk+Dh(kS7DB]V$Evv%+(`<p%/PG##L<SYG"
"1Sl##$@%UT'd#[-YcdC#V6WZ#6rSr6<o8G#L^^80$geLg-<F]k$*[u,+ZwV#lVStuKuA9rEx_ci#kCa*DHcf(tdf/LZA*a*es:U%pOHH3`^D.3s_.AkfnDSOZ6X@%_QBNEWA.t$&Tpu:"
"(7[hF/I@.#RBaH$)/,+%Cl:$#C3F8%`oT##tJ(T5j:SC#OCI8%hTU&$BZ__&v?KAGeYik'SPnh(Yn<S2OF6F>?5?uuDFhfu=bs-v]s$<$gY($#nTdB#wY^q#&MF&#,ae:0g[`s$J%dF+"
"S)K4=K+f]42CDt-kL:c'iLov$AoM)FU6qn/0@((#8*(,)B#)>cP,:5&Sc8>,,(0Q&DC7<$KvTSI[[_/#lA]f9+cc6<UHF:.Z%xC#g`v[-`ODh3rtAa#d3ekuwN@SW+jG7$+aoi)LsAba"
"D7XP84g%D19eEoA6ZF>#Xa`P83C8&ba@pu,UO%&,uvXx#Njln&@4<JMc=AFOK`F8&`B,H;bP)B4gs][#,^Su(%+7dNxC^(9H%K+O3a<7Dq*Lb,U+4&b)O(T%bCCYU`Vpf-=$[l;^4<Q/"
"SnOfLY`0&$JDlwL'pZ##Eqr>#P$e,$x[1>mkR$K2&AcZIB_NO#ean#%QJWc#P(t-u&8>##I.pa#L$=9#dd0'#XFmU7/@1J)(L)W-%*>'#9uU,3hD?#BteXI)aSl3&.W8f3Bk#;/H9$-H"
"3:9k$R?ot-iL0+*k_bEePCU::URC&PRkh6:EJJ=@O_OG<hB8+>KfW89,_lo95cTa:,-e>665dt&[j5J(E@+T%YFZc4clo+M[La:8v4:H4OD&)*MW:fL5GNMp3@NP&K&@D*^7pu,1K0q`"
"JN#,2G3;K1@2<)#Pc/*#inl+#Gmk.#s.e0#qRSW7xn5c*_Ns%,-5s>5=.4]-V7i$#84re=HYs;6SoJL(<IHg)Z$vM(8dQJ(_T/i)7,B+4iRG)4Glls-2VSfLpt:8.aEl-$@F5F%jaZR*"
"iT?R*#r#Z$*TY=-SWxv&M9Ed*3cEW-&@x;8of(B#U>v,%af+XCg5(u$ia7ea(JMG2OVM]Ffm]^b.85F#iqGV#$68X#ZY;9F(+:btIq($u4#Pf#mhZ9I.+9=Y>v-20X9M1Fqu[QN[+x-$"
"4Jh:H29$[72EH<8wgOSFpO(O1RJ*Zf3GFX.W^ql:)OZd3(-B@-7Ktq.E`1G$-6ObFkYreu]^;u#.tK:F$$kLgWd.+F]W@iF+QYM'7boi'4DF1F%mDtL':@nE997.u2#<qIDI1cDZ&lKN"
",w04OR:%c@Yx3^@V$mf8G9WZ%^b3891[DMBOWoA,GIIt.huk[tFAuV$c>^rQW)L$A&5>##?EsL%kGUhL$?BZ-h%AD*^T>s.wG>k0P)###?^*226@-@5l--u%?H7f3&R:a#iRe8.(V]C4"
"Us0T.Km?d)6JwBTW)@A.MA>Yu$jc=>f4<Popi3/<::ZO58s+F=^)$>7'dNN;e'gC6:dc+5vF'H)v-]:?PakF2LmR12Qcn,Ott=b+aXF_5__M-4^rD(6B4n?74EU_6s_P;^/u.5Jr<@5/"
"h=(##x9%,;$*&##GiO^(x;V9&T@n8.3x-s$SAt`##bK1pQx0W-PW[i9QCQj)EB4>-'%OW8Z+5jLw)BJ1@c<G6:34-3v$$@5ab+-?M0).38,aF32.C+*TL.s$GO=j1LmbD4$LrDNoIAd)"
"^^D.3*q7f3foQW.>;gF4w4*Y$+GbF*j:CGDsxR3=KT]h#soOUiou>g?/%I9.J3R4(&cF(7IGfN'_vqrUv&p=uuDg7<DK6G>tX$*'#3B(%akn8jE,(%5#?wbrOX$5C*ifOCG8EX.0.>7*"
")<Jo13+ID>$'H;$DSj1)0k?iIU'BlV1kWwIu8qQVv+@>#0nv(#=ef]b[oW]+P+2`+]q_%,eaBV%N(Sg:tlEn0asDM2AcS&$#dMD3=]^]%^$9N'b0w9.kl)`#uj?q9/[3;7Ub$d3vIX2("
"a,Xd*G:$^.Gvk%l_EahWN/h-3Cn06&@*Cs-&tXx^*gN@k43DDN%/5##3B46%F^>+#s]Q(#%/7o/%a/#,4=mC+oZ=>'5X:N(.A/:&wUdi0omeO(K..d$G$nO(H%M-QHE=^,@iPn(u=[v$"
"%&.i$.=F]-ah)W-Kag40*)TF4+a0i)0)v'&1d_:/rP7st8qCu?eL+)RW.wYQGS=T,fH?.qrOD,ulbh_,lLk^@M:2A7B:>l0c(rw:P+xnU1vB$A%^S*rQR+[:XH>C9f`uq^+9dV_=%N)7"
"bk44VW)Rd$?<>F%+f''#T,>>#MR6g#^L4.#n)&d*Zv56/*[qr$ZAwiL<pukMut;v#-(35&?S3d*q8lp.DY&m$*$'fDriZ_#gl8*##)>>#IHE5$HihpLkUIq.+3^]=bxe-6w$I3^cROU&"
"3*u2vl?8,#bOS(#<VFIu)=W%v[g1$#T5,n]WL6%)*d592U8XUe$ape*-eCB==]$9.[N=tKGqdjN*lfX7^xdX7EX)2I4O9)I*P)mE]%r@6](%]6a@RrJ@;7jKY]sx+jCX]+G>0f)&O<m/"
"s[^c-7A;<-uw<[82+x:/`?7f3j2E.3)F9a#S)Du$].B+*<*<R8(h*H;%0Tv-K=A>ctelXYwX2S=%n]<nxufx=sO_nfAe+ZKX<*uPAei9Wo,=epNP;e#@p,lo7_@FdqTgOlM#`6G^G,(H"
"'T<IdLe7KJg+;t2_Ba?#4j0uuR(dV$CLYB$4ne%#O*CD0p10b*apF9%9X4'I;^*l(t_u-ZbG+jLVGa$#$HfeMRD/[#1SM,*T7.E4.S39/q6#BFQ?Banm@()MA:UZFC&xx6(.#L(LUYD-"
"P5Dj7`^NONA@34F+^@20bB0DNc@&M^_w220j:f'=W3]RA0aZV7Wt9[^p4RZM.Mt]e$),##xC46%7Z.%#JkMV7]oFv,a^_K(roIM+ZC1/#.V(kLDfH127:oh)w6rI3RIR8%TeRs$Fdq%4"
"p_s%'8H5s8)]a3';ogwBZUqnu_5uKfc:qsUCt7QcGeIm<u?%7Ej^gLT>VAM#G@qrZ8Je>`?ud+$4YBb%'eP1pBR:DsoQ*20$j,##*d-v$h2#ZGXVaa*rAPZG?v7nNeV#f*V([p.,maL2"
"cgInN^`?<.H?aDNIwW;Mhsf]$(JUH2TWcmV-,Q,E'YRB]3iw(IaOcP6&E4Ds%k<4F(7wk]gH8=/cDZi#oPbkM4mxn#,RF*<U@/eQQ&(V$cSJ*%$N7%#2Qps$[-9barZs;-vgTTV^Nx8%"
">Of79^:8N0M/'J3>,<iKL1^pIkXJ:9rP5/*$YTQK'SM$<DDaKXh^u31u)uVHVZpj*r,_t-)r]`*D79R/DRUV$(7Bi#tp(N0rOPl]Bf65&aR[`*Wq/t-m@=a*?TZV.(I:;$*Y=Z-QMkTi"
"jvNw9$`$##O[)9+x^[Z5U>hR%i'G6#WqVV7%YOI)-(U3(.S1Z#R>(58oO%Or.']5/47GH3mMRD*OS>c4NOA%'35ah(vaaZ#)in$$3MIrm'R&W:Hm.97s&A:&:TPi_Sn%[#FOL;A%w(E<"
"^/LfhXio%LnF0H>-j,uum`mr?3_duPB0sIUxo)ipkDJR=SDi^QJ@1lu&dFmulUQO#l$7S#`0HVugO#[deCMrb8J@%#Kd0'#jn]A$Ek9'#g`Un/BeHg)gdKY$b)#ZG9vk7%F.IW-JeEwT"
"v/-$$?8`^#DFNk(ix5x6@>17/e>K.*:KAjK`cJ_4pnEZ>rRQ*0rOC4pSwosC;Mlf&sAs=%6kUN(B.lx#EW8QCr2<*Gr$XD<xx8iu^XXKW9s_u,P*J>,a?>6bTh7ul?3<n:&w*QBaCvku"
"86fvNgRtA#4o`Y#2P###8^]M^Sc8>,<m%<&BL@<$Wv^Q&66u2v_UWS)WCpG2tZ)223,B.3tDeh2VRG)4DYS#%GIPU%_,jOf'OGT#1ie9_[Rkeu%^L1f?Epo=G6<L$UH1gf67POaSogSZ"
"HWf2EVtgwK*$'Q/'[:),?Dt5(CYl>#o;GZGNHL,)XQxYGNgateJd/x?eo8K26%eX-g5Gw0?DXI)t29Z-kDsI3qsUT%u`<T#$NVO#m`H1AJfQc)Mcu=l%-i]+e<1f$q)CkuRWeWDkH4;Z"
"htD7ejUp3%L4-##1MOQ%$Q[8#`QSW7&@r%,f;>&#km]YGQnDD%YTs883-7m0jLt9^p]5vGjj/w6PiB.*8v19/e;C,(mpO8ItLb-Te=v-fH`S6(-Wr*V^OMP;5i^mDL.=:'vcl=>A_L&]"
"]a0g@F,(=Z,X,Tu<o+Qu41)vX4;9o0eEh@6bHs_[+s6x93kxM-C),##l:op$S+J-#[KJW7dTk9%o_96&Q:e8%U$ED%xKt:gkVIp0&d<I3qe0o0g_k[$K[%eMYMM,Dj[i/:7l%jlEgI7n"
"+6-#,*9ZXO;g>D5(DrSBTObZ#'Y68%`<gE?gKDDP2qT7Z:Q=)<,.VCgaX5lT*WCHO9v_xSZ,K%Yc=ZY#6mmu5b59/_FsZ`*BS9Q(A$Tm&[mgq%O@hf#@_NT%t,A,M/N,@5.u=g4(I1N("
"4`0H2`cID*B@w8%Gun$$N5(&=(U>xOfU4xt^=<.F%t@A,qxY`<Woc(+H_ATt[g,Nu>c&g1P0[;*#?MF,HnPS814LfL#aQ##n7DK$>xhH-g?iH-M'iH-aTiH-J'iH-J3I*.0Y=GM%r9^#"
"j1Olfr-B[]mutS#-*+6&IrS?5('.8'YK=)<Rof.h*2u-$$R+4MGd1b,WN7g;91[a.De68%cxb@4V9F&#pZeC+UnR<$V;rk']GR-)8qg<LiQPN''8n.%ev*l(,x,@5^xjj$DW_WH4NK/)"
"/xET%;Ss`#GCnXuC9qhuX1wi'0=</fccF@FvNV%,m;FJ:Trc(+G_ATta/ZNu@4#d2x[2iTWl5p+l/I$.M*)p8&mwQWR()##)]?)<Uuou,RLm/*-;2g)Y(1?.b>HN'%o2..)3^'P7&c<%"
"+BmqVQ3GT%.tsL#9T4%DtXXOd.:>$ag3vkaxmo#,,2eC#7tvZ,5:Betdv,ru&crS%X*IY,VR]x48Ax0,UNvuHX'[q9A&sx+SsIJhRXYY,k3.5/C31^#k@/5'#II=-iw``%Bqvq7A8u#^"
"<PYR(d:SC#YBk]#tJ_#$6YTN(hh.[#dUg/)G,3D#*XkA#1?`.CdjS^.$nOL,Jp-#fB*hD,%<Yl+D&/b@@U18[t?kr?`MkguB2U2W9u6J6#e?72%-`c#m^i-EwLPF#*bt$BUO^1p_)Oo@"
"p<F_05G:;$s^>+#903/M_e(^#4d[.MwLSA,9ic*8U#.m0,#R2TkDYcMno(C$rbX'C(-OPKRXW(4g.t#;;ICrIsCG2:b@+c`rTPIq(h<)<+ps&'B@-cuVd<W8?>6Yu&QSX#,N;au5b>f="
"Snk?#8OTjumtUE%Y*]M^m9.5/m&c&-8PHs.XD?8/haW>-?qKb+&Bl]#nIFT%nFB+*%)O_$xF.r8nbi2MK1uP8UqAa>pcxF4C@37%62%&4Qw@8%k-#[Jw$L#Q-s<88gF&N@o>GfM/e0`4"
"numxeNQoluMJSPun_va$i.OT@_B@s6p'U$$$8JYYp[r'@>Yj4fNf%Au8o+Qu&NsbL`@U5MhJ95#B/[O$5B.2.Cf_(O/iQ3')_%a+)d-v$xT$J8ZK0j(5eop%,FnRbSgsJ:a%uaHq'bp`"
";CrF=gIdE>RO3p`L4A*+cCOw`^w8ucqK5.q)k<)<+s/B';8ie$ZIXj;R:s1'>G0s-sU&`aLcRe<Twa=$YGsD#,OTjuV+,##RIuB$ZS0:#P,rY7OsX>-_O-U78`].2(Hhj0Y8###L9H-?"
"glg<&KWV%%dZa>3<9I12m5%2.*kRlLE`cf*-DBZ>e;d;%g>8/&]CI8%K7%<-jaJ$&B<Tv-.W8f3fQ1P:`Qfma&$uU%Xr[P/c_C+*([3v#Tbe4(I+fe2,12v`SI0NdmSThStH13:rbEKG"
"1S]K+2ui(6LhPlM+v3DdG=C<s^rvdSeek9Nm5wuAdwv(.v99>Q(/Ar/q0ociY-<d$5^TH+^#wc[dc,r[$EN3G0NoY,5Ko'5WVeL(b=hY$8C`p;@r5_$eJNRNDI&V7fBSj02)V30]a4)+"
"ne,M2o:%##?qw[u=VXi$'I`,#4)MV.M0.4(EkVd4?hj>7#*qJ1OEdH?vp#t$HAP-3Sxl,4G-Or$/Z:;.x?7f3f$'=-0-n:&eJ,;.GbA30G-3t0O)=]A&5*6C#9_6D6eh8:)[UR9O.qU@"
"1&T/PU+:i5k%qAB$TJEPxaxcQhO^2=@=:?7Dd$59?%Kr2I0JZ?D$bx%U.2W$NShCO/_%N+;35jDe`&j>sum.2SXkw$I<_*WXb7W$m$mj;>(=v#fQoA6$uW?:5aE[.s#5V.=PWR:4)uMC"
")p`&7#)>>#CZqr$N9(n#I7C/#%xHjUJ`PJ_xF[w'L8(##M8QlJ2l*vc](NJ(`Olr-3^[s&vv'Y$mg]YGq^XG%16Q)<a@Wl(n=#G4FNl]#e9Ss$Bo29/E'0GY:b)s*[wDE3.g)jcN&cFD"
"Jnc+TIwc='Vt7bj5q:e24-/5uLq$GUw)DH$+cmjRnwU'STrN1pQSaMBHs?D*[0N'S9MOI)QBF9%nx6=JB[j8<1sG,*kE%<%n&,)<l<7rS4P0>#R4Z?%fZ.N9'qc+RGc#$&PHMLE>5>hu"
"g#ntS:p.9<6.4_T_(c*#Q3n0#vAXa$Y)+&#1-=$#hQ=>'OKSYGN@1/#pk,@57Beh2(7w.1&aCD3[YRD*Y$s1)d;MT%:fI+2Xa,]uX8m_sYXq0*qHF9%w:f1(ofmCHe+8h*4cO&+$+hft"
"sgf5u-:L=BrRLI=tM4&+w+JS(:^cm&OL:?$9IM,#jsfZ0W*Ft$u9GZG_:4gLwS-1MJ0QN=F9Hv$JOgn*^JJT%OeN;$1dJ.*3bD[,-@jZuirIiB3sFS/W1(<-2d9T8@<'^%+_as$fbj'u"
"UN#FuSY:'/tbXE#A`5Q3S1b(N6#fAOjJ9>G+i?8%NSUYlDxml&ZQ_kLpgai0xrZ-&%H4W-^]f]GlbvoJdfTp&V?LkL+4RW%J5>##f=#r%$`n8#U?O&#tKJW7uEB60a`JJ3-,$ZG1Jbf,"
"O/bXZ6Sem0(d4l(g6JHH0jpo%GCx0sS1cRK4UPs.*vdf;P3,H3W@C8.n9GT%_YGc4r@#7U<)N'p:6ZL#;Q`T#Y3_eqV?GGR4r;u]WGH3?W64*qsP$V%'B4F+R]Xl/eSpvC&t7G*u@d>@"
"E>D:87:RYmMgAv$cRM>#8tK:v=:;U24KfH3)&,2K=*A9EetQ:vMk4G)h5eMBF&1A=*c68%44US[,H_s-LhQ,vv(bt$k'+&#nQSW7DpF^,oL?##@Tgo2b0Dn';qRG;4*q3(sVwse#G+F3"
"cljh2fcl955k,&4p$Da4JjE.3kgX=$mYVO'8=Rs$17v<-w'A>-WMjm$)Q2T%Plrf:>Y6<Q]8X3MIxOF'q*5o9AX2##[TcuhnYuj`0s@12A@BpVFuLB(hO/r8Ra$o=pSrP3Y',Nb37lGe"
"=`?V+I&Y$6us>ucGkr[bw-8J3?(3X.$7FrR:grV#m[/U/%4O7S+0?aof6YY#'h*PSdh@-m=?:D3,1?u.BF_J)pD,ZG4w6h2j:%<$(A+DNtSQ-3qg#L/d:SC#qF+gLXXkJM>KB@NDYl]#"
"7Dbs@r^kZgkj@,Ma^8K4E.TBDh?GT7+<Nv%qH$%oq44g3%3j)NWdv>3#+@w5*B/s&P$-AJ&&O.3W&uJG/Z.'p-JZL2-VvwPHS]W#infU/%`MXQa*WnVGm.q/a+cudTWs5-fo)9/b@uW&"
"g@8=%M2m+#cpB'#hPRZ7Wmf$&Jlh)$gbP3'(V]6*6CqF*[,_YGcviT98ZvW8;35H;eL#12VF+F3=V/Fk.dE12jQnkL[TQ-3<&;-mN&+^4T*`I3VO=j1q:Rv$S/oF4FH7l1p:gF4s=`8."
"q8,W.:Z0j(.jL)<:4mg)=VWF3EU@lL6U..M6<`-M7GnA-lmXZ-qMIA.f>dH/Xl:Z#IHH^?x4Z+Hki^-)3#+AAk*c%$`_F&-,Eji2Wx4&HpZpnuwG1eOS^QRUC'P-E0$?-?\?:*<NxxYte"
"8o%KNej#@?CE@/EvZilUA)3VOs`Urit+,^:pd'H;i8lf4_LWX.u#[w-GNW7[&?Cc+v[>_+01fN:9J]#8s,HS#bs6P#8]v042AjM1Z.o#Naa1:Q+6So[rDH31rhS2^ZkpV;2=b#1)+Gnu"
"hp4lu55n%86x,E3&5>##8C46%md7%#_pB'#^,rY7RgxZ&J(nA%8M)D+OHjE.U=*9%`D-ZGWaiT92L$U&tGXp7>8Skr;OLp7*huS/s4n#'3N8f33p0s-h?7f3&jE.32DQs-t[etH0D'N("
"IgYb%t]U9.eg'u$mw4T%[d,I6lfhl<7S*p/:Su?6*:'U;4@5dPT2uE4`W+c.tl2NGX](HF6O9_HtSEb=f$W^/b]I<8TibM.;Pl<@-Yo)Ijgp3:J*(IORu3Si`U3o40[+N0B.nu@%aYn("
"HA+.);2v408j`j1x+&N170h>._LIf*#*hBQAGZR#>?8B,eZpRfJr+WDpnFD#OVO,0(R?Z]W]fs+i;ur.f3_(aZ*e0>[Dx'vUVZ$v__f=%kv^^#,->>#`^WL#TK4.#sdP]$nUdi0S0b5/"
"PYqr$Gee#MJoImMC85civv%F.:WYt$X/o(#IX-gu3gFq$e;F&#:V*0(uVmo)ZxefLpd?12F3xpK%fSg%N>i;]BhF?.2fn$$*l0C4a`]bGDW14;gRtr_4M@hkL:#r1v'AsY6BTp_V<:8e"
"MHYJ.g%vYh:%WxOAs_7@ue)kMD,-OXpQPVFjD/+FHS>J_QQ;>,.H@W&pVPZG44k)uT-`d-&3foU?s<P(UhoBJ#J#G4N,Y.X_d#,OOT'#:Yckw8E)ZcKCD>-OJiWB.qp8u-CP:4OQDU1("
"U9@citS&xgL^MrHM8ql3<BCV$JWY##KjAQ]>.)$#f%5C5?_)m.I4e8%I`lS.e-Ma4b_rS%A%#KHAGfS8,jMT#d64ul`/^muig?;-ks0M8N*Js$w2PUX(0;iT7_dkuk>mo78t2P#K[LiX"
"+Sl##nP/M%$DB2#Kqn%#.pm(#gnl+#Imk.#&mTK:gTg$$EmccE#R>&#Q[]n9D-%F4*^B.*Tu29/vBo8%O9[<-@gmmT'%P5AT#d<B@II@%$:pV8=RN(PXc8crT))XB>@@@%$1T;8xCMYN"
"7%ML3[vqfDU<i>$o[`9.j)4qCWiE[0UBsh^&8]GE(ja32FUF5D>Ulx-Jn`4/O:fe>9$[_u9f/OEF^P#-p8IW?-j;G5JFbD#<YC<..d-pCh9Y^,kjhV?^>t-6]<SS%xtW.L-6nig?\?(,)"
"eqho.5Vnx4]D9G;2Z[s&b$Er7/2Ls-Rriv-hTVZ5MCn8%mQ:a#Pg5N'4bCE41sA+*':n5/J@p+MjWvD4x#=H*4_QIMGt@Y-DE;O+@)-g)X5G)4m>jJ3Xw9a#p[u]8Vo-v@3M>+HZ^)*$"
"VuEd;5v5hW0.?ia(/t9^4uJQ&Pib8An?aZ#77,D4--Nm1=CpF2GkMt@xGQ(-aw4%H80#MaMS>g):pU_&G)9I*92JQ1*u/9(P=s'u`-j*%(3xq7F:4)3Y+[K),YX@X-nPW<c]A-j3G#fG"
"5#6hWM=]w:Rn$:2eW;Fub$ON1*urn/WwVG;+lqP'/IFW-?lMxbJ^g>>X9CD*rNIP/b6xA,8Uh:%[jPR&WO;%#fbBxRD.G6fqv*l(Q%[@3mlaL26+3:.*D#G4L%;8.7.5I)HgXH3FF+F3"
"pJc8/#CGT%`$0[#`M77b<FwS%f0@&a71R8%W5,5nWa=HP+NiM^Xs5%AaHNh%Ia-SI(]5s-aego3(FVM^qB'^Q3FsrZ4]V=cbOsaPI9ke=#Rgu:%,*$Zh*)dDV)`c)vLoKYjTV1Lovl/("
"Ltws$W&mY#R)GZGJnI&MP/<M-X;JrNfrGf%O#iKc+/p=uJfvO$q.to7w*DLus.d:<NTDNBg>2A&f%M3X&K+L#8#it0aJi%uLM#FugATtBt[_/u[Ci4%gb[rDIx^1EBkZ3XuU$##5f^S1"
"iokV7_#;g(8On<$J'][#mX'U%@lSC>CB,8f&2;g)Erql/MDJ)*rLuV-Lcwi$(_CT%_gAVQ$[]@bVV3##1(ssuYJJ[SCq=B]L5r2?<BJa^MX,4aWsgS$-el5uqVCv>%h9^#<KCwK01(H)"
"Tf8>,</=T'Wh%@#N>wH)ZvmD*V0@*./g*xHfjb?K9rKb*;,jv>;USj000<`uT#(QTxI1adrgRkTLv,ruIOa:IT=)H)MobXus@>RDQd$sQ+cA@k(fVs-mYB%MUb<NLD/2/($sQ]4Daw`*"
"$V^F*g8@d)em#7&:LW5&fxI5'VbnT%jl'b=_9kM(CvfDNpVeh2Enn8%oY)w$+O1N(C?b>-8tC.3DQaa4Y2.T%,0%?$;vY1Fj7,(G8f9T#1sw@#.(kHV^`05B+o7(5G-FO$U>qM7,V%b'"
"X.gm$q'DP&]_j-?RLK1=4w^Y#9A3D*g=bMBs>u(3faFa,)+ui(XI.t$vHlZG(@uN1MqnP/CXM`+swOx>9D'N(e>ch#&=Th#q6pI%@Wc8/VEEd*@n>9.VlAL(4Bn#$XCn8%7(mDNYa$g#"
"ob`E@][*T%en%s%xSQ-?3@,j<J8_d-VnktKqiGlL8Y?0)d1&?0l)QL11+KEF;C1$%'[-+=5x>ru=@,-:kp&c&XW[s=>`[JMFX$9%@Tb93b'1PdRq[f1C8A#,Aj52'9&=T'>CN5&9xLZ#"
"Ww``%&u/B=PX?T.616g)4o0N(k;oO(b;>rH$Vj4A#L[g$[;<P/WPjh#2+W]OsAi=/3>uu#laVbPea#ZGcoS3%J'Fs-^UTfL-G7f4?J_o%$v%YY$_[K$7.uUdCC?C#uBVxk65C&c,r-W$"
"K;F:/<5Xp'0(35&s$QG*QHSYGth)i-b6#l=uBo8%x.Kh>?/'J35Hrc)pX-M&g:/T%rG+42+sk5$p7Iu0iH4vqeRR@RCrCQMfY,w9;>m52;vZYG%jC+Vp/u4(6>d+YY[dqsM:S(+KDwU7"
"b%=VU&cpERQ*BP&]]qdmGo18.YXi`+M1Rs$Z&Hv$2tGZGOsgE<L9dF<jOr^o`B40CC35d3OOvB],A?;H?iV,<bDVdL4)%X:vgn<IpAxLY_PgB->Mp*YPJ:T%*ktRcx3v%,$MHGa/vdi%"
"A=DmL#itA#G^'SI%oUM^awdl/@BVK(Nhi?#Xc<.)Z*&&&7.I8%[/`D-*&fTe'9%%H?q%t%7ax9.0+TfLmt:8.KgJC%,89/h`B3J_7,8Ju84B.hJINDuO2nY,a]^Y$i-<DuRi-S#=,-Vu"
"?`lS.0gURu^>T;-JB[*.DZwqL/cW'/Nm>R&E<6XLo_HG$:9-XLr7Ms-DYPDU$Nqxtm-08@8q8l*@oRR/kJgXl*X8,23K1e(L7Q:vWOa%4Z@=8eR)8S[*c68%UFq%F#)P:vGu`%46[cMB"
"X/dofH#1j(<EED#_O.SI<eVM^RxOV-4R#3'?<PN'QRHG$$1WM-OELX%1gw51bYoO(a*$VuE<a,qEOjFM/+CAFo_4V6PEt[bACWrmwU.@'ksq7#$%8lu)GWH$.GMQ2@a^w#>.rV$VaxYG"
"355##<>u@&ssDM2sk[/N;b*g46OBD3^ALCC%Uw1e$j[qu.A`hb$5w.L'_r@$[[Vs#%[Dsuh]g:#?lMxb(AP##3BI#$]_P=.170Q&?uUZ#/xZS%XQSYGx2uNbm?+B5wgIO(Tl&<.6U;8."
"BnnM(7PX^#R)Vg#sf#GG]H=5&oN-Y,8$;AujG+_.(I:;$5WU=NwdHN';:7w#5FaP&WsxYGUop(%hSX'O31x9.K;D0c6DO^#h^IPA0OBGs4_,-0-S0cDkG.GuThq-4Q@6##L#jE-5J.2."
"i5E-Mkgm##@52P-i9ie$Zr2]-d'7u3r&h^#FO3cM/+CAFKu:K$6=pot4:`L.2OJM',CFW--Y.%#I^fW7iaAYG>(iv#KH53']qSJ(Tqe<$M:1/#:wupCDveO([Eho0E&jc)6otM(G4BG0"
"9,B+4PDelAAfR#RMIpG0#.*H)wgT+D>:5Cf]Vc%^;FU`.2PUV$9AI^25_;V7A%M?#@qjT%NI-,$W3F,%Px5t%Z,QJ(XRG)4@FH[$1hcj#tBnFMPpkm#+YK%%J/3edRR]PuSaB%M8wRiL"
"O%j;$KA3L>UoC;$#sAv$%AY8&c@H,3K)^&OOw^>$2JHC#jHP`*7q@l#j%9Q$YFmVup7_BuGEUYV$nYD*9AC&cD,=At;(wn'<5Xp':(@s$UKSYG455##Y);R([jcw$.Xp-$WNAX-@,5L#"
"m3XRg@p9/f;DUMe$lk%O$Pa4A&+J+V-3&:vkP?e?t_=fL5I_E[-)Bv$ARH)uWs^D6Z^cRu1^:p#4JAnu&/nK#'6'>$W;aD#8acFuXc%^.C[qr$DKaJ%i6,n&51.W$M3AA&I,_S%7='`="
":k:?[3_uM(4Ut1gGsN1^NFPhME>bSt#hJN-q/2].)RUV$EQff.8kMV7*qpC->6Gx7KZeZp>XB%>0PI4=jGV6WpWt]uYOiRea$aJVMHmHZ65C&cn_5Q8W7?80BS9Q(/EYA#$#D9/:uY`."
"0Tmo)>b:I3KtqJFZjdh2$B'*Y/[[q$AS7T%aV1&3P`9cn^@.%RBfYTLgY,w99J;Q2I=&>DWmN;e=MC&MQ,/_%v,bUsZ^s?X_oVO^gUCsJ#(#$^5IPT%vERv#2flERK1PuY>,^M^5uF<."
"Flp2)IH.iL-=l>-2Jbf,*H68)3%9k$6>V,*^#&N%`B:7J,@Z;%tl)9.07BB-Bt1.HqIwaG0xX(7cA>qWcFpXdO(kP$D*QZ$lRJ7eWNK6/oqx&$NcXoL1:h^#L[,VZ77#peYGp(<O=Qj)"
"DDqi)1B^YGq,Xh)Ce>C$Gnw8%U(<O:r@5-3p/R-3qk@92afkGMu<Ti)#3g8.u;Tv-0Zo2BUa[0<F,3XI9hD<%rc6<..Px+M`%qV*$91,`9W6a,`cMQ9mDbC5-&Z$:g4q2VPn]fO`QYh8"
"m,`(,wwtOWWvWxf[oHk([P'b4%qDlD#lr+0q:*U8.2`Q9pgS23/Qo6jgNsK(CBqf%CtlL2tX)LbuY/@qN],X*L]3-)p<q<ObD/e$x5)7b9>:-JUQN'`QAHl2Nl[tAYgfR0X,)##=ct*v"
"sm>R$hpm(#)kxW7K67c*/79L(EU^027k3v-5Dh+cXs;I36jCD&mqlL<XPxj%LI2jBB0Hv$C_+w$4XtT.1dtr-1&;)&6pc^#)p8a--3It/[PAgN65^t%(4((QcHCo<-+%%5c=e`,I$KN:"
"(>4hDhxCoH::gQ#Rsw`,%?3j;2[_MB;C4I%LLCMIBFuv#Q(s`IRwb3V'Jn:4Jk<T%NJSj0Xf?:RqEdWhxe[+DWA-9BH%1<RZ'Tm2#)>>#,-s@$oe<p#p^''#-i_v#g-U'#UA8*FTTu%X"
"]YB5/[>RR*RT2uLVm'B#ZA(SI@tVM^Ej?D*%D.<$KESYG6xdS%:p:9fpmeO(RL879`M$9.Hf*F3^AG@h[5+j91egou*:5f$4W5ptA`h7#OU`i0i:r%b9W?j0%VE5&[pxYG=b&6&@1M,3"
"BgaNM(@2N(XRG)4*rZ1TcW+ou^lw0WJGlXlSt5;ZmlRq$CS%h$7OY##PV;e.:kN=$P`GP25LvU7IOeS%?_SM'JCLJ#TD<8:KbtA#%M;4^_qZp.fAMouH<Vu+xH53'%Gt'i2%Oc)d'&a<"
"oI;bY(jMu;&Dtn[Y*P'A0EJqDj.`$#P`IR&AhAm&:C7[#C39U%R#;Z#VqD,3lIVs-UrZOCKhtA#1INh#Om1T%L]sRerL+A+ut'J#4PL]+$H2oeT6U#2Eob19b9G+M:7:cMV;>%2D-sV7"
"FXjp%PT9U%_7pb*P^lp.+D5C5Jk+.+?0&,;1,0Z-,4`[g-k7p.=$)#,$6Qoux&+]uKNR7-x^cj'QnHR$e`oDT$&Z]=cM#FuUD%h6Vg[c#_JRR6NtDd$N'G6#='jV7IhJm&=IaP&VN=>'"
",chV$/Qjw<WocG*F-9g:@ZV6jl/,=$s)sFM'%YU#(:g%m<^dlA97WD#+a.SIsQ%&FZ]A`sTaP:dG_Go7pTfJsnPG]usS?G$5Jq[/B:7<$@O[s$>.Z8&X%C$$xI9xk:%YV#3;>^_)UEo]"
"$5w.L$d#A='b<4^`ck]uo]g:#dW9H*t&wigoJMB,(M.<$fRxfLdg$68:/W&d_&eh2hbtM(ZH7g)2?tM(]INJ1,D.(Wk?bD3hX6J_h]-Q_Z8Dj$doAP8e1-Z$UeNf/K]Rn&G'kp%4r5[$"
"Qtdc$F&'##+A(kLD*E.3D:.)AdvJxtP2K6$q1fl#nRJnu(AN-$hs`3urtV4u.foCMB'@jusY(xLiiJ%#Gpu-*Z6t5&X>rO'FA^>#?FAB-Lx:a%O]j;.w^qU@qKN@$dUVChf.6^f*O,NO"
"#O0d$:DPk-x<uU@k@1'#a8Tiu0?xfL-vJ%#I4K-):cBv$9P0EPH.A7MlQqkLh?LL4Z.DRcT,X,Zv`#=B06=D$l_cFuF[=n*ux6Q/PN=B@o@r@k'bO_&XB:D3X1-G*#Gte)W[rv#@_NT%"
">(;?#mdSYGq/^&Z>.PG-&MXR-blJGM#7-a$XCn8%qhXv-I1fIE5rJe$QS>c46?lKfg.dwIUbVa7$Q#)JpU[mLGVRb#o%pT&o%mf1DR?]BL'j[kkbPV?VsAQ/+?t?M>:$##6W[QsRwx+2"
"Rc9f)$Mte)Ghjp%K-^m&]`*2(AL@D<&p0lOmMCg)GQR7A9(dG*oU5H2&QV+FIblg(_R-kdAq6,6tQ>)<^'<]k-+;Esl&n4][nGjuOu:d#1*(lo/D2B#4;Bc.CRUV$Zkie$?;@e%nQTU%"
".'to7A[cH3tDeh2[b'C4?jT49@)<Q/$vRAccx37Hp;R/JSa4OHTp+QuCEKea8#(n#_0Jv0:lJ*#$4YuYoA@N0Ele[#u<#+#%/5##@B46%c^n8#)Q?(#daN[,dhk/2vnRs$HTgq%;Q7PJ"
":Y)QLo`f3/+ZWe)x6qfF@;Ti)r[t;H>Q><.*W8f3u3Cx$iL0+*vNU&$mE/[#Pn]:/[k,V#0q)+ijxTBV[25K6Dd;;jVeB%($o#W)CvUj'sBDj(N#TrP.cU]8;eUO`W.Wmu]5jPVu=%j1"
"Qs7E4#^n6g833L;[[]#.p]tvqg3h/7L1xIl2MWb060ST$0v7-#R5g0M@JAA,kk=n0_n,'#eGPZGn:p/FPWZ-&_v9q`H-ihLjM0D&Gq[P/$H9&OrkRD*5=W9`*P%$$xK6WF60VVZ2rML?"
"biDDIu=N%%PqEQ(6K13'bC5.)t0?BJBN$p.vTs_OcXkV/<A;<.6Wu@M(rKX/%A>@5BxXOU7sg^TD.5r;o93T0a*.aExr,/1,6+20eQ'Z--9<.3HYY##h;ig(c,Tt78E#se3`=gj8f*F3"
"lM>^HoV2K(N>29/.BvT/<(1icaXZ@'`]:9.'`ohmao6Q/e+7)_l,chAd7Ej0#rHYu?\?Aq)Y,0J`UhkU%r]kr-P`0>u[c,20'wel/9h,n&NX0O1#OHG*>:IP/wl_:%HAP-3A0kteGp*Z$"
"^fUX-`1KvRZRbR-U.i*%E/cKJGGZ&HAmn*NKuMvR*]j^Q+^nx7-O)<%)dsKSCf[T%(ofAD3d[jG6?]T/ghC#$QXuP8<HS/Oq)>>#>.>>#s:(n#U,_'##)>>#,Yqr$DWEh#'4.#Mr9EZ#"
"5^ZC#J_/Q]Bj68@E:/F.15=a*dJHSfI*;'#B%###W(sR&+i=H2AR_V$o'G6#c^fW7ZCWT/M42e$QJDt.^Y-K;sov9K;oVu$Y0`.3VvCI;Bh`#5)0xF4MI#2p+DA**dF49Z3q.gdgA,F7"
"0v0+>8KfI<iig_7]h9/f7tKC#8p+QujO%oUNUdqsT7;]$cRUA]9nHpFs:v>uJVtr.;+tM4S$kNbH%8lu2+df=_w..*-eI],:R))+igg7&_:5j'FBJ,31P@9.ieRs$[:[W-fTqrg(*x9."
"T^1T%/YM;$GK5HK;/kk1HElUK5CUIIC*e>dRBm2nwHkfmTm37u*61uZ_O8`Z5-O5>5hW%=x&+;$9Km%+$WWH:]ITkhUtUV-4gw8'jATYGq;>ZG>.qWJQaM)180[L2Z7F%$8u0N(]IR8%"
"^f*-2RP,G4`cJ_4vJcpmw.]_g/Tri+lns[bcb2W$/b'_I0rJfLGl*Zu9M;S$6le%#>#d9%s;68)Y[hf#^Ga5&HAP-3n9?o*7YJh1*Qx/huFD$g2T@=6uUFj*q$%<60L/[#]<2xkSt5;Z"
"dpsOS0=RT%ca5m8x(C>5RBK0%P;KkL3Z%iLD#G**qL^b*j/68)YFrW$[j1o0o-Ap.AWQJ(Vn[/NDp*P(4o0N(`XW/1'qh9r?Ep*76dM[[MNjY$`R/TtPST:mCd;4^%pclnflwX/bWvl."
"3<[<$xXvl.CkMV7R4/rLeZPgL'VaR%o9)gL+ofk&s?skFpVgYGk2Us:`R^*FOlLK#/#(2eUA>)<HmE*p%Z-V[x&[4cro(`6n-rp$7L_hLCpW$#K]Rn&IKcj'L.1/#Zo%/LRXT:@Lrm-H"
"-jS)N[N']$mx&enlf3R%gns[bLDBHMBoBe$Co[F%pTuN'0r$s$`C,n&p)``*RDYp7j(pG3Sl[:9cu<_$*-].eO`V%iu=)_f=ks8.RQ/xk>Wk;-%m7^.7m)H$ra)n5G3&W7mr[h(<(;?#"
"f^SYGU6wK#/5$H+@<sQ94j&K2sYkJM5RV6&dP$T.49E7cd/OC&9i.SIY-k+VT;,iC^2Y3FY`wILh*#edZP).*Z6t5&T2rO'N`Pv#?u[vP4#TfLUE4jLj^:`.>lSWa]YT%'SF+mutUtA#"
"4;<Z[PWVZ:2+CB#_O.SII@D3ki2,&#Ronn&K43p%]WSYG0Me<(V_no%tUdi0wC+F3h0?#<O9BZ-i]1#$$=C-&V4/T%vH:a#,Y)v#NiXY%2`(Z#_rKB#D+Gf_x/OcU.UKp#8@3p%=Wcon"
"+WM=B2#Vg$3YnD$KZre$?7mD.r_cd,$ibo7k1Hv$pnF%4I9/W7_HkT%dCkI)c0o=$_%s.LD$22%hF+F3&:Us.u-_b*&DAO0J7?m8eTd6aJ,<a*$vvQ/(fvNu093h#r-Xr(An(,#dE>O0"
"(PUV$.UEh#^mk5984s,<Bf65&,$FNNZo/)vV,S5%UlM-%-eKD3qaDe-?>;SIx%+@9XFH@9[M=.#J4T?.(2:',v<DZfk30_-,w.x9vkJ9)qWH'#:]YM^_XB`N.h2#Ms=aJUq8_w'*osw'"
"bt)##b+pE[LI-F[J=qE[K.LB(k,I2:P#_E[wuIfLI$qGNe;[Y#>_O1p1$voeeqho.3^[s&3SNs%$E>ZG[k7t$*,2I;#^5w-l>Z&4twC.3f1pj%SkN^#2+5j1;BRF4qErJ)nqAYcc;5K6"
"eS<mG8VsP]v0kUs=$fLp6T+u%`Er7M^FB#u+-kj$Pv]e*;i4b+?khOoi+H1pbmxQ#EpkmUNSZIL=s`)S,/v^oE]N1pc@s^o?7BSM_<J+/EaA5/>UF-ddLh(Ew1n^oBF%36<O&YDt3x34"
"-1Hr#,l+S0?Ih(Ei<jl&gphgM'GPd$Av*gL^[3r$]VXI#3@B_-RL=HtLm4?-4=B_-?TfkF+Fl+M'hfa+GD'dMA#ZoL$w76M6r7S[GqC#$]M[p8Qj^YGe0EE%%QM1pJ:):;gLd'/Jf95T"
"V8R59H)-H.j@#29NQP;75w>d2owq%,_:mY#vMHr%pj9;(BpC4D?i*F32)w[$fZ-d)3%)j9lB]v5HIZV-`AGYuBAY>#g#o[tDCZ0AR<2RMDDAKbXd<YcTpWM^<Urs%[<4-dFblm^Eh%9K"
"%xt.54<sH]-*eX.dGs.h>tUY,BS9Q('9]],o2TC/k,-v$LcDN9.MBj(:Q%XLA*1ER.DRi$=29f3mikJM]Li($DOj4USkgQs[*]%;s%d.G/Q_p_54Gc`^jA)<'i(S&-O0JQul'w--SD+="
"gp=IP.jUxks*P:v?$*##(m0&kS2So[+i?8%pS`xXBf65&8W;J:F0U#ZgvWSRB$*F.jUQF.n7$##K7hv)x>_-%pVf;%^Og?.fQV+ir:Up&RkbfMxeYl$GNR`kx.>w$6Xp+#bDv)0=q/kM"
"<*S,va1$AMkKu(3u)t;-2K^$.(C7q7TR`)F-@VAFk9B_-K4:vn_Z8%#<P0Q0[xg58]DBX:toNp#Erk.#h5=u&;x&SMNTO;..q@5/=RlA#-nx/.oY4oLCh$51/Xr34%5u^]D,KqDYtpT&"
"#p2s7H.2/Vh&nA#'DRuLuUS7nL'kA5@xh_]xGM1pEF-_]^N:AN)]>.#,L+d*/:J,*)U`W%/#G5/:ie,F6E`hLYu0Q]'T2^QWR6Yue;QZ$ke''#I_=Z-%m4X:tG.uQe6bw9qZbKN@YkR-"
"U@g?.>-uU/])1%YC6mERQ6C3ki6<eQ$]UgMD<4gMJ>,oN*.*G#X@B_-T_[Y:6+n3&=MoQa^s]m]QRe:;6G(##qi^'SHav&$R.x4A>Gb50W3oNkxd$vQ4?`M:_bpA#.&JjLpCi-Mx6YB,"
"[7F,kmEeq$ta^Y$8[u##/3bZ#`,8Z$v_(H,5ikUe7Beh2_I1l%r$f@uPHMCh8Z,ptAi-S#Uj7o9bv^#$[wBLNQ^#ZGcgvKNrEp;/EbK`*hh4S>PxIn3d?/W-?]3edW[Bed<*3JMa63=."
"uGcT.)RUV$6=`P-*vkv-t,m+MtR'HN<DIU0YgwD$R9(n#VKvtLo*av#gcLxLQF-##<&4*$`HvI5O3&W7a)vN'#&j4'n/#ZGQ5r4'JmwJ37J?_-gdn%Fp<G)44<<m$L:=NNL8oO(Dcu>#"
"%]ig*R>>>-u88GT6r4+BPL)B4vMQeu9m%KU*AE+3WB.MgJ`K:Y1*bju(b[1A7D@>#,^h7@BvSM^ekLS.LPKF*UB5j'$S]fLDd'b*ULT6A3$S3OK77#Pt-qx=TMZ9M,r`20wijb3U&p*+"
"%s;b(_fpp4T)i8(8TYm&w_Y3L5jCq.VSWP?V+/eQs1FlA4.@)<+R:<.?HrK(d@6eQc.ZeQ>&+87UqQ4('Ypc.asDM2d7O4'rKV$$lVMK*RAG>-)NP.>q8F#-4d95'TJU<-F88Z$h^%@$"
"EL4;dE8MJ(?Dt5(A#u6'P$J[#c/5ZGI<SYGELaU%>/GK24-lA#1INh#I0fX-YR:hPP>E5KxFDoIk%]H#+D/SIrN=xbnl&suU/DQjHN:rd4H22B$@gY5psOPuAW=Iu_dV4#W][>-f*`21"
"jssKGocTm(61<5&fwKHMCYPgLEx$aNX-kl(0suVQ?xMJ#tlWkmC3pi'pMUR##g+Y#Nj@E#VSwbuRE3L#ZXbI#2%eRTibSfL*',Sn%/5##];cG-N&$a2CL7%#>P[N'<7no%No>$#[2@Y%"
"$]e[-&Q8f3E=rkLkGXq.9O-Z?.K$9.FV^Cj6/aOs-Tuou3Ij)KPC$viQ)7MIrf=+;^lYx4A6;FI:F8Q/.le[#q$T*#]v3ZHADR/L_+x^,r628.]qRA,..RM0t;TYG1QPZGo6:HMp:(o0"
"a9:g1d:SC#>cWF3'K+P(>Sgb*pgoW-xY,eQ4uw*.1ArM'_c`tBOBLQdJ+vK2]m,de20(riMP'(J%TY]HuI@d$RUiTFxVlJ#j-DS]6<PR:,ecqO7x1a7tbfeq34WH>G`S,?;F-DEpdnuR"
"db-5/,Bbi0KN('#Z)WT/h;3E*x>68)m#shLid9b*[)AgLJ9LkL?A1o01JmJ)68UQ';]d8/O;q*%xvq0([]B.*TIS:/;30@#:_?IM>qfg4bu@?$7=MmEEw#MDSc]WKtiQgD4xqstOi0YJ"
"T@S<$Gj6#OW.c=U&G&*%*S%'JUl@L#l#C_CvH*41:a]C>v9OA#Y-=A#G+HO23hFB#A:P@h7tb%`AD#cgd`*87&isLM)iw%4BS9Q()#29/BUZg)WDZr%pijCZfA*22&.-@5N@76%nnn8%"
"pYRD*C+uo7Isaj1Ys6l1f.6m<%0:&7)MvqRTf56P1L.G#R#$D6K.QujVB3nFQc`LDOAqw`:GL%R`xJA:VORfZEX%mVH`Ip+&7PDQjlJB7=O7L#%1KZdumV6V.(iw8Ffb#I@Guu9K&6q8"
"_AjB1`9op$t+J-#gEAW7F#R*0@Ztq.YYDS&kLP[$,-R-3Us4t_L^,;`CS:Q'BG+jLCSc0.#Fg&G5_Y#>ts%v^CRd;9u[Ev#]FxND>I'lCkAQZSvUEv#/lu&R;:6-Iua-T<q4XN$SQ*_A"
"vw^r%mQhW9_U-RKxb0^u;M3^BEki[I8uC;0Vf?5;NtG5ThOH652l)H$FS[8#V'jV7[P&B+lHJ%#WKBU%O^Jc$2^0:fCtc7*ot==?HCj5C4)epShe@kQ);o8cLiZ.Y`HiSPPxRWM_dr3D"
";ho,8fRS=[ff,r[chu[bOo2GV'K2B-sm1p;:rPj;MvHbub<FE$T_''#fN%<%5H.iLQnm3'`WF&#Nf,d$6o8^,W*s'SteBU%+N11Jk@-`@'^+gavB@HO?22IQgO#j=Yl@B?;u;?81ZLEH"
"DF?U7t4NMu10nK#jFu:0TSqo:S$H5TrF$##'8YuuV'#^$^:q'#B22U%$^_h10kR^,Z_W2'j'8j9lO3dt`Wv)4i^+X$?3ot6[2&,D.W['eN>'T`R8v$&x[TF*ZfDc<+i4Q9T;Gf*hmHT%"
"uATM^vnpPE/Ig+`-@Zrb_V;/$3GLWViPgk414p<'F5D3']s%?8GCY>#ofjKYBFK1p*`?)<#VoKYl65N'-p#ZG1gPZGx9K&vUWQ-3v-MkLl=YL'LR^fLtLcd(^/&.C#hCEd_?R&Bkj6.5"
">;vkrcKbO#^WDYKxh9^u=P3^B%H4?SiR$c=lCnC#*#R1pUTWM^ckho.>eF]#t'UU%6pYg)nAmo)h79[%O3Jm8Memv$`r29/Bm?e%xF1gC(6radG)Fs_71c:?k@3>YQaUReH5.HXMF8@["
"1jta=e2L#v*j-O$htm.M1'FA+:w+)3*Q#ZGo9a`%be-,$rUdi0F.W696D9Z-epRq7o5x+44o0N(dAN>,#lI:C=I'lCg;ZvSSuE:mRkIi$Cw1=I0oV$9PGOQB%Bo#SgR-A+U6.)#KhQ,v"
"u,:j$hMb&#VjJ9.@jd)*q@bf,[AR#%jJ1T.pBf[#h/s2/^?7f3,KKA&ePrc)rG.T%J14]--_tr?E2JWYoX1VXeqPqp:ks5;]DAJYv.OOjfB&<<Px>v#&a:>#D9SQ#DJPV#:k6o0fFh@Q"
"'ZkiZ&OJJH?4NfLF2v*v,&RO$+=E/MxQbJMMk,ZGVOo#.fcp(/#0Tv-21`1PSY+gLI^s`#v#L=6aj6Y_4A0LPt8<iIi71?7s0c_]`0nkHXZ1i^Gc#:KOc5LN3&DQ#Z7<U/s&%4CGs9Tk"
"%h,5MLU^wu&UnW$ET@%#;K0[#nAQ7&T<x[#O[gE<KXqwIWA9s79Z$9.?]d8/[TAX-Z$vbuSc^OEN9p[ar7Meu4&Nd]-KW2F1U,G`dR`8.J#%`]YL1m0tfS/`9F#&+Q22e*iZ96&qD#ZG"
"Z@Fwp:eX;D)8B,M/p$/_>B`0FB>2;476`ZAae-`uaY@OV3$8gGL>ni07MVm/XPKA4p*,##[k9l3lQ[8#]w`V7;eum'Vqr$#,m>ZGtkK`$_meO(@W#q.8v19/w8pd><#0i)*17m%QM:vJ"
"_E$T-ng<i,;s@d#]7,2Q+vAr^vHwK?@M;w`iQFF^:<]J6Vun:`&-?G^E7axIN:jM.X^i548i?4<KIR@Lq2k6*MAEe/H_.CL>tHu$*hvxB>WO)v$0M(vu/:N$9Wu>#Evq+;_vgYZo&=w^"
"o#=T'$ZTYG+T>ZG*N'[0xu*87pZa>3o$4g:.RkM(gv:o/7.5I)2F>>Y`&gC%viar^T?Wq4b#`A&u#1Y-<kr6L,3UD0:/OE$4^>+#'W/eZ7^#ZG6Cu,M#*p5'fs4)9Ele0YE`F,0pV<F3"
"O^&I*ZLZP3>7_84A4*t-@]s%+_n/X-BbAeOMCQ:vZq(YP)LWj&DEop$'&>uu)KicuV0(XoX'88%r-$sZ:O('#Sq7j0HpCfO[1#a$%kc+#MtF%>vc88S5<vp8+i?8%,G]YY^Dk8&<QtFP"
"W6BZNIupiB:?Ww9naL3$FFm>#?$6,#;O[LMhCL1pqf2jKJ#@_/Zcl9;#/Lg2OSViKj`9U)lGnR<G(BMp8B?#-:h4H*P+*8n.jQ2CGLRR<YN^S<tm0hNNg(F$]d^C-?t^C-oJm-.<,b$O"
"Ng(F$ic^C-[0_C--%?%.]KRBN;);xP+ux7nk[`R<MQPJUH(H/2v)R]uCrdT-Ni#`-CC=p^[?A8%M0j@kBf65&(=R&Q1**YP/;a>-s;R&#n>Qp/^->>#u;ST$o[V&.X'Su7Hb)W%47a.."
"g,OCN=*VXQ`2OAb29Q/DS.XkXO7>lXKPUV$J*?%..0n$XVxCF$)J_C-oJ6L-CU$`-X^5:V_.#vY9BC5B2JJ,EVA.<V8bvaN?ZK[St4*YP66M)NnUIP/rd@uu@PC8%a5@v>`+u]uKx7p$"
"Rm^d%B;k7[x1q;-W5ji-KPXl`@fC%MAR$-JM#r1pL?*##;Xm%F0IrP/Oe:_8A01R32Gk>$]AX7/@@uu#:K5)M[l'0MIa;;?L/O9ikiO^(COAr%NrN1p@`L5S[QmA#U,oJMxd#98_;kw9"
"f$*x9IeY+#(L4.#'AuvP%7lonq`fv$CAk/#iU?F#wihh$hA&?$gAAZNGupiBJkqKP/Ev)03esjM(B0,.^F;6NjF5gLITS#MA3_m]H+.IQ&P;&5V@X:;o2_`a&oH6'K`d`*GNFVHKS(<-"
">k>=OP`uE$(M>5&q2#`/ergA>CYvwTlgFG`2bF)+va)XU4+HDs&_pMCUwRR<u_?>##9Z@.8+1KC.&3jCcHSR<54SS<1pi,).&3jCW'SR<F_8S<-<,/$4//rMf=IP/9c@j0w0`f:Fa[cD"
"[sp,/tR%q.0IrP/x?]k=CQWjL>]uD8h,KVeB&m6/`6YY#TH5)Mt.L0Mo`;;?;23kXbvv,&mj=<-[=Or.i(Zj#nvaw9ge;5KoiFs%PAS/&6?/YP*;YM^qSW'#t289$b'>uuCr4wL>;$f$"
"xZ`=-S,:1M[p?TM1'8gu-hjl$UZ`=-ik%Y-ls3^dkE%G$?Y`=-3l%Y-@5NR*$KA,MLnmt.0a:;?5vDkX1VC:[MP7kXVI^-6D$i>$Eq)M-0GPo$S4S$vEvjw9hG6FISl_v#6igsLN2_m]"
"Jg+MaGO&-%1%%:M5RpucCmq'/*xi<-uQ9C-*7it/Bq%Z#a]G>#'Y;^%29ej`k(?H.W%8;d'BN,Mtp?u%m^.W%^DH-.6]TsL]Y^C-Mt^C-/]^C-b@$L-2,mTNI'_C-+i^C-,i^C-L6$`-"
"k[`R<,WcR<RPsR</Ncf$W./rMxtIP/PU:#P+i?8%0&9DEBf65&3F;dO56*YPg`o;-Ufaw0K+j#N&K'M^Pbgw0wLRadrSsq-.xe-?tpb4M5dYrHDB=R<>XgJjL'R29s?@5/#.DF.b`4WM"
"js<@?B#fJjvTK$.>lqXMW#JRT%u0*#W-kB-KOkB-)4SRM6t;;?cp=A=h4bv2]_:8S`eG^#2G/_MJbt?Qf>rLN5qt%=06a-QWhrx9fvq]N5qt%=3;gw9QIMx90W(,%BcQeMML88%L1#aN"
"#)P:vse,YPihp(s)i?R*M4j^M>go#M;r3[$Wf''#Whj3MM5;PMV[3kubI.D$+Y`=-9l%Y-;&NR*>NQ1pnK2gL&x@8%_Qw`='KU)+K'ls%,eP1pX/.gLatJfL&X=Z-0kGkX^0*n8N&Ow9"
"MRbI%BOeuLk.A8%c;3#,5u'T//_g?.n#8W-@c1#?Rnr;-e50_-A%w#:V(*8I5jR8/k&%T8I#bw9D1%#:Cs+.QOZkV@pi<s.UOJ9iYS^-QOa[D$G-kB-COkB-iOg?.>$D>,3Q+g2vD/Z$"
"9Xk3.k`Xf:kwxc3p)w]ubJF.%5s8k2qTp1p0<&##p.IcV(UB;d5F,.6nolj`RF$YuTT%h$Wf''#Mg3vL,Qkl]TT`^#_q6p.DHb`dl<K$.g90ZFt9Z;%Hb(:2$>4gLHo=ZMIsb9%r.,HM"
"]@,gL<x@S@_aOg$,g@wpr4>a$7@Vt-Kb9gLcQ*nLbYqSM21DxPcQZaMDuv>#]1%##E<+FONrYeuTjSCM[S<ig35f&#.e(*MBOx>-qN3r$d-[Y#^^[ci$<6jKQVsx+BS9Q(:h11L7N>p%"
"qU40L;.)?#b2]t@Um5Z*n7fX+=1FM2IQl8/m*wCB]x9L3,koR/O50J3KCn8%DpjuF,+D+4UI.D.Wq[ofSiE@.N%OA-W'wL4=5>O.06AC>3q-?\?<mp9A?1%]2dqd[5cP$##Hh2H$or?#O"
"YGc**?Jx.)(]<5&Vci.L;G####m>F+[=b9Ms0aF+)o[9MgK:a#$cCH-l&4J-il;d%xiK.*c$W$uCjfk;*XF/;7^Ql3e-?+F0Fp+Fe/X>6@sL[*.QFT1._mY/tfY%0a-ae5[#^23GaTb%"
"d)OE$GO@-%-=r$#dv4X7K>Lv-<U-0)HJ`v%k^sH$WW,n&6b:I3uF+F3T,(T5V9rUe8si(G@g3^,LffnN.bf?Ta2B.*k]<a*JLJq.AP6/;K@K^-YUn.Uh^Be4#W397+?k-*:$G01]/x9A"
"Q$Fr79/@ct+=xx6V@`E%NN]A,5vwD$%mSxt_G0qu_ELhL.qV'#ouHk0Oq)H*KbZG*v',0Ljt'-M$6p1(3A+>3cY+qe<3@L23H7SW>WGFY]Y3B.vFq8.JjE.3SwC.3A[AS%Z4'].gA[14"
"ho2iDZ]7(-``$H2wRJN:AY350sCQ?&Qn>0;@V350TZ@Gr(CQY/4.;l24axG*(<4T1>?'QChg=B,f(esKaXbf2-?E-Z+nH-ZOZUAbH#[`*6a@W&tIXI)X-X9%3BffL'[kUe?ORpJk.YUe"
"7uvwC(#pG3^]xNOOraO'/@Y)4#sDd%%3ZgUEvjd%^q'F?b#><-E;3[.^]eD,9+wZT'2BD6D]`#*HE,*,^u`a6gYia+=KrO':Z&COjS-1MgqhPQMIw,*3$F:.ZI(*47(Gc-bBCkFnYl@&"
"#=$#6pg'N:%G.1GZqJ=f^7MI*HSW[.n3Uo-?9XKl<Ppu,E2kI)%DB+*,Y6hG$p)F+g4oO(p9ae*?n:D5eYJ#$0m5@Pr'qp?Z_:x$I9bc4iGHo/K:V*ODemf($u@A6sQ[?&F-WD+,B.D$"
"`cGD/]G0quY-(hL&XOjLdCO6/QjjE.&(Sv72HGdj+sY@3paj>3BYq<-Lwle))hTjTI5TE&_nk*?#Dd#P=g^(6-f*T%^hH3Bgv;;0beS),3kfW$V-nfQ[5U+42[]W7X,78/ZO2rAUh=B,"
"1l?>#(m_%4LS^Y#Btjl&QD96&4b:I3<Eeh2g[NT/FS'f)+p2T%TeC+*H.TFk%ZXb71q*C.nS*F3/$s6pioNa#0R4>Av/W2K8JTkLZ-g[u$aV4#w+D=/Bni##AKQ68qVjJ2CqDhFx=^+4"
"9EsI3TeRs$Lv%L*WMP>-T`,4g*(N<-F6<Y$L7Q:vNsLc)@tVM^-2R]4*c68%Xx&/C#)P:vZAMc)A;T/:4>1#?61P&#ws@u-tYS1;8w'B#&3gKMunYeu7`i'6LgX`<UrZ:2rTmb:`.LB#"
"b&dN#m>4gL>OEmLZp0Q])kg`&f8/Z$J;gK%AF,+#`.>>#Jdq9;Gj^w0MXn8.6oOfCU*3a*RvPm/cZ]%#q/PZGBQFj&ET'<-.j'W$v7[8/?;Rv$wF#G4M3lj$q)gS%e,X_JME?`f/4JT$"
"+HJkW%$DP#j/ZV#K=[YJI[:9L8AMmu`EW/lGQ+v`AE1G/Lb3&@F%#4#BO#V@JuB^#^;LPM:_Huu?;?Z$nf0'#7N_5M(<#'$B#J1v-@G3^i*hR'3'u2v'A8,#bOS(#-^[D$6+M,%_`($#"
"GkMV7bs^Q&_o7L(Y<wK#/Xu-3o(B.3uF+F3uJH3bj4MjB[faO'Yem5/>;gF4_R(f)k@C8.gW+]#?.Io';;m2;j>TJGP;Y$66-EH*((8Ake>C%:637OE=;w5J4pK32Pe)H*7O9]$%/5##"
"?(ga#V0O9#EXI%#H9A$#+_U8I%$2@mwco_+X[@RVSA6uV_3AS=^nvL<Tt`P8bBa-pbZfEq8^k<VCWXwU(mE,5C.pa#W*F9#n,_'#Q?*w,X&Dk'5$'E%+A=?/P[VC5<=71MFCCXocM]F4"
"H5Fv$-i)T.UA<,)WZ`$%_4q-4k?(PocZrF*xRAuK1m+?,E+jDRhphvAFDnmBm+V+=StNF5tOL%%4k'dMh]QIOs)IAQ/%^:;j8BXCn(.cuHh>Z.Dw7>@HGRoL3RFjLPO)/%l)+&#T_;V7"
"'$bU.28]1(Lps(O$SR*NjYma$prLW-MQ-UV<;gF4-4x=f.gYl3#;fo@Dd+v/.'ge-.(I:7t`705_5k.D4&(jYP:a2<'AqR1sbUJhi>TXC%+^]%pZkm&+0fu/V^[qMQmmc>/f`W7cvbJ4"
"Zu(YlrW`l]D_D5/T`'##_RbcMID'##*6%<&-AP>#VcG>#2Xci0]DNfL52FrPJTF-vu<l?-mGl?-p.l?-jU2&.U4i2Ow_rAP%'`pP')5kOF2v%Ph(EfUBMwfL_laYQ$RU&#WPm`-6SYF["
"0tbJMa)Gx0$1F#13l#.?T_X.?86L$1S[X.?m+Vw0rQ7K3eoH(#-TRD$.QrO%usC$#bWt&#eEAW72b)D+S739/d48/LKPNI)>b:I3[o>F+2`E.3Us@hLsQ&f$'G2T.h17W$d(ra%K2PDO"
"2s$&4F]l2VpesD#M`lB'g3:F?jd-qI7gsO]AWio0pBYD#nS:D5Bf_`5o_[111I86,0`SZ$JfJW:XhdvI$;@f3I&TF4q1[F%Q(O#+D8jW/@m$:9n$r94gB@C+RYII25qP319PS/LAhfM'"
"49g6aHR.*52f0re;&WC:SnkA#ddJ(%PMwY-3.U#$5.p&v.u$AkLh?5O'J,VHXGoP08HrA#1DGO-o=G`1J<XN1g#Cj:gch`j(C-Ob<J.rO@bg81$s5l/5'+d*HJ3@.fsbL7^mNp#B=Z3#"
"5(V$#fd0'#JoN0(+XNQ/LPuY#7LW5&Bch2(,oG)?CA#J*CAQ/)_IwP/G2N`#Qgf/HiBG[6FG<[.M>f60JM>&Sh&s%T07aU.<w[c*D8jW/h+UK-(@F?18]+2(C<P3'h=B+*2Cf^0ZI(*4"
"8ZRD*5K=f*u[O68PL@bOFE5*,@9F31[F-g#-Z,68pR&x$3cvm:sf169rOihh8)gOo^Mb<6$),##h2Z7$qm+6#8kMV7H0,n&G?,3'`Aw8%lu>d.NVkO(*JVe$1uq8.L&:.$xgYon*6rN_"
"`)Dn^t8a_$i:BxC1d?cXQ)>>#IPUV$<H=&#=1g*#*Mc##:H[8#6k%Y-p(+S*1p*S*%K*S*3swR*EZAcN]xTgM_h>%.=,(TNPk9Q&DdWG<v+I9iZZmLps0>8euQ.5/P;aW%gVf>,[U(D+"
"MT9q%[C55rI[,@5)G+F3re>l$s67DEN<PvG+87<.Ujgf1q:Rv$sZw9.*D#G4jxa(fYu'lo>3T1F5UUU_-K'4?l-6J[(>VH*V5>@ITM3)LE^*p>Z(=%krwC^LZn<D.`Z)L2@SUTM&V[f3"
":;RJXERR)$F<B$OdeN4oeB<m/q845/<;t5(re,(+&0<?$cHSYGAOoF/+9AJ3#dge*OCIgL)wmjuCo29/?+DV't^vZ<S/+)bn=Fquj*nX-pQij(4,%j:eCM1=Miax45;'to%/:M)7mcP9"
"ElI(GH_K^#J4xHu=g[(-0RBb&.MI,I)s1xb%PXS7G&NQ&7m.%#<J6`aM(`c`HSIG)h0IP/=8Xp'_%te))Apu,n:,?,B.1/#7l^>IDMBj(m.gKY;m_>$XeRs$%a.K<(@uM(Jv.:hq78L("
"0U44W7U_gaI@,#,;Mf>ISbjX:5[1kuHN>8u`14lYr2`?Yu,OdIO&.Z`eD$s'p.<+P67+IAu.a>5>SQPEc@C:vCk@.F[)P:v[SCwT(*TM^%?#,2+i?8%Ex*AX.1NP&G^I%#%-GY>_E<.N"
"&rgMBtf'g2Fn1^#,kOZ-NaO(/#L^%4'l[T.g?SiT]O*^-Fr9AR<X`=-G.xU.ggvC#4,?D-Pv3;07V:o(:)5N^lKT*@cH#(J'UXk=T@CgLD^d)Me:-##U?&r/?]>+#=&#3^C,#R%$]e[-"
"6>eC#X.x%McFHuunPR@#^4%4(HPsd&MPg##%Vdi0QsGkM6c>u@kZ7/3A`mY-#)u)>jH;iuG=FdV;3g6CrhW+)I$U4MfS9luT/*1%KVu>#3hZw'Z6>)Mq?rF$,)m<-%=`P-))C@/&ne[#"
"qBonL9^Z##*p=edf_u,M=)#E$KLx>-STe^$-w<$M/Uqm]KnM$#DVRc)S[Gn%^m?S@AT#8nH[bc)?Cgr6Yti,+Vix>,E'I)42x/q/8;&T&E$;L1mptu'/Xu-3Qv/lB3hvdd0-d20*W8f3"
"386J*(A:*mlM1u$uv@.*u>bID8=>d3_q*PC6Id;%Ud:x$RgX]F[LC-FNFCG=w=@w-l'IY6[qAU+]P`%4qX,FP#.J>6^P^l13sD2KjTxJ1]9F`*K^a=ceCQ:vo?b%4Lh901-<DMB#)P:v"
"=V`%4*hNMeVIZY#u[;X1RV/D$1M7%#`?8W71,$ZGr-a`%t>Po/#aSY,ngg7&Zp[WexF+F3L3Rm0[:SC#M.7=-xr(6&;f*F3<]WF3LA^+4F*'7Fj2eH5fxQS1*C?)4;OtD#vmRS&SM=+'"
"L].w6;HtN(@dX528`sF+M(BN5F(Gx[,_wSu)`0F@4;1&4O[LJE+.v`EiRPV-l=x%+VOV)+XI)20^RLJ#V?v4E+j.I*j:SC#J.@s$#VB+*q]3Z%P9'Q/h?7f3bfi#$:BCT%7V^:/FK4J*"
"k_#'VwGd9LP2ganiFCDM712&8XFtDd9.q<=DTm3P$SxQLck-s*@Q280gUR=7?CCwTY&ZXg#>sBLo/q6C0c72=UTFj)u&B,Qq)6BBfR&##?u9+v8ABq#rZI%#6TgW$QZBU%A'M?'I&/87"
"[jBA3vMeL27#4I)4#9:'0?x9.]77<.8L_hLKR-jqT:_s-:Z46NZ()+G7W$>l'O6#6hc@%HJr1G#2FK=JH`b2$)1jE-dj7n29R)V7Rte<$AUes$8rj3K-jatCtBf+O1jGc4&j98.fHE:."
",&1Z#'3aL)+4rXl:AXvq4(m?B6v>P9N#pi)>0QrutZ-R9i%aa-Mk9_AV135&W&[K#h*+6pj)Y:.2`<*Mj&$f-1o?K#&a''#aHe$,-=.=-LD4;-33kd$/w$iLganO(PrF12:lZ&&9q@.*"
"QF6Co)NpTrGGgI*ucMD3aiaF3rZ%&O3u7[9M@]oLCf#-=Bhfq((se,=*I1M+>uY3F>oLguee0:<G68euu%Kg1cx/bun:rG@lw^C4_XEu7nM%_SxAw%+<K@L(eP0K-woN;M7Dvo*hV<<."
"JM2;LdF&01K<Jj3Y/5##T`w[#(kn/4ZEX&#KJr6&m`$v$uOB+*Z:8nJt#e<C'GN`MqC/CN(A7q$_x:&&t1-J**J%U%#;9f3HPZ&M8gWk=`lMZ#(woU(?M;I598A>>K5I&@Vh6PY5S$s/"
";p^[u4H8eAc,=-GFGfYKbKi:LNow88GK3/:7d8703dw8'h6s6/*rPv,pF)w#3#LH35KZ@50^C&=70Y3Fmwkj1$@_`$IdZT%L0l]#;p&6O7va02&v^o:moD99r;jDJ78+S0,6lU8saC=8"
"ZAFr&h3[I)J>B,3]S29/v_@79$#Ej*l2s'+XvBV.OY*f2ewU25lJHC#`=o-#BkMV7drNI)^suN'lB70#ecpG.*`tO(oA@$TIX3-cCB.+4oi4c4+`OF3+S4I)kG..,'0uh:*8XvB/;SXf"
"Pob%&G:/3'j_JVHXT,DSEr#25n1hw/CH:;$B&;$#ErQ.1reG?,A'cf(4>//,Xqi;-/>Y4/'$fF4kAPh#%ddC#CG.0W[K]s$T%:8._h.T%HN[L(4in],2p)au_t:P](K:H2GaS%,45NE#"
"TrGc;fTMF*7j05Ci8Lw6.P;Oup*9q%UncG#wQOE=HlX0;9<rc#cA[b4PF^x4&9cu>a5HW-gHF&#?4QU7kCW6J^/H>#m%+87pe0o0?TNI3joEb3TU5%R_/E.3>Q0EYb@kM(q2&d#hl/0+"
"M6:@Qj)NQ=4W$^6bPke)enwk#H6]:(25YX%=]:K(s+lg3Wn-o1[$dwLB/+%vZ,F$M=*;9/0N@W&8*b>-x29E%JF1/#HY#K3ZS'N2_8d8/XkC+*KG>c4_nAr8c?m;%80h'JGt^L(AM[v$"
"<=Rs$_V(</cn&^--%ja+;//=/_,D^@/Ta,3h`IS&VomO0YSes$+'d29@L6]8=S?uAIAFu)r$JBPtcYwds4@40d/kKP/S$##+7Jm&D<,/(JjET%n@5/#UL(kLLgdh2LVp<L*qH)4fc8^H"
"Y8h^#p8=I)WtR(<q(se</Nb]F=>qx,:Kl-RqSf#6wEp2#&+85#rwWrL>/p%#'*oF10[e],hCa6JJa($#]-HD-K:T18<b9-mv'AT7&S'f)Df*F3=ogFAhpJn1[S8t2+wpnLFq2g6/7U1("
"1@D]d0e_OEM$Tm&7qj8%GmQ)*=3>%75wK(u`Pr$u7+V&M:[GVHG5o#$96PQ#m$_*I/TNI3weN#ZOR#Bt8v9W.q5YY#ggt;.WV1k^mcJXU$8$d3&R3Q'R_cQst7Ad).5U#-4a#]uWrU[*"
"6siK(H%%W$x:(pU=(&h*In.F3XvSTAv-p>.u&A>#8#(<-?`CP/uwu(+wkgDN3TeO(P8,8.w=Q1M'XEU-I6A$&tVxx(6;`MWZ$<g6*t`TrF')UC&n5n0?RKs$6=wS%uE;`+9)1t@Q)uK0"
"C>AY'5%.W$]quWhGeW0vjB4i$$jd(#L6eZ,?IEA&w>v$#wUdi03nQ-3V_Ds-puk/M<p*P(<A0+*:wi^%$sLU.#DXI)_CMo%&jD_9j#u[?ck$<ejU*dEC*sC.'o'K2lsA3)b*%#6;-L34"
"4)->-Awe=-7RX.N:?o],<@G:g*v?Ku@3`27;TwL2-OF)IHOOi2*:+i$1Z4^,/;3[eR5s%:L*uZ-E),##;U*T#H&;$#@253^>;iP-M.Yg.<Eeh27$L`0sB+vN46iS7bLPxL;LMS.Kn1,D"
"hL5;-B,*-&N?9^ZVgdh2UAI^$bf6<.L$gN-`d0t'Z_[iTBW#O03=aW1SmuL:uDnT0L5;.E_&,/2xJP]uQh$4?0#,/2$6,9%=qG>YBIIs$l<j(,%tFHXEG`mL05_&vtPvtLPDx(#(,Hr."
"YNXp7r/#B?->']?lsDeHEOEu7O?Qv$F]X4O%Hrc)xPHT%hWW]BC#a/Dr[S8BKk17'IU0k1`ejC6XcuE*UBYV/V]MiD`##/2#Gol'iV:i:O`BX)T)wG&+hR&,&4+9/26$^Cj'kI3LL@s$"
"7gPQ&e7>b[];p*HU'R]RjJ3)-V(KfLIp4)#23_/%H`($#3dPT%4+Is$so[f$l.1Z3tDeh26ORX-DNAX-?GY>#`?AQuEQ/;Z-e&MpTB7P]_YPNuZ#V$#w[s@X?k2&FYsGJ(Ragi'#L[5J"
"&H`3=:?)o0N1<:8Ii5s.?db29@r-GXMrtW@sre]SUqfh*Ccu:$AAP>-kltH*Z1tMikvoS%#O))+%0JZ?^2H3WYQUJ<ov0Y$.RmxLA$/qLUA]qLYJ](#c>Pq.C'W1)UPp*+P>C*Np`Q-3"
"#jaI30DpG0+6xF4/]NT/O+J'Jd>K.*qFM:8&<tY-_3/W-j*-90BVO*$8CS>#C$#j'A@<p%fQZb*<^9*<:,lW:(@4_,Jqe],X^=5&MSe*3;-?/EQBI4(AQ9;*Zwq]5XI9O1]DDo&md`J)"
"T_eX-eb>%5p:'H@1Q#l#JFS`.NptU.V43A-+XKfLp*tcuX(qb$@4i$#.6s298vAK2WDEb3Ki=LYqYl]#L<6C'UOR8%VJCp%WYeL(=pB&=EjHQ3Y^^Q&<4tUILRNR[;eHbF]xu=lkblr-"
"J]8*'jhPY9x$.m0bnrM9O^:?[?A10*t(4I)=Iw8%1Q*a&l4oL(A[Ya=_HX/4`4n>-0gw-)LwWp%5ifQ2TG-O:gdTU%7609%U0eU7I[723hVi[HeY$lL)Si(E,7;&Fo'_<.wRAD*2U5n&"
"4W%XLn*'oSR=5H3h_k[$ME&(%?D#G4#)]&4f=Ds-UW/o8$*DT/VC[x6FBhT&F'r-M'qjh#v(d6;=cvC+rWY%$^6x=V.j/M5GGh],4I7=-6)d_Rvn<Z-+8$*5ZuS12,&gO;bXN)Fqkh,N"
"3T$#D_hCS3/UFi$(,ip/Hh,R:;?-;T=/EZ1r&f.3]%n;;LDk@-KBP>>gtA.drUf(#dL&&v%,ge$He[%#JXcE*4CEp%K_JM'e)Uh$QL:F**WTI*qD`KcU.ED34:9a#NVE%$@4[s$:=Rs$"
"-68#,Od]FiI4Li=9MPB4JV=Q/33[bu.H5dY,,jZ$FlA2BT');-fU^nG+p@1gk@,L5#KXP'6iiEIb(Oc)ENe;-j+AD*?u:3tdO78%_EV2%mZ)22I]VsfmJ-i$X[W20*I3Q/d.<9/h#00W"
"rTb4:pQ4K1&S'f);vp<1.tiu91k0<.iSP',1aUZ-rOkp3P.2H49,kA5moxeOSd?c,lEK'JVVJfP4rn12a.MNC4evF-c3fs$<N.$7Ugc#AG2(G35Kt:.XD.B7ddSjtYPff.)1ST$6LZ]1"
"rR)V7T[J60Grx:J&CK5qJN``$AWc8/xd)*48hw;-(>ie$8T$I4U6Ss$]=QU)f<W'$j;(u$a^Wh#g]VS@hDO_.m+$h@o0g8/a(i;$Bndq72[lv.*qCm0rJl#-3T:?-cBnW0qP,Y-qx.Q:"
"+r.DQI'1+,u9cn0j[Yi2gu<p%.XBF&UbJQ0lBn*=jcoE7I3.2(K]ZS/4dSdF,9YA#oX_>#QkhX6GfU9;[Pi=7w8Y^,)5E[e_n3%<?TxC+fU:3tp6Lc)$wFm&O2@D*Am3.)41@W$sO958"
"p&1B#&[M1)'X,U%kR.@#Q=,Hh//>++^)TA2>5-Q<RlN=$rC'Uu1oEt$&M#J#(lcvAR3uv6O`nOA(:-K;WBbG2=KZm/EkMV7PwR%#vi9gGCGY>#Sww+2U$nO(qv*l(LWeh2oZnG*YcdC#"
"qU0A8U,m29<=Rs$?G%%>V:hgur0OgjJ;?S9]bPCb_s$h(#O;H*U7NB#.(`>#<rhRuR.S3`N#`vu6ARU91T`w.rCn]uuE^Y#E#?tU@TZwK#v2'o5%2D+w>Hn&R8manp))[KG-%12*]e[-"
"J,9f3&R:a#oXG)4kHgM'=RIw#&VSfLS'w_#Z4(u$eA+9[QHM`skT>c*ZiNI)5b7>evGh)3.DcC=4nlc2TViO]T+8,,G]u,>N4V0GC=T(*H7M*%8df;-H0kl&5m7k9h/c$.[Bo8%:%W/'"
"VEL]X8*J%5YFn%J4bIH7[).^6T<d3'4-R1F@lLv#/LbI%wQM`+8sX<@VCg8MUH<,#'b^sLRo:p/:/Xp'j;ed)`<ow#Mp^oIVg###ma@k(cGoO(@TNI386M2%r_=F39Cn8%GO,w-+*SX-"
"NP,G4p$?^=mJ7&cu=3&6_i4n0&^ln,:UV)5:[lV$OvLG)gDFq9_,XU4(5XI3>qfmU3mu>ulLbR0@BIG#Qk*_L[kIm:/Nj[t]h7hYvsgUd^*9G>kT:?-[XrL2V_5&Ox,#,2,6`v%>a>W."
"(%.)%T@..3r5@L2_8d8/tkXF3'K+P(+wSH=w0_F*6_e;-mP7^$>;Rv$'hUhLwXIh%%a4c35%j69L*m?.n(.*86dTW0qObO'ldGtBhs6a=;fY_+<R@(%,lIH%I4ZA+'cN`WI-Km&hoQf;"
"i6F/4tR+)NpuWq%V/b_ju:LB5+-Gxu%3;,#UT/*%_)$$$;.>>#5ihV$b(1/#a@eB#`AFf%NpS5'K-i#$9ZJ]FS9uw9v2H?$[ov$v.]^C-;[^C-tSMIOtF?##7U[fO;CD+vY]0b$WdSZ0"
"w$###+)Gu#K7>)Mn/)fu@FxU.3A4*v#h%Y-q91S*M8>>#v?J$NPb#Z,hlOw9cf^w9__.mKY/7R*^e1gL6Bo:N8FPGM&,+$%36uxY3gSD=mHbX&8X`=-8m%Y-EFBR*Z?17#t<TSMq1sxL"
"8@,gL2-f#/C3*##9ngq)7huq)tE/nLiB]3%e=SV[NR:8Sug%,a)u#gL<d=vMaRvu#d0o92l=,gL@/CsNoO6n:-n4R*?9`R*&6i+MR=&v$<2jo]wnjV7=NC2Ch5&gLA,lvM5+^fL-vqWJ"
"8X`=-Dm%Y-11dw9a%,gLD>1wMp6),D]?gvn8X`=-I#Au-G>4gLJUqw--7niLHYh<Ni(B-#PTN49#l4R*ITVR*LGluuRJ3o8gm4R*JWVR*G$v+#'(xU.R>uu#+,kB-W<w#'K*&##a$Fm&"
"iY,W-%FWpBs%HU8*m4R*f99:2X6l6#ONC+<Si_lgCG*[Z+i?8%6kXJ(:T95&SSBdOe33V$oAU=cQq78%kxSO]#)P:vxS]ciF0PX(5,1P/9bh^uEWKU#_LF&#u3pau%x<R/`o9Djn<kon"
"*WJ)=*V>F.J_@Dj)SQ;H%*nJ;1JRX(^<l34<FZ`s&33g;r]HX(2Qcf$In2d$tH`t-15MMMH$In$^P)[$%*7*.dl-YM:vU>$D;x5%)`Xv-oM)^M6Dj*MX;jEM)%:A=;*j.QL*W-QHd8]t"
"rH0qu:u?W%k)w%+DY9Q(]in-)_QS%#uAT*%Lw@@#n2]t@Z4p1(g.fO(*m9gCqv&k1Pkj=.pr/f)V0^g1+O-u6lRh(.p726(?#<2MW-Mi<HP1W-SYp[/Y3&`?/'0COZs9#v)WI)$-=ig$"
"8bCZ#f@W)#xof=u&q/iuVh?0#=?<p[X3x+27PXWqMVlOSb=UN220;aui4'Z7h*X#7&bNw96jMc)_1'F7t2q4JZ]C-mKelbu]C]5#JV(?#dfXn*o.W@65e>V6&2ef(FSSa'aUZ$#h7RAM"
"6,M7Bx,s20]]WI)>2rh1RkJ5:@+x%-8]HA6)#9^$;0(71<G]l'E:g(4&W/U7M0g9;PCi9;SX7:;109e$sqDi-6@lKY,LChj^G8f3ueAT%-vL#7]@JA6pgi]53^3_$^B8^7qNc'4uV`k'"
"YRO31mgJxtZ*9.vs9c6#P/#n]E:rv#v'DtegFP-MMCSs$oNv)43O,Z-1&6`%YZg.(b8<I3Z@oC#O?Tk1lMdk0a8^,1M8B+49Qae3jms-*/sc[?Sr5G;'R,)*R=l^(M:)$#sdnZp/BI12"
"<Q:N95Hrc)Ru29/#F/[#'EsI3?:7YuK)8#8eGaV6STs)H/,ck'/hJJG`YnC4%>Nm9t+ew-&HNX.%2Puu?h:$#qx.5#sEqhLp6&h(;GY##4UE.36kfG*_?=<-9qwm/?fds.bx29/gVs;6"
"9:NT/n7<9/uC220mUJ60C;D'#p+vju)$;(v/ZeJ4[EX&#6,3)#k1E-2b$j8.T0a@,+4qHMk3E.3D>8(>=[W=CD6dA>lnGg)$@dAO+..i&bT)S*[KCANx.*J:afm8/Gl`s.=^mQW9osRM"
"WPFjL=*GK.=FnL0Jg[50%:u,/F#iT/Wt%[^luXjL?JPO-CgX;.K-H1gY,<s.KrT%$MDW]+/?`v%Kd10(`?A%#%d592iU`)4La&COU;x#1URQnV_Vqv-exms.VM*jhXC<%kGo=#vnCsp/"
"+$<NOw:e##U3=&#;jYT%h%OI)-Mj.L;.l(.d1N/8%$U#$sbnHH4-NOB;bJ+I'^pL#@S3/1;m]R.clQW.+o'^uRQ,)?h`:CoKBwU/M,>>#69OS%i/m@X6=no%]0xfLntdL2wk%NMI33/:"
"cr29/l3xnL=q29/Vgr^#Q^^i'Pi/VZv?/T.;qJm&=M;u$=1<:8_v$W]^lW:M&<3T/%_^0.`V?s.3]U00HQogu),-t7`s''#FPFV-Yn8e?qTPe?7pw%+suqr$(8&p&>Zw.MF)+F3:4^1("
"0Deh2&JH<--<x,*w<@kteho(J5>Kp7&m:p/HapW$n+u/MVr5X$v=fx4iPSuP/(no%ad-q$Ygdh2@H7g)qA0cd'+&##kw]au@lX2v^Nc##ano.Lp)=F+FlA)8ovjN1V]C.PV6RP?Iuew0"
"84gMK;'V#,54RS%/4###+w0I8=c?C#.^?W.J+bMKA`]Q0q((0M^/G$Ml>s_$OTXgLU[(dMZN+[&=NPs.xq8R3-.jT`t@N9/vevu#q4l_j[A?cVq3/T.'B4],_v[N%_Zj>3ZSafL[5L9&"
"LES^Oj$WH.`P$W.j^U8MP&*9/cjp8/K2-F%1t4^ZG:jp/.9<9/fC(d-U59K3hfVn#$bub#XP_S-ousv%j3fNMU&/s$w7$b[J=&60$,Guuw:$(#mu'/#8Tj5]OV0##kN3^2Fa)F3m:'du"
">Y&autxNs7Z+d&65ZZ`-Yoj8TRIHuu44###LmJ8'f[D<-A`*j.d1NT/XG3v%ORa%=-@3MT53=8%F)70%2pMaGN>X0&haSK#[>wXcZK1oU#)>>#>08t#V>3h#OO_/#OnqxLY/R+##,c:$"
"DTc##]xw6/X5YY#Zu,tLOQ-5N.Ar#v2]o;MDT>VHCpK/)`joAcMuL8%I6cX(d&;e?T;^w9Z7`.N1M%$vumb2$ZT]F-SA)=-5dq@-9Sx>-j@)=-s^Xv-xcgsL7O%JM%RFjLYQ,2$'eq@-"
"*Mr>6B0AYGXo4m8](NJ(Rg,/(`P+X$hL^2'HAP-3lScC/r*EJ4[7,i((83F3WAEb3;GUv-tR,m8e?xZMc4Lg5axZ5Y]p$%tcM_WMX.(4EcM`%k+#bYb]F(S#0)KW#1$2muES&`j#Q&s-"
"b%jR#,k%hux.>>#3wV?^-_WM^E^_c)+%tp%@CI<$1,cgL]8kn0?N<I3bnuM(PS>c4=Iw8%:ImO(*O7v#Jtxu.&JPfU6CL=4eG_a+*ZNUMVm<+;wZ7i;B/&^+g9?>,0N@W&`&pYGuhL`+"
"md5%.HZj[?+p0B#x?7f3:v&<%c-_fLD$(f)v+Y^%<%eX-^K:JLIU^0ND(HO>O;94XD8FYEH.IfuxRT&Vn?0iX_+K`Ij3:73a0D^[F$g7c,7O2&Id$a.j0ST$n2L53XkMV7TGS>,*s;S&"
"4g;g(T,w%+Pbj20-/FM2p*'u$g&J1CWOYD4$@_8/3nm&mi`xF4uqp,2O0arNK2;2t*p*2@5lssKmb)#]8c,`JlKw$KChYqB)RcMlc,<$Seke)*Y1dgcU:j:%&_v.FaxX#I<)GY9#ZbA?"
"A7YY#PU$##Y8PG`BT_c)/,Px$E-'q%9>G>#7Fjl&0brD5&*AJ3[1*q%&<(C&e7H:.'jDb3?O'>.7i(?#Q:r@=k8c+giJJ;$)%(x5bbHK#96]ig^;Llu)Y]X#7_$.>Dob#/A3*##GO-,v"
"8ppY$PAO&#X*rB+,fB+*JlQS%pj5j'1J`q%;[wM(Zro^$8`k7'.6qV%K/TX-=,RA4*huxQE[^QsSF2G9aN73Ex:pC^'<0UAD=HVuIN8aF,P_KbEK14IX(K'13c1:DaqhR&%G8e]R-p/("
",(uA44KR8#wJ6(#G%'h(L@C+*xbA5''J9p7(o%RN-s>x6L]B.*FJ29/qk;Z#x?Mp$$O=[TxnQ>#x+Wd%?awv$q?`[,l*CZ?'mBx14has@;.jj@^Hmr@AB;6af=LQQs+N:;.JL^Sla7jN"
"NDX*GjbcNt`BZ^%D,2n&[<E%X=<PhDHC)Qk+j([q.mn6XIaL>#e2Z>7OEw)#7@%%#voXq$5IY##Q/Qm&/P1v#W-5f%BpP=7dIfC#ZH7g)CM5,;Rl1>l63Sv#=r8CjEEx:HYxO29iKvu,"
"<:gr61W[s&?SOa*Lk)5(UN02')II=7,tDM2LWc8/36968cj*^,X6$5.6LR1Mt*NT/?;[w'/AF.)CU=T%)N0+*9N=TUQVqIO#-O>9^?ivK(Xq9KcuoJ?nFDaAc9c6LiwRxI;V1UDGi7gN"
"wU%`8blbA[ZK(TEb0U0O0PF6=0vDG?^bUQ0V3Y?JhrrK62NRNDutnT.#Gk99H0s4f0kU<9b<)=BhJW*#bl-qL0[2u7aR6Z$^%Qa5Bx*F3DsMG)JV%`=SfTj(8I/[#qh0hlxOr_,N)Qv$"
"B=jG*=In2B5e^D3+5t&p12.D.?OBk_dtMm&+P]b@;9(FHX:9:_VhYl&D'4RN&%Q[.)d#j9$s<XX>(;ZY+5?N#K^_e<jr[oPgua2=Ho$v57lpwT$Q;)78v3%8LF220eVarZ)R<Qq?5SCs"
"355e<m<2]t>BO8%IA'##h`>H3Mvv(N10CX(ODeX(Zr&v5v6t34@L55N?9MhL7iXl$AhCZ#h@)=-d<dD-U20_-J9cX(]laY5NgC;d9paGMY%[lLK[MXM&Cj.NqIhkLjK>i$''@q$fH6s."
".'l]Y+X<Qq9qw6/3F2s#$KpoMA-JCW)q$^uo%Gu#DC8ZM7oI_uOgsh#WC8ZM-8V^u(]o;Mj<8ZM?9MhL0A]o#JC8ZMqaamLgx><M&=8ZM0D`^ujdFm#UC8ZMg$[lLK[MXMV98ZM_IhkL"
"g0Bl#TC8ZMQPFjLXHgl#ZTfv2nnC>#3/tcD(/Dm&.uQS%rvAEuTb3]-,=m@t-PFUWD2?p/K;0U#<r''#1g-W%tV>###Qjh2_tK_4tTqc).wW:.ObaXA#w-<Bvo]+@2,>uYhv`hF;65'T"
"/V&;C>xP:v+B%JCt]x1^_E+g1Yn1uPSNT>6G8mfM5dj)3a?x+2L<SC#d:q8.*q7f3@>K.*sa-@B%67wA%/Gc@28>YY>e6.G&Mw>Uo&[4E6vQfLU1nlLKQ<UMMcm-NDt?b#%ZQG$>h1$#"
"MKFW$#EDA.1b:I3vT6^4]vZ)4)a0i)NIHg)W0ZwjRT4r#5epel#f''#n_u.:04g-6Jqwu#6Ojl&6Xl>#/Xu-39LREn]H7g)0UaJ1At9Bu)JQ$$ln_bmWSFwKV@P]+;4DG;w'I,3-9%vQ"
"g^P)Ou;oCj`;^G3CpK/)rj#Kr<64s$cFJfLoVNrZdXRVmC&mi'h$io.?Hi0(YBx@#TIIT%0-4#-A;Ed2iR792o,fO(3lBL2eNAX-W0X33A]6<.Akh-MmcxF4+BFb$*R<Z-^A&HUDB&*9"
"<=5ule*k`*g/ST^TB6luwR'0Iv1lLT2I:+QM@VS$Wa@8:Rlr+iT#OUiL05##kucn$d4.8#u=N)#aSvx#*3nD4AT-R&d^x8%?M###>#WdaY3FG2m11/#4g=02M^eTe?N<I32KBN(*Weh2"
"k$OnkxJT>GM=Ej3Qu1t''A8T%5?e)_8ujDKYmiMg_wov#UWLq6YO(_GMe?P;OJ&b6%),,FHajs@1cpg<LwWp1Q[qbW'hOg1Vt`d,$f7)3_c_[ubp*i(KJ5)40Y@%-TGDx7h?[.);gW+3"
"-V7@-NEOb5GA4F=QMK?H(Q0s$g:(##P1MhbLrJw5e$###>ikeu61w,v]g1$#C3F8%_=6$#tJ(T5x#F^$OCI8%gK:a#>EL$')_T&GsYUY$H&&*FV3l:.^w3kL4PO`uwfxvLHak/MSrrr$"
"87S)MLM](W<kL]OA'*<%B27/LM-[)#$&###LS9VQCl<PA*DYY#cQx+2iI(##w]I-MrH6L,T$:I$paF'#aA;O#:;+I#C&ss#$]8a#%@gi#:V_n#B,/9$R6`($EJ;S$QGXE$gd>n$V>Vg$"
";%l6%loMw$fS7H%U/OA%_i7l%ubxX%uoed%jaPj%HM2s%.(W%&]+oL&S4H?&pN-k&&ICV&PBjd&us/m&M<es&WXfx&go^''e(v3'1D'B'B@xP'sN'+(6*eo'VhD%(V`c/(B?:;(oR.L("
"kITY(Y)5+)0^$s(tHmC)@ru3)G0A=)W=d_);SPN)0(ic):4+m)hYLC*pxH<*EYnD*37pQ*S4BY*j&Wd*T[.p*v4[@+4#5++4+D4+,KvC+s?*x+Iqgf+<JYj+0*Un+B2:v+T@K#,Fm=',"
";L9+,.&,/,S'28,wV$<,k3m?,^fhC,G>4U,DxAu,7Nx],Rixe,]j'r,PFpu,@mX#-RKY3-TWk6-H:#V-9ZF>-).0B-pKcN-_&UR-R_cr-D/:Z--E7l-wu)p-kQrs-FOL(.#*?,.lX10."
"XdB3.:xoQ.,HF:.uw8>.hP+B.Z*tE.E#jH.eK%T.VxmW.IQ`[.6]q_.w1al.kdRp.]:Et.Oj7x.>%R%/)N;)/ErVL/p'^:/e]X>/W6KB/D;SE/#FeH/Z,?K/HeLk/'<XQ/W8+Y/C=3]/"
"$NM`/hX_c/aL].0:;6o/'FGr/^PXu/J[jx/(^*M0P.W50C^I909CN=0qYuR0l1hV0LnAY08A+^0*qsa0rC]e0R3R-1=Svk0p->n0U8Oq0T,B'1w[4+1k8'/1]eo21UV071M<5;1eo.E1"
"RQ<e1Cr`L17Q[P1*+NT1sY@X1#ZP$2G;Gr13P;-2%'.12g+642`(4:2Sc7A2'PDH2rZ]Q2CUX^2US*i2c%?k2=n+n2qYep2MFHs26KPv2ge0?3>f%*3_qb+3_sh43KK/:38Wl;3?nd@3"
"/>DD3Wc,K3n2jT3f8Wx3PuPg3ENCk34r#o3(Z:84Wl$14'7i[4p88Q4lm0_4W%vc4j^xm42.^'54M<.5=IIX52;*I5kU:s5R3)h5]lUs5PB?w5?i(%6ej..6g(P76q=RT6P--<6N+[L6"
"YlhS6^SAY6m`Z`6clAb6>VNi6aK@x6+@`)7n.u37l:[577O^R7VQ)97YYd@7]fJB71[>K7(6ZS7Iq:`7(=*28R?%&8V$eL8egg;8I?XB8m:SQ8/gDX8j$gb8.Ti19(u6p8[*tq8P`ou8"
"X$D)92/S29/r=;9Yl9G99vGS9J;Ob9Sjjm9eVTv9hc;x9H[1%:72v4:Fxb7:GV2=:iS0C:f-#G:Y`kJ:NBpN:BrbR:2?BV:nbxY:8(Na:3sw*;=wHp:UI]u:v>%*;;ki9;YA0?;(WRE;"
"uw)I;kP#S;EpSf;'ARA<C#I9<7Z*^<K[5s<cQ^?=KQ+;=+)'Z=T/%E=bF#S=:5Bv=nr<b=gc(h=R`Po=OZ-9>hWv)>'0AETu<WW>kWit>?B:[>,aQ5?@kV&?-vh)?<`u0?iM]B?/Aam?"
"=m?X?0F2]?m-ih?cmrx?#M/N@XU3B@ipnm@QxFc@P`Ov@jXO;AF7%MAU/oUAjJvdA72NmA>'I&B=U83B+f[QBR<'gBrrKoBVaG@C0c,dCiQaYC)bLhCwsH&D^g3/D`oI>Dj.?LD#ioND"
"kV.YD2bhdDP(qoDa0*#EjS^,Eg@H5E#0[EELG3bE@,ddEc.?lEnowqE>aa'Fp?83F0vCXFYT_HFe](tFhQa^F4df7GbJ:1G7>'4Gqf8?GDV*NG9gAZGdw7fG&8@6Hi(K(H7200HuZo3H"
"i:k7H>OmTH%p:=H*f.FHkD*JHa*/NHWiEnH4:MaHXj?eHMI;iHB)7mHm4tnHXmorHMOtvHA)g$I6_b(I,Dg,Ix#c0I@ZoRIC7X;I[dtCI%/PSIq>q`ImK=,J&3=$JL?'8J3p0dJt9.YJ"
"*C#-K<'FvJvb/GK]0Z5Kbk8GK)RiIKm.[MKesrQK`e3VKZVJZKUHb_KVRGdKTM$.LN6vlKI(7qKDpMuKE$4$L*ad&Lm9V*L`lZILSHD2LM:[6LH,s:LCt3?LD(pCL;WbGLs@<JLL$mLL"
"9S_PL/9dTL)+%YL$s;^LudRbLwq8gLmG+kL`wsnLRPfrLE*XvL&g2#MVLc%MC&U)M7XG-M)/:1Mr^,5Me7u8MEtN;MvY)>Mc3rAMUfvaMwWWNM?M0%N%P+oMn_)BNs]r2NKBK8N&)&;N"
"aK[>N:]tGNKe/KN_:JVN$H5hN_OqlNbYVqNed<vNiw=@OGN_TO;DrIO;*KOOlf%ROS0eqO`n9hObdWrOQuruO3-.#Po4?&P(E.+PR7q-PZ2(XPIa3?PYvZQPEu,]P8N&gP+wt8Q:h)+Q"
":'1TQ(i^DQ8XHMQ;7mXQGA.fQ46JvQk(i*R?c>9RHDsJR*fNZRZDa3SL5l%SA]8OSJ[V>SAMnBS:qMFSkV(ISE=XKSv#3NSNVPPS':+SSWvZUS2]5XScBfZS=)@^Snep`SHKJcS)V[fS"
"VnThS%0NjSJPc1Tn_@nS<w9pS>mXwSRFK%T3_D'TZ2jJTV7g8TAKBdTQbYTTD_,]T]TveT_twjT@;UtTp?2#U#uW(U%HA,UJiUIUnw30U<9-2UaP&4Ug_j;U1dVbUE;hTUbHRfUSQ>tU"
"sAS(VAYL*VgtE,V43?.VXJ80V'c12VK$+4Vp;$6V>Ss7Vckl9V9^X<V>U#AV>]haVFJAKV%c:MV.3MUVh*kcVGcogV;=#*#QlRfLWk?##8lAHYbie<6,[aT%u@58./1RXsbb(*kd__et"
"EmL0&W$rpL[[Z##T?*1#nYn8#>t(9.9`###gcBs/aAhY>0_<j0BY>8Rc7n]be8^]kgd@civ,tuGPFm-$D3[JCjxNk+('J;-]L-8RCpA5/Wvn;%wns9))rm-$%5VF%&+/F%H#l-$Eqn-$"
"27n-$sLm-$(om-$TT#:2`:/s741xlB5klcEFadoBvS%&+tYBYGUU$##%eOxb^YQlJoAUxbwWn-$]R]w'P0cw'VGVxbWUm-$BUPlAn[4R*S0Pxb7?Pxb)jk-$[LSlJ=$OF%'D+gL*l^-#"
"ZS_/#$76;#k)$;#Nhq/#+#Ld1r%H3#ko53#cD)4#R<u^]nX7J3Xk$/:AjW=u043A=e]DS@oXXuY&cl%FJ5o*FxQi%Fh^VV-N0$&+(1YuY^u5NU&qQxOr)Y_&*[6dMjdCR*QL8_MK$axL"
"=1M+$h+p;/hLb*$w><jL`0KL$LYqr$`pQ63>_^>$MJ1v#;Tl'$e%a@$Fo](Mjs&nLh&=$83XC_&#ap-$g(m-$@bn-$Xm/F.i.M$#Pf4Q#U+s4#m(-;#EjQ(#Mp?x-2Ax3Mb[H##9[2&F"
"'23:)U(AD3)5hi0Q(_[$1Xki$0`Hg#.0KM#qZbgL]8N($ACDmL@#9%%+uF#%[dVw$]Zv$$?BA]$9Un7Rt$$?,6q+N'nvJ;H'SG)NUcvu>^?'v>fI2F%;l2F%'3ffh#m#KN@[_58.Qk.$"
"3Q^2#^Gi9.n?.M^:n;@$mH:;$wFNl$POOM$I,_ARX0EZ#ku$8nx*H>P2+h^#-UDs#R^DuLxsVxL3pjS$G(KV-c'kB-6Ri#Q2Y_2#wG&:)=,IcV/ulS.NSL;$L%ukL-;WFMhD9gLB'iH-"
">IZs-A]B%MxbF7MCx0AtasI>H9tN-ZJRdZ$u5NH$8p.oSm1N*MC`Q:vvI&=#'25>#79V(v^AqS-ZN#<-oO#<-SO#<-aBRm/I8;,#T@F&#a`Ys-V'5wLhnk)#_)g&vB.fu-7'SfL:(8qL"
"VL5s-7>lwLTV6J$XZB%MWi&7N%6AYMn5KnLVt0*#ei?x-lYEjL#86>#3<v8.pb$v#wtJ88_-$gi%@vYP5X9jKLCF&XDfLs$GqbS@]:VF%TmTF%:YB/Ci&4Dj()MH%/e_`*LoKS.9[7Q/"
"n?.M^t+IcV^?8=%e:Bu$F(b=$mH:;$$/=GM?.73#8ER8#]sCG%(U[#M:4`2#mL6$v^#k1#N@Fxu,G5##kUK[NlwurHRR).$1K,F%.g1A=Y^4A=qKn-$,mZw'j2p-$=E5A=qU1Yu;jkU&"
"j#55&/E<YPQOQ:v((V:vmS,F%?iU:v@w3F%5aaCs'rtLgZCU5%9PW)#cO[8#H.krLj(-h-%.QdbQ]vu>CEs>#r#TSoKnY&moVgYG0g0_SSOs'MgW0(M`OKvL7Z`/#uvR+1f@TfCOg]V$"
"xWI]F29`M:_s`>$uIx-s9T*At(BfuuV=h(jmrw+rH[tfLrc7d)Mvbd3[>x+i1@q--,+-;#Zkt;.]PO`ETBYMCZYoAG*(3T%@WI&GU(ffLUoYciMVjMBo(>GiOh*;mEI0Dtb=)8o5Q@k="
"Z$^(M%[V+vElH(#'u.-#r(LS-,(sqLe%T,M24pfL1tVv#w1H9r*T-(#^6]-#)j)M-B<eM0p'g&v&<Z3#t]'%ML#S/M=bCH-'W4K0UU$##5DVxb,tC9rQ^=,WT`#:;@g'v#I=NY>%n0Yc"
"bm@R3kc=_89=Ww0HkDp7&--Z$E(c(.6V?.M?+l<%tf@+MgUn3#tUM,#;6brL:VU2#I?,)M6?VhLg8#&#]f*rL1DH&#(wj1#Tm39#Lo46#^uj1#E778#/-'2#AkajLwS?##Zfs)#wBpV-"
"r.Ok+aW95&'V[w'EJD_&se5Z$(M#<-YX'`$FtD<#xdt&#mm2<#4P,+#BB*jL[]'=#Y;T*#_L1pL[,l1#nTonLHfC%Mu1$>#U6F$M(Eo0#OrXC.7b4/(K:c^#p;4DjVkRKN<v3s#*Nl+%"
"ZK<9%qG+kL'kh9M<:MhLT/n+#d6PwL[h;^8<ALK*G9H2(C_/A=1+iP/k9V=u^sZ]+<G.E+q7bo7@V@a+Ofn+MlScx#9&5B$;k,tL.a],$pI7-%r#%)&mX.H##<r($F1Vc#+JxnLmGgnL"
"S`LM#XHj-$,A6G$3DonLD)a$%[J(xLOaqx$r`dG$XMT^%aN?0%kp>v$^U(C$/og'%D'mlLIu'kLmGgnLZ.BD$&owD$tO)[$')wk$GIMmLGxlE$]0QZ$?Q?,$$MkGM=V_2#D4Kg18b9F$"
".iHO$MtH0#s69sLp^%g#<0LhLcaamLX$[lL,:X@$+lQS%7FYF%R+/5#%)###%-2,#=>/vLLVOjL%mGoLNF@&M'VG8.*qfx=>Z/a#%w8M#aCBY$de<=$(e68%Tg>;%7W/M%YYki$4]W#."
"8=:pLuekFFQ3TlJFed%FHw)M^ISDcVq>W[taoQxb%n0YcxQi%FkGRuYnqw(3q:LP/G1ji'UZgi':Ch--wGOw989Sw9E=[V$@J`w'66_w'ZQ%#,CtJM'jlZw'&EWuY$%TY5F)sx+xWI]F"
"PREfU;YHKN1&M)3kCgP8];YF%21q:#V5<'M03tuLq1(*#8Pp*#I`5+#&wvM4'%*-$<1qF$vEdC%?ii<%P_Mw$:[B%MY+QM$EY)@$`)b=$R]B%M^#QM$DTm$$tJwW$<J,hLDv'QAdB;#P"
"w#&F.Rw6kObnfPJw=55Tvdd>$$d0#?3eHC#Z6ZJCuGF8R>wAm&u>>g108e2C[^H)Nd<N;nqk`2T2IXfraPYD4.=p>-/,B'?#>D8I)$UloB-I29QJF&#I,,^$^fh;$+%Z;%B0'Y$8V2[$"
"9Crg$4m1g$`9VK$ihYs-pw[+M5BX<#b8]-#.HPF%v#coLsjv)Mq-;'#Ue:xLZJmwLU-E*MVjv)MF@6)M[YPgL1oHlL8LoqL,[5@.]+iG%sp`v$Jx$>MhYRj'R`u;-LAo3%Xfl##8.'2#"
"JPP)NC^`:%[O]5#3.x%#-vZ(#0hs)#PmHj9Z]S2`6tCi%Yobo7h(KP8FW$)*NOZY#:%;RW=+K*#Nc;uLR9-)MtfH+#-]6<-hUfZ$2pioexK[G;9v;Jq9q#/q%<Z#5)0Ca3x,?^4H;]AO"
"Ag&T.6IA5Jg=&mo'&BAt[tv5/MEh5AE]fw-ve_5/Zx)#cFx3p@n7Pw9[H*Ata8>>lbOroo(Qap'ELm<-MLm<-*1Eq7#p[H%g5F>P_X$0-N6;,#*AF&#DN#<--O#<-_-A$(wUjV7*1G?e"
"6XYL%G+$eOm$eQ%PtR?Rp8hH%(c4Et;m&##*E&##$HKF%xAUpL9k^KOV%0H#FoKB#dV'M:G&Ilb--'2#hV+41pOv3##liO#VA':#G5T;-NYlS.%PT6vU7T;-V7T;-I[lS.CAs<#tmP8."
"%6cu>sF7_8dY0F$+5_j$4fg>$f[P8.VqUV-:+-E3J1Hi^MjP4okq7`a/0Pxbi*OxbUU$##aoQxb6^r.Cn?.M^F9QuY&qQxO>nPuY.`7onv^qM:VEM'm.2(2#B)m<-%37b3KBt9#&Ta)#"
"rSB:#LwI-#';?;#G52X-F*u9)sS+5826d`*mI->Guneu>dw8A4sd&/1sM3^#Is%'.94f3Mg/$N'xEnV$Jh'6/Qmg#$]6>'$c[f]$4:=i$O;=e#Bpn)$^;-G$htTF$4qxA$BdFoLnEVF$"
"7`_;$`iCC$4GtJ-.W@Y/O`n`$9s`[$;@^.$&,PwKa=>#v)0p&vk>I*.rfj]O)cQmN(Fo(R_;#)1$81V$DW9oIf)T1T0bV8&C'0;e/)4m^[w1YPN5[ihAi<#-p`2GsJVd--4^^%M-/moL"
"m>+jLSC_kLq`5oLc0nlL^B3mLRZ,oLx4voL@5P$M:+?wLGtW#NSM9i^N@X>-h_F59j'(m96J:_A3S2F%<Tk-$[$Uw0#'>F%B;tc<&:H,MtUX_$0R3<->S9@%Ya.u.OMGYGJKFJ18(Vc;"
"^s0>>RrH_&UZ>SI:dFYGY,),)Ul(;H5:=2C?kDc/*CWc;Ho:D<_cQ>6c1w>6(6&/:5LQ$$Cwst$fuK/%+6s?861$?$l[<rLL6P)#q$###,lQS%Wm%5#$]dntPWOu%,Z>hLIt8,2iT<;$"
"Y)wFr(2###-jioe#JPPT,+U^#%fv_#%,%bN-6r:#fRs)#Flw<(Fsk-$9EeK<&uA`awloXu7l>W%2a;uL.;d<MX0E/:iC2F%jR;;$x['^#'rB#$4Z#K)*qTwK`.;;$Egcw'UNq.L4DWlA"
">6$ajZ$eK#1WQC#?0oH#7LpM#xV+Q#Y/sW#B6O]#cT.d#VCno#$t5r#wbCv#DLRw#_p^#$13^.$Wj.1$v<B6$WU:;$DL[<$v6iC$<<FE$4(TI$Mt(g$S)ek$YI<l$NsPs,J_<a,.HHk,"
"EJNt,Sm,(-M?D<-AZF>-V5:?-r.[@-/fWA-nI1G-^OeH-x<tI-9TmK-ZA&M-m&)W-96oX-R)1Z-erH[-su(s-'nIt-9<+u-Js'v-0qP$.0;U$/Lnq2ZsgS]t8dQctE,*dtX.,buwxF:v"
"VHvL/0rC$#g+PwL]t[C#np-qLvaEO#ADG)M$.?u#o':S-os9S--0q4.^bB)NP7%$$0AQl.[v@1$&$:S-Sq9S-oDb(2/04E$ek8I$DeYJ$CG7#MjU(%Mp9fp,HX3a,,AQl.HCEt,OZ2M/"
"wm7=-TDOJM$1R@->LQl.T=IH-,AQl.0*-K-Ks9S-_A:S-](:S-n9:S-br9S-`AQl.EKXat9@Ql.I8<dtCr9S-7EWl%i'@(su]1]bK&h@bGod@b2>Kl]Q*UcMql4J:./EPA#-j:Q)SsFM"
"MDn1BYs*##0qV(jC0*##i+fOfei.oek)GoIjE/iT]niLToGK1T2x+Au<F)B4*,fHu]eAau'Q&aurFmQmNA<wlU+4Tmd/3<m'xc#mg'/DloW.Wli?`VlRXYUlF4#Uld@<gL.i>+#9oMrY"
"h5bi&%;b7Hr@@]*N/Sl%SQUUu)MMCr&cJl.u^o=+vAZo-YuBf'x8/4n*eskei`&##P?sDNf+DmSbic5S^P,TRhcKG;Noh3=N1r%cRobi_F7ZcaH0D,%/4^t(x]p-$$Z1pJi*#15Wdbe$"
",mH@'X,S-%Z03N,$q./:<B<8.1eAVmMl)8n1r@SnS:&5oL>7MpuAZS@hjZ(sfQq+rb9:JqA%+#5/HH>51+soI;J`D34#dG2v`?&46/)d2oQUMB&-<>PO8PPSXBGm:.x#ES;1@UM+>Y<L"
"qJs#KaW6aIPeOGH@ri.G0)-lEv5FRDfB`9CUO#wAE]<^@5jUD?%wo+>hqdL<V((4;F5Aq9l=G2(dP$C#-6HQ8NTx?&.?Jo[pY(C&Z7k31td)Q8taJU)3D5r00Ab,=:21I=v-pd+/?-g+"
"XE4g7B+u59.o[5BR5]NN75voLJ/1lLMbLT00C^/;GiXD--^DQM*+l31]9-A'v=;&5B*bk+5d;W[kY'D?s>)kLI]xb-gRxw'tHpw'D=_['8fci,vNpw'hjd<.DaT`<Nc1$]#NEjL=YId+"
"c7Yb%pZKdOG2TG0^*'aBa;_;.9@Xw0^kr##Z;UuArK0t[7%ms-ds&nL,(,2_n`qpLZM>*?1hq(N[)^D?fdfpCuO8X1n($p.*e(p.e/w%4Ojc_&28%N-x(RV[)2#QBxC;f-XREB.KFMt-"
"tC6jL0;wg+i0:Z-Yu48_/ck.Ma/:Z-w.T_#:S)K.Z=/x[@Jwk=xWpHHUrlcN/s[m#uwdsA?MEg.>r$?\?Wi.S[sRM=-A$).MRHo>-L=iT.f]',;G?:@-Yrmt.7'q)#X@P[0123=(:=xlB"
"X@6:)Yb:N_e),##A];##@ZO2(LMwr[cfVqL6(^fL;OlcMwZsMMG@;=-2@;=-9I;=-w<(@-u>(@-<er=-J?Or.5:P>#uP*1,wL-1#`L`Y#Sxj]7%RCZ%d-[s&OJj]+X?%'MFV5L14(###"
"6$*P]>(`?#q9Fk=UCv6*/1#/L9bHd+/v8kMiJ2R#(F40MYv-tLNtXZ-$<+C/sXZ=:/u,bnVAi.Oeof%@;4A$#98ae$)IK88PQNN1H2Rm#W#h88-3c*=S=*p.jW')<#S/g+x?B['f&u59"
"@h?xBkLtC?U3BpA;8voLR'qY?6,r?-D)cC-u8Eh+fg<Z)x-$>98j)R886&-=rA6g7S0#A7^'kDB0_vs0g,Fj$os`A.^U_A.rpl31EL6N:PC+tAdEq;.*SEjLg)9nLlM#e>b7)aB;g)C&"
"h^&O9q@+v-6916MLG&)*T6-p.T96p.Kegr63k-s$cZ<9.1v=d&*WN4+oq,87=h?v$A93<%:p-Q#J,,d$]7e59Oq5LO,T]E-/8Fg<eCIiCY;%N-uI9=.]7[8.6q1N$R1)Z-n`SfLgtiv["
"uT)kL+wl-=:H1r0GlXD-jW')<.PcOO$I=g7h_ssB<YIq.rg';.p-ZO+I9us8_.*eN6[TB-U(k8B5,He-hbWPMu$l310*?EOcMSE-uAb,=.D/?-JV$A.+86v?@fi62`'HJOqW[2CNsmx["
":>^@B9`Eb+q[/g+rs&$]Tw6qAQS&[TJlj31iJZ*&1+/Q//BkW[j-,qC3Ab2_[,)wA>T?=(w]+pANn9/D^BuiCj1q:)5B^3M62WM-)7j#M#o?u:s<0qLJ=eM0G>f2$2Yt`-CkT>8([b&#"
"_':qLJZpSSfWx(<SpO;N;1RV[,EHb@s:Tw5e@A@'Vj7@'D@Oh#>9^>D_@d<-B5d<-/['s.);+N$Q/l-$FIY8/F#oY/%Vxh+p_8s0l$t*.5JYALP<tI-_'B>.N9nM1]LM=-o+.m/_OEp."
"Tg<wBMpN//HU6N:</x<(vN#=()dZVBj-Y][IK+tAi*>D%V^i>$^l+(.Oh:'P46u>-,?%D?;Be61P1&r0pVgC-^V(pAB7bR/eu)Z-j,Dj$M_g?-1^6g7Yur5#w(vkM$H8%@mT&7.:RBoL"
".WDV?YPMa+2Q%D?^,`NCNTFY-N]IV?\?$k8K2d,pLtM&[[*jQ][S@cOO-I0Q#9$Ds.EN4kL*c3W[I1sZ[)jXh#_ag<.fG%N-B+o:.faSTC,'.^#w8HDO`:>,MJNp49C8voL:=%p.v5]9i"
"xA;n#c0<I$?wTKO#BTV[h,dW[>/kBOo3ZA5k5f6#A7.S[]J#e>HN:Z-e8cP9-(0$.E.CiLSDtI-c60@'hwm<:PaEs%w8fjLfu_P/=NFu0_bTp.9%QX(F,Ue$e)'hMQ_uGMc/QH.$(?s<"
"fAu0,xO)W[-[2[$@gku0%[=%5T`O4MJA:A=5#+N$$UNp.N@1,M9eFtA1J'H=:n@iL5pVD-Y%(PMjs[JMO3T3Mrs[.Mn$V*NHCBC-v&9;->/TV-/d[tL.A#F[fh6b@,eIp&(ZTh#+V)W["
"pFnSF0=ufLk7J>-j4*K8j4;)FRnfj-Zw8R3jjF*[eRI$T2'c6E7fmq;FQ,pA1:q>$[;FwgCKEjLb:[I/K-^V[lhQ/MxjS1_7j]C?LmapL,cFB.-g3[KL@l>$./CLMVgN:ML*P[$D=$W%"
"_wm*Nw&6H.er<d&g0mNbCOge$%Y)W[&7m;%/F%_]&;,?.9C:p7'QD^6p%b;Mowo`?$[=1#OBkW[PHV#$=d?'oYax88ZY;W[hHuYDMMF$8n)?v$qH$N:Ir%qLPG#mB)V%v#PchD?T/:Z-"
"1:9U)?*0?-dBTV[:g=xBLSc;-,[h[-JNPE,]A)N$[XHM9=WUZ[4L@Q/?H.S[:H$F#'r0'#n$@8.K1cfL?UIQ/PI;##2_U&5B)cu%2aQm#2h+Q#A#n31pd5n&jXfDO=$At9GBSd<e2sx4"
"m)3O+#UNp.doah#.V2W[C]nV[*b$a$u>:5MTce5/P:bKM6'h,N@)>v0jL-<IXBkW[AZO,8D<vV%8&IDODt6Z$u_i6<m3R'AvDie$D)3X%g^E54'uO&X5c[p.fuah#<&GnEvQW5/%4vV%"
"CJ39E(]uMV)9Yw7BC-j:<,DP-3`fZ$h8A:/T0AxB:L0t[j<SpA1-K6/Fk%>-VreQ-/GOR-7uTO.)q?)KdN%d<*P_9M7>e['twnoLskhq7%J>s.Zr;U/hgjp.h>*p.(OrS&L>(3(l=M)b"
"l,JjL)Bv2vv1Cv0UUc>$NAIa5A/Gv0[*b>nS)x+NEwBh$A_$[?+hcP9*S%pA1$dP96VGQ:[V=T[1Vh;-#9Yh'EQEjLFiqp7&J;s%fwT;.(Qh5B%nbB.8P0@'`W#m&NX_D$7AqKGM=P2("
"6pH^5-`NjLIVu%Mr57i5XTm5/Dt$Z$rk9-m@IdOOxq^%%)h,Q#45f6#).Y]+xt/S[A)B.Q4-[DK2@aEOX$qpRImJvPY'MHM+/C5%5jcF%FYNK*XdIQ8Wf0HOnJE<B6v[,&*UNp.,OEp."
"F&Nb%CXR0<G0WC?LSCW-?NZ^$J?_p&p/@g%%[Wp.[>t8&Aw^7Mic)v7B^KjC[%Rb?hbGhL&+/u-%djsL`;lT[mp0i[a+lT[&iu8.v?[V[<J@a+0%KgMlN@g%FN`].0M`Y#<^6=.(Q8#M"
"QiDkLMPqfD'YmP8YdL^[skO_&<DSE-ZAtW%1RKm#2CdOO;GD=-_FAH;X;xe*pR1I=q<TP8L.UA.;S8;6OmQn*N#n31Tm]c$gRXD-Ll?@?cGK]?CAr8_-`JY?_:oM1e'A>-lf1p.BR]>-"
"6R1O+^(5j(<TK^[rUMk+HmAgM[VkJME&u>-_p1(/S.a*.4d3rMi=1=3]fY:;QRW5/sS_D?5u*8&bX(m;fi+pS`3&k,BB8#MF?-Q#hQ7GD<A$s[w=j5_tXAsIW*4r.7p^>-X.L0Yacc._"
";/EY?0uriL&?G2_B-xd>4vA,Mslg._GJJiLj4Y][j0Z:2.ICY?XLv<-uq?mL1Av2v;:4jL?_uG-pYuG-nrO8M,6gRNw6Z6#nC,sL6g,Q#>BGd$p?`Bo9Jsx4'V/gMLMR5//wPEO2wiA."
"A*52_bok<:xsiw'tqBY?r5_5BH@S;MK^qe7SG&vIm+2&I?EZYHDPE>HDI'#HdQc]G?5IAG?.+&G>#i`F;pLDFA*,)F7^lcE8VMGE6K5,E3BpfDBnNJD/09/DK'oiCs@QMC-k<2COuulB"
"%DUPB1i:5B'F%pAu(YSA%6A8A#+)s@Xm[V@vnG;@')'v?s[gY?.2F>?#d*#?WOg]>n7OA>l.4&>s>i`=jtOD=hi7)=;5nc<eVVG<(%PvGc`,;-a],;-_Y,;-3[R--G.b&-ZV,;-XS,;-"
"WU/2-UP,;-^/s)-;mT/-QM,;-roe5-MG,;-ML/2-LJ,;-^`o*-HD,;-FA,;-b@(,-C>,;-<n>(->atU4^3jI3H[QL2j7/q/$1wg)0P9Z/[iH8/c0Mu.lH$V@]]-%-HB4#-(5,;-?]o*-"
"Vns(67%U+*'cM%,w^fwuuT]wuS/>++CmI%5;/%K+<&dG*50a2KPVY/*md^e2xEKe2Ia?X?wBB*3g[N)37<_W?>Yq<$ELfT$C'^i'kxO:&cG8x$.e.-#?m^>&VJNYfw+CZ#A#o.#RD]5#"
"'$P/(`vp)3aO<)31$:W?:G:[#A4/t#9ouu#Tbn>#tpLc[Q@_6_sfe4;3ZR@@d?Yb[$cVe$MQB%@iNYb[.%[A@/JM>#tO3LPf(/88X6ql=1PD##=Ls[?r?TP8-)4N$P>V>8m'U'#nd:$H"
";[:C&C-@['H_ge$OV:*O:-J>-WYFkLKKDiL`9VeM*IViLFL:&O]MC@-iiVqL[&A/M`AJjLq/T%#HnNP&Jf*)**=H?$-G.%#;BP)MaVZ)M4dwruff[+MGE`hLB&+<#wnr4#<FJvLBta4#"
"I*B;-c,ST%Tmc+V]wt=Y=*l:dqL'AO)h2DEMJLiTB`@`WA5[Y5;]@X(*C:R*iPNcM:u,/:ED>8JvXhiUleOS];vLgMM>l1TsU;X1JZ3S[Q&_DXSwk+M4RLsLvRKC-0?eA-rfr=-+<Z91"
"AQD4#)E]5#3rs1#k$J^.k([0#,(#[0e^V4#'M<1#r?AvLxBX4#llD<#prH0#@O#N#h@BsL]t.>-7(Ys1A:[4$Wp($#r4a`#Da:xL/Ta$#B7>##_RFgL(*jxL0=`T.D9F&#<5xU.3xL$#"
"PB`T.Z]Q(#ZtG<-N2:W.Ct*A#H3xU.H6OA#V@`T.gfmC#u@`T.FC7@#SA`T.AuU?#b#dW-]AC_&n%n_&R+35&ZIn_&LR5R*.in_&n_a:2BAD_&FN`R*h4n92S-IP8XhdxF(-a^GIYo+D"
"Uw%;H?5c,3,Ug+MiJG&#+5)=-<5)=-:_Xv-$T3rLL)n+#.N#<-Dmls-ilWrLBf>.#q#[QMght)#BN#<-BJVX-8Sm--=rZq;t@+L5J6QY5+v4L#;@LwBOPWq)uP<MB_Hg34xLh%FLgi`F"
"c/M/D[d3X:5ef'&$Fr?9P`8R*5j`-HYPCLGV&p7RCC^nL`x.qL=4urLda5oL84JqLcSxqL>:2/#]#jE-oYwi.92<)#H2RA-pAg;-]n8KN9Ci'#g_m]>$]g]GB,wfDpI]>-IwZ_J#9VJD"
"r>A'o$[2#Hj-m]GuA3&Gk:1j1.PocEr><aFRuo+DHX-H<aP9G;CYV4OQhU3Os%$<-i=9C-xOp$.]=fnL%#YrL=1B-#[u7nL4P#<-GaoF-/Y`=-.nx3/<$###T?*1#8)d3#q<3ZP=GFE$"
"c,>>#)8>##*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#c9F&#gEX&#kQk&#o^''#sj9'#wvK'#%-_'#V&BkLUX.(#Z0ba%`>V`39V7A4"
"=onx4A1OY5EI0;6Ibgr6M$HS7Q<)58UT`l8Ym@M9^/x.:7cbj:f`9G;*fZKlBBH+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#vOD4#-G31#xX#3#=N<1#=lj1#kv%5#"
"#FP##`KY##;O#N#3Vl##f^u##phh7#ce($#m&/5#d;&=#hL;4#E7I8#D?S5#>$S-#n*]-#T1f-#b=R8#kIe8#kUw8#t7o-#sb39#OE]5#&xV<#60k9##+b9#xX)<#EaW1#LH4.#QaX.#"
"Sgb.#3nk.#Zst.#_/:/#c5C/#gG_/#1Oh/#mSq/#rl?0#X%Y6#D#R0#x([0#<`2<##<1R#<xU?#@.i?#C1`$#w+GY#HF7@#KI.%#%2>>#P_[@#=5G>#Vqw@#Z'4A#_3FA#c?XA#gKkA#"
"jNb&#mcoX#od9B#spKB#w&_B#%3qB#)?-C#-K?C#1WQC#4ZH(#TlmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#R`&*#@3QV#WuSE#[+gE#`7#F#dC5F#hOGF#l[YF#phlF#%Jf5#)v(G#"
"x*;G#&7MG#*C`G#.OrG#%ef=#Z0?;#h].H#6h@H#:tRH#>*fH#B6xH#FB4I#JNFI#NZXI#RgkI#Vs'J#Z):J#Q@:R#a;UJ#d>L/#HKvV#Am%P#kY-K#of?K#srQK#vuH0#3t#;#/OB:#"
"3[T:#3C0:#/0nK#';*L#+G<L#/SNL#3`aL#7lsL#;x/M#?.BM#C:TM#GFgM#KR#N#O_5N#SkGN#WwYN#[-mN#`9)O#dE;O#hQMO#l^`O#pjrO#tv.P#x,AP#&9SP#*EfP#.QxP#2^4Q#"
"6jFQ#:vXQ#=#P6#$2uQ#D>1R#HJCR#LVUR#PchR#To$S#X%7S#]1IS#a=[S#eInS#iU*T#mb<T#qnNT#u$bT##1tT#'=0U#+IBU#/UTU#3bgU#7n#V#;$6V#?0HV#C<ZV#GHmV#KT)W#"
"Oa;W#SmMW#W#aW#[/sW#`;/X#dGAX#hSSX#l`fX#plxX#tx4Y#x.GY#'>YY#*GlY#.S(Z#2`:Z#6lLZ#:x_Z#>.rZ#B:.[#FF@[#JRR[#N_e[#Rkw[#Vw3]#Z-F]#_9X]#cEk]#gQ'^#"
"k^9^#ojK^#sv^^#w,q^#%9-_#VUx5#]F?_#]nF6#L+Ro#[b46#Iu$8#ib_7#V,85#sXZ_#hH:7#lR6(#jX?(#Rr)`#;&<`#?2N`#?-Ti#EDj`#IP&a#KJW)#RQa)#Wk>3#d_V4#R'Z3#"
"Z.92#MRp2#wjJa#Uu]a#Y+pa#^7,b#bC>b#fOPb#j[cb#nhub#rt1c#v*Dc#$7Vc#(Cic#,O%d#0[7d#4hId#8t[d#<*od#@6+e#DB=e#HNOe#LZbe#Pgte#Ts0f#X)Cf#]5Uf#aAhf#"
"eM$g#iY6g#mfHg#qrZg#u(ng##5*h#'A<h#+MNh#/Yah#3fsh#7r/i#;(Bi#?4Ti#C@gi#GL#j#KX5j#OeGj#SqYj#W'mj#[3)k#`?;k#dKMk#hW`k#ldrk#pp.l#t&Al#x2Sl#&?fl#"
"*Kxl#.W4m#2dFm#6pXm#:&lm#>2(n#B>:n#FJLn#JV_n#Ncqn#Ro-o#V%@o#Z1Ro#_=eo#cIwo#gU3p#kbEp#onWp#s$kp#w0'q#%=9q#)IKq#-U^q#1bpq#5n,r#9$?r#=0Qr#A<dr#"
"EHvr#IT2s#MaDs#QmVs#U#js#Y/&t#^;8t#bGJt#fS]t#j`ot#nl+u#rx=u#v.Pu#$;cu#)Juu#,S1v#0`Cv#4lUv#8xhv#<.%w#@:7w#DFIw#HR[w#L_nw#Pk*x#Tw<x#X-Ox#]9bx#"
"aEtx#eQ0#$i^B#$mjT#$qvg#$u,$$$#96$$'EH$$+QZ$$/^m$$3j)%$7v;%$;,N%$?8a%$CDs%$GP/&$K]A&$OiS&$Suf&$W+#'$[75'$`CG'$dOY'$h[l'$lh(($pt:($t*M($x6`($"
"&Cr($*O.)$.[@)$2hR)$6te)$:*x)$>64*$BBF*$FNX*$JZk*$Ng'+$Rs9+$V)L+$Z5_+$_Aq+$cM-,$gY?,$kfQ,$ord,$s(w,$w43-$%AE-$)MW-$-Yj-$1f&.$5r8.$9(K.$=4^.$"
"A@p.$EL,/$IX>/$MeP/$NUg2#SxQ4$U'v/$Y320$^?D0$bKV0$fWi0$HQp6$lj.1$pv@1$t,S1$x8f1$&Ex1$*Q42$.^F2$2jX2$6vk2$:,(3$>8:3$BDL3$FP_3$J]q3$Ni-4$Ru?4$"
"V+R4$Z7e4$_Cw4$cO35$g[E5$khW5$otj5$s*'6$w696$%CK6$)O^6$-[p6$1h,7$5t>7$9*Q7$=6d7$ABv7$EN28$WT%6%;%h8$S#s8$W//9$[;A9$`GS9$dSf9$h`x9$ll4:$pxF:$"
"t.Y:$x:l:$&G(;$+V:;$.`L;$2l_;$6xq;$:..<$>:@<$BFR<$FRe<$J_w<$Nk3=$RwE=$V-X=$Z9k=$_E'>$cQ9>$g^K>$kj^>$ovp>$s,-?$w8?\?$%EQ?$)Qd?$-^v?$1j2@$5vD@$"
"9,W@$=8j@$AD&A$EP8A$I]JA$Mi]A$QuoA$U+,B$Y7>B$^CPB$bOcB$h]GF>VtVP8a3SMF.#nt7a_3N0V^K(I'^ouBE<7nMu.#03Ys]G3[/>)4j:[B5:<U=B,SvLFnTJF%J43TB,ZkVC"
"kE&+%YW?dF#HxUC'6emMPKRQ8q;fuB.QBkEpPgjB7O?\?$Y_nFHg-3I$IOjpBwXp_-oT/+%eMtdGpDZD%laDa-raJF%j*JNMc=mJC.X`8&IYAV/[6iTCvP>KC3Id;%,;cc-raJF%k3fjM"
");oQMGKjj9Uq)T&@/dW-1M=b725IqLX$Lo-ZK<ns3^WI3%1I(/A:g0OK9E/:^sKG$:/7m/ProoD`?b'%8W5W-O-d-Z:fWnME?sJ:&*Nv$8W5W-9gb'S:fWnMG9NJ:Cc#4EYPD.?CF#1O"
"a&Ff-p.RF%^/GEEsp%UC#BKKFZBO'%TlqaHpJ-aEV=/+.iRRF%0mpG;jIPv$;W5W-%p+qi7SwqLGmO)<miNDF7]$d3da+T.p<_x$EHL[-xm&+%3J]=B@C=GH3p0SD?[HHFbrlKFgtkkM"
"<J0^F,7Hv$EDOn-#q&+%m)VOD%Lv<B:>-lE&cS>'LAvW-s>KhGGq1MO#,E.N8l4c4s>KhGBCToMwxuOM7g>vG/b%T&(Qiq%YF#&8N4T,3dEV9rB7<:8?cAI6B*'+%YK5VBpXJ;H-X2w$"
"nx^kEiFxVH+=d;%.=>k-,<KF%$61PM%%vVH/OdZ$n.?LF3A98MV+N?>ithYH9Rd;%*G,l$FXpoMX1sv>qm*T&P)H<-2)H<-R/dW-8YY6jIk,TMYFfs?NkIW-:#dC&+UrhFwlMA&NT<rM"
"SFbjNBULp7wRit-vSqQMUMCE<-p(*H<pG<-iPup7GF:&5oX3C5%XvLFxV>0CuAFVCMmESMKqhp7uW)=B>Tft%,(oFHo,=nDF3**GXMb<L='3W%-o6.-.ZV4NYVLE<%judGOP$.-0)*JO"
"xA:E<8wHnMBN=eDwo)SE8m4RENO8QCE?5j9lmV=BA;(H3tQM)Nh#/s?8OC#$<o9Z$lkf'&;;_0Gaes9BlK='%KQrXB8dhc;_#Y_HC;KC%_9r_#2YNA=S+'^=nZV=BQ7_aHdYp(/H>WnD"
"Dol-$s+goDsrDs7;)2R<l#9m'iS4x'F,*$8ao@p&Z)SQ-f5[UMc^c,MLkPW-Qk-.-Qj9F7-mqpL4ID,DY%cZ$#PRFH^^5:;ERKsL6.gfLliP;HR(=U;9cNRMegXRMQ^XC0o2>AF'`;pD"
"<iq8BlP^YGD#54+lHu[$v4US-0u,AN8:'#G``)mBfKK-Q+UV1F^g5w&Y;G<-37*M-9^,PME5%*NErP<-sqQMMAxO(.?Cw-N*dgW$2ZEI3c>#d3+hKH4XsmL2RqIs-EhW.NCN/c$8;>C5"
"G82NVXsmL2+MHs-QZp/N&6,0Nl2pjMWaQL2Wa&g2YA8FIsj9FIsj9FIsj9FIsj9FIsj9FIsj9FIsj9FIsj9FIsj9FIsj9FI@d#0NS.iH-&7.e-xQQR*.Iw-NP-iH-&.iH-&.iH-&.iH-"
"&.iH-&.iH-&.iH-&.iH-&.iH-&.iH-&.iH-&.iH-&.iH-'7.e-.b0FI/Lw-NP-iH-'.iH-'.iH-M0sZ9@(,d3'.iH-'.iH-'.iH-'.iH-'.iH-'.iH-'.iH-'.iH-(7.e-/b0FI0Ow-N"
"P-iH-(.iH-N0sZ9W(,d3S2KV-(.iH-(.iH-(.iH-(.iH-(.iH-(.iH-(.iH-(.iH-)7.e-1e0FIwv9FIwv9FIwv9FILQ*Rspe2W-5w9FIwv9FIwv9FIwv9FIwv9FIwv9FIo-akFo-akF"
"3ORhM'bai2ZICeGgvb'%Jv1T.`lqaHVPPHMU%av>r#BW-4a>(&)WqlMj^$TMoZlW/Ag/+4#P]:C00Cj0dM`='C=oFH,`I'IBw0NCe(-oD?3t71$B#hFep4KCxHM=Bi?FgMi95S9^fEnD"
".a1D5T&C#G+PiEHCH]^HQACL,*.7LMJsvh2JDxfL['3iM2>TF4$$=GH%&DEHfuD^Fqm<nD%sTSD)RMeG>A`T.rcBnDM)dW-[#8kO[-ab.'=m'8=pAF4*0=gG$W'oDl[qKYxlM<B,6AuB"
"$nV=BL?@,M&*EI3'dkBO:&TiFfV4SMBH]b4k@5dMOI]b4f4m<-_`>W-grQF%WuQF%XxQF%Y%RF%R^DhlWAnm:VgZL29e$gLo3kB-2K:@-hY#<-ilu8.;jiT9m*uP9:VRF%<0Fs-On7LM"
"saD:8hD'K2Pn@LMvw.r8d<`w'befw'SacW8_,,F%A(2g2vn2#M_LJR0)X(]$hX7fG5&tRMx.2PMA<:8MdTVGDGNRRMQrvhM3-ae3wwhd-9Hd$K4hE.Ne0N.Nd'3iMXTfF4.W9d3Qp<F7"
"5h<.N[*E.NoFmH.JM>c4gc:&5`SUA5hF/s7iOJ88qkpHm6wjINn>U5%(b`<h75,c4wvbZ-ouQF%+W#a4/pL)cXSt,N+8W.Na:Dm$nehwK+piwK+piwK+piwK+piwK+piwK+piwK+piwK"
"2LRhM%@Wi2b4hwKY^(.?Y^(.?Y^(.?Y^(.?Y^(.?Y^(.?Y^(.?Y^(.?J%vw9vUf-dZ/Vp9_Bj.NvDj.N0*ju7uOUA5s8YA5s8YA5s8YA5s8YA5s8YA5s8YA5s8YA57GC=/le;u7m2MP-"
"5.iH-5.iH-5.iH-5.iH-5.iH-5.iH-5.iH-5.iH-5.iH-5.iH-5.iH-5.iH-5.iH-67.e-=b0FI>$x-NP-iH-6.iH-6.iH-6.iH-6.iH-6.iH-6.iH-6.iH-6.iH-Ykv^8U(,d36.iH-"
"6.iH-77.e->b0FI?'x-NP-iH-7.iH-7.iH-7.iH-7.iH-7.iH-7.iH-7.iH-7.iH-7.iH-c2KV-7.iH-7.iH-87.e-?b0FIY's;-nH`2%Cm#0N81#0N81#0N81#0N81#0N81#0N81#0N"
"81#0N81#0N81#0N81#0N81#0N81#0N97,0N:9#k2@CK59eXO599+K599+K599+K599+K599+K599+K599+K599+K599+K599+K599+K599+K59Bhq5hq<oR9$+iH-:.iH-:.iH-:.iH-"
":.iH-:.iH-:.iH-:.iH-:.iH-:.iH-`'W?9j]sJ2NGjwK:GjwKHE+W-`2t>n[&wL2$1.e-8T`wK/R8L*Z#wL2(EB)4hB)gLx*iH-pEDm$eF_wK/CRhM)%wL2)KK)4$4.e-d/P-v/CRhM"
")%wL2)KK)4PA,8.?7RhMsiv-N%jv-N[^)Y$SacW8Q.,F%P'-F%P'-F%S`Ysof/Uh$`G>)42cg9VbYu`42cg9V.cg9V&Jg9V&Jg9V&Jg9V&Jg9V&Jg9V&Jg9VSacW8Q.,F%IOER<8:$gL"
"&kv^8YwZw'b,q2V@nC*N51Uh$fLhwK8f'gL&kv^8YwZw'b,q2V@nC*N51Uh$SacW8Q.,F%7Zu2V/(m<-cMPa8YwZw'7Zu2V@nC*Na2Uh$;(X-v8f'gLQoXl8YwZw'7Zu2Vkt7798f'gL"
"V_ZhMs0Uh$;(X-v8f'gLQoXl8YwZw'7Zu2V@nC*Na2Uh$;(X-v8f'gLQoXl8YwZw'7Zu2Vkt7798f'gLIoXl8YwZw'7Zu2V@nC*Na2Uh$;(X-v8f'gLQoXl8YwZw'7Zu2V@nC*Na2Uh$"
"SacW8Q.,F%O2P-v(5'gLAdrj8IwZw'mCl&Q>[cHMDsZL2`1Sb<*H46B+)YVCq&sXBK*U/DGgaSMGsW'OEBNhF^n=D/;x^kEW<qs.l7FVC&ujX:.@4%9Gn?L,6X/1,l%Q2(e4P,M8XcUC"
"AYqENw9umDBMVX-$j`HmCh44:G_&g2_'s;-t&S3%?lPg3Uddh2c?s;-$O1]$4m8+4+=d*7#/pgF<d(@9^caN;7`/g2-_%n$Dh+4:e.,F%^F%qpqO(D5.(:oDWO7E5D/#(5Rpxc3NKaJ2"
"p/'g2q]YD4t]':)5kpS8qS.+PxI6EOQT=KM&4`HMEfdh22]gw'g',x'g?ux'po#d3dha.-X57WA$$RgF.jcdGWE2[Aj_j-$<g?RMhQeC%Wq2g2HF4g$YaPo9O/,F%d+u9DI@N:8;2JuB"
"gvr5:cD,d3Epd?>F0nw',LDa>Ve/v$JdoCQiShB=BC3x'pZnJ2a6*b$kjA)4pqJs7SUnhMxoiU8*6^D4`uK.t_wHG>OH<x']l#aN(VO$%Oj%EF$hjI3`@Mt-MT-<8SI=0jVgZL2$n)gL"
"^6W.No<a.Ni7k$%$n)gLC]v:8+*>)40V-W-ivc<8n*Ar8]wZw'9d*F7V&#d3s*dEnaPYD4Ohs9DKhs9DCOs9DdF/s7`Bt9DN$9:D%?[R*l[RF%lXIF%0V-W-pms/W'M#<-$U7+N3nw#%"
"`ms/WP#+,N9Fqh$fLhwK<o'gLW0.e-g7B1E6+pGMd,pGMd,pGMd,pGMd,pGMT.P<8fp%gLTL$1M[,pGMd,pGMTN#<-ke`=-k7pGMt7VIM[,pGM3@P)NZ=*b$^Y8FICt*W-N_`I='M#<-"
"r5/h<YvZw'F&X=8/(m<-10+,Np&]^$MCV_ApJsv%GI.<-Ivej%.J1K2m36u%W6[)N`_.f$hLpk;Ps[JM9,pGM9,pGMxM#<-J)1LM5*m<-,[>h1L630C$e`TC/m=vB=qqmMu_C5%Xi^_m"
"dco+4Dx-W-u.RF%Y%RF%%<B)4I>(W-,P3_ZrNX4:ruZw'%;?)4UVxfLu-kB-^#$X$<3'IZ#Al9'/(m<-X6QD-^#$X$D&V$^#Al9'/(m<-SH#m9re&K2T-+.NKKdu%:GViF>+2iFwhrLF"
"7gGW-PH[X(.UQSD>$<:)&=;hFWJCL,u[BnDUf7:.+6O_I>?/9&-ZkVCE8VT.m&4RDH#vs-5Pw3NuXlGM&](rC;;mpL.v%'..<=dMv[(rC8#Q5M4a#pLJTgoLJNToL1BBoL:q5`-/$>_]"
"Bb;2FR@8ZM0E^4M><s>MAKSvM8sYPMX)mPM];)QMAG;QM`G26MJq+[$-/-W-j$mwR'M#<-ie`=-YY#<-ZY#<-[Y#<-]Y#<-^Y#<-*/Pp7/lF59b[%`$pT^C-.%D*NK[gk$5#^b4xe`=-"
"AL])<%Dm92a2K,3Cne4<V0M.3rm74(i2bnD;9RL2I_nFH1rw+HgJ>)4;-8p&>iZW-Wic^?GgBS/)(:oDu26<8<k$(#'5>##5`($#7P(vu`n'hLWC6##a?*1#<l8*#XVH(#]+vlLI$:*#"
"*TGs-)HwqLrv70#ITGs-aeMuL2PU2#fTGs-Ni*$MZAm6#VUGs-hZA(M-3.;#ZUGs-psf(M1KR;#bUGs-(HP)M8v<<#fUGs-0au)M<8b<#vm8g1c6g*#$$2G#&?Y>#QkOoLSJWG#%TGs-"
"gv6qL^U>I#BTGs-i)GwLP<tk#&;cY#:jZ(#uNPb#sx(t-B=]nLF#*c#uSGs-S$coLP`/d#CTGs-NfpsL$WOh#(UGs-f]e#MYA2n#B6@m/tU6(#^%E%$USGs-?dhpLf6V+$E,%Q/e>C/#"
"L_H,$7Vb#$@8'sL#0x,$gL9o/9uj1#c>p.$`#)t-x6&vL7xd/$dTGs-8*>wLKEp1$Fm8g1bIG##hUAA$cMC;$#8vlLrH-##&####.5T;-R5T;-5G`t-tfX:A][IT(Z[MV$gQ(90,Pu>#"
"wwu+#5,=kOQZ`qM$2]loqgl.q+FOJ(%#,>u[og9;NP(Z#Ws+F%j3%##YF.%#1;G##FPj)#ihc+#ghcdGNBN.FhHJ.#rS5m8`Yx+#r0=gGKnh4Dg0J.#R6)XBVEEiEhpf.#`D_1Mn1x(#"
"ra,%.=5+gL#$TfLi@c(Nig/NMFv=(.Tr[fL+jav3TNj-$VS1Z#=QW)#1o:$#T8P>#)W3F.[ps1#P9mA#]lG<-(N@6/ZHe8#Ple+MnV]F-i[T%%_PUV$SNUD=&@wiCgP>;$]xiP0u^ZD4"
"9Yb>$k[5gL0+Ev#e>C/#JeQ,$(v=S0Ij2<#rDS9$Sn'hLOC:p7#nB^#hV,lL<4gk$MN@?$eYg;$0dvDG1_sw,kvB7--jZ8-E]s9-_R5;-vBM<-86f=-PTuK.7DL88dE%W%0$/%$)`Lv#"
"&25>#OYC;$Lv,?$xe0'#X.`$#]SZ)Mg%<*MmIs*MI7->#u3pfL(XPgL>>b+NBT=Z-@Gi_&D/-F%Fsk-$L/l-$RAl-$VkKfL](Z`OlkFrLgYL@-MDL1Ml[[]-3k>R*/D(F.k4m-$qFm-$"
"wXm-$'lm-$-(n-$+g05/ObecWx^'^Y>`ic<(3QV[VTc3F2iI_&02q-$Z[r-$>+WF%@%3F%Biq-$H%r-$N7r-$mlaR*R;&B#_@:_8%fm-$t/)Gn#9KJDKdMDFIu62LA^E>HGJ>8J[n/,N"
"YB(&PGs%B#cPH_&bpo-$h'JM0L8M8&1<k2(q&w.MjLihLM^_sL:pIiL[?qS-ac[m-MfD_&QE*j1V&#d3XPq]5qZ*wp*Y+wp9)TD=-/L>?mHDJ`6u:DbClO3b7Qv9)7PN#-7N#<-IA;=-"
"B7T;-H7T;-_CT;-RO#<-atG<-eO#<-95_W.hT=v,W5`T.>P4v,Ys:T.CJ+v,hA;=-ln%;3)/5##uQk&#a####pTr,#,GwqL%oR(#^kZY504n0#<kd(#Ne53#UpG3#%3g*#ha^:#DYA(M"
")2(2#V=#s-orf(M1P:2#INm;#xFP)M0]h2#b=#s-+`u)M2i$3#KR`S%h7nFr[_MlS%+8>>katu>mI8ip:v@JC(P',DBva=u</m+V)]e(W[^#&4gVkf:^Qjc;_UZoRtXDA=gA(#>nK8MT"
"LhnrHBx.2KbJ,G`ZEDS[Bc'5]uVd1pNEgxb4]IYcXkHuu:GVD3tW9&4K%_`E`9&vG7&_VH>8;GV%5NJLVd%#P_$1>YMjsrQhrTSRl^-;ZdIefU+6vxXIxBP]IItY54tY;67ASlf#>ae<"
"PYrCkvOlo.w-$R'Z-Ob%2+mK):,f7oM8xEF%v]1q7a+##B'FRB)5/:)1DgY,0t:-MP,6DkNp[J),7Z'GL(QD+b.$##].821`'(:)oWHo-#rd1Mn'X,<L]#v6U0Q[-*j&e,%=q*#/td[/"
"lbYOM=fFB.,'v4Mhnb?mStADF=P0L,.M;^P<p;:)@V]e,v]jY.hY-s,Dh%^.kpkq,fe98/#F$k,v?etL*cFR2kma:MA-.k(h7euQH.O^,,Y7h,t5ip.edjWeF@fq2sADk_#8r'&&X5?."
"iES>MK#.@HBF8P^e6iJ`@4(>dG%4B,@p)[,X;#-2K#F9oech+jVE>car'^%l/?4vm?/Kiq#2scsdW^>$Stp$-[r9/-o6a$#V=d_/?Q..Mfkm-NUC/(&p'G70Axe.M8dnV@,18v-kF$##"
"N^5Q^x=Q_/8n)'5V>/?2K*Lej1&cV7a.MdEwZ]P9i,AQ0'g#(-Y]jY.+Va9--5ip.&^`W%Lje?gvpi'c0w/,E;n]wQX.&&GSUk#-oF17-Q5ip.>xm&PQjp'&/X^V2UbWUMJa;=2q't:M"
"Z3OVR]jLPTt_ZJVto:F%h;ou,(rb;N7=uZ235QW-.#]:)=7A(&>plgNNF<uNC$;)1uQ$'#+3&x,7?T?.V*JRBaQ3j1[YVX-ECA(&am<I-(4C<.Mn@jCLFTv-E,25QR,1khS=F#-]tPW-"
":M]:)Zd_w$bdP2CZv1)&)ji]NfU5PpSt1T/%t:wM,=.JrgJ6(&:6sfMp[5;-N`FfM6xPW-45]:)5s:(&D>MhNFj<bN<J;l0lVN>.?8(^,Qt1T/215,QYk;eNZ'Z8/m029.Z1Teac&N(&"
"6I;TN='RK)kp%Q0v7Z_/uHYV/cxls-aL+7Q_Cx^lTFb>-arPW-L(T:)k/Jk(g>@,EP+<M-MuPW-YoN:).-,KNuWT-<%Xpq2,Te90(dS21qR;=-w;):%Ov_x71J3j1VXVX-7bK^-suiD-"
"r=T?.sH):JJ4t>-to5sMQ,_,jEm((&_N6PNdk;eNKEmcETFb>-oq(w-k0npMGPF?mtZ0(&R/19.5:=8S$M12Umi,<-?Teb%2UY:)8hY:)>$Z:)C0Q:)GV&E-;:n8.]e$4LtQJ&mV%_1&"
"MP@P-k;]Y-LhJ(&R$K(&)=5>m(308oaUov$+J_K0pf7:.%>q+sH(b>-RR-Q%T=:58t)'Z-2l5<-:.29.Axi'Pta:Z-N5O+N:vtY-Ouls-Vi-u71Gp;.DD-hMVkK,3%a*$%R&640<N;=-"
"QEaQ.PjEqo55(^>t'4(&i))d<)ak]>OFeV@80g=)0e&Qq=tBONPrApA6PWe?)P-g-W'8W-hgo_/w^d&8&9I2Lx;^_/?Pfa-B'/K2hZXK9>st1qSo:W-*ba8pZW#pSUO'Z-6/uW%sJBQ8"
"W99Z-o=SuMr7[V[5o7(&`CtumB=MDbr#:(&S9<8fT572hY09(&)_&`/5-'`/>ZxGN4@Rl/Z?*1#SZeEN:qZ,*Zw5W.MNPGM5FIP/@.n2Ti0*Xe+$L-%qQq3.k,(w-<F29NXZQh2BnPW-"
"egQ:))9-[NX*6d3Xx(<-E:<n%upU:)%-V:)+?V:)1QV:)7dV:)=vV:)#a^m8al&K2N7CoM5._wQ%d-(&Z^&^/X.Ms-*'ts7p1Bv-t>%F%3&5%9Z<^;.xfrt-NRG0N68Z,.4`6W-&+Y:)"
"*=Y:)0OY:)6bY:)4)<,av5'&c1D'@%eJuihe:Tcjmf>W-&Brp^V1XVnD@@W-o-lQ:0`'L99Q5W.E$,(NT2i^#M+DT.pBi8.87VT./j-_#vC58.%TFd*(fb&#bke597Ep;.p=qgMWh'K2"
"$l3(&CFNU8IZjJ20&aY$s.QG.i:ip.($TKi#2Z_/f62D1JnPW-DU$-=GdF?-S3*q-*RZQ1Tw[>QSd?jijwW,<QTlq2JqR23IQOs-rN898=;^;.k/k?%NU_C.ug9HN7(]>QmgIjLWt>s."
"aK):%YHwD.Kn`58`7rJWT]-x-+[v1QKNZdNsI=:)g;9F.h)KgLK7=k%7_%qNoHZ[12Ytr.D;=&Y(B:(&J4@K*%h/p]+_C:)cE5r2G:u]cVadq)bhTN.-vg_MuA`i8BCnM1a%?%.P.EEM"
"b7$##t_oe?La1)3nkm##@1#W.ax<w8`Y5W.lUhkL:JG8.]`v8/0YNh#sh)eN_tJY.09S/2&7lC.OhJj:N_Oh#3d%].wwMs-fHk2:bJ,W.X`9Q_MnO31c-Eb.B7#W.o243.m-8T8(ZGW."
")$Hc.#GOV.^-S&0n)Vp.5[=pS>+8(&j.KLYUW;.6FaOrNS$1sN@GhsNPIZ]%?O7p.o$XB--RK@%--dZ.>.7W-3T49KQM;&lj8o3%&g]m.El-x-pBP(NtHT?.*;u&YIAQ,*0_'-%b#''/"
"75c58kot&#:YHs.p#cZ-PFFEcDsw/&pqAu.'B9H'8ntV7Em/r-j:Z8pjq3K;9ea_/DP.iN-feiNIdk#/jga58T8[W&K1$##h%$68QLrP0oVe>%`KSW-CI6@gk>1:..H_2_lL:,a9X4(&"
"se`p8qQGs.^0CE>N2-dj$SYL,4?oKMovOPpO52(&/tA_,#,/DtEZ0T.x#Fm/BRVX-H<Q^Q*4pp&1m@:)OZ'7/AoPW-uB''-@`QW.4At$%k7JC:O-%Q0MBd?eLwBK2LF8J.JjFKrAbS>6"
"ob]88-%G:)WNu,<(T=&>n%om-D6,^ZY@-2&H1C?/X$cZ-'tWQ1(T?ca_2Ys-,0gp8[^),WPqw8.)#-EO8GS>Q=V.(&]L9J/jmPW-/vQv[k(b,W4G0%%23-vZVxh_/hv^?Q@+mcaJI8(&"
"1E`,kE0&Qg9Pt$%r)jL/Lf(/;`p1T/HPABN9Gx38TsU50>^`:%`'ni9=dsJ2[`-QU&Ldx%DrUX:8m1T/`ckfM-#<9/?&w.%;R=4:AbuS/H5'@%7N>KN[kej:S;Qt$@VXY/cGn]-jgg^d"
"6X8EF0([6%&$aY9%el;)n)#`/$N#`/kHk,;`1wdbi]RjU7;f&#4<OC9Zo1T/7ClvM1-Ze<A+8(&NH)EbWZN/25%(P%K5&N8>j&K2VYexM$'_wQCADd;7-%2h=4&H;IDl%l<<t8.EsNpo"
".a3T/h05CNvEn10,lPW-&+H&m5B<-*n6cw$d(Ep/gK=;.kcwj1QwCp/2o7-+`l^W7+wC:)b5IE=S3%Q0:f8vn<.P^c]Z?:)Do0v/3#ms-K.(4N<Bax/K#ms-&kW6N`Hkc%l&bp7]Kv&m"
"l%=KVgkxq2Vi_T8eg:p/6T;=->grt-UN[=N__>,00BTs-SIwp8#mCp/rfm^$]X9Wn_6D2%6rh00MrH=.&Q;w$4n7p&Kmt8.J*&e*]KrP0C?6]%)&rW.`d[21$e$QqP6_K2q?::)*'M80"
"loPW-LN9^dt#2'>`6D2%cq5G0IFxu-P=:4N?W8>0LpPW-U`_pTS_DEONH=W-Js,Wfm7'-WIN=W-N;F@%1m_j_$Q3.6`Pcs-6IjT8O0.Q0u&H18f=23;cq#9o#xw+%94@L0Rk7W-bN7R1"
"R5(w--caK;gfwA,_nkfM<4*XeW`)(&r',R0Yc5<-?G0[%(qCS01L8:.[6,k:j,]2189>WSw;r^>._'-%RnmW0PCxu--E1n9mK*N1g6IpM7:g9&w,BW-6<Uw7?krt-@[EiMnf=3Lc,-(&"
"%UcR0b)29.M9$qSr@hiUhQAW-,R.3Vw3`^YVGvt$Pija0&DQs-(urT8E0.Q0x`B?N0*]e0a$ms-RFsANle6h0xqPW-At2?@+=g9&/T[#%]Y)l0@Bk8.oLAq/tGJm07F<.N-j%,=GAjV7"
"/Zd&=XG*.bvpcE=NJ:W-Z$3:'6IE-Ec;c8.jq<'GZ5u8.#Z#eNJTxoJIJ:W-m7X:)n[X:)r0u&$m1XKV:,:W-&3&'m)*B9]'DZs-&o%U8s2@m0Xnx,=LpkjhX)>W-.cjjDY1]^l#3>Pp"
"hU)G#ZlX0&#^YX-4gCjr.RPq&+QBW-BJJ?RF+3X.D<=W-xc4X&RB$L2mdAW-wQU:)1?KW--wU:)Y]CW-u^8E5-J>qA'B^s-%k+U8QBe21&uQ&8U2R21fMS;%Ac(]9c@[21v7[QUpLB-W"
"c(ju$MPWA1[bh[-&5t2M:l`q%5t(*:/?[21D5t2Mm9e2&94[H1Rm4W-,6C(&NNTJ1Z?*1#YAmK1=(DT.B4/M1Jxls-)rH-NZ7hl%Q&nM;+IwM1%nhp9$N7_>6&O$%W<jT1+RNs-/35U8"
"dUP8JG?A@9;&6U87>nM1BkE?%>.39.x$*_YMD[V[LS&g%vR_gMRp-@HU4p0%qo-I84CnM1@?5L%n0buNm7cDU_Q#wN#wYwN)E;xNqirxNv.8^N1%xjq<gQcsGUG]u#cZ;)tVUA=3YV8&"
":N0<-DR;=-JR;=-Q;/b-k]xwBU*Np16rm8.i,:_PG'<j1&+l<%K95W8>K3j1%S;=-+S;=-1S;=-K:<=-QwF?-%_TT8MI*j1dQJW-6^fkO.]pnNa=uP=wJ*j1;rm8.B[0ejND^/2+8#LW"
"oCtKVmG>W-7q7RU&[e?Z<j28]VJ+2_]h$,acHs%cZ#dofaY[ihrH7j1(nU).(Z`dMJCbKrO;2(&:J/Dtx`O/2H&cZ-IlS:)F:T:)b4NX.@rc/221RhM=[<LDn@3(&Cm4@6Yga/25_L4N"
"1d?q9A[W/2wpj>n$Hi'>.AU5%EM-v?Sk&pAYKuiCQ&f]G`IiPKlZrwBI1x0;2VW/2P4H?.oEt0;BWN/2Su:=%g2:)9*UN/2>T;=-Lgrt-#d0?NFH'C2ibi8.^LR-svvA8o/)+GMesR3q"
"1]f%ue,kJ2IvRN%e4JI29l6W-09+.b3o5<-o^e'Oow@79.VaJ2dP:&5Q?M(&U?gn9u_74;xd3R%w*=G2E'>D%6XKK8IFc'&N4XY8p35A>_X%)$a-o'&neO+35t1=/rEjX21u8#M=IcZ2"
"e,D[2l,=G2Ns&01X/;:#[wE63W)Ae2Jw37/(PlR2T4[)Mm%<*M('gb2S6Xc2=QMdMYV1(,gELv-hXt9)Rfd'&:Jc_5khJW7?,a_GYEg'&m[wEt0`>@?3x%9AT8).EP-g'&ngiqJ;MZeN"
"d)@^P`&ERT+jh'&R?d-6+H(:]2,+2_m#=-aot)##w]Yqf9CKejcw-^lDR6Xn$Uk'&kW'Ft828x$t>Op&`/[-d(Xh,37K*.3kw37/n$s13cZ0nLsGgnL0nR231mls-XOunM1;%'2Al^73"
"Y_v83rQ8:3/o&#MI71<3&Nn3)bl[w@*Y5<-JHQa$O),##a1E)#gfPjuamG<-@XW/1V]Z(#Oxt:*9>D8*:<.6/%&v7*<?^vLT5j8*5I+:*Q6FGM1F7<*v`N=*/QPHMv+5=ZfB+1_rkJZZ"
"lFC>#+q=d2H4k>-%jKe$d0]e$u(4GM2;$f$h7J'.%OBsLRtrV$s6@m/n)N<#)Vqr$S$)t-HD/vLr2O)%CSGs-%9niLHx><MMo9xtmhI_&L$.T79.V`bE-B;epqI_&aSVSnuA'be2'K1T"
"vtNw9xOO&#+1%Q/<E:7#uc+:vp61F%M3)#vcG'%MweSFMV46uut4D_&^tkf()SUV$`.Qe$u%=cM%,*,s/VmQaDY?)<qsrc<v@4AuwHg;-BGg;-eIg;-TN,W-Zv]e$wgSe$t(FcM-LYGM"
"CK_SM#45GM7+ZWM^x>:$Q7aM-A<)=-7Ig;-@Ig;-fAxu-kLeqL)I,OM6ScE$>sN+.Xvk4MGjXD<)GD8A8hH?$1(G6#g-C-M3bJVm+$UYZ*1h^#CUB:#MH+OM5(SQM;mR.MMH=p%=6^`X"
"Cw/;e0`J`tGqlA#TrcW-LP0F.dmLaMQ24JMH=kBMMLZ>#PL9v-MQe;%x7RA-A?VIMeY02^-ut]u$b$s$miq@-UIg;-^Gg;-Q8RA->>n]-#bTX(Zk/:24MZe$vpXOM(bVXM>-1SM<wtRM"
"Kx90M>01AX>i>>#M3/X_W.N_&/*O?M3iWMK_@ZsIML?>#964@0vuOv,foNS7I-M#$<rcW-+OGR*@(T+M>PbPA)lkA#eiq@-*-n=.gHlipZhiY?/(lA#+Ig;-De%Y-U6/@0HajJM&]MXM"
"xqkRMRiqD.q35D*%XslK7LL#$UGg;-$fXv-f)/nLs?wTM/PgVM;'MHM4`JFM.,7a3xnTk=ux*cM6:H`Mf2eTMeh:b$&F3#.nikgLViaUM&B=-$kP&7.$%jUMurlOMX'*$#ip.p$2hG<-"
">(43.nt4gL+x1s-HLP8/*#./rS-2L,mo<gL@4txFqi/sR1+=X(6;(##lL529(vTe$cZu`Me=0vLO8BVMa(B3MMt5/(Sp)dE<?.L,d/-:2`T4@0Zw;F.UP_]k-R7sIpGKe$j.2&k.+H>H"
"cD/sRmCQe$l&d%4s(Y8&%td#$fDCPM[anI->5OJ-)QKC-D5OJ-6Cup7-jUPKBHi`Fb%K/;w>%u/P-A-#wpU_$oCg;-5J*nLliv)M:F+RMb4*2M%Z/v5QwKe$^'*g2bvM#$_Z`A.<us:6"
"@@K`tCk(B#SQPDME3]Y-q?0jiCM7m-sP3XC7J1_A),j[$r$;)=>+>s.TBd&#@c+wLL_D=MJ92,)9kI,3Pq1p/K)sA#+t,D-@;)=-2Gg;-hR:d-62(@04I-IM,N.IMv*I9M14)=-O=%u/"
"uLF&#EB'>$(pA,M@ID,)ZWHp-RFSX(/o=wp%N7wp7^T[MNbU[Mj'X^Mct<BM.X:DN6ta8oK:?>#Sd3F.v*7YM/UdDMwaTM'lUNGN4?7^#cxdT-$1Yc-lrL_&ftINMGvMaMo^IIMJo2EM"
"D$vhL%M$MpH$2^#POKC-%-kB-J10_-DS0ktChxA=Qc%/LDwRe$sJ5&kD5nW_j8%bMP+I9M/hG<-R0LS-_Bg;-'cxb-@59RElqf%4:;Uc`nDN#$]5&F-cJHL-;lG<-^W0K-3l2q/,F;,#"
"e^ro$LX3B-ErUH-%Z=Z-Y'xQae&*1%D2G>#3S[Q/;@B8eq$if:w3mSSeII_&w-LR*;m[>Yl6(#QtVN#$G%H`-V(g3O@=G_AgQS3k?4[3kO;YM^#oJ]XUQM#$LrcW-bR[_&PNbe$pISX("
"jiZ_&PpP_&b*'u$T@:@-Ekq@-xGg;-H4]Y-ik[_&#A[_&t.&446Wge$i%r]5uIr]5uaZJ;uaZJ;<avY-b$uY-(cO&#fQ9o/l@^*#UWA*%7Bg;-VGg;-vg&gLO8)=-k.7*.wIfnL`*^-%"
"FD(v#c=n]-4pP_&:qmIM$-g[u<MC;$MU[Q/WrR%#s`X6%04(B#c00sLD1dWM^#@9M&+HJCK`u?0I5Ue$3I6IMg[HLMZ'n1M2FYcMXejfM*fVA5b`UA548oA#WQKC-WQKC-<a`=-<g%Y-"
"xAY_&d,$:2g&VEMCp;fqkwrlK_RrlKOIo+sjI_VIwPUiq[Tt]5ciq]5qJ*j:+(L#$8oJ;/rtg:##RpSMiT[(%,YSe$O>g0M,B>,21._aFtZ@>#0FhAOe]45T$X(695RD>#+J5RELcM_&"
"IsxXuFE(v#LtcW-:#ER*Wtx_M_v#`M*:>GMVL6##&;cY#CuCt/1De0#kpMsuBBg;-:PKC-`.A>-BGg;-V<)=-b,A>->0A>-f]3B-RGg;--8RA-KSx>-d*^GM(i`XM$oobMm@sEMGZHip"
"n#Ylp[*gcNCQ.&GwuMcs^`6#6ioklT[#mV7C?NX(Fa9@0dhWwT3oR_&PG5hM?3]Y-FRW_&C9f;-gxX?-fsUH-KmG<-g&^GMp=w]4(Sv`4WU^YQPgdgDH1?>#'c?)<GvFR*,ZU_&3Oje$"
"s'e0#F)iaM*<JYMRml.%>;cY#rlG<-:)^GM=iu,M^QkP&*u0^#c``=-VmG<-%0D9.Im-D*Zw4Gsvl2Gs2]W,s>i>>#ISwFMF]tJM(T3:Ms[@DE_2ccW[n0@0VGFL,I(CXCI]V_&0Fje$"
".xbA#kJg;-UE:@-UE:@-'.A>-qL@6/Sug:#*$H9MwF9YcK:?>#g30_-w;n3O.F7F.3I6IM10M-M-=2,Dw`;/Dm-c]Pba_]P*u0^#K>aM-SmG<-Ah(X/VY<a*[h5AOkk]DO1vG_&[3^Vm"
"(_Yq;TGt]OJ'-Pf(9Ag`axB>#dQp34d5EL%ofG<-=a`=-i(>G-UrcW-pb1F.hhWt$F;cY#,(G6#Qe_XM@l,<M<2p+i#glud0Ch^#2.I8#EtINMoeK`$aW4@0<dTe$lB+OMZP;XMVm=ZM"
"om=ZMECNMM$<32M*:>GM6:>GMJpJVQPL?>#QkBR*B?3/MCrc;-,1h>$%KU[-9(N_&7a^[Mr`POMYulOM>EJ>Mw#88[*o'^#Ht(^#`^dA#`^dA#unjV7unjV7n>QQBZjE>##vf.:$i9^#"
"ZeVe$ZA4.#Eq@NMWl&3M7GXA+<c>>#S#Mc)iEkfM5=(^#Thi;-:<4R-m:4R-CBdD-<TNB/&C.%#oeWRMC]=7M@P$2TYQmA#-CDX-]GU_&np>;%2+d%.949sLL*D/%:M,W-+?Oe$DIG_A"
"1KTq;^l^'%X^eYHN;bYHX.ZsI9r>>#,]M_&NjV&H&H]YMfHTlu2X`=-Z)AB.`d=P&%+TV[8`WV[*&O#$%f%Y-PoE_A4+b%4Ft(^#Ht(^#<i#,asa%,aI<%g;.E3g;HK*F.bCKR*`50RE"
"L2gKMa7hKMT8nTMa8nTMoWkJMF^HLM*HUh$I_]_&),.:2Y5Q_&%DOGMUGPGMc>KVMEaJFMaSmY>8Uh>$-7>k-tgKqVS@.X_%6j'8D?n'8D?n'8C]-LGoJAeZjC`e$OORq;k>%FM`>S(s"
"-.3j(e:ij(7l>>#sb.:2(c2LG.:cEe)G1_]M6mTMw7saM98saMK^$i$qrNR*/p5@0bGf3OOO&FIb6k7I8'cJ(?^kM(QK`#$Q16tLvJ1u$>;cY#W7&F-]]uk-O&Uk=L&Zl&-V/Dt/V/Dt"
"?'&sIXr#sI)'Ufr*)Rfr(01,NhHB,NSiGq;O_>R*=**JMX#PZM@#PZMre3^Mre3^MZj6TMZj6TM.$%]Mfo_@M0OeP/5qqS/8i'sRj(-h-55QwT0Fe/:lL529?19_A1D.:2)Sg?g#p-:2"
"S#Q_&:rsRMBH,OM7^)aM%1^NM?IG,M3%8s$2')m9,=`VIQV^VIth.g)C,-g)K-uiC/)+^#011^#0:)=-g+kB-*mG<-B5j#2DF7%#HZgu$Y=cY#m;08MvNKC-[Qx>-O9RA-uscW-9wGR*"
"NpCd2Il0s-SvMDFSvMDFcj$#Q5=(^#:HrA#4b`=-Ku,D-.$H`-dSj3OHOPR*/$b1#0K;UMB?ut$G;cY#`/A>-S<)=-gRx>-DOxF/iFh/#prp@Ma>9YcHFnEIk?q9Mea+@0IsAKM/xBKM"
"vVkJMAWc]uO?:@-s'3:/Lc7-#A]+WMuXg;MNbQM9S1kP9EqtiCEqtiC*NnS/Xr#sIR`#sI]Qip/>i>>#k9?XC%7CR*h%G<MJr)/UwOLMUwOLMUnwF59[dM#$nrcW-)Hn'8dI-_]_I`MM"
"YOaMMtWJbMDVJbMv&RTMus69MhSiG)8Uh>$H_`=-:a`=-.nG<-^D:@-<T#PMMPgVMb2?`Ma3?`M-NfYM9NfYM:F+RMihZLMi3>cMv1jaMv1jaM4N_SMhK_SM3b/H%e=cY#2WJvL<L/H%"
"OnG<-LTGs-9JoNMvUJbM6(8qLAjV?%SQx>-i``=-c3;.MLa;$%@nG<-CRx>-.lG<-4mG<-?x]GM_J/H%#chMU2iNfLXjX`s;_U_&`Ox?K;8w?Kd-tA=*3R/;DwRe$lgi3OovH,MlxtRM"
"u-s$%lvv`FXanI-GenI--L)a-W^T-Q'0^q;ovH,Mhj%6Mas5/(rDnA#<rcW-28=XCFvHX(nbB-m9VEe?CNaJM9iO7M72^gLp4iS.4oODF]rw?0#jN#$EBdD-:CdD-7h<715$b1#[&#+%"
"HD(v#LBdD-PCdD-^Zuk-mcj'8hpuKcV,Q_&%^2[$0St?0A#mJMMQLfLdlG<-gIg;-2Hg;-X-kB-:,kB-[gDE-=iDE-i_`=-Lg`=-`%kB-O$H`-u4J_Ad$j;-[gDE-C+kB-=iDE-p(('="
"V`;,<;^K/)-B<,<(%N/)m;3^#oC&j-E9]q;',Qq;g#7ou9;cY#XH@:0t1Nc)pt_f:M:Me$=7S_&CNaJMLhO7M^(2s-iQZ3OGd_%4fcaDF$kNe$><Y_&i.>XCbTKk=Jt59MN.TJL-.:B#"
"0;Z3#w@g;-eBg;-0BDX-2>IX(>3<X(Fe5TMev#`M^w#`Mi[#WMq6mWMBwtRM:F+RMr=&bMmuMaMn#KNM#%KNMIj%6M]#6/(Ob9_ABlNX(ftCL,DAJR*wLri080CG)xp@>#,>he$f2NB#"
"E'Q3#[Ne>M_,o4]8qDs%qqDi-@29F.2Kge$6C1^#v(vhLiqJ%#d'L-M[hrr$X?iP0-jjP0HGHK)o[C>#X<#44Xjke$1pWp/^$mxOfa(&PvF12U[^D#$[^D#$4C1'#f#t1#kWQIM/2)5M"
"1ZO%tx(j(t;LlA#/CdD-r[3B-2(^GM$$KNM?@FJMclsqM-p`a-L9qKG9qvIMa`aqM9_`=-rMuG--BdD-k[3B-R&uZ-%tTX(DtTX($17REibx349bje$pincEZZ2^#B;)=-B;)=-A6&F-"
"K:)=-U)^GMS^d'$BBrP-G:)=-B)^GMqbl*$*Y`=-EY]F-0+^GM6sfFM)cQM970dP9;3/T&wG'@0FxNX(D-ke$e0&Z?Z-:R*H(OX((-ge$N12#?dkT&#?:)=-jwA,MiovY>?tD>#9qTfL"
"/Ca:mbYKg-bmDL,Zu_q;IxdlS5W;v-j]-d*6OC'#iF7%#<[R[uVSGs-`ZFOM7>(2#?tWt$EfG<-(X=Z-=9T_&RUJ_MO*TNMo6<MMeQB_MdH'CMTNQ>YG^S5'[htY-,1h>$i/A>-'0A>-"
"$tcW-q33LGS(35&ZrGM0bXMs6E2=gG+YPCDg0J.#*8)XB2o_wEi#,J#/&4A#0BH&M#Rl9#><Q;#65`*MLLHY#9.i?#0tZiLt4DB#4PsD#aY5lLg6;J#o@3/1u6MG#Nr_kBfp=J#<Ig;-"
"oIg;-DIg;-gxH#5P-2iFifKrC[RrH#A4+7Dik%mEd9oI#IHg;-OHg;-UHg;-[Hg;-bHg;-hHg;-nHg;-%3%Q/wc;2FeN`5G$ctA#.sWe$ioTl]WLK`NbMooS,?_'YiJ.4;E/)G`+w:B-"
"MQE>#qI9^#Q%,j$R_x?N0'i)N.sb;NjDE6N,eC)NX'<$#Z%I]-ipa_/;2)uNud#`-wDb_/W@kk4J026NU,w5NKvd5NIjQ5NG^?5NEQ-5N<?wTMTPQ>#GGs+N;?VU#cdP+#bH)i<k8&[8"
"0J*^#v%O_&+I#x74o0AX,T@C-r(.x7oQ3Mg2ZIC-d5+##6tbf(<5`D+0tnP',OV8&/kR5'+F;s%20O2()4Z;%(+?v$*=vV%1'4m'39kM(:#)d*;,D)+x['^#w_B#$(i^>$/.$Z$=pp/)"
"Ls_c)OJuR*2UxQa[T;'#*i?8%*VUV$;wm;%Xm5qV2fFcMAjorQ<Es:Q6L>gLb;RA-+b8,ODKI*Ne[H>#x-b9#gd`8$kUGs-#1ofL+3eR$(SGs-(Qc)Mt616$#%`5/N7P##(l)'M.Z]#M"
"P3O'M(B8#MqIs*M/q::#8>o-#[k#V#gtY<-<nls-5$>wL13`:#r6#F#<X'B#VwjN0]XCv#R9;0$)hovLX?b'MQF./$N-)0$kBlY#ecUv#V0;K$KVAA$IJN)#3aqH/Ld''#TXev7T4<X("
"toRfLq5+5AEFk-$DrNX(W3AkF'wcX($QOw9$lRS%VfhiTXr-/UZ(IJU]4efUl,nA#lB1).5J@bMk(^fLN,(=#EfkI#`A_J#prQK#*MEL#gbDuL^8Y)#]?3L#6r&M#V4KM#Ne>N#?aCxL"
"Y:>d$A7NpBQUUp7_hbA#Q^<s%:%G&#>*W8&xm-x0M*Ms)O0Ms)P3Ms)C7#r)I/>>#^9p^oXtbf(>A`D+@x8v-q)SD=)MV8Aj_4R*)Zsx+%5YY#7q4j'$;2(&/f:s-A^aD#T'x+2x>XV$"
"Ip-)*#5lY#0`:;$J'258snRY>-;_^>E`t(ED5B>#KCt#-`p*&F#?P_&Gjo=#)Vu>#A?X7/M;UJ#d6/vLSLdY#WRuG-wM#<-u5T;-w5T;-#6T;-]-b+.^V_pL1(SQM-#j;$YQE'/@)B>#"
"rQn-$+vnA#CN#<-A6T;-C6T;-KmL5/O9P>#_o,tLYuWuLW_-##vB,+#d0&X#]AS9$AQWa'vi=^]/Bn7$-)d11,Sl##]+78#/OB:#ArdX.7jmC#>FuG-_4fu-QJB`N-lw$$Z#)t-%OvtL"
"Dp)($_GwM0Una1#p0g*#-trUMt`POMl?h*#-)G/2,uX&#rX<x'IBr'#Yl22Khkw7nF21>l2j5(&O3YPJae->mSo>8JaL(&l5m#(&o682Ls-slKr$WPK_@l%l9rB_#<0EVQG?aY#Mt^oe"
"O8w.iPA<JiQJWfiS]8GjTfScjUoo(kVx4Dkll>e6W@pxX:rCJ_Le@G`mIvD%g#.8#n#N<#RIg;-iO,W-<=W_&?rde$;t/L,&>OR*UYUe$;usRM='(SMmE)/#p,jW#k:ToMBwwj3QQk&#"
"x,_'#8vv(#Z+^*#mhc+#=L9o/,Sl##&####Dq8g1td08IfTJI#FWt7I?^nrLmjVcDg3oI#A/5##;:r$#q+E:/&8P>#$m-qL#8-##YF8a+Q#L_&?hDe-p5A>,#####$&####m8At/>H#I"
"";

static const char* GetDefaultCompressedFontDataTTFBase85()
{
    return proggy_clean_ttf_compressed_data_base85;
}
