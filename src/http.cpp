#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

#include "http.h"

bool HttpGet(const std::string& url, std::string& out, std::string& err)
{
    HINTERNET hNet = InternetOpenA("Gamplo-Tracker/2.0",
        INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hNet) { err = "InternetOpen failed"; return false; }

    HINTERNET hReq = InternetOpenUrlA(hNet, url.c_str(), nullptr, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0);
    if (!hReq) {
        err = "OpenUrl failed: " + std::to_string(GetLastError());
        InternetCloseHandle(hNet); return false;
    }

    DWORD status = 0, slen = sizeof(status);
    HttpQueryInfoA(hReq, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
        &status, &slen, nullptr);
    if (status < 200 || status >= 300) {
        err = "HTTP " + std::to_string(status);
        InternetCloseHandle(hReq); InternetCloseHandle(hNet); return false;
    }

    char buf[8192]; DWORD read = 0;
    while (InternetReadFile(hReq, buf, sizeof(buf), &read) && read > 0)
        out.append(buf, read);

    InternetCloseHandle(hReq); InternetCloseHandle(hNet);
    return true;
}

bool HttpGetBinary(const std::string& url, std::vector<unsigned char>& out)
{
    HINTERNET hNet = InternetOpenA("Gamplo-Tracker/2.0",
        INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hNet) return false;

    HINTERNET hReq = InternetOpenUrlA(hNet, url.c_str(), nullptr, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0);
    if (!hReq) { InternetCloseHandle(hNet); return false; }

    DWORD status = 0, slen = sizeof(status);
    HttpQueryInfoA(hReq, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
        &status, &slen, nullptr);
    if (status < 200 || status >= 300) {
        InternetCloseHandle(hReq); InternetCloseHandle(hNet); return false;
    }

    char buf[8192]; DWORD read = 0;
    while (InternetReadFile(hReq, buf, sizeof(buf), &read) && read > 0)
        out.insert(out.end(), buf, buf + read);

    InternetCloseHandle(hReq); InternetCloseHandle(hNet);
    return !out.empty();
}
