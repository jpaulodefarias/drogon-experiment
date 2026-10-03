#include "presentation/HealthController.hpp"

#include "presentation/HttpApiUtils.hpp"

namespace example {
void registerHealthController() {
    using presentation::detail::Clock;
    using presentation::detail::logRequest;
    drogon::app().registerHandler(
        "/health",
        [](const drogon::HttpRequestPtr &,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
            const auto started = Clock::now();
            const auto traceId = drogon::utils::getUuid();
            Json::Value body;
            body["status"] = "UP";
            auto response = drogon::HttpResponse::newHttpJsonResponse(body);
            response->addHeader("X-Request-ID", traceId);
            logRequest(traceId, "GET", "/health", 200, started);
            callback(response);
        },
        {drogon::Get});
}
} // namespace example
