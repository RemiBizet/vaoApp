#ifndef LOGIN_VIEW_H
#define LOGIN_VIEW_H

#include <gtkmm.h>
#include "database_handler.h"
#include "chat_list_view.h"
#include "user.h"
#include "new_user_view.h"
#include <iostream>

class LoginView : public Gtk::Box{
private:
    // Database handler
    DatabaseHandler& db_handler;

    // GUI components
    Gtk::Grid main_grid;
    Gtk::Entry username_entry;
    Gtk::Entry password_entry;
    Gtk::Button login_button;
    Gtk::Label status_label;
    Gtk::Button create_account_button;
    
    // Signal handlers
    void on_login_clicked();
    void show_error(const std::string& message);
    void on_create_account_clicked();

    // Signals
    sigc::signal<void> m_signal_create_account;
    sigc::signal<void> m_signal_login_success;  

public:
    explicit LoginView(DatabaseHandler& db);
    ~LoginView() override = default;

    // Signals getters implementations
    sigc::signal<void>& signal_create_account() { return m_signal_create_account; }
    sigc::signal<void>& signal_login_success() { return m_signal_login_success; }
};

#endif // LOGIN_VIEW_H