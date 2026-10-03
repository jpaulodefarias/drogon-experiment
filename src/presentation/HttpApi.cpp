#include "presentation/HttpApi.hpp"

#include "presentation/DocumentationController.hpp"
#include "presentation/HealthController.hpp"
#include "presentation/HttpApiUtils.hpp"
#include "presentation/UsersController.hpp"

namespace example {
void registerHttpApi(const CreateUserUseCase &createUser, const FindUserUseCase &findUser,
                     const ListUsersUseCase &listUsers, const DeleteUserUseCase &deleteUser,
                     const DeleteAllUsersUseCase &deleteAllUsers) {
    using presentation::detail::errorResponse;
    drogon::app().setCustomErrorHandler([](drogon::HttpStatusCode status,
                                           const drogon::HttpRequestPtr &) {
        const auto traceId = drogon::utils::getUuid();
        if (status == drogon::k404NotFound) {
            return errorResponse("NOT_FOUND", "Resource was not found", traceId, status);
        }
        if (status == drogon::k405MethodNotAllowed) {
            return errorResponse("METHOD_NOT_ALLOWED", "Method is not allowed", traceId, status);
        }
        return errorResponse("HTTP_ERROR", "Request could not be processed", traceId, status);
    });

    registerDocumentationController();
    registerHealthController();
    registerUsersController(createUser, findUser, listUsers, deleteUser, deleteAllUsers);
}
} // namespace example
