#!/bin/sh

# Source this file. The value is cached in the caller so every project helper
# participating in one session uses the same PID/start-time identity.
keel_session_pid() {
    if [ -n "${KEEL_SESSION_IDENTITY:-}" ]; then
        printf '%s\n' "$KEEL_SESSION_IDENTITY"
        return 0
    fi

    session_pid=${CODEX_PID:-${CLAUDE_CODE_PID:-$PPID}}
    session_start=unknown
    if [ -r "/proc/$session_pid/stat" ]; then
        session_start=$(awk '{print $22}' "/proc/$session_pid/stat" 2>/dev/null)
    elif command -v ps >/dev/null 2>&1; then
        session_start=$(ps -o lstart= -p "$session_pid" 2>/dev/null | tr -s ' ' '_')
    fi

    KEEL_SESSION_IDENTITY="${session_pid}:${session_start:-unknown}"
    export KEEL_SESSION_IDENTITY
    printf '%s\n' "$KEEL_SESSION_IDENTITY"
}
