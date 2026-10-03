#include "application/DeleteUserUseCase.hpp"

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

constexpr auto userId = "123e4567-e89b-12d3-a456-426614174000";

TEST(DeleteUserUseCaseTest, ExistingUserIsDeleted) {
    MockUserRepository repository;
    EXPECT_CALL(repository, deleteById(userId, testing::_, testing::_))
        .WillOnce([](std::string, UserRepository::OnDeleted deleted, UserRepository::OnError) {
            deleted(true);
        });

    DeleteUserUseCase useCase{repository};
    useCase.execute(userId, [](DeleteUserResult result) {
        EXPECT_TRUE(std::holds_alternative<std::monostate>(result));
    });
}

TEST(DeleteUserUseCaseTest, InvalidIdDoesNotCallRepository) {
    MockUserRepository repository;
    EXPECT_CALL(repository, deleteById(testing::_, testing::_, testing::_)).Times(0);

    DeleteUserUseCase useCase{repository};
    useCase.execute("invalid", [](DeleteUserResult result) {
        ASSERT_TRUE(std::holds_alternative<DeleteUserError>(result));
        EXPECT_EQ(std::get<DeleteUserError>(result), DeleteUserError::InvalidId);
    });
}

TEST(DeleteUserUseCaseTest, MissingUserReturnsNotFound) {
    MockUserRepository repository;
    EXPECT_CALL(repository, deleteById(userId, testing::_, testing::_))
        .WillOnce([](std::string, UserRepository::OnDeleted deleted, UserRepository::OnError) {
            deleted(false);
        });

    DeleteUserUseCase useCase{repository};
    useCase.execute(userId, [](DeleteUserResult result) {
        ASSERT_TRUE(std::holds_alternative<DeleteUserError>(result));
        EXPECT_EQ(std::get<DeleteUserError>(result), DeleteUserError::NotFound);
    });
}

TEST(DeleteUserUseCaseTest, RepositoryFailureReturnsUnavailable) {
    MockUserRepository repository;
    EXPECT_CALL(repository, deleteById(userId, testing::_, testing::_))
        .WillOnce([](std::string, UserRepository::OnDeleted, UserRepository::OnError error) {
            error(RepositoryError::Unavailable);
        });

    DeleteUserUseCase useCase{repository};
    useCase.execute(userId, [](DeleteUserResult result) {
        ASSERT_TRUE(std::holds_alternative<DeleteUserError>(result));
        EXPECT_EQ(std::get<DeleteUserError>(result), DeleteUserError::Unavailable);
    });
}
} // namespace
} // namespace example
