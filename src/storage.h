#pragma once
#include "types.h"
#include <map>
#include <set>
#include <string>
#include <vector>

extern const std::string DATA_DIR;
extern const std::string IMG_DIR;
extern const std::string PROG_FILE;
extern const std::string LIST_FILE;
extern const std::string CFG_FILE;

void EnsureDirs();
std::string GameCachePath(int id);
std::string ImgCachePath(const std::string& cache_key);

void WriteText(const std::string& path, const std::string& data);
bool ReadText(const std::string& path, std::string& out);
void WriteBin(const std::string& path, const std::vector<unsigned char>& data);
bool ReadBin(const std::string& path, std::vector<unsigned char>& out);

std::map<int, std::set<int>> LoadProgress();
void SaveProgress(const std::map<int, std::set<int>>& p);

void LoadConfig();
void SaveConfig();
