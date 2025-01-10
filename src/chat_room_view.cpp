#include "chat_room_view.h"

// Constructor
ChatRoomView::ChatRoomView(DatabaseHandler& db_handler, const std::string& room_id, const std::string& room_name)
    : Gtk::Box(),
      db_handler(db_handler),
      room_id(room_id),
      room_name(room_name)
{
   // Initialize components
    main_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5);
    message_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5);
    input_box = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5);
    send_button = Gtk::Button("Send");
    go_back_button = Gtk::Button("Go Back");


    current_user = db_handler.getCurrentUser();

    set_halign(Gtk::ALIGN_CENTER); // Center horizontally
    set_valign(Gtk::ALIGN_CENTER); // Center vertically
    
    // Setup room label
    room_label.set_text(room_name);
    room_label.set_halign(Gtk::ALIGN_START);
    room_label.get_style_context()->add_class("title-3");

    // Setup message area
    message_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    message_scroll.add(message_box);  
    
    // Setup input area
    message_entry.set_placeholder_text("Type a message...");
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
    main_box.pack_start(message_scroll, true, true, 0);
    main_box.pack_start(input_box, false, false, 0);
    
    add(main_box);
    show_all();

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
        for (const auto& msg : messages) {
            bool is_from_current_user = (msg.sender_id == current_user->getUserId());
            add_message(msg.content, is_from_current_user);
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
        add_message(message_text, true);
        message_entry.set_text("");
        scroll_to_bottom();
    } catch (const std::exception& e) {
        std::cerr << "Error sending message: " << e.what() << std::endl;
    }
}

// add message to the Scrolled Window
void ChatRoomView::add_message(const std::string& content, bool is_from_current_user) {
    auto message_container = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
    auto message_label = Gtk::manage(new Gtk::Label(content));
    message_label->set_line_wrap(true);
    message_label->set_line_wrap_mode(Pango::WRAP_WORD_CHAR);
    message_label->set_max_width_chars(40);
    
    auto message_frame = Gtk::manage(new Gtk::Frame());
    message_frame->add(*message_label);
    message_frame->set_margin_start(5);
    message_frame->set_margin_end(6);
    
    if (is_from_current_user) {
        message_container->pack_end(*message_frame, false, false, 0);
        message_frame->override_background_color(Gdk::RGBA("lightblue"));
    } else {
        message_container->pack_start(*message_frame, false, false, 0);
        message_frame->override_background_color(Gdk::RGBA("lightgrey"));
    }
    
    message_box.pack_start(*message_container, false, false, 0);
    message_container->show_all();
}

void ChatRoomView::scroll_to_bottom() {
    auto adjustment = message_scroll.get_vadjustment();
    adjustment->set_value(adjustment->get_upper());
}