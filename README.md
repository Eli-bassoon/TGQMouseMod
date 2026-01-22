# TGQMouseMod

A mod for the PS2 and PC ports of *Frogger: The Great Quest* that adds mouse controls to the camera.

[comparison.webm](https://github.com/user-attachments/assets/0dd81f7a-a417-425e-9395-67e709bdfc34)

**NOTICE:** This project is in very early pre-alpha state and no stability is guaranteed. Also, don't freak out when your mouse cursor gets locked to one spot. That is intended and can be disabled.

## Download

The latest version can be downloaded from the [releases page](https://github.com/Eli-bassoon/TGQMouseMod/releases) as TGQMouseMod.zip

## Motivation

> I wish I may, I wish I must, find a princess before I bust!
>
> - Frogger (2001)

*Frogger: The Great Quest* is a 2001 platform game by Konami where you play as a humanoid Frogger and go through numerous zany levels to find a princess. It is mostly known nowadays for being a very bad game, and an even worse Frogger game that dramatically departed from the gameplay of previous installments, to its detriment. There aren't many redeeming qualities about it, but I have a certain fondness for the games that I played a lot as a young child.

The game is notorious for having an awful camera that constantly loses sight of Frogger and clips into walls. The only way to control the camera is to reset it behind Frogger, and the camera is so sluggish that you need to reset it *very* frequently. I have felt for a while that the game would be more enjoyable if you could control the camera yourself. The game is on PC but doesn't make use of the mouse, which seems like a waste. So, I made this mod that gives the player total control over the camera.

## Features

* Gain full control over the camera using your mouse. No more lazy camera!
* Works in both third-person and first-person mode.
* Fixes the majority of the camera clipping into walls.
* In third-person mode, shoot goobers where you aim the camera instead of where Frogger is pointed.
* In fullscreen, shoot goobers by left clicking.

## PC Usage

It is assumed you already can run the PC port of Frogger: The Great Quest on Windows.

There are two methods to use this mod on PC: 1) Cheat Engine, and 2) Standalone DLL injection

### 1) Cheat Engine

1. Download [Cheat Engine](https://www.cheatengine.org)
2. Launch The Great Quest and attach Cheat Engine to the process
3. Load the provided cheat table from the release you downloaded, located at `TGQMouseMod/PC/CheatEngine/TGQMouseMod.CT` after you unpack the zip file. By default no tweaks are enabled.
4. Click the checkbox called "Tweaks - ENABLE ME" to enable all tweaks.

All tweaks can be individually enabled or disabled for testing purposes. Below the tweaks menu are numerous debug options and configuration settings. If you want to change mouse sensitivity you can modify the "Mouse Sensitivity" value, which defaults to 1.

### 2) Standalone DLL

1. Find the standalone DLL and launcher exe in the release you downloaded, located inside `TGQMouseMod/PC/ModLauncher` after you unpack the zip file.
2. Put ModLauncher.exe and TGQMouseMod.dll in the same folder as where you have The Great Quest installed. Make sure you are using the right DLL, which is in the same folder as ModLauncher.exe.
3. Run ModLauncher.exe. **If your Antivirus flags it, it is a false positive.**

You can inject the DLL using any method you like, including Cheat Engine. I have provided an injector if you don't want to download anything else.

## PS2 Usage

There is some setup for the PS2 version, so see [PS2 Usage](./doc/usage_ps2.md).

## Building

### PC

1. Install Visual Studio 22, along with the following packages
   * Desktop Development with C/C++
   * Windows 11 SDK
2. Build the project using Visual Studio

### PS2

1. Extract the .elf file containing the executable part of the game's code from the game disk
2. Download and unpack [C/C++ Game Modding Utility](https://github.com/C0mposer/C-Game-Modding-Utility) by C0mposer
3. Load the project at `TGQMouseMod_PS2/TGQMouseMod.modproj` and select the .elf file as the file to mod
4. Compile the project
5. After compiling, the mod can be directly injected into PCSX2 or exported as a .pnach and loaded as a game patch

## Notes

### PC

This mod works best in fullscreen mode. If you run the game in windowed mode, note that by default **this mod will freeze your mouse cursor to one position.** This is normal, and you can regain control of your mouse by alt-tabbing or just clicking away. If you don't like your mouse being hijacked, you can run the mod with Cheat Engine and set the "Capture Mouse Cursor" value to 0.

I use dgVoodoo2 to make the game run on modern Windows and run the game in windowed mode for testing, along with RivaTuner to limit the FPS to 60. I have not gotten DxWnd to work, so this mod is untested using that method.

By default the game uses arrow keys for movement. I recommend rebinding to WASD. Here are my keybinds for PC:

| Keys                   | Actions         |
| ---------------------- | --------------- |
| WASD                   | Movement        |
| Spacebar               | Jump            |
| Left Click / Left Ctrl | Attack          |
| F                      | Free look       |
| E                      | Interact / Dive |
| R                      | Magic Stone     |
| Q                      | Scroll Stones   |
| Left Shift             | Target / Strafe |
| Tab                    | Inventory       |

### PS2

I do not own a game controller, so the mod is untested on controller.

Many thanks to Kneesnap for discovering that the PS2 version of *The Great Quest* shipped with debug symbols, allowing for a fairly good reconstruction of the source code.
