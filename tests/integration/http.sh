#!/bin/sh
set -eu

api="$1"
port=18080
project_root=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$project_root"

test_database="drogon_http_test_$$"
db_created=0
server_pid=
export PGHOST="${DATABASE_HOST:-db}"
export PGPORT="${DATABASE_PORT:-5432}"
export PGUSER="${DATABASE_USER:-drogon}"
export PGPASSWORD="${DATABASE_PASSWORD:-localdev}"

cleanup() {
    if [ -n "$server_pid" ]; then
        kill "$server_pid" 2>/dev/null || true
        wait "$server_pid" 2>/dev/null || true
    fi
    if [ "$db_created" -eq 1 ]; then
        dropdb --if-exists --maintenance-db="${DATABASE_NAME:-drogon}" "$test_database" \
            >/dev/null 2>&1 || true
    fi
}
trap cleanup EXIT INT TERM

createdb --maintenance-db="${DATABASE_NAME:-drogon}" "$test_database"
db_created=1
DATABASE_NAME="$test_database" sh scripts/migrate.sh

DATABASE_NAME="$test_database" HTTP_PORT="$port" "$api" &
server_pid=$!

ready=0
for attempt in 1 2 3 4 5 6 7 8 9 10; do
    if curl --silent --fail "http://127.0.0.1:$port/health" >/dev/null; then
        ready=1
        break
    fi
    sleep 1
done
[ "$ready" -eq 1 ]

base="http://127.0.0.1:$port"
health=$(curl --silent --fail "$base/health")
printf '%s' "$health" | python3 -c 'import json,sys; assert json.load(sys.stdin)["status"] == "UP"'

curl --silent --fail "$base/openapi.json" | python3 -c 'import json,sys; x=json.load(sys.stdin); assert x["openapi"] == "3.0.3"; assert "/health" in x["paths"] and "get" in x["paths"]["/api/v1/users"] and "delete" in x["paths"]["/api/v1/users"] and "delete" in x["paths"]["/api/v1/users/{id}"]'
curl --silent --fail "$base/docs" | python3 -c 'import sys; x=sys.stdin.read(); assert "/openapi.json" in x and "/docs/swagger-ui-bundle.js" in x'
for asset in swagger-ui.css swagger-ui-bundle.js swagger-ui-standalone-preset.js; do
    bytes=$(curl --silent --fail "$base/docs/$asset" | wc -c)
    [ "$bytes" -gt 0 ]
done

missing_status=$(curl --silent --output /tmp/drogon-missing-response.json --write-out '%{http_code}' "$base/does-not-exist")
[ "$missing_status" = 404 ]
python3 -c 'import json; x=json.load(open("/tmp/drogon-missing-response.json")); assert x["code"] == "NOT_FOUND" and x["traceId"]'

invalid_status=$(curl --silent --output /tmp/drogon-invalid-response.json --write-out '%{http_code}' \
    -H 'Content-Type: application/json' -d '{"name":"John"}' "$base/api/v1/users")
[ "$invalid_status" = 400 ]
python3 -c 'import json; x=json.load(open("/tmp/drogon-invalid-response.json")); assert x["code"] == "INVALID_INPUT" and x["traceId"]'

email="http-test-$(date +%s)-$$@example.com"
payload="{\"name\":\"John\",\"email\":\"$email\"}"
created_status=$(curl --silent --output /tmp/drogon-created-response.json --write-out '%{http_code}' \
    -H 'Content-Type: application/json' -d "$payload" "$base/api/v1/users")
