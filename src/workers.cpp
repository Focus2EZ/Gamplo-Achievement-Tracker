#include "workers.h"
#include "appstate.h"
#include "api.h"
#include "storage.h"
#include "http.h"
#include "texture.h"

#include <thread>
#include <algorithm>
#include <filesystem>
#include <set>
#include <unordered_set>
#include <mutex>

namespace fs = std::filesystem;

// ── EnsureGameMeta ────────────────────────────────────────────────────────────

static std::unordered_set<int> s_meta_started;
static std::mutex              s_meta_mtx;

void EnsureGameMeta(int game_id, const std::string& slug) {
    {
        std::lock_guard<std::mutex> lk(s_meta_mtx);
        if (s_meta_started.count(game_id)) return;
        s_meta_started.insert(game_id);
    }

    std::thread([game_id, slug]() {
        std::string raw, err;
        if (!HttpGet(CoverPageUrl(game_id, slug), raw, err)) return;

        // Parse meta (name/description/developer/release date).
        // Cover art is NOT taken from here — not every game page sets an
        // og:image meta tag, so it's fetched from the deterministic capsule
        // URL instead (kicked off up front in DrawGameGrid/DrawAchievementPanel
        // via GetCoverTex). This just keeps the texture cache warm.
        Game tmp; tmp.id = game_id;
        ScrapeGameMeta(raw, tmp);
        LoadTexAsync(GameCoverUrl(game_id), "cover_" + std::to_string(game_id));

        // Patch name/description/developer back into the real game
        {
            std::lock_guard<std::mutex> lk(AppState::mtx);
            for (auto& gm : AppState::games) {
                if (gm.id != game_id) continue;
                if (!tmp.name.empty())        gm.name        = tmp.name;
                if (!tmp.description.empty()) gm.description = tmp.description;
                if (!tmp.developer.empty())   gm.developer   = tmp.developer;
                if (!tmp.release_date.empty())gm.release_date= tmp.release_date;
                gm.cover_tried = true;
                break;
            }
        }
    }).detach();
}

// ── Helpers ───────────────────────────────────────────────────────────────────

static void ApplyProgress(std::vector<Game>& games) {
    for (auto& gm : games)
        for (auto& a : gm.achievements)
            if (AppState::progress.count(gm.id) &&
                AppState::progress[gm.id].count(a.id))
                a.checked = true;
}

// ── LoadFromCacheWorker ───────────────────────────────────────────────────────

void LoadFromCacheWorker() {
    AppState::fetching = true;
    AppState::progress = LoadProgress();
    std::vector<Game> games;

    std::string raw;
    if (ReadText(LIST_FILE, raw)) ScrapeGamesList(raw, games);

    if (games.empty() && fs::exists(DATA_DIR)) {
        for (auto& e : fs::directory_iterator(DATA_DIR)) {
            auto name = e.path().filename().string();
            if (name.rfind("game_", 0) != 0) continue;
            if (name.size() < 11) continue; // "game_" + at least 1 digit + ".json"
            try {
                int id = std::stoi(name.substr(5, name.size() - 10));
                Game gm; gm.id = id; gm.name = "Game " + std::to_string(id);
                games.push_back(gm);
            } catch (...) {}
        }
    }

    for (auto& gm : games) {
        std::string araw;
        if (ReadText(GameCachePath(gm.id), araw)) {
            ParseAchievements(araw, gm.id, gm.achievements);
            gm.loaded = !gm.achievements.empty();
        }
    }

    std::sort(games.begin(), games.end(),
        [](const Game& a, const Game& b){ return a.name < b.name; });
    ApplyProgress(games);

    {
        std::lock_guard<std::mutex> lk(AppState::mtx);
        AppState::games = std::move(games);
    }
    AppState::SetStatus(AppState::games.empty()
        ? "No cache — open Settings and click Full Reload."
        : "Loaded " + std::to_string(AppState::games.size()) + " games from cache.");
    AppState::fetching = false;
}

// ── FullReloadWorker ──────────────────────────────────────────────────────────

