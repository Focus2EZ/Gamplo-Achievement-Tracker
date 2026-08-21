#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

#include "ui.h"
#include "appstate.h"
#include "theme.h"
#include "texture.h"
#include "storage.h"
#include "workers.h"
#include "api.h"

#include "imgui.h"
#include <string>
#include <algorithm>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <filesystem>

// ── UI helpers ────────────────────────────────────────────────────────────────

void ProgressBar(float frac, ImVec2 size, const char* overlay) {
    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 p2 = {p.x+size.x, p.y+size.y};
    dl->AddRectFilled(p, p2, IM_COL32(20,20,50,200), 3.f);
    if (frac > 0.f) {
        ImVec4 col = (frac >= 1.f) ? ga.gold : ga.green;
        dl->AddRectFilled(p, {p.x+size.x*frac, p.y+size.y},
            ImGui::ColorConvertFloat4ToU32(col), 3.f);
    }
    dl->AddRect(p, p2, IM_COL32(80,80,120,180), 3.f, ImDrawFlags_None, 1.f);
    if (overlay) {
        ImVec2 ts = ImGui::CalcTextSize(overlay);
        dl->AddText({p.x+(size.x-ts.x)*.5f, p.y+(size.y-ts.y)*.5f},
            IM_COL32(255,255,255,220), overlay);
    }
    ImGui::Dummy(size);
}

void ImageOrPlaceholder(unsigned int tex, ImVec2 size, ImVec4 tint) {
    if (tex) {
        ImGui::ImageWithBg((ImTextureID)(uintptr_t)tex, size, {0,0}, {1,1}, {0,0,0,0}, tint);
    } else {
        auto* dl = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        dl->AddRectFilled(p, {p.x+size.x,p.y+size.y}, IM_COL32(50,50,60,200), 4.f);
        dl->AddRect(p, {p.x+size.x,p.y+size.y}, IM_COL32(80,80,90,200), 4.f, ImDrawFlags_None, 1.f);
        ImGui::Dummy(size);
    }
}

unsigned int GetCoverTex(int game_id) {
    return GetTex(GameCoverUrl(game_id), "cover_" + std::to_string(game_id));
}

unsigned int GetAchIconTex(int game_id, const char* icon_key) {
    std::string url = AchIconUrl(game_id, icon_key);
    std::string ck  = "ach_" + std::to_string(game_id) + "_" + icon_key;
    return GetTex(url, ck);
}

// ── Game grid panel ───────────────────────────────────────────────────────────

