#include "main_window.h"

MainWindow::MainWindow(DatabaseHandler& db) : db_handler(db) {
    set_title("vaoApp");
    set_border_width(10);
    set_default_size(400, 600);
    
    // Create login view 
    login_view = std::make_unique<LoginView>(db_handler);

    // To listen when login is being successful in LoginView, activates on_login_success to display user's ChatListView
    login_view->signal_login_success().connect(
        sigc::mem_fun(*this, &MainWindow::on_login_success)
    );
    
    // To listen if the user wishes to create an account
    login_view->signal_create_account().connect(
        sigc::mem_fun(*this, &MainWindow::on_create_account_requested)
    );
    
    // Add views to stack with transition
    main_stack.set_transition_type(Gtk::StackTransitionType::STACK_TRANSITION_TYPE_SLIDE_LEFT);
    main_stack.set_transition_duration(250); // 250ms transition
    main_stack.add(*login_view, "login");
    
    // Show login by default
    main_stack.set_visible_child("login");
    
    add(main_stack);
    show_all();
}

// On create new account clicked
void MainWindow::on_create_account_requested() {

    // Create new user view if it doesn't exist
    if (!new_user_view) {
        new_user_view = std::make_unique<NewUserView>(db_handler);
        main_stack.add(*new_user_view, "new-user");
        
        // Connect back to login view signal
        new_user_view->signal_back_to_login_requested().connect(
            sigc::mem_fun(*this, &MainWindow::on_back_to_login)
        );
    }
    
    // Show new user view with transition
    main_stack.set_transition_type(Gtk::StackTransitionType::STACK_TRANSITION_TYPE_SLIDE_LEFT);
    main_stack.set_visible_child("new-user");
}

void MainWindow::on_back_to_login() {
    // Show login view with reverse transition
    main_stack.set_transition_type(Gtk::StackTransitionType::STACK_TRANSITION_TYPE_SLIDE_RIGHT);
    main_stack.set_visible_child("login");
}

void MainWindow::on_login_success() {

    // Connect signals
    chat_view = std::make_unique<ChatListView>(db_handler);
    chat_view->signal_logout().connect(
        sigc::mem_fun(*this, &MainWindow::on_logout)
    );

    // For chat room opening
    chat_view->signal_open_chat_room().connect(
        sigc::mem_fun(*this, &MainWindow::on_open_chat_room)
    );

    // For chat room creation
    chat_view->signal_create_new_chat_room().connect(
        sigc::mem_fun(*this,&MainWindow::on_create_new_chat_room)
    );
    
    // Add chat view to stack and show it with transition
    main_stack.add(*chat_view, "chat");
    main_stack.set_visible_child("chat");
    
    // Hide other views
    login_view->hide();
    if (new_user_view) {
        new_user_view->hide();
    }
    
    set_title("Chats - " + db_handler.getCurrentUser().getUsername());
}

void MainWindow::on_logout() {
    // Show login view before transition
    login_view->show();
    
    // Switch back to login view with reverse transition
    main_stack.set_transition_type(Gtk::StackTransitionType::STACK_TRANSITION_TYPE_SLIDE_RIGHT);
    main_stack.set_visible_child("login");
    
    // Cleanup chat view
    chat_view.reset();
    
    set_title("vaoApp");
}

void MainWindow::on_create_new_chat_room(){
    // Create new chat_room_view if it doesn't exist
    if (!new_chat_room_view) {
        new_chat_room_view = std::make_unique<NewChatRoomView>(db_handler);
        main_stack.add(*new_chat_room_view, "new-chatroom");
        
        // Connect back to chatlist view signal
        new_chat_room_view->signal_back_to_chat_list_requested().connect(
            sigc::mem_fun(*this, &MainWindow::on_back_to_chat_list)
        );
    }
    
    // Show new_chat_room view with transition
    main_stack.set_transition_type(Gtk::StackTransitionType::STACK_TRANSITION_TYPE_SLIDE_LEFT);
    main_stack.set_visible_child("new-chatroom");
}

void MainWindow::on_back_to_chat_list() {
    // Show login view with reverse transition
    main_stack.set_transition_type(Gtk::StackTransitionType::STACK_TRANSITION_TYPE_SLIDE_RIGHT);
    main_stack.set_visible_child("chat");
}

void MainWindow::on_open_chat_room(const std::string& room_id, const std::string& room_name) {
    // Create new chat room view
    chat_room_view = std::make_unique<ChatRoomView>(db_handler, room_id, room_name);
    main_stack.add(*chat_room_view, "chat-room-" + room_name);
    
    // Show chat room view with transition
    main_stack.set_transition_type(Gtk::StackTransitionType::STACK_TRANSITION_TYPE_SLIDE_LEFT);
    main_stack.set_visible_child("chat-room-" + room_name);
    
    // Connect back to chat list signal
    chat_room_view->signal_back_to_chat_list_requested().connect([this]() {
        main_stack.set_transition_type(Gtk::StackTransitionType::STACK_TRANSITION_TYPE_SLIDE_RIGHT);
        main_stack.set_visible_child("chat");
    });
}