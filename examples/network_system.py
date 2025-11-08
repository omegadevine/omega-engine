"""
Network System for multiplayer and online features
Supports HTTP requests, WebSocket connections, and simple client-server architecture
"""

import asyncio
import json
import threading
from typing import Dict, Callable, Optional, Any, List
import requests
from websockets.asyncio.client import connect as ws_connect
from websockets.asyncio.server import serve as ws_serve
import websockets.exceptions

class HTTPClient:
    """Simple HTTP client for REST API calls"""
    def __init__(self, base_url: str = "", timeout: int = 30):
        self.base_url = base_url
        self.timeout = timeout
        self.headers = {'Content-Type': 'application/json'}
        self.session = requests.Session()
    
    def set_header(self, key: str, value: str):
        """Set a custom header"""
        self.headers[key] = value
    
    def get(self, endpoint: str, params: Optional[Dict] = None) -> requests.Response:
        """HTTP GET request"""
        url = f"{self.base_url}{endpoint}"
        return self.session.get(url, params=params, headers=self.headers, timeout=self.timeout)
    
    def post(self, endpoint: str, data: Optional[Dict] = None) -> requests.Response:
        """HTTP POST request"""
        url = f"{self.base_url}{endpoint}"
        return self.session.post(url, json=data, headers=self.headers, timeout=self.timeout)
    
    def put(self, endpoint: str, data: Optional[Dict] = None) -> requests.Response:
        """HTTP PUT request"""
        url = f"{self.base_url}{endpoint}"
        return self.session.put(url, json=data, headers=self.headers, timeout=self.timeout)
    
    def delete(self, endpoint: str) -> requests.Response:
        """HTTP DELETE request"""
        url = f"{self.base_url}{endpoint}"
        return self.session.delete(url, headers=self.headers, timeout=self.timeout)
    
    def download_file(self, url: str, filepath: str) -> bool:
        """Download a file from URL"""
        try:
            response = self.session.get(url, stream=True, timeout=self.timeout)
            response.raise_for_status()
            with open(filepath, 'wb') as f:
                for chunk in response.iter_content(chunk_size=8192):
                    f.write(chunk)
            return True
        except Exception as e:
            print(f"Download failed: {e}")
            return False

class WebSocketClient:
    """WebSocket client for real-time bidirectional communication"""
    def __init__(self, uri: str):
        self.uri = uri
        self.websocket = None
        self.connected = False
        self.running = False
        self.message_handlers: List[Callable] = []
        self.loop = None
        self.thread = None
    
    def on_message(self, handler: Callable[[str], None]):
        """Register a message handler callback"""
        self.message_handlers.append(handler)
    
    async def _connect(self):
        """Internal async connect method"""
        try:
            self.websocket = await ws_connect(self.uri)
            self.connected = True
            print(f"Connected to {self.uri}")
            
            while self.running:
                try:
                    message = await asyncio.wait_for(self.websocket.recv(), timeout=0.1)
                    for handler in self.message_handlers:
                        handler(message)
                except asyncio.TimeoutError:
                    continue
                except websockets.exceptions.ConnectionClosed:
                    print("Connection closed")
                    self.connected = False
                    break
        except Exception as e:
            print(f"WebSocket error: {e}")
            self.connected = False
    
    def connect(self):
        """Connect to WebSocket server in background thread"""
        self.running = True
        self.thread = threading.Thread(target=self._run_async_connect, daemon=True)
        self.thread.start()
    
    def _run_async_connect(self):
        """Run async connection in thread"""
        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)
        self.loop.run_until_complete(self._connect())
    
    def send(self, message: str):
        """Send message to server"""
        if self.connected and self.websocket:
            asyncio.run_coroutine_threadsafe(self.websocket.send(message), self.loop)
    
    def send_json(self, data: Dict):
        """Send JSON data to server"""
        self.send(json.dumps(data))
    
    def disconnect(self):
        """Disconnect from server"""
        self.running = False
        if self.websocket:
            asyncio.run_coroutine_threadsafe(self.websocket.close(), self.loop)
        self.connected = False

