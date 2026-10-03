#pragma once

#include <drogon/drogon.h>

#include <chrono>
#include <string>

namespace example {
namespace presentation {
namespace detail {
using Clock = std::chrono::steady_clock;

inline void logRequest(const std::string &traceId, const char *method, const char *path, int status,
                       Clock::time_point started) {
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - started);
    LOG_INFO << "traceId=" << traceId << " method=" << method << " path=" << path
             << " status=" << status << " durationMs=" << elapsed.count();
}

inline drogon::HttpResponsePtr errorResponse(const char *code, const char *message,
                                             const std::string &traceId,
                                             drogon::HttpStatusCode status) {
    Json::Value body;
    body["code"] = code;
    body["message"] = message;
    body["traceId"] = traceId;
    auto response = drogon::HttpResponse::newHttpJsonResponse(body);
    response->setStatusCode(status);
    response->addHeader("X-Request-ID", traceId);
    return response;
}
} // namespace detail
} // namespace presentation
} // namespace example
