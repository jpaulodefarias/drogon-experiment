# Software Design Document — Modern C++ REST API

**Status:** Draft  
**Version:** 0.1  
**Language:** C++20  
**Architecture:** Clean Architecture / Hexagonal-inspired  
**Primary Framework:** Drogon  

---

# 1. Overview

This document describes the initial technical design for a REST API implemented with modern C++.

The project should prioritize:

- maintainability;
- testability;
- explicit dependencies;
- clear separation between business logic and infrastructure;
- modern C++ practices;
- predictable builds;
- good developer experience;
- production readiness.

The API will use **Drogon** as the HTTP framework while keeping the core application independent from Drogon whenever possible.

The initial implementation should focus on establishing the project structure, build system, testing infrastructure, database integration, observability, and a minimal HTTP API.

---

# 2. Goals

The initial version of the project must provide:

- REST API using Drogon;
- C++20;
- PostgreSQL integration;
- dependency management with Conan 2;
- build system using CMake;
- automated tests using GoogleTest and GoogleMock;
- dependency inversion between domain/application and infrastructure;
- structured logging;
- health-check endpoint;
- Docker-based local environment;
- VS Code Dev Container as the default development environment, with build tools and dependencies inside containers;
- static analysis using clang-tidy;
- automatic formatting using clang-format;
- AddressSanitizer and UndefinedBehaviorSanitizer support;
- initial project structure for local development.

The architecture should allow business features to be added without coupling application logic to:

- Drogon;
- PostgreSQL;
- HTTP;
- JSON serialization;
- operating system APIs.

---

# 3. Non-goals

The first iteration will not attempt to implement:

- authentication;
- authorization;
- distributed tracing;
- Kafka integration;
- Redis;
- Kubernetes;
- service mesh;
- advanced caching;
- CQRS;
- event sourcing;
- complex domain logic;
- generic dependency injection containers;
- premature framework abstractions.

These features may be introduced later through explicit design decisions.

---

# 4. Technology Stack

## 4.1 Language

```text
C++20
```

C++20 should be considered the baseline language version.

Prefer modern language features when they improve readability or safety.

Examples include:

- `std::optional`;
- `std::variant`;
- `std::string_view`;
- ranges;
- concepts where appropriate;
- structured bindings;
- designated initializers;
- smart pointers;
- RAII;
- coroutines where they provide clear value.

Avoid using language features only for novelty.

---

# 5. HTTP Framework

The application will use:

```text
Drogon
```

Drogon will be responsible for infrastructure concerns such as:

- HTTP server;
- routing;
- controllers;
- middleware/filters;
- HTTP request/response handling;
- JSON serialization;
- database connection management;
- asynchronous I/O.

Drogon-specific types should generally remain inside the infrastructure and presentation layers.

Business logic should not depend directly on Drogon.

---

# 6. Build System

The project will use:

```text
CMake
```

Target-based modern CMake must be preferred.

Avoid globally configuring include paths, compile flags, or dependencies whenever possible.

Prefer:

```cmake
target_link_libraries(...)
target_include_directories(...)
target_compile_features(...)
target_compile_options(...)
```

instead of global commands.

Minimum expected version:

```text
CMake >= 3.25
```

---

# 7. Dependency Management

Dependencies will be managed using:

```text
Conan 2
```

Conan should integrate with CMake through:

```text
CMakeToolchain
CMakeDeps
```

Dependencies should not be manually downloaded into the repository.

Initial dependencies may include:

```text
Drogon
GoogleTest
spdlog (optional)
```

Prefer framework-provided functionality when it is sufficient before introducing new dependencies.

---

# 8. Database

The primary database will be:

```text
PostgreSQL
```

The initial implementation should use Drogon's database client.

Example dependency direction:

```text
Application
     |
     v
Repository interface
     ^
     |
PostgresRepository
     |
     v
Drogon DbClient
     |
     v
PostgreSQL
```

Business/application layers must not contain SQL.

