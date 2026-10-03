#include "infrastructure/PostgresUserRepository.hpp"

#include <drogon/drogon.h>

namespace example {
namespace {
void handleDatabaseError(const std::exception_ptr &error, const char *operation,
                         UserRepository::OnError onError) {
    try {
        std::rethrow_exception(error);
    } catch (const std::exception &exception) {
        LOG_ERROR << operation << ": " << exception.what();
    } catch (...) {
        LOG_ERROR << operation << ": unknown database error";
    }
    onError(RepositoryError::Unavailable);
}
} // namespace

PostgresUserRepository::PostgresUserRepository(drogon::orm::DbClientPtr client)
    : client_(std::move(client)) {}

void PostgresUserRepository::save(User user, OnSaved onSaved, OnError onError) {
    const auto name = user.name;
    const auto email = user.email;
    client_->execSqlAsync(
        "INSERT INTO users (id, name, email, created_at, updated_at) "
        "VALUES (gen_random_uuid(), $1, $2, NOW(), NOW()) "
        "ON CONFLICT (email) DO NOTHING RETURNING id",
        [user = std::move(user), onSaved = std::move(onSaved),
         onError](const drogon::orm::Result &result) mutable {
            if (result.empty()) {
                onError(RepositoryError::Conflict);
                return;
            }
            user.id = result[0]["id"].as<std::string>();
            onSaved(std::move(user));
        },
        [onError](const std::exception_ptr &error) {
            handleDatabaseError(error, "Database write failed", onError);
        },
        name, email);
}

void PostgresUserRepository::findById(std::string id, OnFound onFound, OnError onError) {
    client_->execSqlAsync(
        "SELECT id, name, email FROM users WHERE id = $1::uuid",
        [onFound = std::move(onFound)](const drogon::orm::Result &result) {
            if (result.empty()) {
                onFound(std::nullopt);
                return;
            }
            const auto &row = result[0];
            onFound(User{row["id"].as<std::string>(), row["name"].as<std::string>(),
                         row["email"].as<std::string>()});
        },
        [onError = std::move(onError)](const std::exception_ptr &error) mutable {
            handleDatabaseError(error, "Database lookup failed", std::move(onError));
        },
        id);
}

void PostgresUserRepository::list(OnListed onListed, OnError onError) {
    client_->execSqlAsync(
        "SELECT id, name, email FROM users ORDER BY created_at, id",
        [onListed = std::move(onListed)](const drogon::orm::Result &result) {
            std::vector<User> users;
            users.reserve(result.size());
            for (const auto &row : result) {
                users.push_back(User{row["id"].as<std::string>(), row["name"].as<std::string>(),
                                     row["email"].as<std::string>()});
            }
            onListed(std::move(users));
        },
        [onError = std::move(onError)](const std::exception_ptr &error) mutable {
            handleDatabaseError(error, "Database user listing failed", std::move(onError));
        });
}

void PostgresUserRepository::deleteById(std::string id, OnDeleted onDeleted, OnError onError) {
    client_->execSqlAsync(
        "DELETE FROM users WHERE id = $1::uuid RETURNING id",
        [onDeleted = std::move(onDeleted)](const drogon::orm::Result &result) {
            onDeleted(!result.empty());
        },
        [onError = std::move(onError)](const std::exception_ptr &error) mutable {
            handleDatabaseError(error, "Database user deletion failed", std::move(onError));
        },
        id);
}

void PostgresUserRepository::deleteAll(OnDeletedAll onDeleted, OnError onError) {
    client_->execSqlAsync(
        "DELETE FROM users",
        [onDeleted = std::move(onDeleted)](const drogon::orm::Result &) { onDeleted(); },
        [onError = std::move(onError)](const std::exception_ptr &error) mutable {
            handleDatabaseError(error, "Database user cleanup failed", std::move(onError));
        });
}
} // namespace example
