#include "Log.h"
#include <thread>
#include <format>
using namespace std::chrono_literals;

int main() {
    LOG_WARN("😅");
    std::jthread t{[]{
        std::this_thread::sleep_for(100ms);
        LOG_ERROR("🤣");
    }};
    LOG_INFO("👉");
}