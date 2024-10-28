#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

inline void setupLogging() {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs.txt");
    file_sink->set_level(spdlog::level::debug);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%@] [%!] [thread %t] [%oms] [%l] %v");

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S] [thread %t] [%oms] [%l] %v%$");

    auto logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{ file_sink, console_sink });
    spdlog::register_logger(logger);

    spdlog::set_default_logger(logger);
}

// spdlog 要想输出文件、路径、函数、行号，只能借助此宏，才会显示。
// 其实使用 C++20 std::source_location 也能获取这些信息，后面再考虑单独封装吧，目前这样做导致没办法做格式字符串。

#define LOG_INFO(msg, ...) SPDLOG_LOGGER_INFO(spdlog::get("multi_sink"), msg)
#define LOG_WARN(msg, ...) SPDLOG_LOGGER_WARN(spdlog::get("multi_sink"), msg)
#define LOG_ERROR(msg, ...) SPDLOG_LOGGER_ERROR(spdlog::get("multi_sink"), msg)

const auto init_log = (setupLogging(), 0);
