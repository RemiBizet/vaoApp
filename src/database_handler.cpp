#include "database_handler.h"

// Constructor
DatabaseHandler::DatabaseHandler(const std::string& connStr)
    : connStr(connStr) {
}

// Connect to the database
pqxx::connection DatabaseHandler::createConnection(){
    try {
        return pqxx::connection(connStr);
    } catch (const std::exception& e) {
        throw std::runtime_error("Database connection error: " + std::string(e.what()));
    }
}

// User session methods
void DatabaseHandler::setCurrentUser(std::optional<User> user) { current_user = user; }
const User& DatabaseHandler::getCurrentUser() const { 
    if (!current_user) throw std::runtime_error("No user logged in");
    return *current_user;
}
void DatabaseHandler::logout() { current_user = std::nullopt; }

// Hashing the password
std::string DatabaseHandler::hashPassword(const std::string& password) {
    // Buffer to hold the hash
    unsigned char hash[SHA256_DIGEST_LENGTH];

    // Compute the SHA-256 hash
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);

    // Convert the hash to a hexadecimal string
    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return oss.str();
}


// Verify user credentials
std::optional<User> DatabaseHandler::verifyUserCredentials(const std::string& username, const std::string& hashedPassword) {
    try {
        
        pqxx::connection dbConnection = createConnection();
        pqxx::work txn(dbConnection);

        // SQL query to fetch user details
        std::string query = 
            "SELECT user_id, username, password_hash "
            "FROM users WHERE username = " + txn.quote(username) +
            " AND password_hash = " + txn.quote(hashedPassword);

        auto result = txn.exec(query);

        if (!result.empty()) {
            // Extract data from the query result
            std::string fetchedUserId = result[0]["user_id"].as<std::string>();
            std::string fetchedUsername = result[0]["username"].as<std::string>();
            std::string fetchedPasswordHash = result[0]["password_hash"].as<std::string>();

            // Create and return a User object
            return User(fetchedUserId, fetchedUsername, fetchedPasswordHash);
        } else {
            // Return empty optional if no user is found
            return std::nullopt;
        }

    } catch (const std::exception& e) {
        // Log the error
        std::cerr << "Error verifying credentials: " << e.what() << std::endl;
        return std::nullopt;
    };
}

// Retrieve user conversations
std::vector<std::pair<std::string, std::string>> DatabaseHandler::get_user_conversations(const std::string& current_user_id) {
    std::vector<std::pair<std::string, std::string>> conversations;
    try {
        // Query to get all chat rooms the user is a member of
        std::string query = R"(
            SELECT cr.room_id, cr.room_name
            FROM chat_rooms cr
            INNER JOIN chat_room_members crm ON cr.room_id = crm.room_id
            WHERE crm.user_id = $1
            ORDER BY cr.created_at DESC;
        )";
        
        // Create a prepared statement
        pqxx::connection dbConnection = createConnection();
        pqxx::work txn(dbConnection);
        pqxx::result result = txn.exec_params(query, current_user_id);
        
        // Process the results
        for (const auto& row : result) {
            std::string room_id = row[0].as<std::string>();
            std::string room_name = row[1].as<std::string>();
            conversations.emplace_back(room_id, room_name);
        }
        
        txn.commit();
    } catch (const std::exception& e) {
        std::cerr << "Database error in get_user_conversations: " << e.what() << std::endl;
    }
    
    return conversations;
}

// Get or create a chat room
std::string DatabaseHandler::get_or_create_chat_room(const std::vector<std::string>& user_ids, const std::string& room_name) {
    try {
        pqxx::connection dbConnection = createConnection();
        pqxx::work txn(dbConnection);
        
        // Sort user IDs for consistent querying
        std::vector<std::string> sorted_user_ids = user_ids;
        std::sort(sorted_user_ids.begin(), sorted_user_ids.end());
        
        std::string find_query =
            "WITH RoomCounts AS ("
            " SELECT cr.room_id, COUNT(*) as member_count"
            " FROM chat_rooms cr"
            " JOIN chat_room_members crm ON cr.room_id = crm.room_id"
            " GROUP BY cr.room_id"
            " HAVING COUNT(*) = $1"
            ")"
            "SELECT cr.room_id "
            "FROM chat_rooms cr "
            "JOIN chat_room_members crm ON cr.room_id = crm.room_id "
            "WHERE cr.room_id IN (SELECT room_id FROM RoomCounts) "
            "AND crm.user_id = ANY($2::text[]) "
            "GROUP BY cr.room_id "
            "HAVING COUNT(*) = $1 "
            "LIMIT 1;";
        
        // Convert vector to PostgreSQL array string
        std::string array_str = "{";
        for (size_t i = 0; i < sorted_user_ids.size(); ++i) {
            if (i > 0) array_str += ",";
            array_str += "\"" + sorted_user_ids[i] + "\"";
        }
        array_str += "}";
        
        pqxx::result find_result = txn.exec_params(
            find_query,
            static_cast<int>(user_ids.size()),
            array_str
        );
        
        if (!find_result.empty()) {
            std::string existing_room_id = find_result[0][0].as<std::string>();
            return existing_room_id;
        }

        // Generate UUID for the new room
        uuid_t uuid;
        char uuid_str[37];
        uuid_generate_random(uuid);
        uuid_unparse_lower(uuid, uuid_str);
        std::string room_id = std::string(uuid_str);
        
        // Create new room with UUID
        std::string create_room_query = 
            "INSERT INTO chat_rooms (room_id, room_name) VALUES ($1, $2) "
            "RETURNING room_id;";

        std::cout << "Generated room_id: " << room_id << " (length: " << room_id.length() << ")" << std::endl;
            
        pqxx::result create_result = txn.exec_params(create_room_query, room_id, room_name);
        
        std::string add_members_query =
            "INSERT INTO chat_room_members (room_id, user_id) "
            "SELECT $1, unnest($2::text[]);";
        txn.exec_params(add_members_query, room_id, array_str);
        txn.commit();

        std::cout << "Add members query: " << add_members_query << std::endl;

        return room_id;
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR in get_or_create_chat_room: " << e.what() << std::endl;
        throw std::runtime_error("Failed to get or create chat room: " + std::string(e.what()));
    }
}

