#include "utils/WebSocketManager.h"
#include <algorithm>

WebSocketManager& WebSocketManager::getInstance()
{
    static WebSocketManager instance;
    return instance;
}

void WebSocketManager::joinDocument(const std::string& doc_id, crow::websocket::connection* conn, const std::string& user_id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    document_rooms_[doc_id].insert(conn);
    connection_info_[conn] = {doc_id, user_id};
    document_users_[doc_id].insert(user_id);
}

void WebSocketManager::leaveDocument(const std::string& doc_id, crow::websocket::connection* conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = document_rooms_.find(doc_id);
    if (it != document_rooms_.end())
    {
        it->second.erase(conn);
        if (it->second.empty())
        {
            document_rooms_.erase(it);
        }
    }
    
    auto conn_it = connection_info_.find(conn);
    if (conn_it != connection_info_.end())
    {
        std::string user_id = conn_it->second.second;
        connection_info_.erase(conn_it);
        
        // Remove user from document_users
        auto users_it = document_users_.find(doc_id);
        if (users_it != document_users_.end())
        {
            users_it->second.erase(user_id);
            if (users_it->second.empty())
            {
                document_users_.erase(users_it);
            }
        }
    }
}

void WebSocketManager::leaveAll(crow::websocket::connection* conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto conn_it = connection_info_.find(conn);
    if (conn_it != connection_info_.end())
    {
        std::string doc_id = conn_it->second.first;
        std::string user_id = conn_it->second.second;
        
        // Remove from document room
        auto room_it = document_rooms_.find(doc_id);
        if (room_it != document_rooms_.end())
        {
            room_it->second.erase(conn);
            if (room_it->second.empty())
            {
                document_rooms_.erase(room_it);
            }
        }
        
        // Remove from document users
        auto users_it = document_users_.find(doc_id);
        if (users_it != document_users_.end())
        {
            users_it->second.erase(user_id);
            if (users_it->second.empty())
            {
                document_users_.erase(users_it);
            }
        }
        
        connection_info_.erase(conn_it);
    }
}

void WebSocketManager::broadcastToDocument(const std::string& doc_id, const std::string& message, crow::websocket::connection* exclude_conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = document_rooms_.find(doc_id);
    if (it != document_rooms_.end())
    {
        for (auto* conn : it->second)
        {
            if (conn != exclude_conn)
            {
                conn->send_text(message);
            }
        }
    }
}

std::vector<std::string> WebSocketManager::getDocumentUsers(const std::string& doc_id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> users;
    auto it = document_users_.find(doc_id);
    if (it != document_users_.end())
    {
        users.assign(it->second.begin(), it->second.end());
    }
    return users;
}

bool WebSocketManager::isUserInDocument(const std::string& doc_id, const std::string& user_id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = document_users_.find(doc_id);
    if (it != document_users_.end())
    {
        return it->second.find(user_id) != it->second.end();
    }
    return false;
}

