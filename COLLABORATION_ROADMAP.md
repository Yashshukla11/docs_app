# Collaboration Feature - Implementation Roadmap

## 📋 Current Status

### ✅ What's Done
- Routes are defined for collaboration endpoints
- Controller method signatures exist (but are stubs)
- Basic document CRUD is working
- Authentication system is in place

### ❌ What's Pending
- Database schema for collaborators
- Collaborator model
- CollaboratorRepository
- CollaborationService
- Controller implementations
- Access control for shared documents
- User lookup by email/username for sharing

---

## 🎯 Feature Overview

### Core Functionality
1. **Share Documents** - **Owner only** can share documents with other users
2. **Permissions** - Owner can grant Read-only or Read-Write access
3. **Collaborator Management** - **Owner only** can view, add, remove, and update permissions
4. **Access Control** - Shared documents appear in collaborator's document list
5. **Real-time Collaboration** - (Phase 2) Multiple users editing simultaneously

### Owner Privileges
- ✅ **Add Collaborators** - Share document with other users
- ✅ **Remove Collaborators** - Revoke access from any collaborator
- ✅ **Update Permissions** - Change collaborator's permission (read ↔ write)
- ✅ **View Collaborators** - See all users who have access
- ✅ **Full Document Control** - Edit, delete, rename (always has write access)

### Collaborator Privileges
- ✅ **View Document** - Access shared documents
- ✅ **Edit Document** - If granted 'write' permission
- ❌ **Cannot Share** - Cannot share document with others
- ❌ **Cannot Manage** - Cannot add/remove other collaborators
- ❌ **Cannot Delete** - Cannot delete the document

---

## 🗄️ Database Schema

### New Table: `document_collaborators`

```sql
CREATE TABLE document_collaborators (
    id TEXT PRIMARY KEY,
    document_id TEXT NOT NULL,
    user_id TEXT NOT NULL,
    permission TEXT NOT NULL CHECK(permission IN ('read', 'write')),
    shared_by TEXT NOT NULL,  -- User who shared the document
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (document_id) REFERENCES documents(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (shared_by) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(document_id, user_id)  -- One permission per user per document
);
```

**Indexes:**
- `idx_collaborators_document_id` on `document_id`
- `idx_collaborators_user_id` on `user_id`

---

## 🏗️ Architecture Layers

### 1. Model Layer
**File:** `include/models/Collaborator.h`, `src/models/Collaborator.cpp`

**Fields:**
- `id` - Unique identifier
- `document_id` - Document being shared
- `user_id` - Collaborator user ID
- `permission` - "read" or "write"
- `shared_by` - User who shared it
- `created_at`, `updated_at` - Timestamps

**Methods:**
- Getters/Setters
- `isValid()` - Validation
- `isValidPermission()` - Static validation

---

### 2. Repository Layer
**File:** `include/repositories/CollaboratorRepository.h`, `src/repositories/CollaboratorRepository.cpp`

**Methods:**
- `addCollaborator(doc_id, user_id, permission, shared_by)` - Add collaborator
- `findByDocumentId(doc_id)` - Get all collaborators for a document
- `findByUserId(user_id)` - Get all documents shared with user
- `findCollaborator(doc_id, user_id)` - Get specific collaboration
- `updatePermission(doc_id, user_id, permission)` - Update permission
- `removeCollaborator(doc_id, user_id)` - Remove collaborator
- `hasAccess(doc_id, user_id, required_permission)` - Check access
- `isCollaborator(doc_id, user_id)` - Check if user is collaborator

---

### 3. Service Layer
**File:** `include/services/CollaborationService.h`, `src/services/CollaborationService.cpp`

**Methods:**
- `shareDocument(doc_id, owner_id, collaborator_email, permission)` - Share with user by email
- `getCollaborators(doc_id, user_id)` - Get collaborators (owner or collaborator can view)
- `updatePermission(doc_id, owner_id, collaborator_id, permission)` - Update permission (owner only)
- `removeCollaborator(doc_id, owner_id, collaborator_id)` - Remove collaborator (owner only)
- `getSharedDocuments(user_id)` - Get all documents shared with user
- `checkAccess(doc_id, user_id, required_permission)` - Check if user can access document

**Business Logic:**
- Validate document ownership
- Validate user exists (by email)
- Prevent duplicate collaborations
- Owner always has full access
- Validate permission values

---

### 4. Controller Layer
**File:** `src/controllers/DocumentController.cpp` (update existing methods)

**Methods to Implement:**
1. `shareDocument()` - POST `/api/documents/:id/share`
2. `getCollaborators()` - GET `/api/documents/:id/collaborators`
3. `removeCollaborator()` - DELETE `/api/documents/:id/collaborators/:collaborator_id`
4. `updatePermissions()` - PATCH `/api/documents/:id/collaborators/:collaborator_id`

