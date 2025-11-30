#include "repositories/DocumentRepository.h"
#include "db/Database.h"
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <iostream>

DocumentRepository::DocumentRepository() {}

std::string DocumentRepository::generateId()
{
    // Generate UUID-like ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::ostringstream oss;
    oss << std::hex;
    for (int i = 0; i < 32; ++i)
    {
        if (i == 8 || i == 12 || i == 16 || i == 20)
            oss << "-";
        oss << dis(gen);
    }
    return oss.str();
}

Document DocumentRepository::mapRowToDocument(sqlite3_stmt *stmt)
{
    Document doc;
    const char *id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    const char *title = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    const char *content = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    const char *owner_id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
    int version = sqlite3_column_int(stmt, 4);
    const char *created_at = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
    const char *updated_at = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));

    doc.setId(id ? id : "");
    doc.setTitle(title ? title : "");
    doc.setContent(content ? content : "");
    doc.setOwnerId(owner_id ? owner_id : "");
    doc.setVersion(version);
    doc.setCreatedAt(created_at ? created_at : "");
    doc.setUpdatedAt(updated_at ? updated_at : "");

    return doc;
}

std::optional<Document> DocumentRepository::createDocument(const Document &document)
{
    auto &db = Database::getInstance();
    sqlite3 *conn = db.getConnection();

    if (!conn)
        return std::nullopt;

    std::string id = document.getId().empty() ? generateId() : document.getId();
    Document newDoc = document;
    newDoc.setId(id);

    const char *sql = R"(
        INSERT INTO documents (id, title, content, owner_id, version, created_at, updated_at)
        VALUES (?, ?, ?, ?, 1, datetime('now'), datetime('now'))
    )";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return std::nullopt;
    }

    std::string id_str = newDoc.getId();
    std::string title_str = newDoc.getTitle();
    std::string content_str = newDoc.getContent();
    std::string owner_id_str = newDoc.getOwnerId();

    sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, title_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, content_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, owner_id_str.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return std::nullopt;
    }

    // Fetch the created document with timestamps
    return findById(id);
}

std::optional<Document> DocumentRepository::findById(const std::string &id)
{
    auto &db = Database::getInstance();
    sqlite3 *conn = db.getConnection();

    if (!conn)
        return std::nullopt;

    const char *sql = "SELECT id, title, content, owner_id, version, created_at, updated_at FROM documents WHERE id = ?";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
        return std::nullopt;

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW)
    {
        Document doc = mapRowToDocument(stmt);
        sqlite3_finalize(stmt);
        return doc;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Document> DocumentRepository::findByOwnerId(const std::string &owner_id)
{
    std::vector<Document> documents;
    auto &db = Database::getInstance();
    sqlite3 *conn = db.getConnection();

    if (!conn)
        return documents;

    const char *sql = "SELECT id, title, content, owner_id, version, created_at, updated_at FROM documents WHERE owner_id = ? ORDER BY created_at DESC";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
        return documents;

    sqlite3_bind_text(stmt, 1, owner_id.c_str(), -1, SQLITE_TRANSIENT);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        documents.push_back(mapRowToDocument(stmt));
    }

    sqlite3_finalize(stmt);
    return documents;
}

bool DocumentRepository::updateDocument(const Document &document)
{
    auto &db = Database::getInstance();
    sqlite3 *conn = db.getConnection();

    if (!conn)
        return false;

    const char *sql = R"(
        UPDATE documents 
        SET title = ?, content = ?, version = version + 1, updated_at = datetime('now')
        WHERE id = ? AND version = ?
    )";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return false;
    }

    std::string title_str = document.getTitle();
    std::string content_str = document.getContent();
    std::string id_str = document.getId();
    int expected_version = document.getVersion();

    sqlite3_bind_text(stmt, 1, title_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, content_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, id_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, expected_version);

    rc = sqlite3_step(stmt);
    int rows_affected = sqlite3_changes(conn);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return false;
    }

    // If no rows were affected, version mismatch occurred
    if (rows_affected == 0)
    {
        return false;
    }

    return true;
}

bool DocumentRepository::deleteDocument(const std::string &id)
{
    auto &db = Database::getInstance();
    sqlite3 *conn = db.getConnection();

    if (!conn)
        return false;

    const char *sql = "DELETE FROM documents WHERE id = ?";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return false;
    }

    return true;
}

bool DocumentRepository::documentExists(const std::string &id)
{
    return findById(id).has_value();
}

bool DocumentRepository::isOwner(const std::string &doc_id, const std::string &user_id)
{
    auto doc = findById(doc_id);
    if (!doc.has_value())
        return false;

    return doc.value().getOwnerId() == user_id;
}
