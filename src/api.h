#pragma once
#include "types.h"
#include <set>
#include <utility>

extern const std::string BASE_URL;

// Parse /games HTML -> list of games (id, name, slug)
bool ScrapeGamesList(const std::string& html, std::vector<Game>& out);

// Parse /api/game/{id}/achievements JSON -> achievements
bool ParseAchievements(const std::string& raw, int game_id, std::vector<Achievement>& out);

// Scrape /profile/{user} HTML -> set of (game_id, icon_key) earned
std::set<std::pair<int,std::string>> ScrapeUserAchievements(const std::string& html);

// Scrape /game/{id}/{slug} HTML and fill meta fields (cover_url, description, developer, etc.)
// Returns the og:image cover URL (empty on failure).
std::string ScrapeGameMeta(const std::string& html, Game& gm);

// Build URLs
std::string AchIconUrl(int game_id, const std::string& icon_key);
std::string CoverPageUrl(int game_id, const std::string& slug);

// Direct, deterministic game icon/cover URL — doesn't require scraping the
// game page. Not every game page sets an og:image meta tag, but every game
// has a capsule image at this path, so use it instead of ScrapeGameMeta's
// (unreliable) cover_url.
std::string GameCoverUrl(int game_id);
