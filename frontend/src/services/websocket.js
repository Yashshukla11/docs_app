// WebSocket service for real-time collaboration
class WebSocketService {
  constructor() {
    this.ws = null;
    this.docId = null;
    this.reconnectAttempts = 0;
    this.maxReconnectAttempts = 5;
    this.reconnectDelay = 1000;
    this.listeners = new Map();
    this.isConnecting = false;
  }

  // Get WebSocket URL
  getWebSocketUrl(docId) {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const hostname = window.location.hostname;
    const port = '8080';
    return `${protocol}//${hostname}:${port}/api/documents/ws/connect`;
  }

  // Connect to WebSocket
  connect(docId, onMessage, onError) {
    console.log('[WebSocket] connect() called with docId:', docId);
    console.log('[WebSocket] Current state - ws:', this.ws, 'readyState:', this.ws?.readyState, 'docId:', this.docId);
    
    if (this.ws && this.ws.readyState === WebSocket.OPEN && this.docId === docId) {
      console.log('[WebSocket] Already connected to this document');
      return; // Already connected to this document
    }

    if (this.isConnecting) {
      console.log('[WebSocket] Already connecting, skipping...');
      return; // Already connecting
    }

    this.isConnecting = true;
    this.docId = docId;
    
    // Disconnect existing connection if any
    if (this.ws) {
      console.log('[WebSocket] Disconnecting existing connection...');
      this.disconnect();
    }

    const token = localStorage.getItem('auth_token');
    if (!token) {
      console.error('[WebSocket] No auth token found in localStorage');
      this.isConnecting = false;
      if (onError) {
        onError(new Error('No auth token found'));
      }
      return;
    }

    // WebSocket doesn't support custom headers in browser, so we use query parameters
    const wsUrl = this.getWebSocketUrl(docId);
    const wsUrlWithParams = `${wsUrl}?doc_id=${encodeURIComponent(docId)}&token=${encodeURIComponent(token)}`;
    
    console.log('[WebSocket] Attempting to connect to:', wsUrlWithParams);
    console.log('[WebSocket] Token length:', token.length);
    
    try {
      this.ws = new WebSocket(wsUrlWithParams);
      console.log('[WebSocket] WebSocket object created, readyState:', this.ws.readyState);
      
      // Set a timeout to detect if connection doesn't open
      const connectionTimeout = setTimeout(() => {
        if (this.ws && this.ws.readyState !== WebSocket.OPEN) {
          console.error('[WebSocket] Connection timeout - readyState:', this.ws.readyState);
          this.isConnecting = false;
          if (onError) {
            onError(new Error('WebSocket connection timeout'));
          }
        }
      }, 5000);

      this.ws.onopen = () => {
        console.log('[WebSocket] ✅ Connected successfully to:', wsUrlWithParams);
        clearTimeout(connectionTimeout);
        this.reconnectAttempts = 0;
        this.isConnecting = false;
        this.onOpen();
      };

      this.ws.onmessage = (event) => {
        try {
          const message = JSON.parse(event.data);
          console.log('[WebSocket] 📨 Message received:', message.type, message);
          if (onMessage) {
            onMessage(message);
          }
          this.handleMessage(message);
        } catch (err) {
          console.error('[WebSocket] Error parsing message:', err, event.data);
        }
      };

      this.ws.onerror = (error) => {
        console.error('[WebSocket] ❌ Connection error:', error);
        console.error('[WebSocket] URL was:', wsUrlWithParams);
        console.error('[WebSocket] ReadyState:', this.ws?.readyState);
        clearTimeout(connectionTimeout);
        this.isConnecting = false;
        if (onError) {
          onError(error);
        }
      };

      this.ws.onclose = (event) => {
        console.log('[WebSocket] Connection closed. Code:', event.code, 'Reason:', event.reason || 'No reason');
        clearTimeout(connectionTimeout);
        this.isConnecting = false;
        this.ws = null;
        
        // Log close codes for debugging
        if (event.code === 1006) {
          console.error('[WebSocket] Abnormal closure (1006) - connection lost without close frame');
        } else if (event.code === 1002) {
          console.error('[WebSocket] Protocol error (1002)');
        } else if (event.code === 1003) {
          console.error('[WebSocket] Unsupported data (1003)');
        } else if (event.code === 1008) {
          console.error('[WebSocket] Policy violation (1008)');
        }
        
        // Attempt to reconnect if not a normal closure
        if (event.code !== 1000 && this.reconnectAttempts < this.maxReconnectAttempts) {
          this.reconnectAttempts++;
          console.log(`[WebSocket] Attempting to reconnect (${this.reconnectAttempts}/${this.maxReconnectAttempts})...`);
          setTimeout(() => {
            if (this.docId) {
              this.connect(this.docId, onMessage, onError);
            }
          }, this.reconnectDelay * this.reconnectAttempts);
        } else if (event.code === 1000) {
          console.log('[WebSocket] Normal closure, not reconnecting');
        } else {
          console.log('[WebSocket] Max reconnection attempts reached');
        }
      };
    } catch (error) {
      console.error('Error creating WebSocket:', error);
      this.isConnecting = false;
      if (onError) {
        onError(error);
      }
    }
  }

  // Disconnect WebSocket
  disconnect() {
    if (this.ws) {
      this.ws.close(1000, 'Client disconnect');
      this.ws = null;
    }
    this.docId = null;
    this.reconnectAttempts = 0;
  }

  // Send message via WebSocket
  send(message) {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      const messageStr = JSON.stringify(message);
      console.log('[WebSocket] 📤 Sending message (type:', message.type, 'length:', messageStr.length, ')');
      this.ws.send(messageStr);
    } else {
      console.warn('[WebSocket] ⚠️ Cannot send - WebSocket not connected. readyState:', this.ws?.readyState, 'ws:', !!this.ws);
    }
  }

  // Send edit operation
  sendEdit(operation) {
    const message = {
      type: 'edit',
      ...operation
    };
    console.log('[WebSocket] 📤 Sending edit:', message);
    this.send(message);
  }

  // Send cursor position
  sendCursor(data) {
    this.send({
      type: 'cursor',
      position: data.position,
      userId: data.userId || localStorage.getItem('user_id'),
      username: data.username
    });
  }

  // Handle incoming messages
  handleMessage(message) {
    const listeners = this.listeners.get(message.type) || [];
    listeners.forEach(listener => {
      try {
        listener(message);
      } catch (err) {
        console.error('Error in message listener:', err);
      }
    });
  }

  // Add message listener
  on(messageType, callback) {
    if (!this.listeners.has(messageType)) {
      this.listeners.set(messageType, []);
    }
    this.listeners.get(messageType).push(callback);
  }

  // Remove message listener
  off(messageType, callback) {
    const listeners = this.listeners.get(messageType);
    if (listeners) {
      const index = listeners.indexOf(callback);
      if (index > -1) {
        listeners.splice(index, 1);
      }
    }
  }

  // Handle connection open
  onOpen() {
    // Notify that connection is ready
    const listeners = this.listeners.get('open') || [];
    listeners.forEach(listener => listener());
  }

  // Check if connected
  isConnected() {
    return this.ws && this.ws.readyState === WebSocket.OPEN;
  }
}

// Export singleton instance
export const websocketService = new WebSocketService();

