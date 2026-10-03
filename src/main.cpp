#include "application/CreateUserUseCase.hpp"
#include "application/DeleteAllUsersUseCase.hpp"
#include "application/DeleteUserUseCase.hpp"
#include "application/FindUserUseCase.hpp"
#include "application/ListUsersUseCase.hpp"
#include "config/HttpPort.hpp"
#include "infrastructure/PostgresUserRepository.hpp"
#include "presentation/HttpApi.hpp"

#include <drogon/drogon.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
std::string env(const char *key, const char *fallback) {
    const auto *value = std::getenv(key);
    return value == nullptr ? fallback : value;
}

std::string pgValue(const std::string &value) {
    std::string quoted = "'";
    for (const char character : value) {
        if (character == '\'' || character == '\\') {
            quoted += '\\';
        }
        quoted += character;
    }
    return quoted + "'";
}
} // namespace

int run() {
    const int port = example::config::parseHttpPort(env("HTTP_PORT", "8080"));

    const auto connection = "host=" + pgValue(env("DATABASE_HOST", "db")) +
                            " port=" + pgValue(env("DATABASE_PORT", "5432")) +
                            " dbname=" + pgValue(env("DATABASE_NAME", "drogon")) +
                            " user=" + pgValue(env("DATABASE_USER", "drogon")) +
                            " password=" + pgValue(env("DATABASE_PASSWORD", "localdev"));
    auto client = drogon::orm::DbClient::newPgClient(connection, 4);
    example::PostgresUserRepository repository{client};
    example::CreateUserUseCase createUser{repository};
    example::FindUserUseCase findUser{repository};
    example::ListUsersUseCase listUsers{repository};
    example::DeleteUserUseCase deleteUser{repository};
    example::DeleteAllUsersUseCase deleteAllUsers{repository};
    example::registerHttpApi(createUser, findUser, listUsers, deleteUser, deleteAllUsers);

    const auto logLevel = env("LOG_LEVEL", "INFO");
    if (logLevel == "DEBUG") {
        drogon::app().setLogLevel(trantor::Logger::kDebug);
    } else if (logLevel == "WARN") {
        drogon::app().setLogLevel(trantor::Logger::kWarn);
    } else if (logLevel == "ERROR") {
        drogon::app().setLogLevel(trantor::Logger::kError);
    } else {
        drogon::app().setLogLevel(trantor::Logger::kInfo);
    }
    drogon::app().addListener("0.0.0.0", static_cast<uint16_t>(port)).run();
    return 0;
}

int main() {
    try {
        return run();
    } catch (const std::exception &error) {
        std::cerr << "Application startup failed: " << error.what() << '\n';
        return 1;
    }
}
