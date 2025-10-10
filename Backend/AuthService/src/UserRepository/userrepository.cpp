#include "userrepository.h"
#include <sqlite3.h>
#include <iostream>

UserRepository::UserRepository(DataBase& database)
    : database_(database)
{
    createUserDataBase();
}


void UserRepository::createUserDataBase(){
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users(
            ID INTEGER PRIMARY KEY AUTOINCREMENT,
            NAME TEXT NOT NULL,
            EMAIL TEXT NOT NULL,
            TAG TEXT NOT NULL,
            PASSWORD TEXT NOT NULL
        );
    )";

    char* errMsg = nullptr;
    if (sqlite3_exec(database_.getHandle(), sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creating USERS table: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

std::optional<User> UserRepository::createUser(RegisterRequest req){
        sqlite3_stmt* stmt;
        const char* sql =
            "INSERT INTO users (name, email, password, tag) VALUES (?, ?, ?, ?);";

        sqlite3* handle = database_.getHandle();

        if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "SQL error (prepare): " << sqlite3_errmsg(handle) << "\n";
            return std::nullopt;
        }

        // Прив'язуємо параметри
        sqlite3_bind_text(stmt, 1, req.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, req.email.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, req.password.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, req.tag.c_str(), -1, SQLITE_TRANSIENT);

        // Виконуємо запит
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL error (step): " << sqlite3_errmsg(handle) << "\n";
            sqlite3_finalize(stmt);
            return std::nullopt;
        }

        // Отримуємо id вставленого рядка
        int newId = (int)sqlite3_last_insert_rowid(handle);
        User createdUser{
            .name = req.name,
            .email = req.email,
            .id = newId,
            .password = req.password
        };

        sqlite3_finalize(stmt);
        return createdUser;
}

std::optional<User> UserRepository::findByEmail(std::string email){
        sqlite3_stmt* stmt;
        sqlite3* handle = database_.getHandle();

        const char* sql = "SELECT id, name, email, password FROM USERS WHERE email=?";

        if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "SQL prepare error: " << sqlite3_errmsg(handle) << "\n";
            return std::nullopt;
        }

        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

        std::optional<User> findedUser;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            findedUser = User{
                .id = sqlite3_column_int(stmt, 0),
                .name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
                .email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
                .password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            };
        }

        sqlite3_finalize(stmt);
        return findedUser;
}

std::optional<User> UserRepository::findById(int id){
    sqlite3_stmt* stmt;
    sqlite3* handle = database_.getHandle();

    const char* sql = "SELECT id, name, email, tag FROM users WHERE id=?";

    if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(handle) << "\n";
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);

    std::optional<User> findedUser;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        findedUser = User{
            .id = sqlite3_column_int(stmt, 0),
            .name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            .email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            .tag = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
        };
    }

    sqlite3_finalize(stmt);
    return findedUser;
}

QList<User> UserRepository::findByTag(std::string tag){
    QList<User> res;
    sqlite3_stmt* stmt;
    sqlite3* handle = database_.getHandle();

    const char* sql = "SELECT id, name, email, tag FROM users WHERE tag=?";

    if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(handle) << "\n";
        return res;
    }

    sqlite3_bind_text(stmt, 1, tag.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<User> findedUser;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        findedUser = User{
            .id = sqlite3_column_int(stmt, 0),
            .name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            .email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            .tag = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
        };
        res.push_back(*findedUser);
    }

    sqlite3_finalize(stmt);
    return res;
}

void UserRepository::clear() {
    const char* sql = "DELETE FROM users;";

    char* errMsg = nullptr;
    if (sqlite3_exec(database_.getHandle(), sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[ERROR] Failed to clear users table: " << errMsg << "\n";
        sqlite3_free(errMsg);
    } else {
        std::cout << "[INFO] Cleared all users from table successfully.\n";
    }
}

