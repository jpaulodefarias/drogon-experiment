COMPOSE ?= docker compose -f .devcontainer/compose.yaml
PARALLEL ?= 4

.DEFAULT_GOAL := help
.PHONY: help up setup configure build migrate test run docs down

help: ## Show available commands
	@awk 'BEGIN {FS = ":.*##"} /^[a-zA-Z_-]+:.*##/ {printf "%-12s %s\n", $$1, $$2}' $(MAKEFILE_LIST)

up: ## Start the Dev Container and PostgreSQL
	$(COMPOSE) up -d dev

setup: up ## Install Conan dependencies in the Dev Container
	$(COMPOSE) exec -T dev sh -lc 'conan profile detect --force && conan install . --build=missing -s build_type=Debug'

configure: setup ## Configure the Debug build
	$(COMPOSE) exec -T dev cmake --preset conan-debug

build: configure ## Build the API and tests
	$(COMPOSE) exec -T dev cmake --build --preset conan-debug --parallel $(PARALLEL)

migrate: up ## Apply database migrations
	$(COMPOSE) exec -T dev sh scripts/migrate.sh

test: build migrate ## Run the test suite
	$(COMPOSE) exec -T dev ctest --preset conan-debug --output-on-failure

run: build migrate ## Start the API in the foreground
	$(COMPOSE) exec dev ./build/Debug/api

docs: ## Print the local API documentation URLs
	@echo 'Swagger UI:  http://localhost:8080/docs'
	@echo 'OpenAPI JSON: http://localhost:8080/openapi.json'

down: ## Stop the Dev Container and PostgreSQL
	$(COMPOSE) down
