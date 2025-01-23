#include "new_chat_room_view.h"

// Main Constructor
NewChatRoomView::NewChatRoomView(DatabaseHandler& db_handler)
    : db_handler(db_handler) {
    
    // Obtain the current user
    current_user = db_handler.getCurrentUser();

    set_halign(Gtk::ALIGN_CENTER); // Center horizontally
    set_valign(Gtk::ALIGN_CENTER); // Center vertically

    // Initialize components
    main_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10);
    button_box = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10);
    confirm_button = Gtk::Button("Create Chat");
    go_back_button = Gtk::Button("Go Back");

    // Add title label
    auto title_label = Gtk::manage(new Gtk::Label("Create New Chat Room"));
    title_label->get_style_context()->add_class("title-2");
    title_label->set_margin_bottom(10);
    title_label->set_halign(Gtk::ALIGN_START);
   
    // Setup search
    search_entry.set_placeholder_text("Search users...");
    search_entry.set_margin_bottom(10);
    
    // Setup room name entry
    room_name_label.set_text("Room Name (optional):");
    room_name_label.set_halign(Gtk::ALIGN_START);
    room_name_label.set_margin_bottom(5);
    room_name_entry.set_placeholder_text("Enter room name...");
    room_name_entry.set_margin_bottom(10);
    
    // Setup user list with multiple selection
    user_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    user_scroll.add(user_list);
    user_list.set_selection_mode(Gtk::SelectionMode::SELECTION_MULTIPLE);
    user_scroll.set_size_request(400, 300); // Set a fixed size for the scrollable area
    
    // Setup selected users label
    selected_users_label.set_text("Selected users: 0");
    selected_users_label.set_halign(Gtk::ALIGN_START);
    selected_users_label.set_margin_bottom(10);
    
    // Setup confirm button
    confirm_button.set_sensitive(false);
    confirm_button.get_style_context()->add_class("suggested-action");
    confirm_button.set_size_request(-1, 40);
    
    // Setup go back button
    go_back_button.set_size_request(-1, 40);
    
    // Connect signals
    search_entry.signal_search_changed().connect(
        sigc::mem_fun(*this, &NewChatRoomView::on_search_changed)
    );
    
    confirm_button.signal_clicked().connect(
        sigc::mem_fun(*this, &NewChatRoomView::on_confirm_clicked)
    );
    
    go_back_button.signal_clicked().connect(
        sigc::mem_fun(*this, &NewChatRoomView::on_go_back_clicked)
    );

    // Pack widgets in order
    main_box.pack_start(*title_label, false, false, 0);
    main_box.pack_start(search_entry, false, false, 0);
    main_box.pack_start(user_scroll, true, true, 0);
    main_box.pack_start(selected_users_label, false, false, 0);
    main_box.pack_start(room_name_label, false, false, 0);
    main_box.pack_start(room_name_entry, false, false, 0);
    
    button_box.pack_start(confirm_button, true, true, 0);
    button_box.pack_end(go_back_button, true, true, 0);
    button_box.set_margin_top(10);
    button_box.set_margin_bottom(10);

    main_box.pack_start(button_box, false, false, 0);
    
    // Set margins for main container
    set_margin_start(20);
    set_margin_end(20);
    set_margin_top(20);
    set_margin_bottom(20);
    
    add(main_box);
    
    // Load users
    load_users();
    show_all();
}

void NewChatRoomView::load_users() {
    try {
        all_users = db_handler.get_all_users_except(current_user->getUserId());
        filter_users("");
    } catch (const std::exception& e) {
        std::cerr << "Error loading users: " << e.what() << std::endl;
    }
}

// Filter users depending on search input
void NewChatRoomView::filter_users(const Glib::ustring& search_text) {
    // Clear existing rows
    auto children = user_list.get_children();
    for (auto* child : children) {
        user_list.remove(*child);
    }
    user_checkboxes.clear();
    
    // Add filtered users
    for (const auto& [user_id, username] : all_users) {
        if (search_text.empty() ||
            Glib::ustring(username).lowercase().find(search_text.lowercase()) != Glib::ustring::npos) {
            auto row = Gtk::manage(new Gtk::ListBoxRow());
            auto box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5));
            
            auto check = Gtk::manage(new Gtk::CheckButton());
            auto label = Gtk::manage(new Gtk::Label(username));
            label->set_halign(Gtk::ALIGN_START);
            
            // Store checkbox reference
            user_checkboxes[user_id] = check;
            
            // Connect checkbox signal
            check->signal_toggled().connect(
                sigc::bind(sigc::mem_fun(*this, &NewChatRoomView::on_checkbox_toggled),
                            check, user_id)
            );
            
            box->pack_start(*check, false, false, 5);
            box->pack_start(*label, true, true, 5);
            row->add(*box);
            
            user_list.append(*row);
        }
    }
    show_all();
}

void NewChatRoomView::on_search_changed() {
    filter_users(search_entry.get_text());
}

void NewChatRoomView::on_checkbox_toggled(Gtk::CheckButton* checkbox, const std::string& user_id) {
    update_selected_count();
}

void NewChatRoomView::update_selected_count() {
    int selected_count = 0;
    for (const auto& [user_id, checkbox] : user_checkboxes) {
        if (checkbox->get_active()) {
            selected_count++;
        }
    }
    selected_users_label.set_text("Selected users: " + std::to_string(selected_count));
    confirm_button.set_sensitive(selected_count > 0);
}

std::vector<std::string> NewChatRoomView::get_selected_user_ids() {
    std::vector<std::string> selected_ids;
    for (const auto& [user_id, checkbox] : user_checkboxes) {
        if (checkbox->get_active()) {
            selected_ids.push_back(user_id);
        }
    }
    return selected_ids;
}

void NewChatRoomView::on_confirm_clicked() {
    auto selected_ids = get_selected_user_ids();
    if (selected_ids.empty()) return;
    
    std::vector<std::string> user_ids;
    std::vector<std::string> usernames;
    
    // Add current user to the list
    user_ids.push_back(current_user->getUserId());
    
    // Add selected users
    for (const auto& user_id : selected_ids) {
        user_ids.push_back(user_id);
        usernames.push_back(all_users[user_id]); // Get username from all_users map
    }
    
    try {
        // Get room name or generate a default one
        std::string room_name = room_name_entry.get_text();
        if (room_name.empty()) {
            if (usernames.size() == 1) {
                room_name = "Chat with " + usernames[0];
            } else {
                room_name = "Group Chat (" + std::to_string(usernames.size() + 1) + " users)";
            }
        }
        
        // Create the chat room
        std::string room_id = db_handler.get_or_create_chat_room(user_ids, room_name);
        
        // Create and show chat room view
        auto chat_room = new ChatRoomView(db_handler, room_id, room_name);

        m_signal_back_to_chat_list_requested.emit();

    } catch (const std::exception& e) {
        // Show error
        std::cerr << "Error while creating new chat room: " << e.what() << std::endl;
    }
}

void NewChatRoomView::on_go_back_clicked(){ m_signal_back_to_chat_list_requested.emit();}