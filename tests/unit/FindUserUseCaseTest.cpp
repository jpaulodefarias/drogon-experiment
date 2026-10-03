#include "application/FindUserUseCase.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace example {
namespace {
class MockUserRepository final : public UserRepository {
  public:
    MOCK_METHOD(void, save, (User, OnSaved, OnError), (override));
    MOCK_METHOD(void, findById, (std::string, OnFound, OnError), (override));
    MOCK_METHOD(void, list, (OnListed, OnError), (override));
    MOCK_METHOD(void, deleteById, (std::string, OnDeleted, OnError), (override));
    MOCK_METHOD(void, deleteAll, (OnDeletedAll, OnError), (override));
};

TEST(FindUserUseCaseTest, ExistingId_ReturnsUser) {
    MockUserRepository repository;
    EXPECT_CALL(repository, findById(testing::_, testing::_, testing::_))
        .WillOnce([](std::string id, UserRepository::OnFound found, UserRepository::OnError) {
            found(User{id, "John", "john@example.com"});
        });
    FindUserUseCase useCase{repository};
    useCase.execute("123e4567-e89b-12d3-a456-426614174000", [](FindUserResult result) {
        ASSERT_TRUE(std::holds_alternative<User>(result));
        EXPECT_EQ(std::get<User>(result).name, "John");
    });
}

TEST(FindUserUseCaseTest, MalformedId_RejectsWithoutLookup) {
    MockUserRepository repository;
    EXPECT_CALL(repository, findById(testing::_, testing::_, testing::_)).Times(0);
    FindUserUseCase useCase{repository};
    useCase.execute("invalid", [](FindUserResult result) {
        ASSERT_TRUE(std::holds_alternative<FindUserError>(result));
        EXPECT_EQ(std::get<FindUserError>(result), FindUserError::InvalidId);
    });
}

TEST(FindUserUseCaseTest, UUIDWithIncorrectSeparatorsIsRejected) {
    MockUserRepository repository;
    EXPECT_CALL(repository, findById(testing::_, testing::_, testing::_)).Times(0);
    FindUserUseCase useCase{repository};
    useCase.execute("123e4567e89b-12d3-a456-426614174000", [](FindUserResult result) {
        ASSERT_TRUE(std::holds_alternative<FindUserError>(result));
        EXPECT_EQ(std::get<FindUserError>(result), FindUserError::InvalidId);
    });
}

TEST(FindUserUseCaseTest, MissingId_ReturnsNotFound) {
    MockUserRepository repository;
    EXPECT_CALL(repository, findById(testing::_, testing::_, testing::_))
        .WillOnce([](std::string, UserRepository::OnFound found, UserRepository::OnError) {
            found(std::nullopt);
        });
    FindUserUseCase useCase{repository};
    useCase.execute("123e4567-e89b-12d3-a456-426614174000", [](FindUserResult result) {
        ASSERT_TRUE(std::holds_alternative<FindUserError>(result));
        EXPECT_EQ(std::get<FindUserError>(result), FindUserError::NotFound);
    });
}
} // namespace
} // namespace example
