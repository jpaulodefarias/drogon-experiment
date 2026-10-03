# Drogon experiment

C++20 REST API built with Drogon, Conan 2, CMake and PostgreSQL. The default development environment is the Dev Container in `.devcontainer/`, opened with VS Code and the Dev Containers extension. The host needs Docker and VS Code; C++ build tools run in the container.

## Development

Open the project in VS Code and choose **Reopen in Container** when prompted (or run **Dev Containers: Reopen in Container** from the Command Palette). The Compose service starts PostgreSQL at `db:5432`. Alternatively, run these commands from the project root on a host with Docker Compose and Make:

```sh
make help       # list available commands
make setup      # start containers and install dependencies
make test       # build, migrate the database, and run tests
make run        # build, migrate, and run the API in the foreground
make docs       # show Swagger and OpenAPI URLs
make down       # stop the Dev Container and PostgreSQL
```

Run the Make targets from the project root on the host, since they use Docker Compose to execute commands in the Dev Container.

The Dev Container provides CMake, Conan, Clang, clang-format, clang-tidy, `psql` and `curl`. The API listens on port `8080`, forwarded to the host. Set `POSTGRES_PASSWORD` on the host before creating the container to override the local development password, `localdev`. `HTTP_PORT`, `DATABASE_HOST`, `DATABASE_PORT`, `DATABASE_NAME`, `DATABASE_USER`, `DATABASE_PASSWORD` and `LOG_LEVEL` configure the application.

HTTP routes are grouped by resource under `src/presentation/`: `UsersController` owns user endpoints, `HealthController` owns `/health`, and `DocumentationController` owns Swagger and OpenAPI endpoints. `HttpApi` registers these modules and configures shared HTTP error handling.

Open `http://localhost:8080/docs` for Swagger UI. The OpenAPI definition is available at `http://localhost:8080/openapi.json` and maintained in `docs/openapi.json`. Swagger UI assets are copied from the pinned official image into the Dev Container and the local API image, so the documentation does not require a browser connection to a CDN.

```sh
curl http://localhost:8080/health
curl http://localhost:8080/api/v1/users
curl -i -H 'Content-Type: application/json' \
  -d '{"name":"John Doe","email":"john@example.com"}' \
  http://localhost:8080/api/v1/users
curl http://localhost:8080/api/v1/users/REPLACE_WITH_RETURNED_ID
curl -i -X DELETE http://localhost:8080/api/v1/users/REPLACE_WITH_RETURNED_ID
curl -i -X DELETE http://localhost:8080/api/v1/users
```

`GET /health` returns `{"status":"UP"}` when the HTTP process is running. It does not check PostgreSQL. `GET /api/v1/users` returns all users ordered by creation time, oldest first; `GET /api/v1/users/{id}` returns an existing user or a standard `404` error; `DELETE /api/v1/users/{id}` removes one user and returns `204`, or `404` if it does not exist. `DELETE /api/v1/users` removes every user and returns `204`, including when the collection is already empty. This is a destructive operation. Run `sh scripts/migrate.sh` before using the users endpoints; the migration script records each applied SQL file and can be run again. The HTTP integration test creates and removes a temporary database.

For an independent local run outside the Dev Container, `docker compose up --build` builds the API image, starts PostgreSQL, applies migrations and exposes port `8080`.

## Quality checks

Run formatting and static analysis inside the Dev Container:

```sh
clang-format --dry-run --Werror $(find src tests -name '*.cpp' -o -name '*.hpp')
clang-tidy -quiet -p build/Debug src/application/*.cpp src/infrastructure/*.cpp src/presentation/*.cpp src/main.cpp
python3 -m json.tool docs/openapi.json >/dev/null
```

To build with ASan and UBSan, configure Debug with `-DENABLE_SANITIZERS=ON` after installing Conan dependencies. `-DENABLE_WERROR=ON` enables strict project warnings. For Release, install Conan dependencies with `-s build_type=Release`, then use the `conan-release` presets.

The design is described in [SDD.md](SDD.md).

The CI workflow in `.github/workflows/ci.yml` runs the same build, migration, tests, format and analysis checks inside the Dev Container.
