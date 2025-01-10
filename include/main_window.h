#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtkmm/window.h>
#include "database_handler.h" 
#include <gtkmm/stack.h>      
#include "login_view.h"       
#include "chat_list_view.h"   
#include "new_user_view.h"    
#include "user.h" 

class MainWindow : public Gtk::Window {
private:
    DatabaseHandler& db_handler;
    Gtk::Stack main_stack;
    
    // Views
    std::unique_ptr<LoginView> login_view;
    std::unique_ptr<ChatListView> chat_view;
    std::unique_ptr<NewUserView> new_user_view;
    std::unique_ptr<NewChatRoomView> new_chat_room_view;
    std::unique_ptr<ChatRoomView> chat_room_view;
    
    // Signals
    void on_open_chat_room(const std::string& room_id, const std::string& room_name);
    void on_login_success();
    void on_logout();
    void on_create_account_requested();
    void on_back_to_login();
    void on_create_new_chat_room();
    void on_back_to_chat_list();
    
public:
    MainWindow(DatabaseHandler& db);
};

#endif