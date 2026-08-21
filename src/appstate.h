#pragma once
#include "types.h"
#include <map>
#include <set>
#include <mutex>
#include <atomic>
#include <string>
#include <chrono>
#include <unordered_map>

using Clock = std::chrono::steady_clock;

namespace AppState {

// ── Game data ─────────────────────────────────────────────────────────────────
extern std::vector<Game>            games;
extern std::map<int, std::set<int>> progress;   // game_id -> checked ach ids
extern std::mutex                   mtx;

// ── UI state ──────────────────────────────────────────────────────────────────
extern int  selected;       // index into games, -1 = none
extern char search[256];

// ── Fetch state ───────────────────────────────────────────────────────────────
extern std::atomic<bool> fetching;
extern std::atomic<int>  fetch_cur;
extern std::atomic<int>  fetch_tot;

// ── Sync state ────────────────────────────────────────────────────────────────
extern std::atomic<bool>  syncing;
extern int                sync_found_new;
extern Clock::time_point  last_sync;

// ── Misc ──────────────────────────────────────────────────────────────────────
extern bool show_settings;

// Thread-safe status setters
void SetStatus(const std::string& s);
void SetSyncStatus(const std::string& s);
std::string GetStatus();
std::string GetSyncStatus();

} // namespace AppState
