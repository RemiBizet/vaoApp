#ifndef NEW_USER_VIEW_H
#define NEW_USER_VIEW_H

#include <gtkmm.h>
#include "database_handler.h"
#include <iostream>
#include "user.h"

class NewUserView : public Gtk::Box {
private:

    // Components
    DatabaseHandler& db_handler;
    Gtk::Grid main_grid;
    Gtk::Entry username_entry;
    Gtk::Entry password_entry;
    Gtk::Entry confirm_password_entry;
    Gtk::Button create_button;
    Gtk::Button back_to_login_button;
    Gtk::Label status_label;

    // Signals
    void on_create_clicked();
    void on_back_to_login_clicked();
    void show_error(const std::string& message);
    void show_success(const std::string& message);

    sigc::signal<void> m_signal_back_to_login_requested;

public:

    // Constructor
    NewUserView(DatabaseHandler& db);

    // Signal to go back to the login view
    sigc::signal<void>& signal_back_to_login_requested() { return m_signal_back_to_login_requested; };
};


#endif