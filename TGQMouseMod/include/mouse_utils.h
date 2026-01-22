#pragma once

#include <utility>

void __declspec(dllexport) __stdcall storeMousePos();

int __declspec(dllexport) __stdcall storeMouseClick();

std::pair<float, float> getCameraDeltas();