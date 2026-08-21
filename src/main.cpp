#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objbase.h>   // CoInitializeEx / CoUninitialize (excluded by WIN32_LEAN_AND_MEAN)
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#include <SDL_opengl.h>

#include "types.h"
#include "appstate.h"
#include "storage.h"
#include "texture.h"
#include "theme.h"
#include "workers.h"
#include "ui.h"

#include <thread>
#include <chrono>

Config g_cfg;   // defined here, declared extern in types.h

int main(int, char**)
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    EnsureDirs();
    LoadConfig();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return 1;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* win = SDL_CreateWindow(
        "Gamplo Achievement Tracker",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1400, 860,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    SDL_GL_MakeCurrent(win, ctx);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = "gamplo_ui.ini";

    ApplyTheme(g_cfg.theme_idx);
    ImGui_ImplSDL2_InitForOpenGL(win, ctx);
    ImGui_ImplOpenGL3_Init("#version 130");

    std::thread([]{ LoadFromCacheWorker(); }).detach();

    if (g_cfg.auto_sync && !g_cfg.username.empty())
        AppState::last_sync = Clock::now() - std::chrono::seconds(99999);

    bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (ev.type == SDL_QUIT) running = false;
        }

        if (g_cfg.auto_sync && !g_cfg.username.empty() &&
            !AppState::syncing && !AppState::fetching)
        {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                Clock::now() - AppState::last_sync).count();
            if (elapsed >= g_cfg.sync_interval_s) {
                AppState::syncing = true;
                std::string u = g_cfg.username;
                std::thread([u]{ SyncProfileWorker(u); }).detach();
            }
        }

        FlushTexQueue();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        int sw, sh;
        SDL_GetWindowSize(win, &sw, &sh);
        float fsw=(float)sw, fsh=(float)sh;
        float grid_w    = 520.f;
        float content_h = fsh - 22.f;

        DrawGameGrid(grid_w, content_h);
        DrawAchievementPanel(grid_w, fsw-grid_w, content_h);
        DrawSettings(fsw, fsh);
        DrawStatusBar(fsw, fsh);

        ImGui::Render();
        glViewport(0, 0, sw, sh);
        auto& bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
        glClearColor(bg.x, bg.y, bg.z, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(win);
    }

    SaveConfig();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    CoUninitialize();
    return 0;
}
