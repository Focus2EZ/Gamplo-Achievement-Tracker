#include "api.h"
#include <regex>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const std::string BASE_URL = "https://gamplo.com";

// ── URL helpers ───────────────────────────────────────────────────────────────

std::string AchIconUrl(int game_id, const std::string& icon_key) {
    return BASE_URL + "/api/files/achievements/" + std::to_string(game_id) + "/" + icon_key + ".webp";
}

std::string CoverPageUrl(int game_id, const std::string& slug) {
    return BASE_URL + "/game/" + std::to_string(game_id) + (slug.empty() ? "" : "/" + slug);
}

std::string GameCoverUrl(int game_id) {
    return BASE_URL + "/api/files/games/" + std::to_string(game_id) + "/capsule.webp";
}

// ── Scrape /games HTML ────────────────────────────────────────────────────────

bool ScrapeGamesList(const std::string& html, std::vector<Game>& out) {
    // Matches href="/game/55/Nomio" in anchor tags
    std::regex re(R"(/game/(\d+)/([^"#\s]+))");
    std::set<int> seen;
    for (auto it = std::sregex_iterator(html.begin(), html.end(), re);
         it != std::sregex_iterator(); ++it)
    {
        int id = std::stoi((*it)[1].str());
        if (seen.count(id)) continue;
        seen.insert(id);
        Game g;
        g.id   = id;
        g.slug = (*it)[2].str();
        // Name: strip hyphens from slug as fallback; gets overwritten by meta fetch
        g.name = g.slug;
        for (auto& c : g.name) if (c == '-') c = ' ';
        out.push_back(std::move(g));
    }
    return !out.empty();
}

// ── Parse achievement JSON ────────────────────────────────────────────────────

static std::string IconKeyFromUrl(const std::string& url) {
    auto pos = url.rfind('/');
    std::string fn = (pos == std::string::npos) ? url : url.substr(pos + 1);
    auto q = fn.find('?'); if (q != std::string::npos) fn = fn.substr(0, q);
    auto dot = fn.rfind('.'); if (dot != std::string::npos) fn = fn.substr(0, dot);
    return fn;
}

bool ParseAchievements(const std::string& raw, int game_id, std::vector<Achievement>& out) {
    try {
        auto j = json::parse(raw);
        auto arr = j.is_array() ? j
            : j.contains("achievements") ? j["achievements"]
            : j.contains("data")         ? j["data"]
            : json::array();
        for (auto& a : arr) {
            Achievement ach;
            ach.id          = a.value("id",          0);
            ach.name        = a.value("name",        a.value("title", "?"));
            ach.description = a.value("description", a.value("desc",  ""));
            ach.points      = a.value("points",      a.value("xp",     0));

            // Icon key: from explicit fields first, then derive from name
            std::string icon_url = a.value("icon", a.value("badge", a.value("image", std::string(""))));
            if (!icon_url.empty()) {
                ach.icon_key = IconKeyFromUrl(icon_url);
            } else {
                ach.icon_key = ach.name;
                std::transform(ach.icon_key.begin(), ach.icon_key.end(),
                    ach.icon_key.begin(), ::tolower);
                for (auto& c : ach.icon_key) if (c == ' ') c = '_';
            }

            if (ach.id > 0) out.push_back(std::move(ach));
        }
        return !out.empty();
    } catch (...) { return false; }
}

// ── Scrape /profile/{user} HTML ───────────────────────────────────────────────

std::set<std::pair<int,std::string>> ScrapeUserAchievements(const std::string& html) {
    std::set<std::pair<int,std::string>> earned;
    // Matches: /api/files/achievements/417/bomb_kill.webp
    std::regex re(R"(/api/files/achievements/(\d+)/([^"?.\s]+))");
    for (auto it = std::sregex_iterator(html.begin(), html.end(), re);
         it != std::sregex_iterator(); ++it)
    {
        int game_id = std::stoi((*it)[1].str());
        std::string key = (*it)[2].str();
        earned.insert({game_id, key});
    }
    return earned;
}

// ── Scrape /game/{id}/{slug} HTML ─────────────────────────────────────────────

std::string ScrapeGameMeta(const std::string& html, Game& gm) {
    std::smatch m;
    std::string cover_url;

    // og:image -> cover
    std::regex img_re(R"(meta-og:image: (https://gamplo\.com[^\s\n]+))");
    if (std::regex_search(html, m, img_re))
        cover_url = m[1].str();

    // og:title -> real game name
    std::regex title_re(R"(meta-og:title: ([^\n\-]+))");
    if (std::regex_search(html, m, title_re)) {
        std::string t = m[1].str();
        // trim trailing whitespace
        while (!t.empty() && (t.back()==' '||t.back()=='\r'||t.back()=='\n')) t.pop_back();
        if (!t.empty()) gm.name = t;
    }

    // meta-description
    std::regex desc_re(R"(meta-description: ([^\n]+))");
    if (std::regex_search(html, m, desc_re))
        gm.description = m[1].str();

    // Developer: look for @username pattern near "Developer"
    std::regex dev_re(R"(eveloper[^@]{0,30}@([A-Za-z0-9_]+))");
    if (std::regex_search(html, m, dev_re))
        gm.developer = "@" + m[1].str();

    // Release date: look for a date pattern near "Release"
    std::regex rd_re(R"((?:Release|Published)[^\n]{0,20}(\d{4}[-/]\d{1,2}[-/]\d{1,2}|\w+ \d{1,2},? \d{4}))");
    if (std::regex_search(html, m, rd_re))
        gm.release_date = m[1].str();

    return cover_url;
}
