# Concurrency Control for Collaborative Editing

## Problem: Lost Update (Race Condition)

### Scenario:
1. **User 1** loads document → Content: "Hello"
2. **User 2** loads document → Content: "Hello"  
3. **User 1** edits → "Hello World" → Saves ✅
4. **User 2** edits → "Hello There" → Saves ✅
5. **Result**: User 1's "World" is **LOST** ❌

### Current Implementation Issue:
- **Last Write Wins (LWW)**: No conflict detection
- Simple `UPDATE` overwrites everything
- No version tracking
- No awareness of concurrent edits

---

## Solution Options

### 1. **Optimistic Locking (Version-Based)** ⭐ RECOMMENDED
**How it works:**
- Add `version` field to documents (integer, auto-increments on each save)
- Client sends the version it read when saving
- Server checks: `WHERE id = ? AND version = ?`
- If version matches → Update succeeds, increment version
- If version differs → Reject with 409 Conflict, return current version

**Pros:**
- ✅ Simple to implement
- ✅ Prevents lost updates
- ✅ Works with existing REST API
- ✅ No real-time infrastructure needed

**Cons:**
- ❌ User must refresh and retry on conflict
- ❌ Not real-time (polling needed)

**Implementation:**
```sql
ALTER TABLE documents ADD COLUMN version INTEGER DEFAULT 1;
UPDATE documents SET version = 1 WHERE version IS NULL;
```

**API Changes:**
- GET `/api/documents/:id` → Returns `version` field
- PATCH `/api/documents/:id` → Requires `version` in request body
- Response: `409 Conflict` if version mismatch

---

### 2. **Operational Transformation (OT)** - Advanced
**How it works:**
- Track operations (insert, delete) instead of full content
- Transform operations to resolve conflicts
- Real-time sync via WebSocket
- Used by Google Docs, Notion

**Pros:**
- ✅ Real-time collaboration
- ✅ Preserves all changes
- ✅ Smooth user experience

**Cons:**
- ❌ Very complex to implement
- ❌ Requires WebSocket infrastructure
- ❌ Complex conflict resolution algorithms

---

### 3. **Three-Way Merge** - Medium Complexity
**How it works:**
- Store base version when user starts editing
- Compare: base → local changes vs base → remote changes
- Automatically merge non-conflicting changes
- Show conflict markers for conflicts

**Pros:**
- ✅ Preserves most changes automatically
- ✅ Better than optimistic locking

**Cons:**
- ❌ Complex merge logic
- ❌ Still needs conflict resolution UI
- ❌ Can be confusing for users

---

### 4. **Conflict Detection with Manual Resolution**
**How it works:**
- Detect conflicts on save
- Return both versions (local and remote)
- Let user choose which to keep or merge manually

**Pros:**
- ✅ User has full control
- ✅ No automatic decisions

**Cons:**
- ❌ Requires user intervention
- ❌ Poor UX (interrupts workflow)

---

## Recommended Implementation Plan

### Phase 1: Optimistic Locking (Immediate)
1. Add `version` column to documents table
2. Update Document model with version field
3. Modify update logic to check version
4. Return 409 Conflict on version mismatch
5. Frontend: Show conflict message, refresh, allow retry

### Phase 2: Real-time Sync (Future)
1. Add WebSocket support
2. Implement operational transformation
3. Real-time cursor positions
4. Live presence indicators

---

## Implementation Details

### Database Schema Change:
```sql
ALTER TABLE documents ADD COLUMN version INTEGER DEFAULT 1 NOT NULL;
CREATE INDEX idx_documents_version ON documents(version);
```

### API Request/Response:

**GET Response:**
```json
{
  "document": {
    "id": "...",
    "title": "...",
    "content": "...",
    "version": 5,  // ← NEW
    "updated_at": "..."
  }
}
```

**PATCH Request:**
```json
{
  "title": "...",
  "content": "...",
  "version": 5  // ← Client sends version it read
}
```

**PATCH Success Response (200):**
```json
{
  "message": "Document updated successfully",
  "document": {
    "version": 6,  // ← Incremented
    ...
  }
}
```

**PATCH Conflict Response (409):**
```json
{
  "error": "Document was modified by another user. Please refresh and try again.",
  "conflict": true,
  "current_version": 6,
  "current_content": "...",  // Latest content
  "your_version": 5
}
```

### Frontend Flow:
1. Load document → Store `version` in state
2. User edits → Auto-save with current `version`
3. If 409 Conflict:
   - Show notification: "Document was updated. Refresh?"
   - Load latest version
   - Show diff/merge UI (optional)
   - User can retry save

---

## Benefits of Optimistic Locking:
- ✅ **Prevents lost updates** - No overwrites
- ✅ **Simple** - Easy to understand and debug
- ✅ **Backward compatible** - Can add without breaking existing code
- ✅ **Works offline** - Version check happens on save
- ✅ **Scalable** - No locking, just version checks

---

## Next Steps:
1. Implement optimistic locking (Phase 1)
2. Test with 2+ concurrent users
3. Add conflict resolution UI in frontend
4. Consider WebSocket for real-time (Phase 2)