---

### 5. Access Control Updates

**Update `DocumentService`:**
- `getDocumentById()` - Allow access if user is owner OR collaborator
- `getAllUserDocuments()` - Include shared documents
- `updateDocument()` - Allow if owner OR collaborator with 'write' permission
- `deleteDocument()` - Only owner can delete

**Update `DocumentRepository`:**
- `findByOwnerId()` - Keep as is (owner's documents)
- Add `findSharedWithUser(user_id)` - Documents shared with user

---

## 🔄 User Flow

### Flow 1: Sharing a Document

```
1. User A (Owner) opens document
2. Clicks "Share" button
3. Enters collaborator's email
4. Selects permission (Read/Write)
5. Clicks "Share"
   
Backend:
- Validates document ownership
- Looks up user by email
- Creates collaboration record
- Returns success
   
Frontend:
- Shows success message
- Updates collaborators list
```

### Flow 2: Collaborator Accessing Shared Document

```
1. User B (Collaborator) logs in
2. Views document list
3. Sees shared documents (marked as "Shared with me")
4. Clicks on shared document
   
Backend:
- Checks if user is collaborator
- Returns document if access granted
   
Frontend:
- Shows document
- Shows read-only indicator if read-only
```

### Flow 3: Collaborator Editing (Write Permission)

```
1. User B opens shared document
2. Makes edits
3. Clicks "Save"
   
Backend:
- Validates write permission
- Updates document
- Returns success
   
Frontend:
- Shows "Saved" indicator
```

### Flow 4: Managing Collaborators (Owner Only)

```
1. Owner opens collaborators list
2. Views all collaborators
3. Owner can:
   - Add new collaborator (share with new user)
   - Change permission (Read ↔ Write)
   - Remove collaborator
   
Backend:
- Validates ownership (only owner can manage)
- Updates/Deletes collaboration record
- Returns error if non-owner tries to manage
   
Frontend:
- Updates UI
- Shows "Share" button only to owner
- Shows collaborators list only to owner
```

### Flow 5: Collaborator Attempting to Share (Should Fail)

```
1. Collaborator (not owner) opens document
2. Tries to click "Share" button
   
Backend:
- Validates ownership
- Returns 403 Forbidden
   
Frontend:
- Hides "Share" button for non-owners
- Shows error message if attempted
```

---

## 📱 App Flow (Application Logic)

### Document Access Check Flow

```
User requests document
    ↓
Is user the owner?
    ├─ YES → Grant full access
    └─ NO → Check if collaborator
            ├─ YES → Check permission
            │         ├─ 'read' → Read-only access
            │         └─ 'write' → Full edit access
            └─ NO → Return 403 Forbidden
```

### Document List Flow

```
User requests all documents
    ↓
Get owner's documents (DocumentRepository.findByOwnerId)
    +
Get shared documents (CollaboratorRepository.findByUserId)
    ↓
Merge and return
```

### Share Document Flow

```
Owner shares document
    ↓
Validate ownership
    ↓
Lookup collaborator by email (UserRepository.findByEmail)
    ↓
Check if already collaborator
    ├─ YES → Update permission
    └─ NO → Create new collaboration
    ↓
Return success
```

---

## 🚀 Implementation Steps

### Phase 1: Database & Models (Foundation)
1. ✅ Create `document_collaborators` table in `Database::initializeSchema()`
2. ✅ Create `Collaborator` model
3. ✅ Add validation methods

### Phase 2: Repository Layer (Data Access)
4. ✅ Create `CollaboratorRepository`
5. ✅ Implement all CRUD operations
6. ✅ Add access check methods

### Phase 3: Service Layer (Business Logic)
7. ✅ Create `CollaborationService`
8. ✅ Implement sharing logic
9. ✅ Add permission validation
10. ✅ Update `DocumentService` for access control

### Phase 4: Controller Layer (API)
11. ✅ Implement `shareDocument()` controller
12. ✅ Implement `getCollaborators()` controller
13. ✅ Implement `removeCollaborator()` controller
14. ✅ Implement `updatePermissions()` controller

### Phase 5: Integration & Testing
15. ✅ Update `getAllDocuments()` to include shared documents
16. ✅ Update `getDocument()` to check collaboration access
17. ✅ Update `updateDocument()` to check write permission
18. ✅ Test all flows with Postman

### Phase 6: Frontend Integration (Future)
19. ⏳ Add "Share" button in document editor
20. ⏳ Add collaborators list UI
21. ⏳ Show shared documents in document list
22. ⏳ Add permission indicators

---

## 🔐 Security Considerations

1. **Ownership Validation** - **CRITICAL**: Only owner can:
   - Share document with others
   - Add collaborators
   - Remove collaborators
   - Update collaborator permissions
   - View full collaborators list
   - Non-owners get 403 Forbidden if they try

2. **Permission Validation** - Only 'read' or 'write' allowed
3. **Access Control** - Check collaboration before document access
4. **User Validation** - Verify user exists before sharing
5. **Prevent Self-Sharing** - Owner shouldn't share with themselves
6. **Cascade Deletes** - Remove collaborations when document/user deleted
7. **Collaborator Restrictions** - Collaborators cannot:
   - Share the document with others
   - Manage other collaborators
   - Delete the document
   - Change their own permissions

---

## 📊 API Endpoints Summary

| Method | Endpoint | Description | Auth Required |
|--------|----------|-------------|---------------|
| POST | `/api/documents/:id/share` | Share document with user | ✅ |
| GET | `/api/documents/:id/collaborators` | Get collaborators list | ✅ |
| DELETE | `/api/documents/:id/collaborators/:user_id` | Remove collaborator | ✅ |
| PATCH | `/api/documents/:id/collaborators/:user_id` | Update permission | ✅ |

---

## 📝 API Request/Response Examples

### 1. Share Document
**POST** `/api/documents/:doc_id/share`

**Request:**
```json
{
  "email": "collaborator@example.com",
  "permission": "write"
}
```

**Response (201):**
```json
{
  "message": "Document shared successfully",
  "collaboration": {
    "id": "collab-123",
    "document_id": "doc-456",
    "user_id": "user-789",
    "permission": "write",
    "shared_by": "owner-123",
    "created_at": "2024-01-15 10:30:00"
  }
}
```

### 2. Get Collaborators
**GET** `/api/documents/:doc_id/collaborators`

**Response (200):**
```json
{
  "collaborators": [
    {
      "id": "collab-123",
      "user_id": "user-789",
      "username": "collaborator",
      "email": "collaborator@example.com",
      "permission": "write",
      "shared_by": "owner-123",
      "created_at": "2024-01-15 10:30:00"
    }
  ]
}
```

### 3. Update Permission
**PATCH** `/api/documents/:doc_id/collaborators/:user_id`

**Request:**
```json
{
  "permission": "read"
}
```

**Response (200):**
```json
{
  "message": "Permission updated successfully",
  "collaboration": {
    "id": "collab-123",
    "permission": "read",
    "updated_at": "2024-01-15 11:00:00"
  }
}
```

### 4. Remove Collaborator
**DELETE** `/api/documents/:doc_id/collaborators/:user_id`

**Response (200):**
```json
{
  "message": "Collaborator removed successfully"
}
```

---

## 🧪 Testing Checklist

- [ ] Share document with valid user
- [ ] Share document with invalid email (error)
- [ ] Share document user doesn't own (error)
- [ ] Share document with self (error)
- [ ] Get collaborators list
- [ ] Update collaborator permission
- [ ] Remove collaborator
- [ ] Collaborator can access shared document
- [ ] Collaborator with 'read' cannot edit
- [ ] Collaborator with 'write' can edit
- [ ] Shared documents appear in collaborator's list
- [ ] Owner can see all collaborators
- [ ] Non-owner cannot manage collaborators
- [ ] Cascade delete when document deleted
- [ ] Cascade delete when user deleted

---

## 🎯 Next Steps

1. Start with **Phase 1** - Database schema and models
2. Build up layer by layer (Repository → Service → Controller)
3. Test each layer before moving to next
4. Update existing DocumentService for access control
5. Test complete flows end-to-end

---

## 📝 Notes

- **Real-time collaboration** (WebSocket) will be Phase 2
- For now, focus on **sharing and permissions**
- Use email for sharing (most user-friendly)
- Owner always has full access (no need to store in collaborators table)
- Consider adding "shared_with_me" flag in document list response

---

## 🔄 Data Flow Diagram

```
Owner Shares Document
    ↓
CollaborationService.shareDocument()
    ↓
UserRepository.findByEmail() → Get collaborator user_id
    ↓
DocumentRepository.findById() → Validate ownership
    ↓
CollaboratorRepository.addCollaborator() → Create record
    ↓
Return success

Collaborator Accesses Document
    ↓
DocumentService.getDocumentById()
    ↓
Check: Is owner? OR Is collaborator?
    ↓
If collaborator: CollaboratorRepository.hasAccess()
    ↓
Return document with appropriate access level
```

---

## 🎨 Frontend Integration Points (Future)

1. **Document List**
   - Show "Shared with me" badge
   - Different icon for shared documents

2. **Document Editor**
   - "Share" button in header
   - Collaborators list sidebar
   - Permission indicators (read-only mode)

3. **Share Modal**
   - Email input
   - Permission selector (Read/Write)
   - Collaborators list with actions

4. **Access Indicators**
   - Show "Read-only" banner for read-only collaborators
   - Disable edit buttons for read-only users
