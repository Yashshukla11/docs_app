#ifndef ROUTES_H
#define ROUTES_H

#include "crow.h"
#include "crow/middlewares/cors.h"
#include <string>
#include <utility>

/**
 * Setup all application routes for the Google Docs-like backend
 *
 * Features:
 * - Authentication & Authorization
 * - Document CRUD operations
 * - Real-time collaboration via WebSockets
 * - Sharing & Permissions
 * - Version history
 * - Comments & Suggestions
 * - Export functionality (PDF, DOCX)
 */
void setupRoutes(crow::App<crow::CORSHandler> &app);

/**
 * Middleware function to verify JWT token and extract user ID
 *
 * @param req The incoming HTTP request
 * @return Pair of (is_valid, user_id)
 */
std::pair<bool, std::string> verifyAndExtractUser(const crow::request &req);

#endif // ROUTES_H