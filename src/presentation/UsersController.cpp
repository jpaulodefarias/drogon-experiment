#include "presentation/UsersController.hpp"

#include "presentation/HttpApiUtils.hpp"

#include <utility>
#include <vector>

namespace example {
void registerUsersController(const CreateUserUseCase &createUser, const FindUserUseCase &findUser,
                             const ListUsersUseCase &listUsers, const DeleteUserUseCase &deleteUser,
                             const DeleteAllUsersUseCase &deleteAllUsers) {
    using presentation::detail::Clock;
    using presentation::detail::errorResponse;
    using presentation::detail::logRequest;

    drogon::app().registerHandler(
        "/api/v1/users",
        [&deleteAllUsers](const drogon::HttpRequestPtr &,
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
            const auto started = Clock::now();
            const auto traceId = drogon::utils::getUuid();
            deleteAllUsers.execute([callback = std::move(callback), traceId,
                                    started](DeleteAllUsersResult result) mutable {
                drogon::HttpResponsePtr response;
                int status = 204;
                if (std::holds_alternative<std::monostate>(result)) {
                    response = drogon::HttpResponse::newHttpResponse();
                    response->setStatusCode(drogon::k204NoContent);
                    response->addHeader("X-Request-ID", traceId);
                } else {
                    status = 500;
                    response = errorResponse("INTERNAL_ERROR", "Request failed", traceId,
                                             drogon::k500InternalServerError);
                }
                logRequest(traceId, "DELETE", "/api/v1/users", status, started);
                callback(response);
            });
        },
        {drogon::Delete});

    drogon::app().registerHandler(
        "/api/v1/users",
        [&listUsers](const drogon::HttpRequestPtr &,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
            const auto started = Clock::now();
            const auto traceId = drogon::utils::getUuid();
            listUsers.execute(
                [callback = std::move(callback), traceId, started](ListUsersResult result) mutable {
                    drogon::HttpResponsePtr response;
                    int status = 200;
                    if (const auto *users = std::get_if<std::vector<User>>(&result)) {
                        Json::Value body{Json::arrayValue};
                        for (const auto &user : *users) {
                            Json::Value item;
                            item["id"] = user.id;
                            item["name"] = user.name;
                            item["email"] = user.email;
                            body.append(std::move(item));
                        }
                        response = drogon::HttpResponse::newHttpJsonResponse(body);
                        response->addHeader("X-Request-ID", traceId);
                    } else {
                        status = 500;
                        response = errorResponse("INTERNAL_ERROR", "Request failed", traceId,
                                                 drogon::k500InternalServerError);
                    }
                    logRequest(traceId, "GET", "/api/v1/users", status, started);
                    callback(response);
                });
        },
        {drogon::Get});

    drogon::app().registerHandler(
        "/api/v1/users",
        [&createUser](const drogon::HttpRequestPtr &request,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
            const auto started = Clock::now();
            const auto traceId = drogon::utils::getUuid();
            const auto json = request->getJsonObject();
            if (!json || !json->isObject() || !(*json)["name"].isString() ||
                !(*json)["email"].isString()) {
                auto response = errorResponse("INVALID_INPUT", "Name and email are required",
                                              traceId, drogon::k400BadRequest);
                logRequest(traceId, "POST", "/api/v1/users", 400, started);
                callback(response);
                return;
            }

            CreateUserCommand command{(*json)["name"].asString(), (*json)["email"].asString()};
            createUser.execute(std::move(command), [callback = std::move(callback), traceId,
                                                    started](CreateUserResult result) mutable {
                drogon::HttpResponsePtr response;
                int status = 201;
                if (const auto *user = std::get_if<User>(&result)) {
                    Json::Value body;
                    body["id"] = user->id;
                    body["name"] = user->name;
                    body["email"] = user->email;
                    response = drogon::HttpResponse::newHttpJsonResponse(body);
                    response->setStatusCode(drogon::k201Created);
                    response->addHeader("X-Request-ID", traceId);
                } else {
                    switch (std::get<CreateUserError>(result)) {
                    case CreateUserError::InvalidInput:
                        status = 400;
                        response = errorResponse("INVALID_INPUT", "Invalid name or email", traceId,
                                                 drogon::k400BadRequest);
                        break;
                    case CreateUserError::Conflict:
                        status = 409;
                        response = errorResponse("EMAIL_CONFLICT", "Email already exists", traceId,
                                                 drogon::k409Conflict);
                        break;
                    case CreateUserError::Unavailable:
                        status = 500;
                        response = errorResponse("INTERNAL_ERROR", "Request failed", traceId,
                                                 drogon::k500InternalServerError);
                        break;
                    }
                }
                logRequest(traceId, "POST", "/api/v1/users", status, started);
                callback(response);
            });
        },
        {drogon::Post});

    drogon::app().registerHandler(
        "/api/v1/users/{1}",
        [&findUser](const drogon::HttpRequestPtr &,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                    const std::string &id) {
            const auto started = Clock::now();
            const auto traceId = drogon::utils::getUuid();
            findUser.execute(id, [callback = std::move(callback), traceId,
                                  started](FindUserResult result) mutable {
                drogon::HttpResponsePtr response;
                int status = 200;
                if (const auto *user = std::get_if<User>(&result)) {
                    Json::Value body;
                    body["id"] = user->id;
                    body["name"] = user->name;
                    body["email"] = user->email;
                    response = drogon::HttpResponse::newHttpJsonResponse(body);
                    response->addHeader("X-Request-ID", traceId);
                } else {
                    switch (std::get<FindUserError>(result)) {
                    case FindUserError::InvalidId:
                        status = 400;
                        response = errorResponse("INVALID_ID", "Invalid user id", traceId,
                                                 drogon::k400BadRequest);
                        break;
                    case FindUserError::NotFound:
                        status = 404;
                        response = errorResponse("USER_NOT_FOUND", "User was not found", traceId,
                                                 drogon::k404NotFound);
                        break;
                    case FindUserError::Unavailable:
                        status = 500;
                        response = errorResponse("INTERNAL_ERROR", "Request failed", traceId,
                                                 drogon::k500InternalServerError);
                        break;
                    }
                }
                logRequest(traceId, "GET", "/api/v1/users/{id}", status, started);
                callback(response);
            });
        },
        {drogon::Get});

    drogon::app().registerHandler(
        "/api/v1/users/{1}",
        [&deleteUser](const drogon::HttpRequestPtr &,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                      const std::string &id) {
            const auto started = Clock::now();
            const auto traceId = drogon::utils::getUuid();
            deleteUser.execute(id, [callback = std::move(callback), traceId,
                                    started](DeleteUserResult result) mutable {
                drogon::HttpResponsePtr response;
                int status = 204;
                if (std::holds_alternative<std::monostate>(result)) {
                    response = drogon::HttpResponse::newHttpResponse();
                    response->setStatusCode(drogon::k204NoContent);
                    response->addHeader("X-Request-ID", traceId);
                } else {
                    switch (std::get<DeleteUserError>(result)) {
                    case DeleteUserError::InvalidId:
                        status = 400;
                        response = errorResponse("INVALID_ID", "Invalid user id", traceId,
                                                 drogon::k400BadRequest);
                        break;
                    case DeleteUserError::NotFound:
                        status = 404;
                        response = errorResponse("USER_NOT_FOUND", "User was not found", traceId,
                                                 drogon::k404NotFound);
                        break;
                    case DeleteUserError::Unavailable:
                        status = 500;
                        response = errorResponse("INTERNAL_ERROR", "Request failed", traceId,
                                                 drogon::k500InternalServerError);
                        break;
                    }
                }
                logRequest(traceId, "DELETE", "/api/v1/users/{id}", status, started);
                callback(response);
            });
        },
        {drogon::Delete});
}
} // namespace example