SQL and database mapping belong to the infrastructure layer.

---

# 9. Architecture

The project will use a pragmatic Clean Architecture approach.

Primary dependency direction:

```text
Presentation
     |
     v
Application
     |
     v
Domain

Infrastructure
     |
     +---- implements interfaces defined by inner layers
```

The inner layers must not depend on outer layers.

The expected flow for a request is:

```text
HTTP Request
     |
     v
Controller
     |
     v
Use Case
     |
     v
Domain
     |
     v
Repository Interface
     ^
     |
Repository Implementation
     |
     v
PostgreSQL
```

---

# 10. Layers

## 10.1 Domain

Contains business concepts.

Expected contents:

```text
domain/
├── model/
├── repository/
├── service/
└── error/
```

The domain layer must not depend on:

- Drogon;
- PostgreSQL;
- JSON;
- HTTP;
- filesystem APIs;
- external frameworks.

Example:

```cpp
struct User {
    std::string id;
    std::string name;
    std::string email;
};
```

Domain entities should contain behavior when business rules exist.

Avoid anemic domain models when meaningful domain behavior exists.

---

# 11. Application Layer

Contains application use cases.

Example:

```text
application/
├── dto/
├── usecase/
└── port/
```

Example use case:

```cpp
class CreateUserUseCase {
public:
    explicit CreateUserUseCase(UserRepository& repository);

    User execute(const CreateUserCommand& command);

private:
    UserRepository& repository_;
};
```

Use cases orchestrate application behavior.

They may:

- validate application-level rules;
- invoke domain behavior;
- invoke repository interfaces;
- coordinate multiple domain operations.

They must not know about:

- HTTP;
- Drogon controllers;
- SQL;
- PostgreSQL connection objects.

---

# 12. Repository Interfaces

Repository interfaces should normally live in an inner layer.

Example:

```cpp
class UserRepository {
public:
    virtual ~UserRepository() = default;

    virtual User save(const User& user) = 0;

    virtual std::optional<User> findById(
        std::string_view id
    ) = 0;
};
```

Infrastructure will provide concrete implementations.

Example:

```text
PostgresUserRepository
```

---

# 13. Infrastructure Layer

Infrastructure contains implementations for external systems.

Initial structure:

```text
infrastructure/
├── database/
│   ├── postgres/
│   └── migration/
├── repository/
└── logging/
```

Example:

```cpp
class PostgresUserRepository final : public UserRepository {
public:
    explicit PostgresUserRepository(
        drogon::orm::DbClientPtr dbClient
    );

    User save(const User& user) override;

    std::optional<User> findById(
        std::string_view id
    ) override;

private:
    drogon::orm::DbClientPtr dbClient_;
};
```

Drogon-specific database code should stay here.

---

# 14. Presentation Layer

HTTP controllers belong to:

```text
presentation/http/
```

Responsibilities:

- HTTP routing;
- parsing requests;
- request validation;
- converting HTTP DTOs to application commands;
- invoking use cases;
- mapping application responses to HTTP responses;
- translating application errors to HTTP status codes.

Controllers should contain minimal business logic.

Example:

```text
POST /api/v1/users
        |
        v
UserController
        |
        v
CreateUserUseCase
```

---

# 15. Dependency Injection

The application should initially use explicit constructor injection.

Example:

```cpp
PostgresUserRepository repository{dbClient};

CreateUserUseCase createUser{
    repository
};

UserController controller{
    createUser
};
```

Do not introduce a dependency injection framework initially.

Dependencies should be visible and explicit.

---

# 16. Proposed Directory Structure

Initial project structure:

