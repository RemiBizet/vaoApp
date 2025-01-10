#ifndef NEW_CHAT_ROOM_VIEW_H
#define NEW_CHAT_ROOM_VIEW_H

#pragma once
#include <gtkmm.h>
#include "database_handler.h"
#include "user.h"
#include "chat_room_view.h"

class NewChatRoomView : public Gtk::Box {
private:

    DatabaseHandler& db_handler;
    std::optional<User> current_user;
    std::map<std::string, std::string> all_users;
    std::map<std::string, Gtk::CheckButton*> user_checkboxes; 

    // Components
    Gtk::Box main_box;
    Gtk::Box button_box;
    Gtk::SearchEntry search_entry;
    Gtk::ScrolledWindow user_scroll;
    Gtk::ListBox user_list;
    Gtk::Label selected_users_label;
    Gtk::Label room_name_label;
    Gtk::Entry room_name_entry;
    Gtk::Button confirm_button;
    Gtk::Button go_back_button;
    
    // Methods handling signals 
    void load_users();
    void filter_users(const Glib::ustring& search_text);
    void on_checkbox_toggled(Gtk::CheckButton* checkbox, const std::string& user_id);
    void update_selected_count();
    void on_search_changed();
    void on_confirm_clicked();
    std::vector<std::string> get_selected_user_ids();
    void on_go_back_clicked();

    sigc::signal<void> m_signal_back_to_chat_list_requested;
    
public:
    NewChatRoomView(DatabaseHandler& db_handler);
    sigc::signal<void>& signal_back_to_chat_list_requested() { return m_signal_back_to_chat_list_requested; }
};

#endif