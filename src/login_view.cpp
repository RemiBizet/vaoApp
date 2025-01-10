#include "login_view.h"

LoginView::LoginView(DatabaseHandler& db)
    : db_handler(db)
{
    set_halign(Gtk::ALIGN_CENTER); // Center horizontally
    set_valign(Gtk::ALIGN_CENTER); // Center vertically

    // Configure grid
    main_grid.set_row_spacing(10);
    main_grid.set_column_spacing(10);

    // Username field
    auto username_label = Gtk::manage(new Gtk::Label("Username:"));
    main_grid.attach(*username_label, 0, 0, 1, 1);

    username_entry.set_placeholder_text("Enter username");
    username_entry.set_margin_bottom(10);
    main_grid.attach(username_entry, 1, 0, 1, 1);

    // Password field
    auto password_label = Gtk::manage(new Gtk::Label("Password:"));
    main_grid.attach(*password_label, 0, 1, 1, 1);

    password_entry.set_placeholder_text("Enter password");
    password_entry.set_visibility(false); // Hide password
    password_entry.set_margin_bottom(20);
    main_grid.attach(password_entry, 1, 1, 1, 1);

    // Login button
    login_button.set_label("Login");
    login_button.set_margin_top(10);

    // Signal for on_login_clicked
    login_button.signal_clicked().connect(
        sigc::mem_fun(*this, &LoginView::on_login_clicked)
    );
    main_grid.attach(login_button, 0, 2, 2, 1);

    // Status label for errors
    status_label.set_line_wrap(true);
    status_label.set_margin_top(10);
    main_grid.attach(status_label, 0, 3, 2, 1);
    
    create_account_button.set_label("Create Account");
    create_account_button.set_margin_top(10);

    // Signal for on_create_account_clicked
    create_account_button.signal_clicked().connect(
        sigc::mem_fun(*this, &LoginView::on_create_account_clicked)
    );
    main_grid.attach(create_account_button, 0, 4, 2, 1);

    add(main_grid);
    show_all();
}

void LoginView::on_login_clicked() {
    std::string username = username_entry.get_text();
    std::string password = password_entry.get_text();
    
    if (username.empty() || password.empty()) {
        show_error("Please fill in all fields");
        return;
    }
    
    try {
        // Hash the password using DatabaseHandler
        std::string passwordHashed = db_handler.hashPassword(password);
        
        // Verify the user credentials
        auto user = db_handler.verifyUserCredentials(username, passwordHashed);
        
        if (user) {
            // Clear fields for security
            username_entry.set_text("");
            password_entry.set_text("");

            db_handler.setCurrentUser(user);
            
            // Emit the login success signal
            m_signal_login_success.emit();
            
        } else {
            show_error("Invalid username or password");
        }
    } catch (const std::exception& e) {
        show_error("Login error: " + std::string(e.what()));
    }
}

void LoginView::show_error(const std::string& message) {
    status_label.set_markup("<span color='red'>" + message + "</span>");
}

// emit signal on create account
void LoginView::on_create_account_clicked() {
    m_signal_create_account.emit();
}