void DrawGameGrid(float w, float h) {
    ImGui::SetNextWindowPos({0,0});
    ImGui::SetNextWindowSize({w,h});
    ImGui::Begin("##games", nullptr,
        ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|
        ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus|
        ImGuiWindowFlags_NoScrollbar);

    ImGui::PushStyleColor(ImGuiCol_Text, ga.gold);
    ImGui::SetWindowFontScale(1.3f);
    ImGui::TextUnformatted("GAMPLO");
    ImGui::SetWindowFontScale(1.f);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextDisabled("Achievement Tracker");
    ImGui::Separator(); ImGui::Spacing();

    // Search + toolbar
    ImGui::SetNextItemWidth(w - 155.f);
    ImGui::InputTextWithHint("##search", "Search games...",
        AppState::search, sizeof(AppState::search));
    ImGui::SameLine();
    if (ImGui::Button("Settings", {80,0})) AppState::show_settings = true;
    ImGui::SameLine();
    bool busy = AppState::fetching;
    if (busy) ImGui::BeginDisabled();
    if (ImGui::Button("↺", {28,0})) {
        AppState::selected = -1;
        std::thread([]{ LoadFromCacheWorker(); }).detach();
    }
    if (busy) ImGui::EndDisabled();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reload from cache");
    ImGui::Spacing();

    // Sync status
    auto ss = AppState::GetSyncStatus();
    if (!ss.empty()) {
        ImGui::TextColored(ga.accent, "⟳ %s", ss.c_str());
        ImGui::Spacing();
    }

    // Build filter
    std::string filter(AppState::search);
    std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

    // Grid: 2 columns
    const float CARD_W = (w - 36.f) / 2.f;
    const float IMG_W  = 96.f;
    const float IMG_H  = 54.f;   // real capsule art is 1080x608 (~16:9)

    ImGui::BeginChild("##grid", {w-4.f, h - ImGui::GetCursorPosY() - 8.f}, false,
        ImGuiWindowFlags_NoScrollbar);

    std::lock_guard<std::mutex> lk(AppState::mtx);
    int col = 0;
    for (int i = 0; i < (int)AppState::games.size(); i++) {
        Game& gm = AppState::games[i];

        if (!filter.empty()) {
            std::string lo = gm.name;
            std::transform(lo.begin(), lo.end(), lo.begin(), ::tolower);
            if (lo.find(filter) == std::string::npos) continue;
        }

        if (g_cfg.filter_has_ach && gm.achievements.empty()) continue;

        // Kick off cover/meta load lazily
        if (!gm.cover_tried) EnsureGameMeta(gm.id, gm.slug);

        bool sel = (AppState::selected == i);
        if (col == 1) ImGui::SameLine(CARD_W + 20.f);

        ImGui::PushID(i);
        ImVec4 card_bg = sel ? ga.selBg : ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, card_bg);

        // Height = just enough for the text block (title/dev/release/progress),
        // never shorter than the cover image.
        float line_h    = ImGui::GetTextLineHeightWithSpacing();
        float content_h = line_h;                                  // title
        if (!gm.developer.empty())    content_h += line_h;
        if (!gm.release_date.empty()) content_h += line_h;
        content_h += ImGui::GetStyle().ItemSpacing.y;               // pre-progress Spacing()
        content_h += gm.achievements.empty() ? line_h : 14.f;       // "No data" line / progress bar
        float CARD_H = std::max(IMG_H, content_h) + ImGui::GetStyle().WindowPadding.y * 2.f;

        ImGui::BeginChild(("##c"+std::to_string(i)).c_str(), {CARD_W,CARD_H}, true,
            ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);

        // Cover
        ImageOrPlaceholder(GetCoverTex(gm.id), {IMG_W, IMG_H});
        ImGui::SameLine();

        // Info
        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_Text, sel ? ga.gold : ImGui::GetStyleColorVec4(ImGuiCol_Text));
        std::string dn = gm.name.size() > 22 ? gm.name.substr(0,20)+".." : gm.name;
        ImGui::TextUnformatted(dn.c_str());
        ImGui::PopStyleColor();
        if (!gm.developer.empty())    ImGui::TextDisabled("%s", gm.developer.c_str());
        if (!gm.release_date.empty()) ImGui::TextDisabled("%s", gm.release_date.c_str());

        int tot=0, done=0;
        for (auto& a : gm.achievements) { tot++; if (a.checked) done++; }
        ImGui::Spacing();
        if (tot > 0) {
            char ov[16]; snprintf(ov,sizeof(ov),"%d/%d",done,tot);
            ProgressBar((float)done/tot, {CARD_W-IMG_W-26.f, 10.f}, ov);
        } else {
            ImGui::TextDisabled("No data");
        }
        ImGui::EndGroup();

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) &&
            ImGui::IsMouseClicked(0))
            AppState::selected = i;

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopID();

        col++;
        if (col >= 2) { col = 0; ImGui::Spacing(); }
    }
    ImGui::EndChild();
    ImGui::End();
}

// ── Achievement panel ─────────────────────────────────────────────────────────

