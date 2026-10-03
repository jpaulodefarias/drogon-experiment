#include "application/CreateUserUseCase.hpp"

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

TEST(CreateUserUseCaseTest, ValidInput_PersistsUser) {
    MockUserRepository repository;
    EXPECT_CALL(repository, save(testing::_, testing::_, testing::_))
        .WillOnce([](User user, UserRepository::OnSaved saved, UserRepository::OnError) {
            user.id = "test-id";
            saved(std::move(user));
        });
    CreateUserUseCase useCase{repository};
    bool called = false;
    useCase.execute({"John", "john@example.com"}, [&](CreateUserResult result) {
        called = true;
        ASSERT_TRUE(std::holds_alternative<User>(result));
        EXPECT_EQ(std::get<User>(result).id, "test-id");
    });
    EXPECT_TRUE(called);
}

TEST(CreateUserUseCaseTest, InvalidEmail_RejectsWithoutPersistence) {
    MockUserRepository repository;
    EXPECT_CALL(repository, save(testing::_, testing::_, testing::_)).Times(0);
    CreateUserUseCase useCase{repository};
    useCase.execute({"John", "invalid"}, [](CreateUserResult result) {
        ASSERT_TRUE(std::holds_alternative<CreateUserError>(result));
        EXPECT_EQ(std::get<CreateUserError>(result), CreateUserError::InvalidInput);
    });
}

TEST(CreateUserUseCaseTest, DuplicateEmail_ReturnsConflict) {
    MockUserRepository repository;
    EXPECT_CALL(repository, save(testing::_, testing::_, testing::_))
        .WillOnce([](User, UserRepository::OnSaved, UserRepository::OnError error) {
            error(RepositoryError::Conflict);
        });
    CreateUserUseCase useCase{repository};
    useCase.execute({"John", "john@example.com"}, [](CreateUserResult result) {
        ASSERT_TRUE(std::holds_alternative<CreateUserError>(result));
        EXPECT_EQ(std::get<CreateUserError>(result), CreateUserError::Conflict);
    });
}
} // namespace
} // namespace example
