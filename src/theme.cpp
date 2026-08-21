#include "theme.h"
#include "imgui.h"

// Global accents — defined in main.cpp
ThemeAccents ga;

void ApplyTheme(int idx) {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding=6; s.FrameRounding=4; s.ScrollbarRounding=4;
    s.GrabRounding=4; s.ItemSpacing={8,6}; s.FramePadding={8,5};
    s.WindowPadding={12,12}; s.PopupRounding=6;

    auto* c = s.Colors;

    switch (idx) {

    // ── 0: ImGui Dark ─────────────────────────────────────────
    case 0:
        ImGui::StyleColorsDark();
        ga = {Hex(0x4C8BF5),Hex(0x6EA8FF),Hex(0x2563D4),
              Hex(0xFFD700),Hex(0x4CAF50),Hex(0x1B3A1B),{0.2f,0.3f,0.5f,0.4f}};
        break;

    // ── 1: ImGui Light ────────────────────────────────────────
    case 1:
        ImGui::StyleColorsLight();
        ga = {Hex(0x2563D4),Hex(0x4C8BF5),Hex(0x1A4BAA),
              Hex(0xB8860B),Hex(0x2E7D32),Hex(0xC8E6C9),{0.2f,0.4f,0.8f,0.2f}};
        break;

    // ── 2: ImGui Classic ──────────────────────────────────────
    case 2:
        ImGui::StyleColorsClassic();
        ga = {Hex(0x7B6F4E),Hex(0x9A8C6A),Hex(0x5C5237),
              Hex(0xE8C96B),Hex(0x6BAF6B),Hex(0x1A2E1A),{0.4f,0.3f,0.1f,0.4f}};
        break;

    // ── 3: Catppuccin Mocha ───────────────────────────────────
    case 3:
        ImGui::StyleColorsDark();
        c[ImGuiCol_WindowBg]         ={0.118f,0.118f,0.180f,1};
        c[ImGuiCol_ChildBg]          ={0.094f,0.094f,0.145f,1};
        c[ImGuiCol_PopupBg]          ={0.118f,0.118f,0.180f,1};
        c[ImGuiCol_Border]           ={0.271f,0.278f,0.353f,1};
        c[ImGuiCol_FrameBg]          ={0.192f,0.196f,0.251f,1};
        c[ImGuiCol_FrameBgHovered]   ={0.271f,0.278f,0.353f,1};
        c[ImGuiCol_FrameBgActive]    ={0.345f,0.353f,0.435f,1};
        c[ImGuiCol_TitleBg]          ={0.094f,0.094f,0.145f,1};
        c[ImGuiCol_TitleBgActive]    ={0.118f,0.118f,0.180f,1};
        c[ImGuiCol_ScrollbarBg]      ={0.094f,0.094f,0.145f,1};
        c[ImGuiCol_ScrollbarGrab]    =Hex(0xCBA6F7);
        c[ImGuiCol_ScrollbarGrabHovered]=Hex(0xD0BCFF);
        c[ImGuiCol_ScrollbarGrabActive] =Hex(0xB4BEFE);
        c[ImGuiCol_CheckMark]        =Hex(0xCBA6F7);
        c[ImGuiCol_SliderGrab]       =Hex(0xCBA6F7);
        c[ImGuiCol_SliderGrabActive] =Hex(0xB4BEFE);
        c[ImGuiCol_Button]           ={0.192f,0.196f,0.251f,1};
        c[ImGuiCol_ButtonHovered]    ={0.271f,0.278f,0.353f,1};
        c[ImGuiCol_ButtonActive]     =Hex(0xCBA6F7);
        c[ImGuiCol_Header]           ={0.192f,0.196f,0.251f,1};
        c[ImGuiCol_HeaderHovered]    ={0.271f,0.278f,0.353f,1};
        c[ImGuiCol_HeaderActive]     =Hex(0xCBA6F7);
        c[ImGuiCol_Separator]        ={0.271f,0.278f,0.353f,1};
        c[ImGuiCol_Text]             =Hex(0xCDD6F4);
        c[ImGuiCol_TextDisabled]     =Hex(0x6C7086);
        c[ImGuiCol_Tab]              ={0.094f,0.094f,0.145f,1};
        c[ImGuiCol_TabHovered]       =Hex(0xCBA6F7);
        c[ImGuiCol_TabActive]        ={0.192f,0.196f,0.251f,1};
        ga = {Hex(0xCBA6F7),Hex(0xD0BCFF),Hex(0xB4BEFE),
              Hex(0xF9E2AF),Hex(0xA6E3A1),Hex(0x1C2B1C),Hex(0x313244)};
        break;

    // ── 4: Catppuccin Latte ───────────────────────────────────
    case 4:
        ImGui::StyleColorsLight();
        c[ImGuiCol_WindowBg]         =Hex(0xEFF1F5);
        c[ImGuiCol_ChildBg]          =Hex(0xE6E9EF);
        c[ImGuiCol_PopupBg]          =Hex(0xEFF1F5);
        c[ImGuiCol_Border]           =Hex(0xBCC0CC);
        c[ImGuiCol_FrameBg]          =Hex(0xCCD0DA);
        c[ImGuiCol_FrameBgHovered]   =Hex(0xBCC0CC);
        c[ImGuiCol_FrameBgActive]    =Hex(0xACB0BE);
        c[ImGuiCol_TitleBg]          =Hex(0xE6E9EF);
        c[ImGuiCol_TitleBgActive]    =Hex(0xEFF1F5);
        c[ImGuiCol_ScrollbarBg]      =Hex(0xE6E9EF);
        c[ImGuiCol_ScrollbarGrab]    =Hex(0x8839EF);
        c[ImGuiCol_ScrollbarGrabHovered]=Hex(0x7287FD);
        c[ImGuiCol_ScrollbarGrabActive] =Hex(0x6C6F85);
        c[ImGuiCol_CheckMark]        =Hex(0x8839EF);
        c[ImGuiCol_SliderGrab]       =Hex(0x8839EF);
        c[ImGuiCol_SliderGrabActive] =Hex(0x6C6F85);
        c[ImGuiCol_Button]           =Hex(0xCCD0DA);
        c[ImGuiCol_ButtonHovered]    =Hex(0xBCC0CC);
        c[ImGuiCol_ButtonActive]     =Hex(0x8839EF);
        c[ImGuiCol_Header]           =Hex(0xCCD0DA);
        c[ImGuiCol_HeaderHovered]    =Hex(0xBCC0CC);
        c[ImGuiCol_HeaderActive]     =Hex(0x8839EF);
        c[ImGuiCol_Separator]        =Hex(0xBCC0CC);
        c[ImGuiCol_Text]             =Hex(0x4C4F69);
        c[ImGuiCol_TextDisabled]     =Hex(0x8C8FA1);
        c[ImGuiCol_Tab]              =Hex(0xE6E9EF);
        c[ImGuiCol_TabHovered]       =Hex(0x8839EF);
        c[ImGuiCol_TabActive]        =Hex(0xCCD0DA);
        ga = {Hex(0x8839EF),Hex(0x7287FD),Hex(0x6C6F85),
              Hex(0xDF8E1D),Hex(0x40A02B),Hex(0xD0EED0),Hex(0xCCD0DA)};
        break;

    // ── 5: Nord ───────────────────────────────────────────────
    case 5:
        ImGui::StyleColorsDark();
        c[ImGuiCol_WindowBg]         =Hex(0x2E3440);
        c[ImGuiCol_ChildBg]          =Hex(0x272C36);
        c[ImGuiCol_PopupBg]          =Hex(0x2E3440);
        c[ImGuiCol_Border]           =Hex(0x4C566A);
        c[ImGuiCol_FrameBg]          =Hex(0x3B4252);
        c[ImGuiCol_FrameBgHovered]   =Hex(0x434C5E);
        c[ImGuiCol_FrameBgActive]    =Hex(0x4C566A);
        c[ImGuiCol_TitleBg]          =Hex(0x272C36);
        c[ImGuiCol_TitleBgActive]    =Hex(0x2E3440);
        c[ImGuiCol_ScrollbarBg]      =Hex(0x272C36);
        c[ImGuiCol_ScrollbarGrab]    =Hex(0x5E81AC);
        c[ImGuiCol_ScrollbarGrabHovered]=Hex(0x81A1C1);
        c[ImGuiCol_ScrollbarGrabActive] =Hex(0x88C0D0);
        c[ImGuiCol_CheckMark]        =Hex(0x88C0D0);
        c[ImGuiCol_SliderGrab]       =Hex(0x5E81AC);
        c[ImGuiCol_SliderGrabActive] =Hex(0x88C0D0);
        c[ImGuiCol_Button]           =Hex(0x3B4252);
        c[ImGuiCol_ButtonHovered]    =Hex(0x434C5E);
        c[ImGuiCol_ButtonActive]     =Hex(0x5E81AC);
        c[ImGuiCol_Header]           =Hex(0x3B4252);
        c[ImGuiCol_HeaderHovered]    =Hex(0x434C5E);
        c[ImGuiCol_HeaderActive]     =Hex(0x5E81AC);
        c[ImGuiCol_Separator]        =Hex(0x4C566A);
        c[ImGuiCol_Text]             =Hex(0xECEFF4);
        c[ImGuiCol_TextDisabled]     =Hex(0x4C566A);
        c[ImGuiCol_Tab]              =Hex(0x272C36);
        c[ImGuiCol_TabHovered]       =Hex(0x5E81AC);
        c[ImGuiCol_TabActive]        =Hex(0x3B4252);
        ga = {Hex(0x5E81AC),Hex(0x81A1C1),Hex(0x88C0D0),
              Hex(0xEBCB8B),Hex(0xA3BE8C),Hex(0x1E2A1E),Hex(0x3B4252)};
        break;

    // ── 6: Dracula ────────────────────────────────────────────
    case 6:
        ImGui::StyleColorsDark();
        c[ImGuiCol_WindowBg]         =Hex(0x282A36);
        c[ImGuiCol_ChildBg]          =Hex(0x21222C);
        c[ImGuiCol_PopupBg]          =Hex(0x282A36);
        c[ImGuiCol_Border]           =Hex(0x44475A);
        c[ImGuiCol_FrameBg]          =Hex(0x44475A);
        c[ImGuiCol_FrameBgHovered]   =Hex(0x6272A4);
        c[ImGuiCol_FrameBgActive]    =Hex(0x6272A4);
        c[ImGuiCol_TitleBg]          =Hex(0x21222C);
        c[ImGuiCol_TitleBgActive]    =Hex(0x282A36);
        c[ImGuiCol_ScrollbarBg]      =Hex(0x21222C);
        c[ImGuiCol_ScrollbarGrab]    =Hex(0xBD93F9);
        c[ImGuiCol_ScrollbarGrabHovered]=Hex(0xCAA9FA);
        c[ImGuiCol_ScrollbarGrabActive] =Hex(0xFF79C6);
        c[ImGuiCol_CheckMark]        =Hex(0x50FA7B);
        c[ImGuiCol_SliderGrab]       =Hex(0xBD93F9);
        c[ImGuiCol_SliderGrabActive] =Hex(0xFF79C6);
        c[ImGuiCol_Button]           =Hex(0x44475A);
        c[ImGuiCol_ButtonHovered]    =Hex(0x6272A4);
        c[ImGuiCol_ButtonActive]     =Hex(0xBD93F9);
        c[ImGuiCol_Header]           =Hex(0x44475A);
        c[ImGuiCol_HeaderHovered]    =Hex(0x6272A4);
        c[ImGuiCol_HeaderActive]     =Hex(0xBD93F9);
        c[ImGuiCol_Separator]        =Hex(0x44475A);
        c[ImGuiCol_Text]             =Hex(0xF8F8F2);
        c[ImGuiCol_TextDisabled]     =Hex(0x6272A4);
        c[ImGuiCol_Tab]              =Hex(0x21222C);
        c[ImGuiCol_TabHovered]       =Hex(0xBD93F9);
        c[ImGuiCol_TabActive]        =Hex(0x44475A);
        ga = {Hex(0xBD93F9),Hex(0xCAA9FA),Hex(0xFF79C6),
              Hex(0xF1FA8C),Hex(0x50FA7B),Hex(0x1A3320),Hex(0x44475A)};
        break;

    // ── 7: Gruvbox Dark ───────────────────────────────────────
    case 7:
        ImGui::StyleColorsDark();
        c[ImGuiCol_WindowBg]         =Hex(0x282828);
        c[ImGuiCol_ChildBg]          =Hex(0x1D2021);
        c[ImGuiCol_PopupBg]          =Hex(0x282828);
        c[ImGuiCol_Border]           =Hex(0x504945);
        c[ImGuiCol_FrameBg]          =Hex(0x3C3836);
        c[ImGuiCol_FrameBgHovered]   =Hex(0x504945);
        c[ImGuiCol_FrameBgActive]    =Hex(0x665C54);
        c[ImGuiCol_TitleBg]          =Hex(0x1D2021);
        c[ImGuiCol_TitleBgActive]    =Hex(0x282828);
        c[ImGuiCol_ScrollbarBg]      =Hex(0x1D2021);
        c[ImGuiCol_ScrollbarGrab]    =Hex(0xD79921);
        c[ImGuiCol_ScrollbarGrabHovered]=Hex(0xFABD2F);
        c[ImGuiCol_ScrollbarGrabActive] =Hex(0xD65D0E);
        c[ImGuiCol_CheckMark]        =Hex(0xB8BB26);
        c[ImGuiCol_SliderGrab]       =Hex(0xD79921);
        c[ImGuiCol_SliderGrabActive] =Hex(0xFABD2F);
        c[ImGuiCol_Button]           =Hex(0x3C3836);
        c[ImGuiCol_ButtonHovered]    =Hex(0x504945);
        c[ImGuiCol_ButtonActive]     =Hex(0xD79921);
        c[ImGuiCol_Header]           =Hex(0x3C3836);
        c[ImGuiCol_HeaderHovered]    =Hex(0x504945);
        c[ImGuiCol_HeaderActive]     =Hex(0xD79921);
        c[ImGuiCol_Separator]        =Hex(0x504945);
        c[ImGuiCol_Text]             =Hex(0xEBDBB2);
        c[ImGuiCol_TextDisabled]     =Hex(0x928374);
        c[ImGuiCol_Tab]              =Hex(0x1D2021);
        c[ImGuiCol_TabHovered]       =Hex(0xD79921);
        c[ImGuiCol_TabActive]        =Hex(0x3C3836);
        ga = {Hex(0xD79921),Hex(0xFABD2F),Hex(0xD65D0E),
              Hex(0xFABD2F),Hex(0xB8BB26),Hex(0x1C2300),Hex(0x3C3836)};
        break;

    default:
        ImGui::StyleColorsDark();
        ga = {Hex(0x4C8BF5),Hex(0x6EA8FF),Hex(0x2563D4),
              Hex(0xFFD700),Hex(0x4CAF50),Hex(0x1B3A1B),{0.2f,0.3f,0.5f,0.4f}};
    }
}