```text
.
├── CMakeLists.txt
├── conanfile.py
├── Dockerfile
├── docker-compose.yml
├── .devcontainer/
│   ├── devcontainer.json
│   ├── Dockerfile
│   └── compose.yaml
├── README.md
├── sdd.md
│
├── cmake/
│
├── config/
│   ├── development.json
│   └── test.json
│
├── migrations/
│
├── src/
│   ├── main.cpp
│   │
│   ├── domain/
│   │   ├── model/
│   │   ├── repository/
│   │   ├── service/
│   │   └── error/
│   │
│   ├── application/
│   │   ├── dto/
│   │   ├── usecase/
│   │   └── port/
│   │
│   ├── infrastructure/
│   │   ├── database/
│   │   │   └── postgres/
│   │   ├── repository/
│   │   └── logging/
│   │
│   └── presentation/
│       └── http/
│           ├── controller/
│           ├── dto/
│           ├── mapper/
│           └── middleware/
│
└── tests/
    ├── unit/
    │   ├── domain/
    │   └── application/
    │
    └── integration/
        ├── database/
        └── http/
```

The structure may evolve when real domain boundaries become clear.

Do not create unnecessary abstractions simply to preserve this directory structure.

---

# 17. HTTP API

The initial API must expose:

```http
GET /health
```

Expected response:

```json
{
  "status": "UP"
}
```

Expected HTTP status:

```text
200 OK
```

A second endpoint may be introduced as the first reference feature:

```http
POST /api/v1/users
GET /api/v1/users/{id}
```

`User` is initially only a reference implementation and may later be replaced by the actual project domain.

---

# 18. HTTP Conventions

Use resource-oriented URLs.

Preferred:

```text
GET    /api/v1/users
GET    /api/v1/users/{id}
POST   /api/v1/users
PUT    /api/v1/users/{id}
DELETE /api/v1/users/{id}
DELETE /api/v1/users
```

The collection-level `DELETE /api/v1/users` removes every user and returns `204`, including when the collection is already empty. This operation is destructive and should be protected by authorization before production use.

Avoid action-oriented routes such as:

```text
POST /api/createUser
POST /api/updateUser
```

HTTP status codes should reflect the operation result.

Examples:

```text
200 OK
201 Created
204 No Content
400 Bad Request
404 Not Found
409 Conflict
500 Internal Server Error
```

---

# 19. Error Model

The API should expose a consistent error structure.

Example:

```json
{
  "code": "USER_NOT_FOUND",
  "message": "User was not found",
  "traceId": "..."
}
```

Expected fields:

```text
code
message
traceId
```

Optional fields:

```text
details
fieldErrors
```

Internal implementation details must never be returned to clients.

For example, never expose:

- SQL queries;
- PostgreSQL errors;
- filesystem paths;
- stack traces.

---

# 20. Logging

Logging must use structured information whenever possible.

Expected fields include:

```text
timestamp
level
message
traceId
method
path
status
duration
```

Avoid logging sensitive request payloads by default.

Drogon's logging facilities should initially be preferred.

`spdlog` may be introduced if additional logging capabilities are required.

---

# 21. Testing Strategy

Testing will use:

```text
GoogleTest
GoogleMock
```

The project should have two main test categories.

## Unit tests

Unit tests cover:

- domain entities;
- domain services;
- use cases;
- application services.

Infrastructure should be mocked through interfaces.

Example:

```cpp
TEST(CreateUserUseCaseTest, ShouldCreateUser) {
    MockUserRepository repository;

    EXPECT_CALL(repository, save(testing::_))
        .Times(1);

    CreateUserUseCase useCase{
        repository
    };

    auto result = useCase.execute({
        .name = "John",
        .email = "john@example.com"
    });

    EXPECT_EQ(result.name, "John");
}
```

---

# 22. Integration Tests

Integration tests should cover:

```text
Application
      +
Infrastructure
      +
PostgreSQL
```

HTTP integration tests may additionally run the Drogon application.

PostgreSQL should preferably run using Docker for integration testing.

Integration tests should not mock the database.

---

# 23. Test Naming

Tests should follow the pattern:

```text
Method_Scenario_ExpectedResult
```

or readable behavior-based names.

Examples:

