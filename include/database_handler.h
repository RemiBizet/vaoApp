#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

#include "user.h"
#include "message.h"
#include <pqxx/pqxx>
#include <openssl/sha.h>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <utility>
#include <iostream>  
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <boost/algorithm/string/join.hpp>

class DatabaseHandler {
private:
    // Connection string and current user informations
    std::string connStr;
    std::optional<User> current_user;
    std::chrono::system_clock::time_point parseTimestamp(const std::string& timestamp_str);

public:
    // Connection method and constructor
    explicit DatabaseHandler(const std::string& connStr);
    pqxx::connection createConnection();

    // User management related methods
    void setCurrentUser(const std::optional<User> user);
    const User& getCurrentUser() const;
    void logout();

    // User logging-in management methods
    static std::string hashPassword(const std::string& password);
    std::optional<User> verifyUserCredentials(const std::string& username, const std::string& hashedPassword);

    // Chat list related methods
    std::vector<std::pair<std::string, std::string>> get_user_conversations(const std::string& current_user_id);

    // Create new chat room
    std::string get_or_create_chat_room(const std::vector<std::string>& user_ids, const std::string& room_name);
    std::map<std::string, std::string> get_all_users_except(const std::string& current_user_id);

    // Chat room related methods
    std::vector<Message> get_room_messages(const std::string& room_id);
    void send_message(const std::string room_id, const std::string& sender_id, const std::string& content);
};

#endif // DATABASE_HANDLER_H
