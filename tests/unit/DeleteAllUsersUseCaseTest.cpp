#include "application/DeleteAllUsersUseCase.hpp"

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

TEST(DeleteAllUsersUseCaseTest, DeletesAllUsers) {
    MockUserRepository repository;
    EXPECT_CALL(repository, deleteAll(testing::_, testing::_))
        .WillOnce([](UserRepository::OnDeletedAll deleted, UserRepository::OnError) { deleted(); });

    DeleteAllUsersUseCase useCase{repository};
    useCase.execute([](DeleteAllUsersResult result) {
        EXPECT_TRUE(std::holds_alternative<std::monostate>(result));
    });
}

TEST(DeleteAllUsersUseCaseTest, RepositoryFailureReturnsUnavailable) {
    MockUserRepository repository;
    EXPECT_CALL(repository, deleteAll(testing::_, testing::_))
        .WillOnce([](UserRepository::OnDeletedAll, UserRepository::OnError error) {
            error(RepositoryError::Unavailable);
        });

    DeleteAllUsersUseCase useCase{repository};
    useCase.execute([](DeleteAllUsersResult result) {
        ASSERT_TRUE(std::holds_alternative<DeleteAllUsersError>(result));
        EXPECT_EQ(std::get<DeleteAllUsersError>(result), DeleteAllUsersError::Unavailable);
    });
}
} // namespace
} // namespace example
