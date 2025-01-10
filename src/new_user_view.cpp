#include "new_user_view.h"

NewUserView::NewUserView(DatabaseHandler& db)
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
    password_entry.set_visibility(false);
    password_entry.set_margin_bottom(10);
    main_grid.attach(password_entry, 1, 1, 1, 1);

    // Confirm Password field
    auto confirm_label = Gtk::manage(new Gtk::Label("Confirm Password:"));
    main_grid.attach(*confirm_label, 0, 2, 1, 1);
    confirm_password_entry.set_placeholder_text("Confirm password");
    confirm_password_entry.set_visibility(false);
    confirm_password_entry.set_margin_bottom(20);
    main_grid.attach(confirm_password_entry, 1, 2, 1, 1);

    // Create button
    create_button.set_label("Create Account");
    create_button.set_margin_top(10);

    create_button.signal_clicked().connect(
        sigc::mem_fun(*this, &NewUserView::on_create_clicked)
    );
    main_grid.attach(create_button, 0, 3, 2, 1);

    back_to_login_button.set_label("Back to login");
    back_to_login_button.set_margin_top(10);

    // Go back to login when clicked on
    back_to_login_button.signal_clicked().connect(
        sigc::mem_fun(*this, &NewUserView::on_back_to_login_clicked)
    );
    main_grid.attach(back_to_login_button, 0, 4, 2, 1);

    // Status label for errors/success
    status_label.set_line_wrap(true);
    status_label.set_margin_top(10);
    main_grid.attach(status_label, 0, 5, 2, 1);

    add(main_grid);
    show_all();
}

// On create new user
void NewUserView::on_create_clicked() {
    std::string username = username_entry.get_text();
    std::string password = password_entry.get_text();
    std::string confirm_password = confirm_password_entry.get_text();

    // Credentials validation
    if (username.empty() || password.empty() || confirm_password.empty()) {
        show_error("Please fill in all fields");
        return;
    }

    if (password != confirm_password) {
        show_error("Passwords do not match");
        return;
    }

    if (password.length() < 8) {
        show_error("Password must be at least 8 characters long");
        return;
    }

    try {
        // Hash the password
        std::string hashedPassword = db_handler.hashPassword(password);

        pqxx::connection dbConnection = db_handler.createConnection();
        pqxx::work txn(dbConnection);
        
        // First check if username already exists
        std::string check_query = "SELECT COUNT(*) FROM users WHERE username = " + 
                                txn.quote(username);
        auto result = txn.exec1(check_query)[0].as<int>();
        
        if (result > 0) {
            show_error("Username already exists");
            return;
        }

        
        User registered_user = User(username,hashedPassword);

        // Insert new user
        std::string insert_query = "INSERT INTO users (user_id, username, password_hash) VALUES (" +
                                 txn.quote(registered_user.getUserId()) + ", " +
                                 txn.quote(username) + ", " +
                                 txn.quote(hashedPassword) + ")";
        
        txn.exec(insert_query);
        txn.commit();

        // Emit the signal
        m_signal_back_to_login_requested.emit();

        show_success("Account created successfully!");
        
        // Clear the fields
        username_entry.set_text("");
        password_entry.set_text("");
        confirm_password_entry.set_text("");

        // Close the view after a short delay
        Glib::signal_timeout().connect_once(
            [this]() {
                hide();
            },
            2000
        );

    } catch (const std::exception& e) {
        show_error("Error creating account: " + std::string(e.what()));
    }
}

void NewUserView::show_error(const std::string& message) {
    status_label.set_markup("<span color='red'>" + message + "</span>");
}

void NewUserView::show_success(const std::string& message) {
    status_label.set_markup("<span color='green'>" + message + "</span>");
}

// Emit signal on create account
void NewUserView::on_back_to_login_clicked() {
    m_signal_back_to_login_requested.emit();
}