class WebSocketServer:
    """WebSocket server for hosting multiplayer games"""
    def __init__(self, host: str = "localhost", port: int = 8765):
        self.host = host
        self.port = port
        self.clients = set()
        self.running = False
        self.message_handlers: List[Callable] = []
        self.connect_handlers: List[Callable] = []
        self.disconnect_handlers: List[Callable] = []
        self.loop = None
        self.thread = None
    
    def on_message(self, handler: Callable[[Any, str], None]):
        """Register message handler (receives client and message)"""
        self.message_handlers.append(handler)
    
    def on_connect(self, handler: Callable[[Any], None]):
        """Register client connect handler"""
        self.connect_handlers.append(handler)
    
    def on_disconnect(self, handler: Callable[[Any], None]):
        """Register client disconnect handler"""
        self.disconnect_handlers.append(handler)
    
    async def _handle_client(self, websocket):
        """Handle individual client connection"""
        self.clients.add(websocket)
        print(f"Client connected: {websocket.remote_address}")
        
        for handler in self.connect_handlers:
            handler(websocket)
        
        try:
            async for message in websocket:
                for handler in self.message_handlers:
                    handler(websocket, message)
        except websockets.exceptions.ConnectionClosed:
            pass
        finally:
            self.clients.remove(websocket)
            for handler in self.disconnect_handlers:
                handler(websocket)
            print(f"Client disconnected: {websocket.remote_address}")
    
    async def _start_server(self):
        """Internal async server start"""
        async with ws_serve(self._handle_client, self.host, self.port):
            print(f"WebSocket server running on ws://{self.host}:{self.port}")
            while self.running:
                await asyncio.sleep(0.1)
    
    def start(self):
        """Start server in background thread"""
        self.running = True
        self.thread = threading.Thread(target=self._run_async_server, daemon=True)
        self.thread.start()
    
    def _run_async_server(self):
        """Run async server in thread"""
        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)
        self.loop.run_until_complete(self._start_server())
    
    def broadcast(self, message: str, exclude: Optional[Any] = None):
        """Broadcast message to all connected clients"""
        if self.loop:
            asyncio.run_coroutine_threadsafe(
                self._async_broadcast(message, exclude), self.loop
            )
    
    async def _async_broadcast(self, message: str, exclude: Optional[Any] = None):
        """Internal async broadcast"""
        for client in self.clients:
            if client != exclude:
                try:
                    await client.send(message)
                except:
                    pass
    
    def broadcast_json(self, data: Dict, exclude: Optional[Any] = None):
        """Broadcast JSON data to all clients"""
        self.broadcast(json.dumps(data), exclude)
    
    def send_to(self, client: Any, message: str):
        """Send message to specific client"""
        if self.loop and client in self.clients:
            asyncio.run_coroutine_threadsafe(client.send(message), self.loop)
    
    def stop(self):
        """Stop server"""
        self.running = False

class NetworkSystem:
    """Main network system coordinating HTTP and WebSocket functionality"""
    def __init__(self):
        self.http_client = HTTPClient()
        self.ws_client: Optional[WebSocketClient] = None
        self.ws_server: Optional[WebSocketServer] = None
    
    def set_http_base_url(self, url: str):
        """Set base URL for HTTP requests"""
        self.http_client.base_url = url
    
    def connect_websocket(self, uri: str) -> WebSocketClient:
        """Create and connect WebSocket client"""
        self.ws_client = WebSocketClient(uri)
        self.ws_client.connect()
        return self.ws_client
    
    def start_websocket_server(self, host: str = "localhost", port: int = 8765) -> WebSocketServer:
        """Create and start WebSocket server"""
        self.ws_server = WebSocketServer(host, port)
        self.ws_server.start()
        return self.ws_server
    
    def shutdown(self):
        """Shutdown all network connections"""
        if self.ws_client:
            self.ws_client.disconnect()
        if self.ws_server:
            self.ws_server.stop()

# Example usage
if __name__ == "__main__":
    import time
    
    # HTTP example
    http = HTTPClient("https://api.github.com")
    response = http.get("/")
    print(f"HTTP Status: {response.status_code}")
    
    # WebSocket server example
    server = WebSocketServer("localhost", 8765)
    
    def on_server_message(client, message):
        print(f"Server received: {message}")
        server.send_to(client, f"Echo: {message}")
    
    server.on_message(on_server_message)
    server.start()
    
    time.sleep(1)
    
    # WebSocket client example
    client = WebSocketClient("ws://localhost:8765")
    
    def on_client_message(message):
        print(f"Client received: {message}")
    
    client.on_message(on_client_message)
    client.connect()
    
    time.sleep(1)
    client.send("Hello Server!")
    
    time.sleep(2)
    
    client.disconnect()
    server.stop()