```text
CreateUser_WhenInputIsValid_ShouldPersistUser

FindUser_WhenUserDoesNotExist_ShouldReturnEmpty

CreateUser_WhenEmailAlreadyExists_ShouldReturnConflict
```

Consistency is more important than a specific naming convention.

---

# 24. Compiler

The project should work with:

```text
Clang
```

and preferably also:

```text
GCC
```

Primary development compiler:

```text
Clang
```

---

# 25. Static Analysis

The project should use:

```text
clang-tidy
```

Initial checks should focus on:

```text
bugprone-*
performance-*
modernize-*
readability-*
cppcoreguidelines-*
```

Rules may be selectively disabled if they reduce readability or conflict with project design decisions.

Warnings should be treated seriously.

---

# 26. Formatting

Source code formatting will use:

```text
clang-format
```

A `.clang-format` file must exist at repository root.

Formatting should be deterministic and automated.

---

# 27. Compiler Warnings

Development builds should enable strict warnings.

Equivalent Clang/GCC options should include:

```text
-Wall
-Wextra
-Wpedantic
-Wconversion
-Wshadow
```

Warnings should ideally be treated as errors:

```text
-Werror
```

Third-party dependencies must not generate project compilation failures because of external warnings.

---

# 28. Sanitizers

Debug/test builds should support:

```text
AddressSanitizer
UndefinedBehaviorSanitizer
```

Equivalent compiler flags:

```text
-fsanitize=address
-fsanitize=undefined
```

Sanitizers should be easy to enable through CMake.

Example:

```text
-DENABLE_SANITIZERS=ON
```

---

# 29. Memory Management

Prefer RAII.

Prefer stack allocation and value semantics whenever reasonable.

Prefer:

```text
std::unique_ptr
```

when exclusive ownership is required.

Use:

```text
std::shared_ptr
```

only when ownership is genuinely shared.

Avoid manual:

```cpp
new
delete
malloc
free
```

in application code unless required by low-level integrations.

---

# 30. Exception Policy

Exceptions may be used for exceptional conditions.

They should not be used as normal control flow.

Domain/application-specific errors should preferably use explicit error types.

The HTTP layer will translate application errors into HTTP responses.

Database exceptions must not leak outside infrastructure.

---

# 31. Configuration

Configuration must not be hard-coded into application logic.

Environment-specific values include:

```text
HTTP_PORT
DATABASE_HOST
DATABASE_PORT
DATABASE_NAME
DATABASE_USER
DATABASE_PASSWORD
LOG_LEVEL
```

Secrets must be supplied externally.

Do not commit secrets to the repository.

---

# 32. Docker

The default development workflow uses VS Code with the Dev Container in
`.devcontainer/`. The host uses Docker and VS Code; CMake, Conan, compilers,
the debugger, clang-format, and clang-tidy run inside the development
container. The source tree is mounted into the container, while the Conan
cache and PostgreSQL data use Docker volumes.

The Dev Container's Compose configuration starts PostgreSQL alongside the
development container. The database is reachable from the development
container at `db:5432`. Its default credentials are for local development
only; `POSTGRES_PASSWORD` can override the password. Do not use these
defaults for deployed environments.

The repository should include:

```text
Dockerfile
docker-compose.yml
```

Local development should allow:

```bash
docker compose up
```

to start external infrastructure such as PostgreSQL.

The application should run inside the Dev Container during development.
Production image construction and deployment remain separate from this
development environment. Initial development should optimize for a fast
edit/build/test cycle through the mounted source tree and persistent Conan
cache.

---

# 33. Development Workflow

Open the project in VS Code and use **Dev Containers: Reopen in Container**
from the Command Palette to create the Dev Container with mounted sources.
Use the integrated terminal in the container for the basic workflow once the
CMake and Conan project files exist:

```bash
conan install . \
    --output-folder=build \
    --build=missing

cmake \
    --preset conan-debug

cmake \
    --build \
    --preset conan-debug

ctest \
    --preset conan-debug
```

