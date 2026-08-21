#pragma once
#include "imgui.h"

void DrawGameGrid(float w, float h);
void DrawAchievementPanel(float x, float w, float h);
void DrawSettings(float sw, float sh);
void DrawStatusBar(float sw, float sh);

// Shared UI helpers
void ProgressBar(float frac, ImVec2 size, const char* overlay = nullptr);
void ImageOrPlaceholder(unsigned int tex, ImVec2 size, ImVec4 tint = {1,1,1,1});

// Cover texture lookup by game id (reads from cover URL side-map)
unsigned int GetCoverTex(int game_id);
unsigned int GetAchIconTex(int game_id, const char* icon_key);
