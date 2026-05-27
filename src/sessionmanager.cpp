#include "sessionmanager.h"
#include "config.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

static const char* CONNECTION_NAME = "chess_session_db";

SessionManager::SessionManager()
{
    m_initialized = initDatabase();
}

SessionManager::~SessionManager()
{
    QSqlDatabase::removeDatabase(CONNECTION_NAME);
}

bool SessionManager::initDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
    db.setDatabaseName(Config::DB_PATH);

    if (!db.open()) {
        qWarning() << "SessionManager: no se pudo abrir la BD SQLite:"
                   << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    const bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS session ("
        "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  token    TEXT    NOT NULL,"
        "  user_id  INTEGER NOT NULL,"
        "  username TEXT    NOT NULL"
        ")"
    );

    if (!ok) {
        qWarning() << "SessionManager: no se pudo crear la tabla:"
                   << query.lastError().text();
        return false;
    }

    return true;
}

void SessionManager::saveSession(const UserData& user)
{
    if (!m_initialized) return;

    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery query(db);

    // Solo guardamos una sesión a la vez
    query.exec("DELETE FROM session");

    query.prepare(
        "INSERT INTO session (token, user_id, username) "
        "VALUES (:token, :uid, :uname)"
    );
    query.bindValue(":token", user.token);
    query.bindValue(":uid",   user.userId);
    query.bindValue(":uname", user.username);

    if (!query.exec()) {
        qWarning() << "SessionManager: saveSession falló:"
                   << query.lastError().text();
    }
}

std::optional<UserData> SessionManager::loadSession() const
{
    if (!m_initialized) return std::nullopt;

    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery query(db);
    query.exec("SELECT token, user_id, username FROM session LIMIT 1");

    if (query.next()) {
        UserData user;
        user.token    = query.value(0).toString();
        user.userId   = query.value(1).toInt();
        user.username = query.value(2).toString();
        return user;
    }

    return std::nullopt;
}

void SessionManager::clearSession()
{
    if (!m_initialized) return;

    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery query(db);
    query.exec("DELETE FROM session");
}
