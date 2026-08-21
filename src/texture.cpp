#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")

#include "texture.h"
#include "http.h"
#include "storage.h"

#include <SDL_opengl.h>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>

// ── Internal state ────────────────────────────────────────────────────────────

static std::unordered_map<std::string, GLuint> s_cache;   // url -> tex (0=loading)
static std::mutex                               s_cache_mtx;

struct TexUpload { std::string url; std::vector<unsigned char> data; };
static std::vector<TexUpload> s_queue;
static std::mutex             s_queue_mtx;

// ── WIC decode ────────────────────────────────────────────────────────────────

static GLuint UploadToGL(const std::vector<unsigned char>& data)
{
    if (data.empty()) return 0;

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, data.size());
    if (!hMem) return 0;
    memcpy(GlobalLock(hMem), data.data(), data.size());
    GlobalUnlock(hMem);

    IStream* stream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(hMem, TRUE, &stream))) {
        GlobalFree(hMem); return 0;
    }

    IWICImagingFactory* factory = nullptr;
    CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory, (void**)&factory);
    if (!factory) { stream->Release(); return 0; }

    IWICBitmapDecoder* decoder = nullptr;
    factory->CreateDecoderFromStream(stream, nullptr,
        WICDecodeMetadataCacheOnLoad, &decoder);
    stream->Release();
    if (!decoder) { factory->Release(); return 0; }

    IWICBitmapFrameDecode* frame = nullptr;
    decoder->GetFrame(0, &frame);
    decoder->Release();
    if (!frame) { factory->Release(); return 0; }

    IWICFormatConverter* conv = nullptr;
    factory->CreateFormatConverter(&conv);
    conv->Initialize(frame, GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    frame->Release();

    UINT w = 0, h = 0;
    conv->GetSize(&w, &h);
    if (w == 0 || h == 0) { conv->Release(); factory->Release(); return 0; }

    std::vector<BYTE> pixels(w * h * 4);
    conv->CopyPixels(nullptr, w * 4, (UINT)pixels.size(), pixels.data());
    conv->Release();
    factory->Release();

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

// ── Public API ────────────────────────────────────────────────────────────────

void LoadTexAsync(const std::string& url, const std::string& cache_key)
{
    {
        std::lock_guard<std::mutex> lk(s_cache_mtx);
        if (s_cache.count(url)) return;   // already loading or loaded
        s_cache[url] = 0;                 // reserve slot
    }

    std::thread([url, cache_key]() {
        std::vector<unsigned char> data;
        std::string path = ImgCachePath(cache_key.empty() ? url : cache_key);

        if (!ReadBin(path, data)) {
            if (!HttpGetBinary(url, data)) return;
            WriteBin(path, data);
        }

        std::lock_guard<std::mutex> lk(s_queue_mtx);
        s_queue.push_back({url, std::move(data)});
    }).detach();
}

GLuint GetTex(const std::string& url, const std::string& cache_key)
{
    std::lock_guard<std::mutex> lk(s_cache_mtx);
    auto it = s_cache.find(url);
    if (it != s_cache.end()) return it->second;

    // Not started yet — kick off
    s_cache[url] = 0;
    std::thread([url, cache_key]() {
        std::vector<unsigned char> data;
        std::string path = ImgCachePath(cache_key.empty() ? url : cache_key);
        if (!ReadBin(path, data)) {
            if (!HttpGetBinary(url, data)) return;
            WriteBin(path, data);
        }
        std::lock_guard<std::mutex> lk2(s_queue_mtx);
        s_queue.push_back({url, std::move(data)});
    }).detach();
    return 0;
}

void FlushTexQueue()
{
    std::vector<TexUpload> pending;
    {
        std::lock_guard<std::mutex> lk(s_queue_mtx);
        pending.swap(s_queue);
    }
    for (auto& u : pending) {
        GLuint tex = UploadToGL(u.data);
        std::lock_guard<std::mutex> lk(s_cache_mtx);
        s_cache[u.url] = tex;
    }
}

void ClearTexCache()
{
    std::lock_guard<std::mutex> lk(s_cache_mtx);
    for (auto& [url, tex] : s_cache)
        if (tex) glDeleteTextures(1, &tex);
    s_cache.clear();
}
