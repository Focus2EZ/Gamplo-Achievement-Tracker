#pragma once
#include <string>
#include <SDL_opengl.h>

// Call once per frame on the main (GL) thread to upload pending textures.
void FlushTexQueue();

// Request texture load. Returns 0 while loading, tex id once ready.
// cache_key is used as the local filename under data\img\.
GLuint GetTex(const std::string& url, const std::string& cache_key = "");

// Kick off async load without blocking. Safe to call multiple times.
void LoadTexAsync(const std::string& url, const std::string& cache_key);

// Free all cached textures (call before shutdown or cache clear).
void ClearTexCache();
