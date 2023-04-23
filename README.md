# Stryke-IL2CPP-Template
A template project for making internal (.dll) Unity IL2CPP hack clients.

# 1 - Intro
This repo is a combination of inspiration and additional code (all attributed) from different repos. It is meant to be a template for creating modules, hooks and a GUI for an internal (.dll) C++ hack client, more specifically, for il2cpp unity games. All the implementations and functions will be explained below.

# 2 - Credit
Not all this code is mine. Well-- most of it is, but heres the references and what I used from them:
<a href="https://github.com/rdbo">rdbo</a> -> ImGui DirectX 11 Kiero Implementation - Gave the functionality of ImGui + MinHook, for the GUI and the hooks of this client. Setup The Basic Framework.
<a href="https://github.com/horionclient/Horion">Horion Client</a> -> Layout - Didn't really steal code from them, but I used their layout, considering they're a well established and known mod utility.

# 3 - Guide Of Template
In these next sections I will be describing to you what is inside of this template, and how you can use it to create hacks, including the classes, functionality, etc. There are also comments scattered throughout to help you understand along the way, sort of like tooltips.
<b>SDK</b>
<b>GUI</b>
<b>Modules</b>
<b>Hooks</b>

# 3.1 - SDK
The SDK is how you implement classes from your il2cpp.h file, and define methods inside your game that you can call inside your mod files
Step 1 - <b>Get Game Contents</b> - Use IL2CPPDumper to dump your games info, and paste 'il2cpp.h' contents into 'SDK/src/il2cpp.h' file. From here you will have access to all the types inside of your game.
Step 2 - <b>Implement Class</b> - In the stryke SDK, there is an example implementation. Broken down, it is creating your own class from the template of the il2cpp.h file. Look at CExample.cpp & CExample.h to understand.
Step 3 - <b>Define Nethods</b> - Within your implemented class, define methods that are found within your game, in this class. See CExample.cpp & CExample.h to understand.

# 3.2 - GUI
The GUI is how you draw and toggle your modules. This GUI is drawn with <a href="https://github.com/ocornut/imgui">ImGui</a>. The class for the GUI is within ClickGui.cpp/ClickGui.h, and the theme settings are within css.cpp/css.h. They both have their own classes and methods to get/set colors and sizes, etc. Look at files for more details.

# 3.3 - Modules
The modules are created as classes that inherit from the Stryke Module class, 'SModule'. This class can be found inside 'Module.cpp'/'Module.h', look inside for more details. Inside these modules, there are 3 main methods:
<b>onEnable</b> - This method is called on the first frame that the given module is enabled. Usually inside this method you won't do anything related to the actual hack, you should mainly focus on checking if the hook(s) / classes you are using for this mod are valid pointers, and if not, immediately disabling as not to crash. 
<b>onFrame</b> - This method is called every frame the mod is enabled. This is usually where the main cheat implementation goes. Every time before doing the hack in this, however, you should ensure, again, that the hook(s) / classes you are using for this mod are valid pointers, and if not, immediately disabling as not to crash.
<b>onDisable</b> - This method is called when the mod is disabled. It becomes the final call before the mod is enabled again. In this you should 'unhack' everything, or set everything back to normal.

# 3.4 - Hooks
The hooks are where you can use your implemented classes (<b>3.1 - SDK</b>) to get their instances within the game you're hacking. This hooking is with MinHook.
Step 1 - <b>Create Detour Functions</b> - When you hook, you want to reroute the function you hook, and get the types found within that function. You want to store these types somewhere, and in the 'Hooks' class, you can define areas to store them.
Step 2 - <b>Enable / Disable Functions</b> - Inside of the 'Hook' class, there is an 'AllHooks' functions, that can enable/disable all hooks. You need to create the definition for enabling all your hooks and where they should detour, but the disabling is taken care of. Additionally, in the Loader.cpp with the DllMain, you should MH_Initialize() to initialize MinHook, and enable all hooks with the 'AllHooks()' function. (This is already done, just to remember).
