#include "db/Database.h"
#include <sqlite3.h>
#include <iostream>
#include <stdexcept>

Database &Database::getInstance()
{
    static Database instance;
    return instance;
}

bool Database::initialize(const std::string &db_path)
{
    db_path_ = db_path;

    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    // Enable foreign keys
    sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    // Initialize schema
    if (!initializeSchema())
    {
        std::cerr << "Failed to initialize database schema" << std::endl;
        return false;
    }

    return true;
}

bool Database::initializeSchema()
{
    const char *create_users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id TEXT PRIMARY KEY,
            email TEXT UNIQUE NOT NULL,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    const char *create_index_email = "CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);";
    const char *create_index_username = "CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);";

    const char *create_documents_table = R"(
        CREATE TABLE IF NOT EXISTS documents (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            content TEXT DEFAULT '',
            owner_id TEXT NOT NULL,
            version INTEGER DEFAULT 1 NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )";

    const char *create_index_documents_owner = "CREATE INDEX IF NOT EXISTS idx_documents_owner_id ON documents(owner_id);";
    const char *create_index_documents_created = "CREATE INDEX IF NOT EXISTS idx_documents_created_at ON documents(created_at);";

    // Migration: Add version column if it doesn't exist (for existing databases)
    const char *migrate_add_version = R"(
        ALTER TABLE documents ADD COLUMN version INTEGER DEFAULT 1 NOT NULL;
    )";

    const char *create_collaborators_table = R"(
        CREATE TABLE IF NOT EXISTS document_collaborators (
            id TEXT PRIMARY KEY,
            document_id TEXT NOT NULL,
            user_id TEXT NOT NULL,
            permission TEXT NOT NULL CHECK(permission IN ('read', 'write')),
            shared_by TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (document_id) REFERENCES documents(id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (shared_by) REFERENCES users(id) ON DELETE CASCADE,
            UNIQUE(document_id, user_id)
        );
    )";

    const char *create_index_collaborators_document = "CREATE INDEX IF NOT EXISTS idx_collaborators_document_id ON document_collaborators(document_id);";
    const char *create_index_collaborators_user = "CREATE INDEX IF NOT EXISTS idx_collaborators_user_id ON document_collaborators(user_id);";

    if (!execute(create_users_table))
    {
        return false;
    }

    execute(create_index_email);
    execute(create_index_username);

    if (!execute(create_documents_table))
    {
        return false;
    }

    execute(create_index_documents_owner);
    execute(create_index_documents_created);

    if (!execute(create_collaborators_table))
    {
        return false;
    }

    execute(create_index_collaborators_document);
    execute(create_index_collaborators_user);

    // Migration: Add version column if it doesn't exist (for existing databases)
    // Check if version column exists by querying table info
    const char *check_version_sql = "PRAGMA table_info(documents)";
    sqlite3_stmt *stmt;
    bool version_exists = false;

    if (sqlite3_prepare_v2(getConnection(), check_version_sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *col_name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            if (col_name && std::string(col_name) == "version")
            {
                version_exists = true;
                break;
            }
        }
        sqlite3_finalize(stmt);
    }

    // Add version column if it doesn't exist
    if (!version_exists)
    {
        const char *migrate_add_version = "ALTER TABLE documents ADD COLUMN version INTEGER DEFAULT 1 NOT NULL";
        execute(migrate_add_version);
        // Update existing documents to have version 1
        execute("UPDATE documents SET version = 1 WHERE version IS NULL");
    }

    return true;
}

bool Database::execute(const std::string &sql)
{
    if (!db_)
    {
        std::cerr << "Database not initialized" << std::endl;
        return false;
    }

    char *err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << (err_msg ? err_msg : "Unknown error") << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

void Database::close()
{
    if (db_)
    {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

Database::~Database()
{
    close();
}