[ "$created_status" = 201 ]
python3 -c 'import json; x=json.load(open("/tmp/drogon-created-response.json")); assert x["id"] and x["name"] == "John"'
user_id=$(python3 -c 'import json; print(json.load(open("/tmp/drogon-created-response.json"))["id"])')
listed_status=$(curl --silent --output /tmp/drogon-listed-response.json --write-out '%{http_code}' "$base/api/v1/users")
[ "$listed_status" = 200 ]
python3 -c 'import json,sys; users=json.load(open("/tmp/drogon-listed-response.json")); assert isinstance(users,list); assert any(user["id"] == sys.argv[1] for user in users)' "$user_id"
found_status=$(curl --silent --output /tmp/drogon-found-response.json --write-out '%{http_code}' "$base/api/v1/users/$user_id")
[ "$found_status" = 200 ]
python3 -c 'import json; x=json.load(open("/tmp/drogon-found-response.json")); assert x["name"] == "John"'

invalid_id_status=$(curl --silent --output /tmp/drogon-invalid-id-response.json --write-out '%{http_code}' "$base/api/v1/users/invalid")
[ "$invalid_id_status" = 400 ]
python3 -c 'import json; x=json.load(open("/tmp/drogon-invalid-id-response.json")); assert x["code"] == "INVALID_ID" and x["traceId"]'

absent_status=$(curl --silent --output /tmp/drogon-absent-response.json --write-out '%{http_code}' "$base/api/v1/users/00000000-0000-0000-0000-000000000000")
[ "$absent_status" = 404 ]
python3 -c 'import json; x=json.load(open("/tmp/drogon-absent-response.json")); assert x["code"] == "USER_NOT_FOUND" and x["traceId"]'

invalid_delete_status=$(curl --silent --output /tmp/drogon-invalid-delete-response.json --write-out '%{http_code}' \
    --request DELETE "$base/api/v1/users/invalid")
[ "$invalid_delete_status" = 400 ]
python3 -c 'import json; x=json.load(open("/tmp/drogon-invalid-delete-response.json")); assert x["code"] == "INVALID_ID" and x["traceId"]'

duplicate_status=$(curl --silent --output /tmp/drogon-duplicate-response.json --write-out '%{http_code}' \
    -H 'Content-Type: application/json' -d "$payload" "$base/api/v1/users")
[ "$duplicate_status" = 409 ]
python3 -c 'import json; x=json.load(open("/tmp/drogon-duplicate-response.json")); assert x["code"] == "EMAIL_CONFLICT" and x["traceId"]'

deleted_status=$(curl --silent --output /tmp/drogon-deleted-response.json --write-out '%{http_code}' \
    --request DELETE "$base/api/v1/users/$user_id")
[ "$deleted_status" = 204 ]
deleted_again_status=$(curl --silent --output /tmp/drogon-deleted-again-response.json --write-out '%{http_code}' \
    --request DELETE "$base/api/v1/users/$user_id")
[ "$deleted_again_status" = 404 ]
python3 -c 'import json; x=json.load(open("/tmp/drogon-deleted-again-response.json")); assert x["code"] == "USER_NOT_FOUND" and x["traceId"]'

for index in 1 2; do
    bulk_email="bulk-http-$index-$(date +%s)-$$@example.com"
    bulk_payload="{\"name\":\"Bulk $index\",\"email\":\"$bulk_email\"}"
    bulk_created_status=$(curl --silent --output "/tmp/drogon-bulk-created-$index.json" \
        --write-out '%{http_code}' -H 'Content-Type: application/json' \
        -d "$bulk_payload" "$base/api/v1/users")
    [ "$bulk_created_status" = 201 ]
done

delete_all_status=$(curl --silent --output /tmp/drogon-delete-all-response.json \
    --write-out '%{http_code}' --request DELETE "$base/api/v1/users")
[ "$delete_all_status" = 204 ]
empty_list_status=$(curl --silent --output /tmp/drogon-empty-users-response.json \
    --write-out '%{http_code}' "$base/api/v1/users")
[ "$empty_list_status" = 200 ]
python3 -c 'import json; assert json.load(open("/tmp/drogon-empty-users-response.json")) == []'
delete_all_again_status=$(curl --silent --output /tmp/drogon-delete-all-again-response.json \
    --write-out '%{http_code}' --request DELETE "$base/api/v1/users")
[ "$delete_all_again_status" = 204 ]
