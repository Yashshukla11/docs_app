import { useState, useEffect, useRef } from 'react';
import { useNavigate, useParams } from 'react-router-dom';
import { useAuth } from '../contexts/AuthContext';
import { documentsAPI, collaborationAPI } from '../services/api';
import { websocketService } from '../services/websocket';
import CursorOverlay from '../components/CursorOverlay';
import './DocumentEditor.css';

export default function DocumentEditor() {
  const { id } = useParams();
  const [document, setDocument] = useState(null);
  const [title, setTitle] = useState('');
  const [content, setContent] = useState('');
  const [version, setVersion] = useState(1);
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState('');
  const [isEditingTitle, setIsEditingTitle] = useState(false);
  const [isOwner, setIsOwner] = useState(false);
  const [permission, setPermission] = useState('write'); // 'read' or 'write'
  const [wsConnected, setWsConnected] = useState(false);
  const [conflictData, setConflictData] = useState(null);
  const [activeUsers, setActiveUsers] = useState([]); // Track active users
  const [userCursors, setUserCursors] = useState(new Map()); // Track cursor positions: userId -> {position, username}
  const contentRef = useRef('');
  const isApplyingRemoteEdit = useRef(false);
  const textareaRef = useRef(null);
  const lastSaveVersionRef = useRef(1); // Track last successfully saved version
  const [showShareModal, setShowShareModal] = useState(false);
  const [showCollaborators, setShowCollaborators] = useState(false);
  const [collaborators, setCollaborators] = useState([]);
  const [shareEmail, setShareEmail] = useState('');
  const [sharePermission, setSharePermission] = useState('write');
  const [sharing, setSharing] = useState(false);
  const { user, logout } = useAuth();
  const navigate = useNavigate();

  useEffect(() => {
    loadDocument();
    
    // Cleanup WebSocket on unmount
    return () => {
      websocketService.disconnect();
      setActiveUsers([]);
      setUserCursors(new Map());
    };
  }, [id]);

  // Periodic cursor position update (every 500ms)
  useEffect(() => {
    if (!wsConnected || !textareaRef.current) return;

    const interval = setInterval(() => {
      if (textareaRef.current && (isOwner || permission === 'write')) {
        const position = textareaRef.current.selectionStart;
        websocketService.sendCursor({
          position,
          userId: localStorage.getItem('user_id'),
          username: user?.username || user?.email || 'User'
        });
      }
    }, 500);

    return () => clearInterval(interval);
  }, [wsConnected, isOwner, permission, user]);

  // Auto-save functionality (debounced) - only save if version matches last saved version
  useEffect(() => {
    if (!document || !title.trim() || loading) return;
    
    // Don't auto-save if content hasn't changed from document
    if (content === (document.content || '')) return;
    
    // Don't auto-save if we're applying a remote edit
    if (isApplyingRemoteEdit.current) return;
    
    // Debounce auto-save - wait 2 seconds after user stops typing
    const autoSaveTimer = setTimeout(async () => {
      if (!isOwner && permission !== 'write') return;
      
      // Use the last successfully saved version, not the current version state
      const versionToUse = lastSaveVersionRef.current;
      
      try {
        const data = await documentsAPI.update(id, title.trim(), content, versionToUse);
        // Update both version state and last saved version
        setVersion(data.document.version);
        lastSaveVersionRef.current = data.document.version;
        setDocument(data.document);
        // Clear any conflict errors on successful save
        setConflictData(null);
        setError('');
      } catch (err) {
        // Handle conflicts silently (single-user scenario)
        if (err.status === 409 || (err.data && err.data.conflict)) {
          const errorData = err.data || JSON.parse(err.message || '{}');
          const serverContent = errorData.current_content || contentRef.current;

          // Always trust server version without showing banner
          if (errorData.current_version) {
            setVersion(errorData.current_version);
            lastSaveVersionRef.current = errorData.current_version;
          }

          setContent(serverContent);
          contentRef.current = serverContent;
          setDocument({ ...document, content: serverContent, version: errorData.current_version || document?.version });
          setConflictData(null);
          setError('');
        }
        // Otherwise, silent fail for auto-save (user can manually save)
      }
    }, 2000); // 2 second debounce

    return () => clearTimeout(autoSaveTimer);
  }, [content, title, id, document, isOwner, permission]);

  const loadDocument = async () => {
    try {
      setLoading(true);
      const data = await documentsAPI.getById(id);
      setDocument(data.document);
      setTitle(data.document.title);
      setContent(data.document.content || '');
      contentRef.current = data.document.content || '';
      const docVersion = data.document.version || 1;
      setVersion(docVersion);
      lastSaveVersionRef.current = docVersion; // Initialize last saved version
      
      // Check if user is owner
      const userId = localStorage.getItem('user_id');
      const owner = data.document.owner_id === userId;
      setIsOwner(owner);
      
      // If not owner, check permission
      if (!owner) {
        await checkPermission();
      } else {
        setPermission('write');
      }
      
      // Load collaborators if owner
      if (owner) {
        await loadCollaborators();
      }
      
      // Connect to WebSocket IMMEDIATELY for real-time collaboration
      // Don't wait - connect as soon as document is loaded
      // Permission check happens on backend, so we can attempt connection
      console.log('[DocumentEditor] Connecting WebSocket for document:', id);
      connectWebSocket();
    } catch (err) {
      setError(err.message || 'Failed to load document');
      if (err.message.includes('Access denied') || err.message.includes('not found')) {
        setTimeout(() => navigate('/documents'), 2000);
      }
    } finally {
      setLoading(false);
    }
  };

  const connectWebSocket = () => {
    console.log('[DocumentEditor] connectWebSocket called for document:', id);
    
    // Disconnect any existing connection first
    websocketService.disconnect();
    
    // Connect immediately - no delay
    console.log('[DocumentEditor] Attempting WebSocket connection...');
    websocketService.connect(
      id,
      (message) => {
        console.log('[DocumentEditor] WebSocket message received:', message.type, message);
        handleWebSocketMessage(message);
      },
      (error) => {
        console.error('[DocumentEditor] WebSocket error:', error);
        setWsConnected(false);
        // Try to reconnect after a delay
        setTimeout(() => {
          if (id) {
            console.log('[DocumentEditor] Retrying WebSocket connection...');
            connectWebSocket();
          }
        }, 3000);
      }
    );

    websocketService.on('open', () => {
      console.log('[DocumentEditor] WebSocket connected successfully!');
      setWsConnected(true);
      // Clear any existing conflict errors when WebSocket connects
      setConflictData(null);
      setError('');
      
      // Send initial cursor position
      if (textareaRef.current) {
        const position = textareaRef.current.selectionStart;
        websocketService.sendCursor({
          position,
          userId: localStorage.getItem('user_id'),
          username: user?.username || user?.email || 'User'
        });
      }
    });

    // Listen for edit messages
    websocketService.on('edit', (message) => {
      console.log('[DocumentEditor] Edit message received via listener:', message);
      handleRemoteEdit(message);
    });
  };

  const handleWebSocketMessage = (message) => {
    switch (message.type) {
      case 'user_joined':
        // Add user to active users list
        if (message.user_id && message.user_id !== localStorage.getItem('user_id')) {
          setActiveUsers(prev => {
            if (!prev.find(u => u.user_id === message.user_id)) {
              return [...prev, { user_id: message.user_id, username: message.username || 'User' }];
            }
            return prev;
          });
        }
        break;
      case 'user_left':
        // Remove user from active users and their cursor
        if (message.user_id) {
          setActiveUsers(prev => prev.filter(u => u.user_id !== message.user_id));
          setUserCursors(prev => {
            const newCursors = new Map(prev);
            newCursors.delete(message.user_id);
            return newCursors;
          });
        }
        break;
      case 'edit':
        handleRemoteEdit(message);
        break;
      case 'saved':
        // Document was saved by another user, update version
        if (message.version) {
          setVersion(message.version);
          lastSaveVersionRef.current = message.version;
        }
        if (message.content !== undefined) {
          handleRemoteEdit(message);
        }
        break;
      case 'cursor':
        // Update cursor position for other users
        if (message.userId && message.userId !== localStorage.getItem('user_id') && message.position !== undefined) {
          setUserCursors(prev => {
            const newCursors = new Map(prev);
            newCursors.set(message.userId, {
              position: message.position,
              username: message.username || 'User',
              timestamp: Date.now()
            });
            return newCursors;
          });
        }
        break;
      default:
        console.log('Unknown WebSocket message type:', message.type);
    }
  };

  const handleRemoteEdit = (message) => {
    console.log('[DocumentEditor] handleRemoteEdit called:', message);
    
    // Don't apply our own edits back to ourselves
    const currentUserId = localStorage.getItem('user_id');
    const messageUserId = message.userId || message.user_id;
    
    console.log('[DocumentEditor] Current userId:', currentUserId, 'Message userId:', messageUserId);
    
    if (messageUserId === currentUserId) {
      console.log('[DocumentEditor] Ignoring own edit');
      // This is our own edit - just update version if provided, don't update content
      if (message.version) {
        setVersion(message.version);
        lastSaveVersionRef.current = message.version;
      }
      return;
    }

    // Apply remote edit to content immediately - don't show conflict errors for real-time updates
    if (message.content !== undefined) {
      const currentContent = contentRef.current;
      const newContent = message.content;
      
      // Always update if content is different (even slightly)
      if (currentContent !== newContent) {
        console.log('Applying remote edit from user:', messageUserId, 'Content length:', newContent.length);
        
        // Set flag to prevent auto-save from triggering during remote edit
        isApplyingRemoteEdit.current = true;
        
        // Preserve cursor position if possible
        const textarea = textareaRef.current;
        const cursorPos = textarea ? textarea.selectionStart : 0;
        
        // Clear conflict errors FIRST before updating content
        setConflictData(null);
        setError('');
        
        // Update content immediately
        setContent(newContent);
        contentRef.current = newContent;
        
        // Update document state immediately
        if (document) {
          setDocument({ ...document, content: newContent, version: message.version || document.version });
        }
        
        // Update version if provided
        if (message.version) {
          setVersion(message.version);
          lastSaveVersionRef.current = message.version;
        }
        
        // Restore cursor position after content update
        if (textarea && cursorPos <= newContent.length) {
          setTimeout(() => {
            textarea.setSelectionRange(cursorPos, cursorPos);
          }, 0);
        }
        
        // Reset flag after a short delay
        setTimeout(() => {
          isApplyingRemoteEdit.current = false;
        }, 200);
      } else {
        // Content is the same, but still update version if provided
        if (message.version) {
          setVersion(message.version);
          lastSaveVersionRef.current = message.version;
        }
        // Still clear conflicts even if content is same
        setConflictData(null);
        setError('');
      }
    }
  };

  const checkPermission = async () => {
    try {
      const collabs = await collaborationAPI.getCollaborators(id);
      const userId = localStorage.getItem('user_id');
      const userCollab = collabs.collaborators.find(c => c.user_id === userId);
      if (userCollab) {
        setPermission(userCollab.permission);
      }
    } catch (err) {
      console.error('Failed to check permission:', err);
    }
  };

  const loadCollaborators = async () => {
    try {
      const data = await collaborationAPI.getCollaborators(id);
      setCollaborators(data.collaborators || []);
    } catch (err) {
      console.error('Failed to load collaborators:', err);
    }
  };

  const handleSave = async () => {
    if (!title.trim()) {
      setError('Title cannot be empty');
      return;
    }

    // Check permission for non-owners
    if (!isOwner && permission !== 'write') {
      setError('You have read-only access. Cannot save changes.');
      return;
    }

    try {
      setSaving(true);
      setError('');
      setConflictData(null);
      
      const data = await documentsAPI.update(id, title.trim(), content, version);
      
      // Update version after successful save
      setVersion(data.document.version);
      setDocument(data.document);
      setSaving(false);
    } catch (err) {
      // Handle conflicts silently (single-user scenario)
      if (err.status === 409 || (err.data && err.data.conflict)) {
        const errorData = err.data || JSON.parse(err.message || '{}');
        const serverContent = errorData.current_content || contentRef.current;

        if (errorData.current_version) {
          setVersion(errorData.current_version);
          lastSaveVersionRef.current = errorData.current_version;
        }

        setContent(serverContent);
        contentRef.current = serverContent;
        setDocument({ ...document, content: serverContent, version: errorData.current_version || document?.version });
        setConflictData(null);
        setError('');
      } else {
        try {
          const errorData = JSON.parse(err.message || '{}');
          setError(errorData.error || err.message || 'Failed to save document');
        } catch (parseErr) {
          setError(err.message || 'Failed to save document');
        }
      }
      setSaving(false);
    }
  };

  const handleRefresh = async () => {
    setConflictData(null);
    await loadDocument();
  };

  const handleTitleBlur = async () => {
    setIsEditingTitle(false);
    if (title.trim() && title.trim() !== document?.title) {
      try {
        await documentsAPI.rename(id, title.trim());
        const data = await documentsAPI.getById(id);
        setDocument(data.document);
      } catch (err) {
        setError(err.message || 'Failed to update title');
      }
    }
  };

  const handleDelete = async () => {
    if (!confirm('Are you sure you want to delete this document? This action cannot be undone.')) {
      return;
    }

    try {
      await documentsAPI.delete(id);
      navigate('/documents');
    } catch (err) {
      setError(err.message || 'Failed to delete document');
    }
  };

  const handleShare = async (e) => {
    e.preventDefault();
    if (!shareEmail.trim()) return;

    try {
      setSharing(true);
      setError('');
      await collaborationAPI.share(id, shareEmail.trim(), sharePermission);
      setShowShareModal(false);
      setShareEmail('');
      await loadCollaborators();
    } catch (err) {
      setError(err.message || 'Failed to share document');
    } finally {
      setSharing(false);
    }
  };

  const handleRemoveCollaborator = async (collaboratorId) => {
    if (!confirm('Remove this collaborator?')) return;

    try {
      await collaborationAPI.removeCollaborator(id, collaboratorId);
      await loadCollaborators();
    } catch (err) {
      setError(err.message || 'Failed to remove collaborator');
    }
  };

  const handleUpdatePermission = async (collaboratorId, newPermission) => {
    try {
      await collaborationAPI.updatePermission(id, collaboratorId, newPermission);
      await loadCollaborators();
    } catch (err) {
      setError(err.message || 'Failed to update permission');
    }
  };

  const applyContentChange = (newContent) => {
    if (isApplyingRemoteEdit.current) return;

    setContent(newContent);
    contentRef.current = newContent;

    if (websocketService.isConnected() && (isOwner || permission === 'write')) {
      // Clear any conflict errors when user is actively editing
      setConflictData(null);
      setError('');

      console.log('[DocumentEditor] 📤 Sending edit via WebSocket, content length:', newContent.length);
      websocketService.sendEdit({
        content: newContent,
        version: lastSaveVersionRef.current,
        userId: localStorage.getItem('user_id')
      });
    } else {
      console.log('[DocumentEditor] ⚠️ Not sending edit - websocket not connected or no write permission');
    }
  };

  const wrapSelection = (before, after = before) => {
    const textarea = textareaRef.current;
    if (!textarea) return;
    const value = contentRef.current;
    const start = textarea.selectionStart ?? 0;
    const end = textarea.selectionEnd ?? 0;
    const selected = value.slice(start, end);
    const newText = value.slice(0, start) + before + selected + after + value.slice(end);

    applyContentChange(newText);

    // Restore selection inside wrapped text
    requestAnimationFrame(() => {
      const cursorStart = start + before.length;
      const cursorEnd = cursorStart + selected.length;
      textarea.focus();
      textarea.setSelectionRange(cursorStart, cursorEnd);
    });
  };

  const toggleListForSelection = (prefix) => {
    const textarea = textareaRef.current;
    if (!textarea) return;
    const value = contentRef.current;
    const start = textarea.selectionStart ?? 0;
    const end = textarea.selectionEnd ?? 0;

    // Expand to full lines
    const before = value.lastIndexOf('\n', start - 1);
    const after = value.indexOf('\n', end);
    const lineStart = before === -1 ? 0 : before + 1;
    const lineEnd = after === -1 ? value.length : after;

    const block = value.slice(lineStart, lineEnd);
    const lines = block.split('\n');
    const allHavePrefix = lines.every((l) => l.startsWith(prefix));

    const updated = lines
      .map((l) => (allHavePrefix ? l.replace(new RegExp('^' + prefix.replace(/[-/\\^$*+?.()|[\]{}]/g, '\\$&')), '') : (l.trim().length ? prefix + l : l)))
      .join('\n');

    const newText = value.slice(0, lineStart) + updated + value.slice(lineEnd);
    applyContentChange(newText);

    requestAnimationFrame(() => {
      textarea.focus();
      const delta = updated.length - block.length;
      textarea.setSelectionRange(start + delta, end + delta);
    });
  };

  const wordCount = contentRef.current.trim().length
    ? contentRef.current.trim().split(/\s+/).length
    : 0;
  const charCount = contentRef.current.length;

  if (loading) {
    return (
      <div className="editor-container">
        <div className="loading-state">Loading document...</div>
      </div>
    );
  }

  if (error && !document) {
    return (
      <div className="editor-container">
        <div className="error-state">{error}</div>
      </div>
    );
  }

  return (
    <div className="editor-container">
      <header className="editor-header">
        <div className="editor-header-content">
          <button className="back-button" onClick={() => navigate('/documents')}>
            ← Back
          </button>
          {isEditingTitle ? (
            <input
              type="text"
              value={title}
              onChange={(e) => setTitle(e.target.value)}
              onBlur={handleTitleBlur}
              onKeyDown={(e) => {
                if (e.key === 'Enter') {
                  handleTitleBlur();
                }
              }}
              className="title-input"
              autoFocus
            />
          ) : (
            <h1 className="document-title-display" onClick={() => setIsEditingTitle(true)}>
              {title || 'Untitled Document'}
            </h1>
          )}
          <div className="editor-actions">
            {!isOwner && permission === 'read' && (
              <span className="read-only-badge">Read-only</span>
            )}
            {saving && <span className="saving-indicator">Saving...</span>}
            <button 
              className="save-button" 
              onClick={handleSave} 
              disabled={saving || (!isOwner && permission !== 'write')}
            >
              Save
            </button>
            {isOwner && (
              <>
                <button className="share-button" onClick={() => setShowShareModal(true)}>
                  Share
                </button>
                <button className="collaborators-button" onClick={() => setShowCollaborators(!showCollaborators)}>
                  Collaborators
                </button>
                <button className="delete-button" onClick={handleDelete}>
                  Delete
                </button>
              </>
            )}
            <button className="logout-button" onClick={logout}>
              Logout
            </button>
          </div>
        </div>
      </header>

      {error && (
        <div className="error-banner">
          {error}
          {conflictData && (
            <div style={{ marginTop: '10px' }}>
              <button onClick={handleRefresh} style={{ padding: '8px 16px', background: '#667eea', color: 'white', border: 'none', borderRadius: '4px', cursor: 'pointer' }}>
                Refresh & Reload
              </button>
            </div>
          )}
        </div>
      )}
      {wsConnected && (
        <div style={{ padding: '8px 16px', background: '#e6fffa', color: '#234e52', fontSize: '12px' }}>
          ● Real-time sync active
        </div>
      )}

      <main className="editor-main">
        <div className="editor-content-wrapper">
          <div style={{ position: 'relative' }}>
            {/* Formatting toolbar */}
            <div className="editor-toolbar">
              <button type="button" onClick={() => wrapSelection('**')} title="Bold">
                B
              </button>
              <button type="button" onClick={() => wrapSelection('*')} title="Italic">
                I
              </button>
              <button type="button" onClick={() => wrapSelection('`')} title="Inline code">
                {'</>'}
              </button>
              <button type="button" onClick={() => toggleListForSelection('- ')} title="Bullet list">
                • List
              </button>
              <button type="button" onClick={() => toggleListForSelection('1. ')} title="Numbered list">
                1.
              </button>
              <div className="editor-toolbar-spacer" />
              <div className="editor-stats">
                <span>{wordCount} words</span>
                <span>{charCount} chars</span>
              </div>
            </div>
            <textarea
              ref={textareaRef}
              className="editor-textarea"
              value={content}
              onChange={(e) => {
                applyContentChange(e.target.value);
              }}
              onKeyUp={(e) => {
                // Send cursor position on key events
                if (wsConnected && websocketService.isConnected() && textareaRef.current) {
                  const position = textareaRef.current.selectionStart;
                  websocketService.sendCursor({
                    position,
                    userId: localStorage.getItem('user_id'),
                    username: user?.username || user?.email || 'User'
                  });
                }
              }}
              onMouseUp={(e) => {
                // Send cursor position on mouse click
                if (wsConnected && websocketService.isConnected() && textareaRef.current) {
                  const position = textareaRef.current.selectionStart;
                  websocketService.sendCursor({
                    position,
                    userId: localStorage.getItem('user_id'),
                    username: user?.username || user?.email || 'User'
                  });
                }
              }}
              placeholder="Start writing..."
              spellCheck={true}
              readOnly={!isOwner && permission !== 'write'}
            />
            
            {/* Cursor overlay for other users */}
            {wsConnected && userCursors.size > 0 && (
              <CursorOverlay
                textareaRef={textareaRef}
                userCursors={userCursors}
                content={content}
              />
            )}
            
            {/* Display active users */}
            {activeUsers.length > 0 && (
              <div style={{ 
                position: 'absolute', 
                top: '10px', 
                right: '10px', 
                background: 'rgba(255, 255, 255, 0.95)', 
                padding: '8px 12px', 
                borderRadius: '4px',
                boxShadow: '0 2px 8px rgba(0,0,0,0.15)',
                fontSize: '12px',
                zIndex: 100,
                border: '1px solid #e2e8f0'
              }}>
                <div style={{ fontWeight: 'bold', marginBottom: '6px', color: '#4a5568' }}>Active users:</div>
                {activeUsers.map(u => (
                  <div key={u.user_id} style={{ marginBottom: '4px', display: 'flex', alignItems: 'center' }}>
                    <span style={{ 
                      display: 'inline-block',
                      width: '8px',
                      height: '8px',
                      borderRadius: '50%',
                      background: '#10b981',
                      marginRight: '6px',
                      animation: 'pulse 2s infinite'
                    }}></span>
                    <span>{u.username || u.user_id}</span>
                  </div>
                ))}
                <style>{`
                  @keyframes pulse {
                    0%, 100% { opacity: 1; }
                    50% { opacity: 0.5; }
                  }
                `}</style>
              </div>
            )}
          </div>
          
          {showCollaborators && isOwner && (
            <div className="collaborators-sidebar">
              <div className="collaborators-header">
                <h3>Collaborators</h3>
                <button className="close-sidebar" onClick={() => setShowCollaborators(false)}>×</button>
              </div>
              <div className="collaborators-list">
                {collaborators.length === 0 ? (
                  <p className="no-collaborators">No collaborators yet</p>
                ) : (
                  collaborators.map((collab) => (
                    <div key={collab.id} className="collaborator-item">
                      <div className="collaborator-info">
                        <div className="collaborator-name">{collab.username || collab.email}</div>
                        <div className="collaborator-permission">
                          <select
                            value={collab.permission}
                            onChange={(e) => handleUpdatePermission(collab.user_id, e.target.value)}
                            className="permission-select"
                          >
                            <option value="read">Read</option>
                            <option value="write">Write</option>
                          </select>
                        </div>
                      </div>
                      <button
                        className="remove-collab-button"
                        onClick={() => handleRemoveCollaborator(collab.user_id)}
                        title="Remove collaborator"
                      >
                        ×
                      </button>
                    </div>
                  ))
                )}
              </div>
            </div>
          )}
        </div>
      </main>

      {showShareModal && (
        <div className="modal-overlay" onClick={() => setShowShareModal(false)}>
          <div className="modal-content" onClick={(e) => e.stopPropagation()}>
            <h2>Share Document</h2>
            <form onSubmit={handleShare}>
              <div className="form-group">
                <label>Email</label>
                <input
                  type="email"
                  value={shareEmail}
                  onChange={(e) => setShareEmail(e.target.value)}
                  placeholder="user@example.com"
                  required
                  disabled={sharing}
                />
              </div>
              <div className="form-group">
                <label>Permission</label>
                <select
                  value={sharePermission}
                  onChange={(e) => setSharePermission(e.target.value)}
                  disabled={sharing}
                >
                  <option value="read">Read-only</option>
                  <option value="write">Can edit</option>
                </select>
              </div>
              <div className="modal-actions">
                <button
                  type="button"
                  onClick={() => {
                    setShowShareModal(false);
                    setShareEmail('');
                  }}
                  disabled={sharing}
                >
                  Cancel
                </button>
                <button type="submit" disabled={sharing || !shareEmail.trim()}>
                  {sharing ? 'Sharing...' : 'Share'}
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
}

