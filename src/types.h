#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <SDL_opengl.h>
#include "imgui.h"

struct Achievement {
    int         id = 0;
    std::string name;
    std::string description;
    std::string icon_key;
    int         points = 0;
    bool        checked = false;
    bool        earned_online = false;
};

struct Game {
    int         id = 0;
    std::string name;
    std::string slug;
    std::string description;
    std::string developer;
    std::string release_date;
    std::vector<Achievement> achievements;
    bool        loaded = false;
    bool        cover_tried = false;
};

struct Config {
    std::string username;
    int         theme_idx       = 0;
    int         max_id          = 1300;
    bool        use_cache       = true;
    int         sync_interval_s = 300;
    bool        auto_sync       = true;
    bool        filter_has_ach  = false;   // only show games that have achievements
};

struct ThemeAccents {
    ImVec4 accent, accentHov, accentAct;
    ImVec4 gold, green, greenDim, selBg;
};

// ── Global singletons (defined in main.cpp) ───────────────────────────────────
extern Config      g_cfg;
extern ThemeAccents ga;
