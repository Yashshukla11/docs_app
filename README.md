# 📝 Real-Time Document Collaboration App

A simple, fast document editor where multiple people can edit the same document **at the same time** and see each other's changes instantly.

## ✨ What It Does

- **Real-time collaboration** - See what others are typing as they type
- **Live cursors** - See where other users are editing with their names
- **Share documents** - Invite others with read or write permissions
- **Auto-save** - Your changes are saved automatically
- **Simple formatting** - Bold, italic, code, and lists

## 🏗️ Tech Stack

### Frontend
- **React** - User interface
- **WebSocket** - Real-time updates
- **Vite** - Fast development

### Backend
- **C++** with **Crow** framework - Fast API server
- **SQLite** - Database
- **WebSocket** - Real-time communication

## 🚀 Quick Start

### Prerequisites
- Node.js (for frontend)
- C++ compiler (for backend)
- CMake (for building backend)

### Backend Setup

```bash
cd backend
cmake -S . -B build
cmake --build build
cd build
./docs_app
```

Server runs on `http://localhost:8080`

### Frontend Setup

```bash
cd frontend
npm install
npm run dev
```

App runs on `http://localhost:5173`

## 📖 How to Use

1. **Register** - Create an account
2. **Create** - Start a new document
3. **Share** - Click "Share" to invite others
4. **Collaborate** - Edit together in real-time!

## 🎯 Key Features

- ✅ Real-time text synchronization
- ✅ Live cursor positions
- ✅ User presence indicators
- ✅ Permission management (read/write)
- ✅ Auto-save
- ✅ Formatting toolbar

## 📁 Project Structure

```
LLD/
├── backend/          # C++ API server
│   ├── src/         # Source code
│   └── build/        # Compiled binary
├── frontend/         # React app
│   └── src/         # React components
└── README.md         # This file
```

## 🔧 Development

- Backend logs show WebSocket connections and API requests
- Frontend console shows real-time sync status
- Changes sync instantly between all connected users

---

**Made with ❤️ for real-time collaboration**

