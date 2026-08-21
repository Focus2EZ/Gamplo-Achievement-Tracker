#pragma once
#include "types.h"

static const char* THEME_NAMES[] = {
    "ImGui Dark", "ImGui Light", "ImGui Classic",
    "Catppuccin Mocha", "Catppuccin Latte",
    "Nord", "Dracula", "Gruvbox Dark"
};
static const int THEME_COUNT = 8;

void ApplyTheme(int idx);

// Helper used everywhere
inline ImVec4 Hex(uint32_t h, float a = 1.f) {
    return { ((h>>16)&0xFF)/255.f, ((h>>8)&0xFF)/255.f, (h&0xFF)/255.f, a };
}
