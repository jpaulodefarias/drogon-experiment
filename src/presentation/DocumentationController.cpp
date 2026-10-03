#include "presentation/DocumentationController.hpp"

#include "ApiDocumentation.hpp"
#include "presentation/HttpApiUtils.hpp"

#include <string>

namespace example {
void registerDocumentationController() {
    using presentation::detail::Clock;
    using presentation::detail::logRequest;

    drogon::app().registerHandler(
        "/openapi.json",
        [](const drogon::HttpRequestPtr &,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
            const auto started = Clock::now();
            const auto traceId = drogon::utils::getUuid();
            auto response = drogon::HttpResponse::newHttpResponse();
            response->setContentTypeString("application/json; charset=utf-8");
            response->setBody(std::string{kOpenApiJson});
            response->addHeader("X-Request-ID", traceId);
            logRequest(traceId, "GET", "/openapi.json", 200, started);
            callback(response);
        },
        {drogon::Get});

    drogon::app().registerHandler(
        "/docs",
        [](const drogon::HttpRequestPtr &,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
            const auto started = Clock::now();
            const auto traceId = drogon::utils::getUuid();
            auto response = drogon::HttpResponse::newHttpResponse();
            response->setContentTypeString("text/html; charset=utf-8");
            response->setBody(std::string{kSwaggerHtml});
            response->addHeader("X-Request-ID", traceId);
            logRequest(traceId, "GET", "/docs", 200, started);
            callback(response);
        },
        {drogon::Get});

    for (const auto *asset :
         {"swagger-ui.css", "swagger-ui-bundle.js", "swagger-ui-standalone-preset.js"}) {
        drogon::app().registerHandler(
            std::string{"/docs/"} + asset,
            [asset](const drogon::HttpRequestPtr &,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
                const auto started = Clock::now();
                const auto traceId = drogon::utils::getUuid();
                auto response =
                    drogon::HttpResponse::newFileResponse(std::string{"/opt/swagger-ui/"} + asset);
                response->addHeader("X-Request-ID", traceId);
                logRequest(traceId, "GET", "/docs/asset", 200, started);
                callback(response);
            },
            {drogon::Get});
    }
}
} // namespace example