No host installation of CMake, Conan, compilers, or PostgreSQL is required.
Exact presets may change based on generated Conan configuration.

---

# 34. CMake Targets

Avoid creating one giant executable target.

Prefer targets representing architectural modules.

Example:

```text
domain
application
infrastructure
presentation
api
unit_tests
integration_tests
```

Dependency direction should be enforced through target dependencies.

Example:

```text
domain

application
  -> domain

infrastructure
  -> domain
  -> application

presentation
  -> application

api
  -> presentation
  -> infrastructure
```

This should make invalid architectural dependencies harder to introduce accidentally.

---

# 35. Build Types

At minimum support:

```text
Debug
Release
```

Debug:

```text
debug symbols
sanitizers optionally enabled
tests enabled
```

Release:

```text
optimizations enabled
sanitizers disabled
```

---

# 36. Initial Database Schema

If the reference `User` feature is implemented, the first migration may create:

```sql
CREATE TABLE users (
    id UUID PRIMARY KEY,
    name VARCHAR(200) NOT NULL,
    email VARCHAR(320) NOT NULL UNIQUE,
    created_at TIMESTAMP WITH TIME ZONE NOT NULL,
    updated_at TIMESTAMP WITH TIME ZONE NOT NULL
);
```

Schema changes should be versioned using migrations.

Do not rely on automatic schema creation in production.

---

# 37. Initial Feature

The first complete vertical slice should preferably be:

```text
Create User
```

Flow:

```text
POST /api/v1/users

        |
        v

UserController

        |
        v

CreateUserUseCase

        |
        v

UserRepository

        |
        v

PostgresUserRepository

        |
        v

PostgreSQL
```

Request:

```json
{
  "name": "John Doe",
  "email": "john@example.com"
}
```

Response:

```json
{
  "id": "uuid",
  "name": "John Doe",
  "email": "john@example.com"
}
```

HTTP status:

```text
201 Created
```

---

# 38. Implementation Phases

## Phase 1 — Project bootstrap

Implement:

- CMake;
- Conan 2;
- Drogon;
- project executable;
- GoogleTest;
- clang-format;
- clang-tidy;
- basic directory structure.
- Dev Container with compiler, Conan 2, CMake, analysis tools, and PostgreSQL.

Acceptance criteria:

```bash
cmake --build ...
```

succeeds.

Tests execute successfully.

---

## Phase 2 — Minimal HTTP server

Implement:

```http
GET /health
```

Acceptance criteria:

```http
GET /health
```

returns:

```http
200 OK
```

with:

```json
{
  "status": "UP"
}
```

---

## Phase 3 — PostgreSQL

Add:

- PostgreSQL container;
- database configuration;
- Drogon DbClient;
- database migration infrastructure;
- database health verification.

---

## Phase 4 — First vertical slice

Implement:

```text
User
CreateUserUseCase
UserRepository
PostgresUserRepository
UserController
```

Endpoint:

```http
POST /api/v1/users
```

---

## Phase 5 — Testing

Implement:

- domain tests;
- use-case tests;
- repository integration tests;
- HTTP integration tests.

---

## Phase 6 — Quality tooling

Enable:

```text
clang-tidy
clang-format
ASan
UBSan
compiler warnings
```

Add reproducible commands for local quality checks.

---

# 39. Definition of Done

A feature is considered complete when:

- code compiles without project warnings;
- unit tests pass;
- relevant integration tests pass;
- formatting passes;
- clang-tidy reports no blocking issues;
- public APIs are documented where necessary;
- HTTP errors follow the standard error format;
- no business logic exists inside controllers;
- infrastructure dependencies do not leak into domain/application code.

---

# 40. Design Principles

The implementation should prefer:

```text
explicit > implicit
composition > inheritance
RAII > manual resource management
value semantics > unnecessary pointers
constructor injection > service locator
simple abstraction > generic abstraction
compile-time guarantees > runtime checks when practical
```

