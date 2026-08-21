cmake_minimum_required(VERSION 3.16)
project(GamploTracker CXX)
set(CMAKE_CXX_STANDARD 17)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)
set(JSON_DIR  ${CMAKE_SOURCE_DIR}/vendor/json/include)
set(SDL2_DIR  ${CMAKE_SOURCE_DIR}/vendor/SDL2)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

set(APP_SOURCES
    src/main.cpp
    src/appstate.cpp
    src/storage.cpp
    src/http.cpp
    src/texture.cpp
    src/api.cpp
    src/workers.cpp
    src/theme.cpp
    src/ui.cpp
)

add_executable(gamplo_tracker
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(gamplo_tracker PRIVATE
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${JSON_DIR}
    ${SDL2_DIR}/include
)

target_link_libraries(gamplo_tracker PRIVATE
    ${SDL2_DIR}/lib/x64/SDL2.lib
    ${SDL2_DIR}/lib/x64/SDL2main.lib
    opengl32
    wininet
    windowscodecs
    ole32
    oleaut32
    ws2_32
)

if(MSVC)
    target_compile_options(gamplo_tracker PRIVATE /W3 /wd4244 /wd4267 /wd4996)
    set_target_properties(gamplo_tracker PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()

add_custom_command(TARGET gamplo_tracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL2_DIR}/lib/x64/SDL2.dll
        $<TARGET_FILE_DIR:gamplo_tracker>)

add_custom_command(TARGET gamplo_tracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:gamplo_tracker>/data)
