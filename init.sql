-- init.sql

-- As super user during setup, create the vaoapp user
CREATE USER vaoapp_user WITH PASSWORD 'vaoapp_user_password';

-- Create the database
CREATE DATABASE vaodb;
\c vaodb

-- Create the users table
CREATE TABLE users (
    user_id VARCHAR(36) PRIMARY KEY,            -- UUID string
    username VARCHAR(50) UNIQUE NOT NULL,       -- Username
    password_hash VARCHAR(255) NOT NULL         -- Hashed Password
);

-- Create the chat_rooms table
CREATE TABLE chat_rooms (
    room_id VARCHAR(36) PRIMARY KEY,            -- UUID string
    room_name VARCHAR(101) NOT NULL,            -- Name of the chat room
    created_at TIMESTAMP DEFAULT NOW()          -- Timestamp of room creation
);

-- Create the messages table
CREATE TABLE messages (
    message_id VARCHAR(36) PRIMARY KEY,         -- UUID string
    content TEXT NOT NULL,                      -- Message content
    sender_id VARCHAR(36) NOT NULL,             -- Reference to the user who sent the message
    room_id VARCHAR(36) NOT NULL,               -- Reference to the chat room where the message was sent
    timestamp TIMESTAMP DEFAULT NOW(),          -- Timestamp when the message was sent
    is_read BOOLEAN DEFAULT FALSE,              -- Read status of the message
    FOREIGN KEY (sender_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (room_id) REFERENCES chat_rooms(room_id) ON DELETE CASCADE
);

-- Create the chat_room_members table to manage the many-to-many relationship
CREATE TABLE chat_room_members (
    room_id VARCHAR(36) NOT NULL,               -- Reference to chat room
    user_id VARCHAR(36) NOT NULL,               -- Reference to user
    PRIMARY KEY (room_id, user_id),             -- Composite primary key
    FOREIGN KEY (room_id) REFERENCES chat_rooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

-- Grant rights to the vaoapp_user
GRANT CONNECT ON DATABASE vaodb TO vaoapp_user;
GRANT USAGE ON SCHEMA public TO vaoapp_user;
GRANT SELECT, INSERT, UPDATE ON users TO vaoapp_user;
GRANT SELECT, INSERT, UPDATE ON messages TO vaoapp_user;
GRANT SELECT, INSERT, UPDATE ON chat_rooms TO vaoapp_user;
GRANT SELECT, INSERT, UPDATE ON chat_room_members TO vaoapp_user;