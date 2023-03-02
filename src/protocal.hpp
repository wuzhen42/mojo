#pragma once
#include <chrono>

const int PORT_HOST = 24351;
const int PORT_PUBLISH = 23452;
enum Message { HEARTBEAT, CONNECT, FRAME, PENCIL };

enum class LogLevel { Critical, Error, Warn, Info, Debug };

using TimePoint = std::chrono::system_clock::time_point;

inline TimePoint currentTime() { return std::chrono::system_clock::now(); }