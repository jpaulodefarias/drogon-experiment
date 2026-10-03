cmake_minimum_required(VERSION 3.25)
project(drogon_experiment VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
option(ENABLE_WERROR "Treat project warnings as errors" OFF)
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR "ENABLE_SANITIZERS is supported for Debug builds only")
endif()

find_package(Drogon CONFIG REQUIRED)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/docs/openapi.json" OPENAPI_JSON)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/docs/swagger.html" SWAGGER_HTML)
configure_file(cmake/ApiDocumentation.hpp.in generated/ApiDocumentation.hpp @ONLY)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(project_options INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(ENABLE_WERROR)
        target_compile_options(project_options INTERFACE -Werror)
    endif()
    if(ENABLE_SANITIZERS)
        target_compile_options(project_options INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(project_options INTERFACE -fsanitize=address,undefined)
    endif()
endif()

add_library(domain INTERFACE)
target_include_directories(domain INTERFACE src)
target_link_libraries(domain INTERFACE project_options)

add_library(application src/application/CreateUserUseCase.cpp src/application/FindUserUseCase.cpp
                        src/application/ListUsersUseCase.cpp src/application/DeleteUserUseCase.cpp
                        src/application/DeleteAllUsersUseCase.cpp)
target_link_libraries(application PUBLIC domain)

add_library(infrastructure src/infrastructure/PostgresUserRepository.cpp)
target_link_libraries(infrastructure PUBLIC application Drogon::Drogon)

add_library(presentation src/presentation/HttpApi.cpp src/presentation/UsersController.cpp
                         src/presentation/HealthController.cpp
                         src/presentation/DocumentationController.cpp)
target_include_directories(presentation PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(presentation PUBLIC application Drogon::Drogon)

add_executable(api src/main.cpp)
target_link_libraries(api PRIVATE presentation infrastructure)

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    add_executable(unit_tests tests/unit/CreateUserUseCaseTest.cpp tests/unit/FindUserUseCaseTest.cpp
                             tests/unit/ListUsersUseCaseTest.cpp
                             tests/unit/DeleteUserUseCaseTest.cpp
                             tests/unit/DeleteAllUsersUseCaseTest.cpp tests/unit/HttpPortTest.cpp)
    target_link_libraries(unit_tests PRIVATE application GTest::gtest_main GTest::gmock)
    include(GoogleTest)
    gtest_discover_tests(unit_tests)

    add_executable(integration_tests tests/integration/IntegrationTest.cpp)
    target_link_libraries(integration_tests PRIVATE infrastructure presentation GTest::gtest_main)
    gtest_discover_tests(integration_tests PROPERTIES LABELS integration)
    add_test(NAME http_integration
             COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/http.sh $<TARGET_FILE:api>)
    set_tests_properties(http_integration PROPERTIES LABELS integration)
endif()
