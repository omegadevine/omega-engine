"""
Network Multiplayer Example
Simple multiplayer game demo with WebSocket server/client
"""

from network_system import NetworkSystem, WebSocketServer, WebSocketClient
import json
import time
import threading

class MultiplayerServer:
    """Simple multiplayer game server"""
    def __init__(self, host="localhost", port=8765):
        self.network = NetworkSystem()
        self.server = self.network.start_websocket_server(host, port)
        
        self.players = {}
        self.game_state = {
            "players": {},
            "time": 0
        }
        
        # Setup handlers
        self.server.on_connect(self.on_player_connect)
        self.server.on_disconnect(self.on_player_disconnect)
        self.server.on_message(self.on_player_message)
        
        print(f"Multiplayer server running on ws://{host}:{port}")
    
    def on_player_connect(self, client):
        """Handle new player connection"""
        player_id = id(client)
        self.players[player_id] = {
            "id": player_id,
            "x": 400,
            "y": 300,
            "name": f"Player{len(self.players)}"
        }
        
        self.game_state["players"][player_id] = self.players[player_id]
        
        # Send welcome message
        welcome = {
            "type": "welcome",
            "player_id": player_id,
            "game_state": self.game_state
        }
        self.server.send_to(client, json.dumps(welcome))
        
        # Broadcast new player to others
        broadcast_msg = {
            "type": "player_joined",
            "player": self.players[player_id]
        }
        self.server.broadcast_json(broadcast_msg, exclude=client)
        
        print(f"Player {player_id} connected")
    
    def on_player_disconnect(self, client):
        """Handle player disconnect"""
        player_id = id(client)
        if player_id in self.players:
            del self.players[player_id]
            del self.game_state["players"][player_id]
            
            # Broadcast player left
            broadcast_msg = {
                "type": "player_left",
                "player_id": player_id
            }
            self.server.broadcast_json(broadcast_msg)
            
            print(f"Player {player_id} disconnected")
    
    def on_player_message(self, client, message):
        """Handle player messages"""
        try:
            data = json.loads(message)
            msg_type = data.get("type")
            player_id = id(client)
            
            if msg_type == "move":
                # Update player position
                if player_id in self.players:
                    self.players[player_id]["x"] = data["x"]
                    self.players[player_id]["y"] = data["y"]
                    
                    # Broadcast to other players
                    broadcast_msg = {
                        "type": "player_moved",
                        "player_id": player_id,
                        "x": data["x"],
                        "y": data["y"]
                    }
                    self.server.broadcast_json(broadcast_msg, exclude=client)
            
            elif msg_type == "chat":
                # Broadcast chat message
                broadcast_msg = {
                    "type": "chat",
                    "player_id": player_id,
                    "message": data["message"]
                }
                self.server.broadcast_json(broadcast_msg)
                print(f"Player {player_id}: {data['message']}")
        
        except Exception as e:
            print(f"Error processing message: {e}")
    
    def update(self):
        """Update game state"""
        self.game_state["time"] += 1
        
        # Send periodic state updates
        if self.game_state["time"] % 60 == 0:
            state_msg = {
                "type": "state_update",
                "game_state": self.game_state
            }
            self.server.broadcast_json(state_msg)
    
    def run(self):
        """Run server loop"""
        try:
            while True:
                time.sleep(0.016)  # ~60 FPS
                self.update()
        except KeyboardInterrupt:
            print("\nShutting down server...")
            self.network.shutdown()

class MultiplayerClient:
    """Simple multiplayer game client"""
    def __init__(self, uri="ws://localhost:8765"):
        self.network = NetworkSystem()
        self.client = self.network.connect_websocket(uri)
        
        self.player_id = None
        self.players = {}
        
        self.client.on_message(self.on_message)
        
        print(f"Connecting to {uri}...")
        time.sleep(1)  # Wait for connection
    
    def on_message(self, message):
        """Handle server messages"""
        try:
            data = json.loads(message)
            msg_type = data.get("type")
            
            if msg_type == "welcome":
                self.player_id = data["player_id"]
                self.players = data["game_state"]["players"]
                print(f"Connected! Your ID: {self.player_id}")
            
            elif msg_type == "player_joined":
                player = data["player"]
                self.players[player["id"]] = player
                print(f"Player {player['name']} joined")
            
            elif msg_type == "player_left":
                player_id = data["player_id"]
                if player_id in self.players:
                    print(f"Player {self.players[player_id]['name']} left")
                    del self.players[player_id]
            
            elif msg_type == "player_moved":
                player_id = data["player_id"]
                if player_id in self.players:
                    self.players[player_id]["x"] = data["x"]
                    self.players[player_id]["y"] = data["y"]
            
            elif msg_type == "chat":
                player_id = data["player_id"]
                print(f"Player {player_id}: {data['message']}")
        
        except Exception as e:
            print(f"Error processing message: {e}")
    
    def send_move(self, x, y):
        """Send movement to server"""
        msg = {
            "type": "move",
            "x": x,
            "y": y
        }
        self.client.send_json(msg)
    
    def send_chat(self, message):
        """Send chat message to server"""
        msg = {
            "type": "chat",
            "message": message
        }
        self.client.send_json(msg)
    
    def disconnect(self):
        """Disconnect from server"""
        self.client.disconnect()

# Example usage
if __name__ == "__main__":
    import sys
    
    if len(sys.argv) > 1 and sys.argv[1] == "server":
        # Run as server
        server = MultiplayerServer("localhost", 8765)
        server.run()
    else:
        # Run as client
        print("Multiplayer Client Demo")
        print("Starting in 2 seconds... (Start server first!)")
        time.sleep(2)
        
        client = MultiplayerClient("ws://localhost:8765")
        time.sleep(1)
        
        # Send some test movements
        client.send_chat("Hello from client!")
        time.sleep(0.5)
        
        for i in range(5):
            client.send_move(400 + i * 10, 300 + i * 5)
            time.sleep(0.5)
        
        client.send_chat("Goodbye!")
        time.sleep(1)
        
        client.disconnect()
        print("Client disconnected")