void FullReloadWorker(int max_id, bool force) {
    AppState::fetching = true;
    AppState::fetch_cur = 0;
    AppState::fetch_tot = max_id;

    std::vector<Game> games;

    // Try /games page
    {
        std::string raw, err, cached;
        bool ok = false;
        if (!force && ReadText(LIST_FILE, cached)) ok = ScrapeGamesList(cached, games);
        if (!ok) {
            if (HttpGet(BASE_URL + "/games", raw, err)) {
                WriteText(LIST_FILE, raw);
                ok = ScrapeGamesList(raw, games);
            }
        }

        // Fallback: probe IDs
        if (!ok || games.empty()) {
            games.clear();
            AppState::SetStatus("Probing IDs 1.." + std::to_string(max_id) + "...");
            for (int id = 1; id <= max_id && AppState::fetching; id++) {
                AppState::fetch_cur = id;
                std::string araw, aerr;
                bool cached2 = (!force && ReadText(GameCachePath(id), araw));
                if (!cached2) {
                    araw.clear();
                    if (!HttpGet(BASE_URL + "/api/game/" + std::to_string(id) + "/achievements", araw, aerr))
                        continue;
                    WriteText(GameCachePath(id), araw);
                }
                std::vector<Achievement> test;
                if (!ParseAchievements(araw, id, test)) continue;
                Game gm; gm.id = id; gm.name = "Game " + std::to_string(id);
                gm.achievements = std::move(test); gm.loaded = true;
                games.push_back(std::move(gm));
            }
        }
    }

    // Fetch achievements for games from list that don't have them yet
    AppState::fetch_tot = (int)games.size();
    AppState::fetch_cur = 0;

    for (auto& gm : games) {
        if (!AppState::fetching) break;
        AppState::fetch_cur++;
        AppState::SetStatus("Loading: " + gm.name);

        if (!gm.loaded) {
            std::string araw, aerr;
            bool cached = (!force && ReadText(GameCachePath(gm.id), araw));
            if (!cached) {
                araw.clear();
                if (!HttpGet(BASE_URL + "/api/game/" + std::to_string(gm.id) + "/achievements", araw, aerr))
                    continue;
                WriteText(GameCachePath(gm.id), araw);
            }
            ParseAchievements(araw, gm.id, gm.achievements);
            gm.loaded = true;
        }

        // Fetch game page for cover + name (fire off async — don't block the loop)
        EnsureGameMeta(gm.id, gm.slug);
    }

    std::sort(games.begin(), games.end(),
        [](const Game& a, const Game& b){ return a.name < b.name; });
    AppState::progress = LoadProgress();
    ApplyProgress(games);

    {
        std::lock_guard<std::mutex> lk(AppState::mtx);
        AppState::games = std::move(games);
    }
    AppState::SetStatus("Done — " + std::to_string(AppState::games.size()) + " games loaded.");
    AppState::fetching = false;
}

// ── SyncProfileWorker ─────────────────────────────────────────────────────────

void SyncProfileWorker(const std::string& username) {
    if (username.empty()) {
        AppState::SetSyncStatus("No username set.");
        AppState::syncing = false; return;
    }
    AppState::SetSyncStatus("Syncing " + username + "...");

    std::string raw, err;
    if (!HttpGet(BASE_URL + "/profile/" + username, raw, err)) {
        AppState::SetSyncStatus("Sync failed: " + err);
        AppState::syncing = false; return;
    }

    auto earned = ScrapeUserAchievements(raw);
    if (earned.empty()) {
        AppState::SetSyncStatus("No achievements found for " + username);
        AppState::syncing = false; return;
    }

    int newly = 0;
    {
        std::lock_guard<std::mutex> lk(AppState::mtx);
        for (auto& gm : AppState::games) {
            for (auto& ach : gm.achievements) {
                if (ach.checked) continue;
                if (earned.count({gm.id, ach.icon_key})) {
                    ach.checked = true;
                    ach.earned_online = true;
                    AppState::progress[gm.id].insert(ach.id);
                    newly++;
                }
            }
        }
        if (newly > 0) SaveProgress(AppState::progress);
    }

    AppState::sync_found_new = newly;
    AppState::last_sync = std::chrono::steady_clock::now();
    AppState::SetSyncStatus("Synced! +" + std::to_string(newly) + " new achievements.");
    AppState::syncing = false;
}
