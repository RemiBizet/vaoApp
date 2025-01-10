#include "chat_list_view.h"

// Constructor 
ChatListView::ChatListView(DatabaseHandler& db)
    : db_handler(db) {

    // Initialize components
    main_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5);

    current_user = db_handler.getCurrentUser();

    set_halign(Gtk::ALIGN_CENTER); // Center horizontally
    set_valign(Gtk::ALIGN_CENTER); // Center vertically
      
    // Setup scrolled view for chat list
    scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    scroll.add(chat_list);
    
    // Connect the row-activated signal
    chat_list.signal_row_activated().connect(
        sigc::mem_fun(*this, &ChatListView::on_chat_row_activated)
    );

    // Double-click with left button event
    chat_list.add_events(Gdk::BUTTON_PRESS_MASK);
    chat_list.signal_button_press_event().connect(
        sigc::mem_fun(*this, &ChatListView::on_button_press_event),
        false
    );

    // Setup new chat button
    new_chat_button.set_label("New Chat");
    new_chat_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ChatListView::on_new_chat_room_clicked)
    );

    // Setup logout button
    logout_button.set_label("Logout");
    logout_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ChatListView::on_logout_clicked)
    );
    
    main_box.pack_start(scroll, true, true, 0);
    main_box.pack_start(new_chat_button, false, false, 0);
    main_box.pack_start(logout_button,false,false,0);
    add(main_box);

    load_conversations();
    show_all();

}

bool ChatListView::on_button_press_event(GdkEventButton* event) {
    if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {  // Double-click with left button
        // Get the row at the clicked position
        Gtk::ListBoxRow* row = chat_list.get_row_at_y(event->y);
        if (row) {
            on_chat_row_activated(row);
            return true;  // Event handled
        }
    }
    return false;  // Event not handled
}

void ChatListView::on_chat_row_activated(Gtk::ListBoxRow* row) {
    if (!row) return;
    
    // Get the room data from the row
    auto room_id_ptr = static_cast<std::string*>(row->get_data("room_id"));
    auto room_name_ptr = static_cast<std::string*>(row->get_data("room_name"));
    
    // Send signal to open chat room with room_id and room_name data
    if (room_id_ptr && room_name_ptr) {
        m_signal_open_chat_room.emit(*room_id_ptr, *room_name_ptr);
    }
}

void ChatListView::on_new_chat_room_clicked() {
    m_signal_create_new_chat_room.emit();
}

void ChatListView::load_conversations() {
    try {
        // Clear existing rows
        auto children = chat_list.get_children();
        for (auto* child : children) {
            // Clean up stored data
            auto room_id_ptr = static_cast<int*>(child->get_data("room_id"));
            auto room_name_ptr = static_cast<std::string*>(child->get_data("room_name"));
            delete room_id_ptr;
            delete room_name_ptr;
            chat_list.remove(*child);
        }

        // Load conversations from database
        auto conversations = db_handler.get_user_conversations(current_user->getUserId());
        for (const auto& [room_id, room_name] : conversations) {

            auto row = Gtk::manage(new Gtk::ListBoxRow());
            auto box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5));

            auto label = Gtk::manage(new Gtk::Label(room_name));
            label->set_halign(Gtk::ALIGN_START);
            
            // Pack widgets
            box->pack_start(*label, true, true, 5);
            row->add(*box);
            
            row->set_data("room_id", new std::string(room_id));
            row->set_data("room_name", new std::string(room_name));
            
            row->show_all();
            chat_list.append(*row);
        }
        
        show_all();

    } catch (const std::exception& e) {
        // Show error
    }
}

void ChatListView::on_logout_clicked(){
    m_signal_logout.emit();
}