// Method to get all users except the current user
std::map<std::string, std::string> DatabaseHandler::get_all_users_except(const std::string& current_user_id) {
    std::map<std::string, std::string> users;
    try {
        pqxx::connection dbConnection = createConnection();
        pqxx::work txn(dbConnection);

        std::string query = R"(
            SELECT user_id, username
            FROM users
            WHERE user_id != $1
            ORDER BY username;
        )";
        
        pqxx::result result = txn.exec_params(query, current_user_id);
        
        for (const auto& row : result) {
            std::string user_id = row[0].as<std::string>();
            std::string username = row[1].as<std::string>();
            users.emplace(user_id, username);
        }
        
        txn.commit();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get users: " + std::string(e.what()));
    }
    
    return users;
}

// Method to get messages from a specific room
std::vector<Message> DatabaseHandler::get_room_messages(const std::string& room_id) {
    std::vector<Message> messages;
    try {
        pqxx::connection dbConnection = createConnection();
        pqxx::work txn(dbConnection);
        
        std::string query = R"(
            SELECT message_id, content, sender_id, timestamp, is_read
            FROM messages
            WHERE room_id = $1
            ORDER BY timestamp ASC;
        )";
        
        pqxx::result result = txn.exec_params(query, room_id);
        
        for (const auto& row : result) {
            messages.emplace_back(
                row["message_id"].as<std::string>(),     
                row["content"].as<std::string>(),        
                row["sender_id"].as<std::string>(),      
                parseTimestamp(row["timestamp"].as<std::string>()), 
                row["is_read"].as<bool>()                
            );
        }
        
        // Mark messages as read for the current user
        std::string update_query = R"(
            UPDATE messages
            SET is_read = TRUE
            WHERE room_id = $1 AND is_read = FALSE;
        )";
        
        txn.exec_params(update_query, room_id);
        txn.commit();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get room messages: " + std::string(e.what()));
    }
    
    return messages;
}

// Method to send a new message
void DatabaseHandler::send_message(const std::string room_id, const std::string& sender_id, const std::string& content) {
    try {
        pqxx::connection dbConnection = createConnection();
        pqxx::work txn(dbConnection);

        // Generate a unique message ID

        std::string message_id;
        uuid_t uuid;
        char uuid_str[37];
        uuid_generate_random(uuid);
        uuid_unparse_lower(uuid, uuid_str);
        message_id = std::string(uuid_str);
        
        std::string query = R"(
            INSERT INTO messages (message_id, content, sender_id, room_id)
            VALUES ($1, $2, $3, $4);
        )";
        
        txn.exec_params(query, message_id, content, sender_id, room_id);
        txn.commit();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to send message: " + std::string(e.what()));
    }
}

std::chrono::system_clock::time_point DatabaseHandler::parseTimestamp(const std::string& timestamp_str) {
    std::tm tm = {};
    std::stringstream ss(timestamp_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string DatabaseHandler::get_username_by_id(const std::string& user_id) {
    try {
        pqxx::connection dbConnection = createConnection();
        pqxx::work txn(dbConnection);
        
        // Using parameterized query to prevent SQL injection
        pqxx::result result = txn.exec_params(
            "SELECT username FROM users WHERE user_id = $1",
            user_id
        );
        
        if (result.empty()) {
            throw std::runtime_error("User not found");
        }
        
        std::string username = result[0]["username"].as<std::string>();
        txn.commit();
        
        return username;
    } catch (const std::exception& e) {
        throw std::runtime_error("Error getting username: " + std::string(e.what()));
    }
}

std::vector<std::string> DatabaseHandler::get_room_users(const std::string& room_id) {
    std::vector<std::string> usernames;
    try {
        pqxx::connection dbConnection = createConnection();
        pqxx::work txn(dbConnection);
        
        std::string query = R"(
            SELECT DISTINCT u.username 
            FROM users u 
            INNER JOIN chat_room_members crm ON u.user_id = crm.user_id 
            WHERE crm.room_id = $1 
            AND u.user_id != $2 
            ORDER BY u.username;
        )";
        
        pqxx::result result = txn.exec_params(query, room_id, current_user->getUserId());
        
        for (const auto& row : result) {
            usernames.push_back(row["username"].as<std::string>());
        }

        // If no other users found, return appropriate message
        if (usernames.empty()) {
            usernames.push_back("no other users");
        }

        txn.commit();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get room users: " + std::string(e.what()));
    }
    
    return usernames;
}