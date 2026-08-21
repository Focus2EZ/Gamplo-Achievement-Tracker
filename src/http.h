#pragma once
#include <string>
#include <vector>

bool HttpGet(const std::string& url, std::string& out, std::string& err);
bool HttpGetBinary(const std::string& url, std::vector<unsigned char>& out);