void DrawAchievementPanel(float x, float w, float h) {
    ImGui::SetNextWindowPos({x,0});
    ImGui::SetNextWindowSize({w,h});
    ImGui::Begin("##ach", nullptr,
        ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|
        ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus|
        ImGuiWindowFlags_NoScrollbar);

    if (AppState::selected < 0) {
        auto av = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPos({av.x/2-130.f, av.y/2-10.f});
        ImGui::TextDisabled("<- Select a game to view achievements");
        ImGui::End(); return;
    }

    std::lock_guard<std::mutex> lk(AppState::mtx);
    if (AppState::selected >= (int)AppState::games.size()) { ImGui::End(); return; }
    Game& gm = AppState::games[AppState::selected];

    // Open-in-browser button, top-right corner of the header
    {
        ImVec2 top = ImGui::GetCursorPos();
        const ImVec2 btn_sz = {150.f, 0.f};
        ImGui::SetCursorPos({w - btn_sz.x - 20.f, top.y});
        if (ImGui::Button("Open in Browser", btn_sz)) {
            std::string url = CoverPageUrl(gm.id, gm.slug);
            ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        }
        ImGui::SetCursorPos(top);
    }

    // Header: cover + game info
    const float COVER_W = 180.f, COVER_H = 101.f;   // real capsule art is 1080x608 (~16:9)
    ImageOrPlaceholder(GetCoverTex(gm.id), {COVER_W, COVER_H});
    ImGui::SameLine();
    ImGui::BeginGroup();

    ImGui::PushStyleColor(ImGuiCol_Text, ga.gold);
    ImGui::SetWindowFontScale(1.2f);
    ImGui::TextUnformatted(gm.name.c_str());
    ImGui::SetWindowFontScale(1.f);
    ImGui::PopStyleColor();

    if (!gm.developer.empty())    ImGui::TextDisabled("Dev: %s", gm.developer.c_str());
    if (!gm.release_date.empty()) ImGui::TextDisabled("Released: %s", gm.release_date.c_str());

    int tot=0,done=0,tot_pts=0,done_pts=0;
    for (auto& a : gm.achievements) {
        tot++; tot_pts+=a.points;
        if (a.checked) { done++; done_pts+=a.points; }
    }
    if (tot > 0)
        ImGui::TextColored(done==tot ? ga.gold : ImGui::GetStyleColorVec4(ImGuiCol_Text),
            "%d / %d achievements", done, tot);
    if (tot_pts > 0)
        ImGui::TextColored(ga.gold, "%d / %d pts", done_pts, tot_pts);

    ImGui::EndGroup();

    if (tot > 0) {
        char ov[8]; snprintf(ov,sizeof(ov),"%.0f%%",(float)done/tot*100.f);
        ProgressBar((float)done/tot, {w-28.f,14.f}, ov);
    }
    if (!gm.description.empty())
        ImGui::TextDisabled("%s", gm.description.c_str());

    ImGui::Separator(); ImGui::Spacing();

    // Filter + bulk buttons
    static int filt = 0;
    ImGui::TextDisabled("Show:"); ImGui::SameLine();
    if (ImGui::RadioButton("All",      filt==0)) filt=0; ImGui::SameLine();
    if (ImGui::RadioButton("Locked",   filt==1)) filt=1; ImGui::SameLine();
    if (ImGui::RadioButton("Unlocked", filt==2)) filt=2;
    ImGui::SameLine(w-220.f);
    if (ImGui::SmallButton("Check All")) {
        for (auto& a : gm.achievements) { a.checked=true; AppState::progress[gm.id].insert(a.id); }
        SaveProgress(AppState::progress);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Uncheck All")) {
        for (auto& a : gm.achievements) a.checked=false;
        AppState::progress.erase(gm.id);
        SaveProgress(AppState::progress);
    }
    ImGui::Spacing();

    ImGui::BeginChild("##achlist", {w-16.f, h-ImGui::GetCursorPosY()-8.f}, false,
        ImGuiWindowFlags_NoScrollbar);

    if (!gm.loaded && gm.achievements.empty())
        ImGui::TextDisabled("No cached data for this game.");

    const float ICON_SZ = 48.f;

    for (auto& ach : gm.achievements) {
        if (filt==1 && ach.checked)  continue;
        if (filt==2 && !ach.checked) continue;
        ImGui::PushID(ach.id);

        ImVec4 bg = ach.checked
            ? ga.greenDim
            : ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);

        float card_h = ach.description.empty() ? 64.f : 84.f;
        ImGui::BeginChild(("##a"+std::to_string(ach.id)).c_str(), {w-28.f,card_h}, true,
            ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);

        // Icon (greyed out if locked)
        ImVec4 tint = ach.checked ? ImVec4{1,1,1,1} : ImVec4{0.45f,0.45f,0.45f,0.8f};
        ImageOrPlaceholder(GetAchIconTex(gm.id, ach.icon_key.c_str()), {ICON_SZ,ICON_SZ}, tint);
        ImGui::SameLine();

        ImGui::BeginGroup();
        bool prev = ach.checked;
        if (ImGui::Checkbox("##c", &ach.checked) && ach.checked != prev) {
            if (ach.checked) AppState::progress[gm.id].insert(ach.id);
            else             AppState::progress[gm.id].erase(ach.id);
            SaveProgress(AppState::progress);
        }
        ImGui::SameLine();
        ImGui::TextColored(ach.checked ? ga.green : ImGui::GetStyleColorVec4(ImGuiCol_Text),
            "%s", ach.name.c_str());
        if (ach.earned_online) { ImGui::SameLine(); ImGui::TextColored(ga.accent,"(synced)"); }
        if (!ach.description.empty())
            ImGui::TextDisabled("  %s", ach.description.c_str());
        ImGui::EndGroup();

        if (ach.points > 0) {
            char pts[16]; snprintf(pts,sizeof(pts),"%d pts",ach.points);
            ImGui::SameLine(w - 56.f - ImGui::CalcTextSize(pts).x);
            ImGui::TextColored(ach.checked ? ga.gold : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled),
                "%s", pts);
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopID();
        ImGui::Spacing();
    }
    ImGui::EndChild();
    ImGui::End();
}