Avoid:

```text
global mutable state
singletons used as dependency containers
unnecessary shared_ptr
deep inheritance hierarchies
framework types inside the domain
premature generic abstractions
```

---

# 41. Codex Implementation Guidelines

When implementing this specification:

1. Prefer incremental changes.
2. Keep each architectural layer independently understandable.
3. Do not introduce a dependency without a clear requirement.
4. Do not move business logic into Drogon controllers.
5. Use constructor injection for dependencies.
6. Prefer RAII and value semantics.
7. Avoid raw owning pointers.
8. Add tests alongside application behavior.
9. Maintain dependency direction between layers.
10. Update this specification when architectural decisions change.

Before implementing a large architectural change, document the decision first.

---

# 42. Initial Codex Tasks

Suggested first tasks for Codex:

```text
1. Bootstrap a C++20 project using CMake and Conan 2.

2. Add Drogon as the HTTP framework.

3. Add GoogleTest and GoogleMock.

4. Create the initial architectural modules:
   - domain
   - application
   - infrastructure
   - presentation

5. Create the application executable.

6. Implement GET /health returning:
   {
     "status": "UP"
   }

7. Add unit/integration test infrastructure.

8. Add Docker Compose with PostgreSQL.

8a. Use the Dev Container as the default environment for all build and test tasks.

9. Add clang-format and clang-tidy configuration.

10. Add optional AddressSanitizer and UndefinedBehaviorSanitizer support.

11. Verify the application builds and tests successfully.

12. Do not implement business-domain features beyond what is necessary
    to validate the architecture.
```

---

# 43. Future Decisions

The following decisions should be made when requirements justify them:

```text
Authentication strategy
Authorization model
Database migration tool
OpenAPI generation
Tracing / OpenTelemetry
Metrics / Prometheus
Redis
Kafka
Rate limiting
Idempotency
Pagination conventions
API versioning strategy
Graceful shutdown
Secrets management
Production deployment
```

These decisions should not be introduced prematurely.

---

# 44. Architecture Decision Log

Important architectural decisions should be recorded as ADRs.

Suggested structure:

```text
docs/
└── adr/
    ├── 0001-use-drogon.md
    ├── 0002-use-clean-architecture.md
    ├── 0003-use-conan.md
    └── 0004-use-postgresql.md
```

Each ADR should contain:

```text
Context
Decision
Consequences
Alternatives Considered
```

---

# 45. Current Architectural Decisions

## ADR-001 — Drogon

Decision:

```text
Use Drogon as the HTTP framework.
```

Rationale:

```text
Provides routing, asynchronous HTTP, database integration,
connection pooling and production-oriented functionality while
remaining compatible with modern C++.
```

---

## ADR-002 — Clean Architecture

Decision:

```text
Keep domain and application code independent from Drogon.
```

Rationale:

```text
Improves testability and keeps framework concerns outside
business logic.
```

---

## ADR-003 — CMake + Conan

Decision:

```text
Use CMake for builds and Conan 2 for dependency management.
```

---

## ADR-004 — PostgreSQL

Decision:

```text
Use PostgreSQL as the primary relational database.
```

---

## ADR-005 — GoogleTest

Decision:

```text
Use GoogleTest and GoogleMock for automated testing.
```

---

## ADR-006 — Dev Container

Decision:

```text
Use VS Code with a Dev Container as the default development environment. Run
the toolchain, Conan, builds, and tests inside it, with PostgreSQL provided
by Docker Compose.
```

Rationale:

```text
Developers use Docker and VS Code on the host without installing C++ tooling.
Named volumes retain dependency and database data across container rebuilds.
```

---

# 46. Guiding Constraint

The project should remain a **modern C++ application that happens to expose HTTP**, rather than attempting to recreate Spring Boot in C++.

Framework convenience must not override:

```text
ownership clarity
dependency clarity
resource safety
compile-time safety
testability
```
