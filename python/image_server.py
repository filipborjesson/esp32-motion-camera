import socket
import os
import time

# Configuration
SAVE_DIR = "test_logs"
PORT = 8080

# Create directory if it doesn't exist
if not os.path.exists(SAVE_DIR):
    os.makedirs(SAVE_DIR)
    print(f"Created directory: {SAVE_DIR}")

# Create socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Reuse address to avoid "Address already in use" errors on restart
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

try:
    server_socket.bind(('0.0.0.0', PORT))
    server_socket.listen(5)
    print(f"--- Server Started ---")
    print(f"Listening on Port: {PORT}")
    print(f"Photos will save to: {os.path.abspath(SAVE_DIR)}")
    print("Waiting for ESP32-CAM...")
except Exception as e:
    print(f"Could not start server: {e}")
    exit()

while True:
    try:
        client_socket, addr = server_socket.accept()
        print(f"\nConnection from: {addr}")
        
        # Generate filename: test_logs/img_20240101_120000.jpg
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        filepath = os.path.join(SAVE_DIR, f"capture_{timestamp}.jpg")
        
        with open(filepath, 'wb') as f:
            while True:
                data = client_socket.recv(1024)
                if not data:
                    break
                f.write(data)
        
        print(f"Successfully saved: {filepath}")
        client_socket.close()
    except KeyboardInterrupt:
        print("\nStopping Server...")
        break
    except Exception as e:
        print(f"Error: {e}")

server_socket.close()