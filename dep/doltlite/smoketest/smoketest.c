/* Spike for the DoltLite dependency (see .local/plan/wms/database_tool/plan.md):
 * proves the pinned native library can be linked from MSVC and that version
 * control works as plain SQL on an ordinary sqlite3 connection. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

static void fail(sqlite3 *db, const char *what, char *err) {
    fprintf(stderr, "doltlite_smoketest: %s: %s\n", what, err ? err : sqlite3_errmsg(db));
    exit(1);
}

int main(void) {
    const char *path = "doltlite_smoketest.doltlite";
    remove(path);

    sqlite3 *db;
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        fail(db, "open", NULL);
    }

    char *err = NULL;
    if (sqlite3_exec(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT)", NULL, NULL, &err) != SQLITE_OK) {
        fail(db, "create table", err);
    }
    if (sqlite3_exec(db, "INSERT INTO t(id, name) VALUES (1, 'hello')", NULL, NULL, &err) != SQLITE_OK) {
        fail(db, "insert", err);
    }
    if (sqlite3_exec(db, "SELECT dolt_commit('-A', '-m', 'smoketest commit')", NULL, NULL, &err) != SQLITE_OK) {
        fail(db, "dolt_commit", err);
    }

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, "SELECT message FROM dolt_log ORDER BY date DESC LIMIT 1", -1, &stmt, NULL) != SQLITE_OK) {
        fail(db, "prepare dolt_log", NULL);
    }
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        fail(db, "dolt_log returned no rows", NULL);
    }
    const unsigned char *message = sqlite3_column_text(stmt, 0);
    if (!message || strcmp((const char *)message, "smoketest commit") != 0) {
        fprintf(stderr, "doltlite_smoketest: unexpected dolt_log message: %s\n", message ? (const char *)message : "(null)");
        exit(1);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    printf("doltlite_smoketest: OK\n");
    return 0;
}
