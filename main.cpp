#include <gtkmm/application.h>
#include "main_window.h"
#include "database_handler.h"

int main(int argc, char* argv[]) {

    // App and credentials creation
    auto app = Gtk::Application::create(argc, argv, "org.vaoapp");
    std::string conn_str = "host=localhost port=5432 dbname=vaodb user=vaoapp_user password=vaoapp_user_password";

    // Start the window and database connection
    DatabaseHandler db_handler(conn_str);
    MainWindow window(db_handler);
    
    return app->run(window);
}