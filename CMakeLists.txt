cmake_minimum_required(VERSION 3.16)

# CMakeLists to compile the project
project(vaoApp VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GTK and gtkmm packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(PQXX REQUIRED libpqxx)

# OpenSSL for hashing
find_package(OpenSSL REQUIRED)

# Add include directories
include_directories(
 ${CMAKE_SOURCE_DIR}/include
 ${GTKMM_INCLUDE_DIRS}
 ${PQXX_INCLUDE_DIRS}
)

# Link directory
link_directories(
 ${GTKMM_LIBRARY_DIRS}
 ${PQXX_LIBRARY_DIRS}
)

# Add executable
add_executable(vaoApp
 main.cpp
 src/main_window.cpp
 src/login_view.cpp
 src/chat_room_view.cpp
 src/message.cpp
 src/user.cpp
 src/new_user_view.cpp
 src/chat_list_view.cpp
 src/database_handler.cpp
 src/new_chat_room_view.cpp
)

# Add compiler options
add_definitions(${GTKMM_CFLAGS} ${GTKMM_CFLAGS_OTHER})

# Target link libraries
target_link_libraries(vaoApp
 PRIVATE OpenSSL::Crypto
 ${GTKMM_LIBRARIES}
 pqxx
 uuid
 ${PQXX_LIBRARIES}
)

# Print out some diagnostic information
message(STATUS "GTKMM_INCLUDE_DIRS: ${GTKMM_INCLUDE_DIRS}")
message(STATUS "GTKMM_LIBRARIES: ${GTKMM_LIBRARIES}")
message(STATUS "PQXX_INCLUDE_DIRS: ${PQXX_INCLUDE_DIRS}")
message(STATUS "PQXX_LIBRARIES: ${PQXX_LIBRARIES}")