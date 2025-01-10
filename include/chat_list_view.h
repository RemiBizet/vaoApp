#ifndef CHAT_LIST_VIEW_H
#define CHAT_LIST_VIEW_H

#pragma once

#include <gtkmm.h>
#include "database_handler.h"
#include "user.h"
#include "chat_room_view.h"
#include "new_chat_room_view.h"

class ChatListView : public Gtk::Box {
private:
    // Database handler
    DatabaseHandler& db_handler;
    std::optional<User> current_user;

    // Widgets
    Gtk::Box main_box;
    Gtk::ScrolledWindow scroll;
    Gtk::ListBox chat_list;
    Gtk::Button new_chat_button;
    Gtk::Button logout_button;
    
    // Private methods
    void on_chat_row_activated(Gtk::ListBoxRow* row);
    bool on_button_press_event(GdkEventButton* event);
    void on_new_chat_room_clicked();
    void load_conversations();
    void on_logout_clicked();

    // Signals
    sigc::signal<void> m_signal_create_new_chat_room;
    sigc::signal<void> m_signal_logout;

    typedef sigc::signal<void, std::string, std::string> type_signal_open_chat_room;
    type_signal_open_chat_room m_signal_open_chat_room;
    
public:
    ChatListView(DatabaseHandler& db);

    // Signals getters
    sigc::signal<void>& signal_create_new_chat_room() { return m_signal_create_new_chat_room; }
    sigc::signal<void>& signal_logout() { return m_signal_logout; }
    type_signal_open_chat_room signal_open_chat_room() { return m_signal_open_chat_room; }
};

#endif