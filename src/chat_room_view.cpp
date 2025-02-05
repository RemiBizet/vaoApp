#include "chat_room_view.h"

// Constructor
ChatRoomView::ChatRoomView(DatabaseHandler& db_handler, const std::string& room_id, const std::string& room_name)
    : Gtk::Box(),
      db_handler(db_handler),
      room_id(room_id),
      room_name(room_name)
{
   // Initialize components
    main_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10);
    message_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10);
    input_box = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5);
    send_button = Gtk::Button("Send");
    go_back_button = Gtk::Button("Go Back");

    current_user = db_handler.getCurrentUser();

    set_halign(Gtk::ALIGN_CENTER); // Center horizontally
    set_valign(Gtk::ALIGN_CENTER); // Center vertically
    
    // Setup room label
    room_label.set_text(room_name);
    room_label.set_halign(Gtk::ALIGN_START);
    room_label.get_style_context()->add_class("title-2");
    room_label.set_margin_bottom(5);

    // Add users list label
    users_label = Gtk::manage(new Gtk::Label());
    try {
        auto room_users = db_handler.get_room_users(room_id);
        std::string users_text = "with ";
        for (size_t i = 0; i < room_users.size(); ++i) {
            users_text += room_users[i];
            if (i < room_users.size() - 2) users_text += ", ";
            else if (i == room_users.size() - 2) users_text += " and ";
        }
        users_label->set_text(users_text);
    } catch (const std::exception& e) {
        users_label->set_text("with unknown users");
    }
    users_label->set_halign(Gtk::ALIGN_START);
    users_label->get_style_context()->add_class("subtitle-1");
    users_label->set_margin_bottom(10);

    // Setup message area
    message_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    message_scroll.add(message_box);  
    message_scroll.set_size_request(600,500);
    
    // Setup input area
    message_entry.set_placeholder_text("Type a message...");
    message_entry.set_size_request(-1,40);
    input_box.set_margin_top(10);
    input_box.pack_start(message_entry, true, true, 0);
    input_box.pack_start(send_button, false, false, 0);
    input_box.pack_start(go_back_button, false, false, 0);

    // Connect signals
    send_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ChatRoomView::on_send_clicked)
    );

    go_back_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ChatRoomView::on_go_back_clicked)
    );

    // Pack widgets
    main_box.pack_start(room_label, false, false, 0);
    main_box.pack_start(*users_label, false, false, 0);
    main_box.pack_start(message_scroll, true, true, 0);
    main_box.pack_start(input_box, false, false, 0);
    
    add(main_box);
    show_all();

    set_margin_start(20);
    set_margin_end(20);
    set_margin_top(20);
    set_margin_bottom(20);

    // Load existing messages
    load_messages();

    // Set focus to message entry
    message_entry.grab_focus();
}

void ChatRoomView::on_go_back_clicked(){
    m_signal_back_to_chat_list_requested.emit();
}

void ChatRoomView::load_messages() {
    try {
        auto messages = db_handler.get_room_messages(room_id);
        last_message_sender_id = "";
        for (const auto& msg : messages) {
            bool is_from_current_user = (msg.sender_id == current_user->getUserId());
            add_message(msg.content, msg.sender_id,is_from_current_user);
        }
        scroll_to_bottom();
    } catch (const std::exception& e) {
        std::cerr << "Error loading messages: " << e.what() << std::endl;
    }
}

void ChatRoomView::on_send_clicked() {
    std::string message_text = message_entry.get_text();
    if (message_text.empty()) return;
    
    try {
        db_handler.send_message(room_id, current_user->getUserId(), message_text);
        add_message(message_text, current_user->getUserId(), true);
        message_entry.set_text("");
        scroll_to_bottom();
    } catch (const std::exception& e) {
        std::cerr << "Error sending message: " << e.what() << std::endl;
    }
}

// add message to the Scrolled Window
void ChatRoomView::add_message(const std::string& content, const std::string& sender_id, bool is_from_current_user) {
    auto message_container = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));

    // Add username label if sender has changed
    if (sender_id != last_message_sender_id) {

        auto username_label = Gtk::manage(new Gtk::Label());
        std::string username;
        try {
            username = db_handler.get_username_by_id(sender_id);
        } catch (const std::exception& e) {
            username = "Unknown User";
        }
        
        username_label->set_text(username);
        username_label->set_halign(is_from_current_user ? Gtk::ALIGN_END : Gtk::ALIGN_START);
        username_label->get_style_context()->add_class("title-4");
        username_label->set_margin_start(10);
        username_label->set_margin_end(10);
        username_label->set_margin_bottom(2);
            
        username_label->override_color(Gdk::RGBA("white"));
        message_container->pack_start(*username_label, false, false, 0);
    }

    auto message_box_horizontal = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));

    auto message_label = Gtk::manage(new Gtk::Label(content));
    message_label->set_line_wrap(true);
    message_label->set_line_wrap_mode(Pango::WRAP_WORD_CHAR);
    message_label->set_max_width_chars(50);
    
    auto font_desc = message_label->get_pango_context()->get_font_description();
    font_desc.set_size(13 * PANGO_SCALE);
    message_label->override_font(font_desc);

    auto message_frame = Gtk::manage(new Gtk::Frame());
    message_frame->add(*message_label);
    message_frame->set_margin_start(10);
    message_frame->set_margin_end(10);
    message_frame->set_margin_top(5);
    message_frame->set_margin_bottom(5);

    // Add CSS provider for rounded corners
    auto css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(
        "frame { border-radius: 15px; padding: 8px; }"
    );
    message_frame->get_style_context()->add_provider(
        css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    
    if (is_from_current_user) {
        message_box_horizontal->pack_end(*message_frame, false, false, 0);
        message_frame->override_background_color(Gdk::RGBA("purple"));
    } else {
        message_box_horizontal->pack_start(*message_frame, false, false, 0);
        message_frame->override_background_color(Gdk::RGBA("darkorange"));
    }
    
    message_container->pack_start(*message_box_horizontal,false,false,0);
    message_box.pack_start(*message_container, false, false, 0);
    message_container->show_all();

    last_message_sender_id = sender_id;
}

void ChatRoomView::scroll_to_bottom() {
    auto adjustment = message_scroll.get_vadjustment();
    adjustment->set_value(adjustment->get_upper());
}