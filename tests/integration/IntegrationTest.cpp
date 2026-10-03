#include "infrastructure/PostgresUserRepository.hpp"

#include <drogon/drogon.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <future>
#include <string>
#include <vector>

namespace example {
namespace {
std::string env(const char *key, const char *fallback) {
    const auto *value = std::getenv(key);
    return value == nullptr ? fallback : value;
}

TEST(PostgresUserRepositoryTest, SaveListFindAndDeleteUser) {
    const auto connection =
        "host=" + env("DATABASE_HOST", "db") + " port=" + env("DATABASE_PORT", "5432") +
        " dbname=" + env("DATABASE_NAME", "drogon") + " user=" + env("DATABASE_USER", "drogon") +
        " password=" + env("DATABASE_PASSWORD", "localdev");
    auto client = drogon::orm::DbClient::newPgClient(connection, 1);
    PostgresUserRepository repository{client};
    const auto email = "test-" + drogon::utils::getUuid() + "@example.com";

    std::promise<User> created;
    repository.save(
        User{"", "John", email}, [&created](User user) { created.set_value(std::move(user)); },
        [&created](RepositoryError) {
            created.set_exception(std::make_exception_ptr(std::runtime_error("save failed")));
        });
    const auto user = created.get_future().get();
    EXPECT_FALSE(user.id.empty());
    const auto rows =
        client->execSqlSync("SELECT name, email FROM users WHERE id = $1::uuid", user.id);
    ASSERT_EQ(rows.size(), 1U);
    EXPECT_EQ(rows[0]["name"].as<std::string>(), "John");
    EXPECT_EQ(rows[0]["email"].as<std::string>(), email);

    std::promise<RepositoryError> duplicate;
    repository.save(
        User{"", "Another", email},
        [&duplicate](User) {
            duplicate.set_exception(std::make_exception_ptr(std::runtime_error("unexpected save")));
        },
        [&duplicate](RepositoryError error) { duplicate.set_value(error); });
    EXPECT_EQ(duplicate.get_future().get(), RepositoryError::Conflict);

    std::promise<std::optional<User>> found;
    repository.findById(
        user.id, [&found](std::optional<User> value) { found.set_value(std::move(value)); },
        [&found](RepositoryError) {
            found.set_exception(std::make_exception_ptr(std::runtime_error("lookup failed")));
        });
    const auto stored = found.get_future().get();
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->email, email);

    std::promise<std::optional<User>> missing;
    repository.findById(
        "00000000-0000-0000-0000-000000000000",
        [&missing](std::optional<User> value) { missing.set_value(std::move(value)); },
        [&missing](RepositoryError) {
            missing.set_exception(std::make_exception_ptr(std::runtime_error("lookup failed")));
        });
    EXPECT_FALSE(missing.get_future().get().has_value());

    std::promise<std::vector<User>> listed;
    repository.list([&listed](std::vector<User> users) { listed.set_value(std::move(users)); },
                    [&listed](RepositoryError) {
                        listed.set_exception(
                            std::make_exception_ptr(std::runtime_error("list failed")));
                    });
    const auto users = listed.get_future().get();
    EXPECT_TRUE(std::any_of(users.begin(), users.end(),
                            [&user](const User &candidate) { return candidate.id == user.id; }));

    std::promise<bool> deleted;
    repository.deleteById(
        user.id, [&deleted](bool value) { deleted.set_value(value); },
        [&deleted](RepositoryError) {
            deleted.set_exception(std::make_exception_ptr(std::runtime_error("delete failed")));
        });
    EXPECT_TRUE(deleted.get_future().get());

    std::promise<std::optional<User>> removed;
    repository.findById(
        user.id, [&removed](std::optional<User> value) { removed.set_value(std::move(value)); },
        [&removed](RepositoryError) {
            removed.set_exception(std::make_exception_ptr(std::runtime_error("lookup failed")));
        });
    EXPECT_FALSE(removed.get_future().get().has_value());
}
} // namespace
} // namespace example
