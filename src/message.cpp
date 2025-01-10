#include "message.h"

Message::Message(const std::string& message_id,
                const std::string& content,
                const std::string& sender_id,
                const std::chrono::system_clock::time_point& timestamp,
                bool is_read)
    : message_id(message_id),
      content(content),
      sender_id(sender_id),
      timestamp(timestamp),
      is_read(is_read) {
}

void Message::markAsRead() {
    is_read = true;
}

std::string Message::getFormattedTimestamp() const {
    auto time = std::chrono::system_clock::to_time_t(timestamp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}