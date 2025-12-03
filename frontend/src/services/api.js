// Auto-detect API URL based on current host
const getApiBaseUrl = () => {
  // If running on localhost, use localhost backend
  if (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1') {
    return 'http://localhost:8080/api';
  }
  // Otherwise, use the same hostname with port 8080
  return `http://${window.location.hostname}:8080/api`;
};

const API_BASE_URL = getApiBaseUrl();

// Helper function to get auth token from localStorage
const getAuthToken = () => {
  return localStorage.getItem('auth_token');
};

// Helper function to make authenticated requests
const fetchWithAuth = async (url, options = {}) => {
  const token = getAuthToken();
  const headers = {
    'Content-Type': 'application/json',
    ...options.headers,
  };

  if (token) {
    headers['Authorization'] = `Bearer ${token}`;
  }

  const response = await fetch(`${API_BASE_URL}${url}`, {
    ...options,
    headers,
  });

  if (response.status === 401) {
    // Unauthorized - clear token and redirect to login
    localStorage.removeItem('auth_token');
    localStorage.removeItem('user_id');
    window.location.href = '/login';
    throw new Error('Unauthorized');
  }

  if (!response.ok) {
    const error = await response.json().catch(() => ({ error: 'Request failed' }));
    // For 409 conflicts, preserve the full error object
    if (response.status === 409) {
      error.status = 409;
      const err = new Error(JSON.stringify(error));
      err.status = 409;
      err.data = error;
      throw err;
    }
    throw new Error(error.error || 'Request failed');
  }

  return response.json();
};

// Auth API
export const authAPI = {
  register: async (email, username, password) => {
    const response = await fetch(`${API_BASE_URL}/auth/register`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ email, username, password }),
    });

    if (!response.ok) {
      const error = await response.json().catch(() => ({ error: 'Registration failed' }));
      throw new Error(error.error || 'Registration failed');
    }

    const data = await response.json();
    if (data.token) {
      localStorage.setItem('auth_token', data.token);
      localStorage.setItem('user_id', data.user_id);
    }
    return data;
  },

  login: async (email, password) => {
    const response = await fetch(`${API_BASE_URL}/auth/login`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ email, password }),
    });

    if (!response.ok) {
      const error = await response.json().catch(() => ({ error: 'Login failed' }));
      throw new Error(error.error || 'Login failed');
    }

    const data = await response.json();
    if (data.token) {
      localStorage.setItem('auth_token', data.token);
      localStorage.setItem('user_id', data.user_id);
    }
    return data;
  },

  getCurrentUser: async () => {
    return fetchWithAuth('/auth/me');
  },

  logout: () => {
    localStorage.removeItem('auth_token');
    localStorage.removeItem('user_id');
  },
};

// Documents API
export const documentsAPI = {
  getAll: async () => {
    return fetchWithAuth('/documents');
  },

  getById: async (docId) => {
    return fetchWithAuth(`/documents/${docId}`);
  },

  create: async (title, content = '') => {
    return fetchWithAuth('/documents', {
      method: 'POST',
      body: JSON.stringify({ title, content }),
    });
  },

  update: async (docId, title, content, _version = null) => {
    // Ignore optimistic locking version on the client side.
    // Always send the latest title/content and let the server persist it (last-write-wins).
    return fetchWithAuth(`/documents/${docId}`, {
      method: 'PATCH',
      body: JSON.stringify({ title, content }),
    });
  },

  rename: async (docId, title) => {
    return fetchWithAuth(`/documents/${docId}/rename`, {
      method: 'PATCH',
      body: JSON.stringify({ title }),
    });
  },

  delete: async (docId) => {
    return fetchWithAuth(`/documents/${docId}`, {
      method: 'DELETE',
    });
  },
};

// Collaboration API
export const collaborationAPI = {
  share: async (docId, email, permission) => {
    return fetchWithAuth(`/documents/${docId}/share`, {
      method: 'POST',
      body: JSON.stringify({ email, permission }),
    });
  },

  getCollaborators: async (docId) => {
    return fetchWithAuth(`/documents/${docId}/collaborators`);
  },

  updatePermission: async (docId, collaboratorId, permission) => {
    return fetchWithAuth(`/documents/${docId}/collaborators/${collaboratorId}`, {
      method: 'PATCH',
      body: JSON.stringify({ permission }),
    });
  },

  removeCollaborator: async (docId, collaboratorId) => {
    return fetchWithAuth(`/documents/${docId}/collaborators/${collaboratorId}`, {
      method: 'DELETE',
    });
  },
};

