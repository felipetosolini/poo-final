#pragma once
#include <optional>
#include "userdata.h"

// Persiste la sesión del usuario entre reinicios usando SQLite local.
// Tabla: session(id, token, user_id, username)
class SessionManager {
public:
    SessionManager();
    ~SessionManager();

    void saveSession(const UserData& user);
    std::optional<UserData> loadSession() const;
    void clearSession();

private:
    bool m_initialized = false;

    bool initDatabase();
};
