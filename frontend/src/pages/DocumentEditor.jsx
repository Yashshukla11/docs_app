import { useState, useEffect } from 'react';
import { useNavigate, useParams } from 'react-router-dom';
import { useAuth } from '../contexts/AuthContext';
import { documentsAPI } from '../services/api';
import './DocumentEditor.css';

export default function DocumentEditor() {
  const { id } = useParams();
  const [document, setDocument] = useState(null);
  const [title, setTitle] = useState('');
  const [content, setContent] = useState('');
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState('');
  const [isEditingTitle, setIsEditingTitle] = useState(false);
  const { logout } = useAuth();
  const navigate = useNavigate();

  useEffect(() => {
    loadDocument();
  }, [id]);

  const loadDocument = async () => {
    try {
      setLoading(true);
      const data = await documentsAPI.getById(id);
      setDocument(data.document);
      setTitle(data.document.title);
      setContent(data.document.content || '');
    } catch (err) {
      setError(err.message || 'Failed to load document');
      if (err.message.includes('Access denied') || err.message.includes('not found')) {
        setTimeout(() => navigate('/documents'), 2000);
      }
    } finally {
      setLoading(false);
    }
  };

  const handleSave = async () => {
    if (!title.trim()) {
      setError('Title cannot be empty');
      return;
    }

    try {
      setSaving(true);
      setError('');
      await documentsAPI.update(id, title.trim(), content);
      setSaving(false);
    } catch (err) {
      setError(err.message || 'Failed to save document');
      setSaving(false);
    }
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
            {saving && <span className="saving-indicator">Saving...</span>}
            <button className="save-button" onClick={handleSave} disabled={saving}>
              Save
            </button>
            <button className="delete-button" onClick={handleDelete}>
              Delete
            </button>
            <button className="logout-button" onClick={logout}>
              Logout
            </button>
          </div>
        </div>
      </header>

      {error && <div className="error-banner">{error}</div>}

      <main className="editor-main">
        <textarea
          className="editor-textarea"
          value={content}
          onChange={(e) => setContent(e.target.value)}
          placeholder="Start writing..."
          spellCheck={true}
        />
      </main>
    </div>
  );
}

