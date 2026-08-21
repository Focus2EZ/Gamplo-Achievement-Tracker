#include "storage.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

const std::string DATA_DIR  = "data";
const std::string IMG_DIR   = "data\\img";
const std::string PROG_FILE = DATA_DIR + "\\progress.json";
const std::string LIST_FILE = DATA_DIR + "\\games_list.html";
const std::string CFG_FILE  = DATA_DIR + "\\config.json";

void EnsureDirs() {
    fs::create_directories(DATA_DIR);
    fs::create_directories(IMG_DIR);
}

std::string GameCachePath(int id) {
    return DATA_DIR + "\\game_" + std::to_string(id) + ".json";
}

std::string ImgCachePath(const std::string& cache_key) {
    std::string s = cache_key;
    for (auto& c : s)
        if (c=='/'||c==':'||c=='?'||c=='&'||c=='='||c=='*'||c=='<'||c=='>'||c=='|'||c=='"')
            c = '_';
    return IMG_DIR + "\\" + s + ".bin";
}

void WriteText(const std::string& p, const std::string& d) {
    std::ofstream f(p); f << d;
}
bool ReadText(const std::string& p, std::string& o) {
    std::ifstream f(p); if (!f) return false;
    o.assign(std::istreambuf_iterator<char>(f), {}); return !o.empty();
}
void WriteBin(const std::string& p, const std::vector<unsigned char>& d) {
    std::ofstream f(p, std::ios::binary);
    f.write((const char*)d.data(), d.size());
}
bool ReadBin(const std::string& p, std::vector<unsigned char>& o) {
    std::ifstream f(p, std::ios::binary); if (!f) return false;
    o.assign(std::istreambuf_iterator<char>(f), {}); return !o.empty();
}

std::map<int, std::set<int>> LoadProgress() {
    std::map<int, std::set<int>> prog;
    std::string raw; if (!ReadText(PROG_FILE, raw)) return prog;
    try {
        auto j = json::parse(raw);
        for (auto& [k, v] : j.items()) {
            int gid = std::stoi(k);
            for (int a : v) prog[gid].insert(a);
        }
    } catch (...) {}
    return prog;
}

void SaveProgress(const std::map<int, std::set<int>>& prog) {
    EnsureDirs();
    json j;
    for (auto& [gid, aids] : prog) {
        json arr = json::array();
        for (int a : aids) arr.push_back(a);
        j[std::to_string(gid)] = arr;
    }
    WriteText(PROG_FILE, j.dump(2));
}

void LoadConfig() {
    std::string raw; if (!ReadText(CFG_FILE, raw)) return;
    try {
        auto j = json::parse(raw);
        g_cfg.username        = j.value("username",        "");
        g_cfg.theme_idx       = j.value("theme_idx",        0);
        g_cfg.max_id          = j.value("max_id",        1300);
        g_cfg.use_cache       = j.value("use_cache",      true);
        g_cfg.sync_interval_s = j.value("sync_interval",  300);
        g_cfg.auto_sync       = j.value("auto_sync",      true);
        g_cfg.filter_has_ach  = j.value("filter_has_ach", false);
    } catch (...) {}
}

void SaveConfig() {
    EnsureDirs();
    json j;
    j["username"]      = g_cfg.username;
    j["theme_idx"]     = g_cfg.theme_idx;
    j["max_id"]        = g_cfg.max_id;
    j["use_cache"]     = g_cfg.use_cache;
    j["sync_interval"] = g_cfg.sync_interval_s;
    j["auto_sync"]     = g_cfg.auto_sync;
    j["filter_has_ach"] = g_cfg.filter_has_ach;
    WriteText(CFG_FILE, j.dump(2));
}
