#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <chrono>
#include <uuid/uuid.h>
#include <iomanip>
#include <sstream>

class Message {

public:

    std::string message_id;
    std::string content;
    std::string sender_id;
    std::string room_id;
    std::chrono::system_clock::time_point timestamp;
    bool is_read;
    
    // Constructor for loading existing messages from database
    Message(const std::string& message_id,
            const std::string& content,
            const std::string& sender_id,
            const std::chrono::system_clock::time_point& timestamp,
            bool is_read);

    // Getters
    const std::string& getMessageId() const { return message_id; }
    const std::string& getContent() const { return content; }
    const std::string& getSenderId() const { return sender_id; }
    const std::string& getRoomId() const { return room_id; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp; }
    bool getIsRead() const { return is_read; }

    // Setters
    void markAsRead();

    // Utility methods
    std::string getFormattedTimestamp() const;
};

#endif