// ── Settings panel ────────────────────────────────────────────────────────────

void DrawSettings(float sw, float sh) {
    if (!AppState::show_settings) return;
    ImGui::SetNextWindowSize({520,500}, ImGuiCond_Always);
    ImGui::SetNextWindowPos({sw/2-260.f, sh/2-250.f}, ImGuiCond_Always);
    ImGui::Begin("Settings", &AppState::show_settings,
        ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse);

    // ── Profile sync ──────────────────────────────────────────
    ImGui::TextColored(ga.gold, "Profile Sync");
    ImGui::Separator(); ImGui::Spacing();

    static char uname_buf[64] = {};
    static bool uname_init = false;
    if (!uname_init) { strncpy_s(uname_buf, g_cfg.username.c_str(), 63); uname_init=true; }

    ImGui::Text("Gamplo username:");
    ImGui::SetNextItemWidth(200);
    ImGui::InputText("##uname", uname_buf, sizeof(uname_buf));
    ImGui::SameLine();
    if (ImGui::Button("Save##u")) { g_cfg.username=uname_buf; SaveConfig(); }
    ImGui::SameLine();
    bool syncing = AppState::syncing;
    if (syncing) ImGui::BeginDisabled();
    if (ImGui::Button("Sync Now")) {
        AppState::syncing = true;
        std::string u = uname_buf;
        std::thread([u]{ SyncProfileWorker(u); }).detach();
    }
    if (syncing) ImGui::EndDisabled();

    ImGui::Checkbox("Auto-sync periodically", &g_cfg.auto_sync);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::InputInt("sec##si", &g_cfg.sync_interval_s, 30, 300);
    if (g_cfg.sync_interval_s < 60) g_cfg.sync_interval_s = 60;

    auto ss = AppState::GetSyncStatus();
    if (!ss.empty()) ImGui::TextColored(ga.accent, "Status: %s", ss.c_str());

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    // ── Appearance ────────────────────────────────────────────
    ImGui::TextColored(ga.gold, "Appearance");
    ImGui::Separator(); ImGui::Spacing();

    ImGui::Text("Theme:");
    ImGui::SetNextItemWidth(220);
    if (ImGui::BeginCombo("##theme", THEME_NAMES[g_cfg.theme_idx])) {
        for (int i = 0; i < THEME_COUNT; i++) {
            bool sel = (g_cfg.theme_idx == i);
            if (ImGui::Selectable(THEME_NAMES[i], sel)) {
                g_cfg.theme_idx = i;
                ApplyTheme(g_cfg.theme_idx);
                SaveConfig();
            }
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    // ── Data management ───────────────────────────────────────
    ImGui::TextColored(ga.gold, "Data Management");
    ImGui::Separator(); ImGui::Spacing();

    ImGui::Text("Max game ID to scan:");
    ImGui::SetNextItemWidth(160);
    ImGui::SliderInt("##mid", &g_cfg.max_id, 10, 2000);
    ImGui::Checkbox("Use cache when available", &g_cfg.use_cache);
    if (ImGui::Checkbox("Only show games with achievements", &g_cfg.filter_has_ach))
        SaveConfig();
    ImGui::TextDisabled("Cache: data\\");

    auto st = AppState::GetStatus();
    ImGui::TextDisabled("Status: %s", st.c_str());

    bool busy = AppState::fetching;
    if (busy && AppState::fetch_tot > 0) {
        float frac = (float)AppState::fetch_cur / (float)AppState::fetch_tot;
        char ov[32]; snprintf(ov,sizeof(ov),"%d/%d",(int)AppState::fetch_cur,(int)AppState::fetch_tot);
        ProgressBar(frac, {ImGui::GetContentRegionAvail().x, 18.f}, ov);
    }
    ImGui::Spacing();
    if (busy) ImGui::BeginDisabled();
    if (ImGui::Button("Reload from Cache", {200,32})) {
        AppState::selected = -1;
        std::thread([]{ LoadFromCacheWorker(); }).detach();
    }
    ImGui::SameLine();
    if (ImGui::Button("Full Reload (Network)", {200,32})) {
        AppState::selected = -1;
        int mid = g_cfg.max_id; bool fc = !g_cfg.use_cache;
        std::thread([mid,fc]{ FullReloadWorker(mid,fc); }).detach();
    }
    ImGui::Spacing();
    if (ImGui::Button("Force Re-download All", {200,32})) {
        AppState::selected = -1;
        int mid = g_cfg.max_id;
        std::thread([mid]{ FullReloadWorker(mid,true); }).detach();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Cache Files", {200,32}))
        ImGui::OpenPopup("ConfirmClear");
    if (busy) ImGui::EndDisabled();

    if (ImGui::BeginPopupModal("ConfirmClear", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored({1,.3f,.3f,1}, "Delete all cached game/image data?");
        ImGui::Text("Progress (checked achievements) will be kept.");
        ImGui::Spacing();
        if (ImGui::Button("Yes, clear", {120,0})) {
            namespace fs = std::filesystem;
            if (fs::exists(DATA_DIR))
                for (auto& e : fs::directory_iterator(DATA_DIR))
                    if (e.path().filename()!="progress.json" &&
                        e.path().filename()!="config.json")
                        fs::remove_all(e.path());
            AppState::games.clear();
            AppState::selected = -1;
            ClearTexCache();
            AppState::SetStatus("Cache cleared.");
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", {80,0})) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    if (ImGui::Button("Save & Close", {150,0})) {
        g_cfg.username = uname_buf;
        SaveConfig();
        AppState::show_settings = false;
    }

    ImGui::End();
}

// ── Status bar ────────────────────────────────────────────────────────────────

void DrawStatusBar(float sw, float sh) {
    ImGui::SetNextWindowPos({0, sh-22.f});
    ImGui::SetNextWindowSize({sw, 22.f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {6,2});
    ImGui::Begin("##sb", nullptr,
        ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|
        ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|
        ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopStyleVar();

    if (AppState::fetching) {
        ImGui::TextColored(ga.accent, "● Fetching");
        int cur=AppState::fetch_cur, tot=AppState::fetch_tot;
        if (tot > 0) { ImGui::SameLine(); ProgressBar((float)cur/tot, {100,13}); }
        ImGui::SameLine();
    } else {
        ImGui::TextColored(ga.green, "●"); ImGui::SameLine();
    }
    ImGui::TextDisabled("%s", AppState::GetStatus().c_str());

    {
        std::lock_guard<std::mutex> lk(AppState::mtx);
        char gc[32]; snprintf(gc,sizeof(gc),"%zu games",(size_t)AppState::games.size());
        ImGui::SameLine(sw - ImGui::CalcTextSize(gc).x - 16.f);
        ImGui::TextDisabled("%s", gc);
    }
    ImGui::End();
}
