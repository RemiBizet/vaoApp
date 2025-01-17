#ifndef CHAT_ROOM_VIEW_H
#define CHAT_ROOM_VIEW_H

#pragma once
#include <gtkmm.h>
#include "database_handler.h"
#include "user.h"
#include <iostream>

class ChatRoomView : public Gtk::Box {
private:
    DatabaseHandler& db_handler;
    std::optional<User> current_user;
    std::string room_id;
    std::string room_name;
    std::string last_message_sender_id;

    // GUI Components 
    Gtk::Box main_box;
    Gtk::ScrolledWindow message_scroll;
    Gtk::Box message_box; 
    Gtk::Box input_box;
    Gtk::Entry message_entry;
    Gtk::Button send_button;
    Gtk::Button go_back_button;
    Gtk::Label room_label;

    // Methods
    void on_send_clicked();
    void on_go_back_clicked();
    void load_messages();
    void add_message(const std::string& content, const std::string& sender_id, bool is_from_current_user);
    void scroll_to_bottom();

    // Signal
    sigc::signal<void> m_signal_back_to_chat_list_requested;

public:
    ChatRoomView(DatabaseHandler& db_handler, const std::string& room_id, const std::string& room_name);
    virtual ~ChatRoomView() {}
    sigc::signal<void>& signal_back_to_chat_list_requested() { return m_signal_back_to_chat_list_requested;}
};

#endif 