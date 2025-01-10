#!/bin/bash
# run.sh

# Allow access to the X server
xhost +SI:localuser:$(whoami)

# Build folder ..
rm -rf build
mkdir -p build
cd build

# Build the application
cmake ..
make

# Start PostgreSQL container if not running
cd ..  # Go back to root directory
docker compose up -d # Runs the container

# Wait for PostgreSQL to be ready
echo "Waiting for PostgreSQL to start..."
sleep 1

# Run the app
env -i DISPLAY=:0 LD_LIBRARY_PATH=/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu ./build/vaoApp

# Revoke access
xhost -SI:localuser:$(whoami)