#!/bin/sh
set -eu

export PGHOST="${DATABASE_HOST:-db}"
export PGPORT="${DATABASE_PORT:-5432}"
export PGDATABASE="${DATABASE_NAME:-drogon}"
export PGUSER="${DATABASE_USER:-drogon}"
export PGPASSWORD="${DATABASE_PASSWORD:-localdev}"

psql -v ON_ERROR_STOP=1 -c 'CREATE TABLE IF NOT EXISTS schema_migrations (version TEXT PRIMARY KEY);'
for migration in "$(dirname "$0")"/../migrations/[0-9]*.sql; do
    filename=$(basename "$migration" .sql)
    version=${filename%%_*}
    case "$version" in
        *[!0-9_]*|'') echo "Invalid migration name: $version" >&2; exit 1 ;;
    esac
    applied=$(psql -v ON_ERROR_STOP=1 -At -c "SELECT 1 FROM schema_migrations WHERE version = '$version'")
    if [ "$applied" != 1 ]; then
        psql -v ON_ERROR_STOP=1 -1 -f "$migration" -c "INSERT INTO schema_migrations (version) VALUES ('$version')"
        echo "Applied $version"
    fi
done
