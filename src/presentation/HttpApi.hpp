#pragma once

#include "application/CreateUserUseCase.hpp"
#include "application/DeleteAllUsersUseCase.hpp"
#include "application/DeleteUserUseCase.hpp"
#include "application/FindUserUseCase.hpp"
#include "application/ListUsersUseCase.hpp"

namespace example {
void registerHttpApi(const CreateUserUseCase &createUser, const FindUserUseCase &findUser,
                     const ListUsersUseCase &listUsers, const DeleteUserUseCase &deleteUser,
                     const DeleteAllUsersUseCase &deleteAllUsers);
} // namespace example
