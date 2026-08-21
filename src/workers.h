#pragma once
#include <string>

// Load games + achievements from local cache only (fast, no network).
void LoadFromCacheWorker();

// Fetch full game list + achievements from network, cache results.
void FullReloadWorker(int max_id, bool force_redownload);

// Scrape user profile page and auto-check earned achievements.
void SyncProfileWorker(const std::string& username);

// Kick off async fetch of a single game's page (cover + meta).
// Safe to call many times; internally debounces per game id.
void EnsureGameMeta(int game_id, const std::string& slug);
