#pragma once
#include "crow/websocket.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <memory>

// Manages WebSocket connections for document collaboration
class WebSocketManager
{
public:
    static WebSocketManager& getInstance();
    
    // Document room management
    void joinDocument(const std::string& doc_id, crow::websocket::connection* conn, const std::string& user_id);
    void leaveDocument(const std::string& doc_id, crow::websocket::connection* conn);
    void leaveAll(crow::websocket::connection* conn);
    
    // Broadcast messages to all users in a document room
    void broadcastToDocument(const std::string& doc_id, const std::string& message, crow::websocket::connection* exclude_conn = nullptr);
    
    // Get users currently viewing a document
    std::vector<std::string> getDocumentUsers(const std::string& doc_id);
    
    // Check if user is in document room
    bool isUserInDocument(const std::string& doc_id, const std::string& user_id);

private:
    WebSocketManager() = default;
    ~WebSocketManager() = default;
    WebSocketManager(const WebSocketManager&) = delete;
    WebSocketManager& operator=(const WebSocketManager&) = delete;
    
    // doc_id -> set of connections
    std::unordered_map<std::string, std::unordered_set<crow::websocket::connection*>> document_rooms_;
    
    // connection -> (doc_id, user_id)
    std::unordered_map<crow::websocket::connection*, std::pair<std::string, std::string>> connection_info_;
    
    // doc_id -> set of user_ids
    std::unordered_map<std::string, std::unordered_set<std::string>> document_users_;
    
    mutable std::mutex mutex_;
};


