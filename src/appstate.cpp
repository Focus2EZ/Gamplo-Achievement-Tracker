#include "appstate.h"

namespace AppState {

std::vector<Game>            games;
std::map<int, std::set<int>> progress;
std::mutex                   mtx;

int  selected = -1;
char search[256] = {};

std::atomic<bool> fetching{false};
std::atomic<int>  fetch_cur{0};
std::atomic<int>  fetch_tot{0};

std::atomic<bool>  syncing{false};
int                sync_found_new = 0;
Clock::time_point  last_sync = Clock::now() - std::chrono::seconds(99999);

bool show_settings = false;

static std::string s_status;
static std::string s_sync_status;
static std::mutex  s_status_mtx;
static std::mutex  s_sync_mtx;

void SetStatus(const std::string& s) {
    std::lock_guard<std::mutex> lk(s_status_mtx); s_status = s;
}
void SetSyncStatus(const std::string& s) {
    std::lock_guard<std::mutex> lk(s_sync_mtx); s_sync_status = s;
}
std::string GetStatus() {
    std::lock_guard<std::mutex> lk(s_status_mtx); return s_status;
}
std::string GetSyncStatus() {
    std::lock_guard<std::mutex> lk(s_sync_mtx); return s_sync_status;
}

} // namespace AppState
