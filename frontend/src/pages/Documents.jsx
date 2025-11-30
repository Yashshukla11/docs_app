import { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { useAuth } from '../contexts/AuthContext';
import { documentsAPI } from '../services/api';
import './Documents.css';

export default function Documents() {
  const [documents, setDocuments] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  const [showCreateModal, setShowCreateModal] = useState(false);
  const [newDocTitle, setNewDocTitle] = useState('');
  const [creating, setCreating] = useState(false);
  const { user, logout } = useAuth();
  const navigate = useNavigate();

  useEffect(() => {
    loadDocuments();
  }, []);

  const loadDocuments = async () => {
    try {
      setLoading(true);
      const data = await documentsAPI.getAll();
      setDocuments(data.documents || []);
    } catch (err) {
      setError(err.message || 'Failed to load documents');
    } finally {
      setLoading(false);
    }
  };

  const handleCreateDocument = async (e) => {
    e.preventDefault();
    if (!newDocTitle.trim()) return;

    try {
      setCreating(true);
      const data = await documentsAPI.create(newDocTitle.trim());
      setShowCreateModal(false);
      setNewDocTitle('');
      navigate(`/documents/${data.document.id}`);
    } catch (err) {
      setError(err.message || 'Failed to create document');
    } finally {
      setCreating(false);
    }
  };

  const handleDeleteDocument = async (docId, e) => {
    e.stopPropagation();
    if (!confirm('Are you sure you want to delete this document?')) return;

    try {
      await documentsAPI.delete(docId);
      loadDocuments();
    } catch (err) {
      setError(err.message || 'Failed to delete document');
    }
  };

  const formatDate = (dateString) => {
    if (!dateString) return '';
    const date = new Date(dateString);
    return date.toLocaleDateString('en-US', {
      month: 'short',
      day: 'numeric',
      year: 'numeric',
    });
  };

  const isOwner = (doc) => {
    const userId = localStorage.getItem('user_id');
    return doc.owner_id === userId;
  };

  return (
    <div className="documents-container">
      <header className="documents-header">
        <div className="header-content">
          <h1>My Documents</h1>
          <div className="header-actions">
            <span className="user-name">{user?.username || user?.email}</span>
            <button onClick={logout} className="logout-button">
              Logout
            </button>
          </div>
        </div>
      </header>

      <main className="documents-main">
        {error && <div className="error-banner">{error}</div>}

        <div className="documents-toolbar">
          <button
            className="create-button"
            onClick={() => setShowCreateModal(true)}
          >
            + New Document
          </button>
        </div>

        {loading ? (
          <div className="loading-state">Loading documents...</div>
        ) : documents.length === 0 ? (
          <div className="empty-state">
            <div className="empty-icon">📄</div>
            <h2>No documents yet</h2>
            <p>Create your first document to get started</p>
            <button
              className="create-button"
              onClick={() => setShowCreateModal(true)}
            >
              Create Document
            </button>
          </div>
        ) : (
          <div className="documents-grid">
            {documents.map((doc) => {
              const owned = isOwner(doc);
              return (
                <div
                  key={doc.id}
                  className="document-card"
                  onClick={() => navigate(`/documents/${doc.id}`)}
                >
                  <div className="document-icon">📄</div>
                  <div className="document-info">
                    <div className="document-title-row">
                      <h3 className="document-title">{doc.title}</h3>
                      {!owned && (
                        <span className="shared-badge" title="Shared with you">
                          Shared
                        </span>
                      )}
                    </div>
                    <p className="document-meta">
                      {formatDate(doc.updated_at) || formatDate(doc.created_at)}
                    </p>
                  </div>
                  {owned && (
                    <button
                      className="delete-button"
                      onClick={(e) => handleDeleteDocument(doc.id, e)}
                      title="Delete document"
                    >
                      ×
                    </button>
                  )}
                </div>
              );
            })}
          </div>
        )}
      </main>

      {showCreateModal && (
        <div className="modal-overlay" onClick={() => setShowCreateModal(false)}>
          <div className="modal-content" onClick={(e) => e.stopPropagation()}>
            <h2>Create New Document</h2>
            <form onSubmit={handleCreateDocument}>
              <input
                type="text"
                value={newDocTitle}
                onChange={(e) => setNewDocTitle(e.target.value)}
                placeholder="Document title"
                autoFocus
                disabled={creating}
                required
              />
              <div className="modal-actions">
                <button
                  type="button"
                  onClick={() => {
                    setShowCreateModal(false);
                    setNewDocTitle('');
                  }}
                  disabled={creating}
                >
                  Cancel
                </button>
                <button type="submit" disabled={creating || !newDocTitle.trim()}>
                  {creating ? 'Creating...' : 'Create'}
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
}

