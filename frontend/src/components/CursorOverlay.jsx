import { useEffect, useRef, useState } from 'react';

// Component to display other users' cursors in the textarea
// Note: Accurate cursor positioning in textarea is complex, so we show a simplified version
export default function CursorOverlay({ textareaRef, userCursors, content }) {
  const [visibleCursors, setVisibleCursors] = useState([]);

  useEffect(() => {
    // Filter out stale cursors and prepare visible cursors
    const now = Date.now();
    const active = Array.from(userCursors.entries())
      .filter(([userId, data]) => now - data.timestamp < 3000)
      .map(([userId, data]) => ({
        userId,
        position: data.position,
        username: data.username,
        line: content.substring(0, data.position).split('\n').length - 1
      }));

    setVisibleCursors(active);
  }, [userCursors, content]);

  if (!textareaRef.current || visibleCursors.length === 0) {
    return null;
  }

  const textarea = textareaRef.current;
  const style = window.getComputedStyle(textarea);
  const lineHeight = parseFloat(style.lineHeight) || 20;
  const paddingTop = parseFloat(style.paddingTop) || 0;
  const paddingLeft = parseFloat(style.paddingLeft) || 0;

  return (
    <div
      style={{
        position: 'absolute',
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        pointerEvents: 'none',
        zIndex: 5,
        overflow: 'visible',
      }}
    >
      {visibleCursors.map((cursor) => {
        const top = (cursor.line * lineHeight) + paddingTop + textarea.scrollTop;
        const left = paddingLeft + 10; // Approximate position

        return (
          <div key={cursor.userId} style={{ position: 'relative' }}>
            <div
              style={{
                position: 'absolute',
                top: `${top}px`,
                left: `${left}px`,
                width: '2px',
                height: `${lineHeight}px`,
                backgroundColor: '#667eea',
                animation: 'cursorBlink 1s infinite',
                pointerEvents: 'none',
              }}
            />
            <div
              style={{
                position: 'absolute',
                top: `${top - 20}px`,
                left: `${left}px`,
                background: '#667eea',
                color: 'white',
                padding: '2px 6px',
                borderRadius: '4px',
                fontSize: '11px',
                whiteSpace: 'nowrap',
                fontWeight: '500',
                pointerEvents: 'none',
                boxShadow: '0 2px 4px rgba(0,0,0,0.2)',
              }}
            >
              {cursor.username}
            </div>
          </div>
        );
      })}
      <style>{`
        @keyframes cursorBlink {
          0%, 50% { opacity: 1; }
          51%, 100% { opacity: 0.4; }
        }
      `}</style>
    </div>
  );
}

