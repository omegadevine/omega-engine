"""
Advanced Systems Example
Demonstrates Physics, Networking, and Scripting integration
"""

import pygame
import sys
from physics_system import PhysicsSystem, PhysicsBody
from network_system import NetworkSystem, WebSocketServer
from scripting_system import ScriptingSystem

class AdvancedGameDemo:
    """Demo showcasing advanced engine features"""
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((800, 600))
        pygame.display.set_caption("Omega Engine - Advanced Systems Demo")
        self.clock = pygame.time.Clock()
        self.running = True
        
        # Initialize systems
        self.physics = PhysicsSystem(gravity=(0, 980))  # Positive Y is down in pygame
        self.network = NetworkSystem()
        self.scripting = ScriptingSystem()
        
        # Enable physics debug draw
        self.physics.enable_debug_draw(self.screen)
        
        # Setup demo
        self.setup_physics_demo()
        self.setup_scripting_demo()
        self.setup_networking_demo()
    
    def setup_physics_demo(self):
        """Create physics objects"""
        # Ground
        self.ground = self.physics.create_static_body(
            'segment', (800, 0), (400, 550),
            friction=0.7
        )
        
        # Walls
        self.left_wall = self.physics.create_static_body(
            'segment', (0, 600), (50, 300),
            friction=0.5
        )
        self.right_wall = self.physics.create_static_body(
            'segment', (0, 600), (750, 300),
            friction=0.5
        )
        
        # Dynamic boxes
        self.boxes = []
        for i in range(5):
            box = self.physics.create_dynamic_body(
                'box', (40, 40), (200 + i * 100, 100),
                mass=1.0,
                friction=0.5,
                elasticity=0.3
            )
            self.boxes.append(box)
        
        # Dynamic circles
        self.circles = []
        for i in range(3):
            circle = self.physics.create_dynamic_body(
                'circle', (20, 20), (300 + i * 150, 50),
                mass=0.5,
                friction=0.3,
                elasticity=0.8
            )
            self.circles.append(circle)
        
        print("Physics demo setup complete!")
    
    def setup_scripting_demo(self):
        """Setup Lua scripting"""
        # Register game functions
        def log_message(msg):
            print(f"[LUA LOG] {msg}")
        
        def get_physics_body_count():
            return len(self.physics.bodies)
        
        self.scripting.register_function("log", log_message)
        self.scripting.register_function("get_body_count", get_physics_body_count)
        
        # Execute demo script
        demo_lua = """
            log("Lua scripting system initialized!")
            
            function on_game_start()
                log("Game started!")
                body_count = get_body_count()
                log("Total physics bodies: " .. body_count)
            end
            
            function calculate_score(kills, time)
                return kills * 100 - time * 0.5
            end
            
            game_state = {
                level = 1,
                score = 0,
                player_health = 100
            }
        """
        
        self.scripting.execute_string(demo_lua)
        self.scripting.call_global_function("on_game_start")
        
        print("Scripting demo setup complete!")
    
    def setup_networking_demo(self):
        """Setup networking (optional)"""
        # Uncomment to start a multiplayer server
        # self.server = self.network.start_websocket_server("localhost", 8765)
        # self.server.on_message(self.on_network_message)
        # print("WebSocket server started on ws://localhost:8765")
        
        print("Networking demo ready (server disabled by default)")
    
    def on_network_message(self, client, message):
        """Handle network messages"""
        print(f"Received: {message}")
        self.network.ws_server.send_to(client, f"Echo: {message}")
    
    def handle_events(self):
        """Handle pygame events"""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    self.running = False
                elif event.key == pygame.K_SPACE:
                    # Spawn new box on spacebar
                    import random
                    x = random.randint(100, 700)
                    box = self.physics.create_dynamic_body(
                        'box', (30, 30), (x, 50),
                        mass=1.0,
                        elasticity=0.5
                    )
                    self.boxes.append(box)
                elif event.key == pygame.K_r:
                    # Reset simulation
                    self.physics.clear()
                    self.boxes.clear()
                    self.circles.clear()
                    self.setup_physics_demo()
                elif event.key == pygame.K_c:
                    # Spawn circle
                    import random
                    x = random.randint(100, 700)
                    circle = self.physics.create_dynamic_body(
                        'circle', (25, 25), (x, 50),
                        mass=0.5,
                        elasticity=0.9
                    )
                    self.circles.append(circle)
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:  # Left click
                    # Apply impulse to nearby bodies
                    mouse_pos = pygame.mouse.get_pos()
                    for box in self.boxes:
                        dx = box.position[0] - mouse_pos[0]
                        dy = box.position[1] - mouse_pos[1]
                        dist = (dx**2 + dy**2)**0.5
                        if dist < 100 and dist > 0:
                            impulse = (-dx/dist * 500, -dy/dist * 500)
                            box.apply_impulse(impulse)
    
    def update(self, dt):
        """Update game logic"""
        self.physics.update(dt)
        
        # Call Lua update if needed
        score = self.scripting.call_global_function("calculate_score", 10, 60.5)
    
    def render(self):
        """Render game"""
        self.screen.fill((30, 30, 40))
        
        # Draw physics debug
        self.physics.draw_debug()
        
        # Draw UI
        font = pygame.font.Font(None, 24)
        texts = [
            "Omega Engine - Advanced Systems Demo",
            f"Physics Bodies: {len(self.physics.bodies)}",
            "Controls:",
            "  SPACE - Spawn Box",
            "  C - Spawn Circle",
            "  R - Reset",
            "  Left Click - Apply Force",
            "  ESC - Quit"
        ]
        
        for i, text in enumerate(texts):
            color = (255, 255, 255) if i > 0 else (100, 255, 100)
            surface = font.render(text, True, color)
            self.screen.blit(surface, (10, 10 + i * 25))
        
        pygame.display.flip()
    
    def run(self):
        """Main game loop"""
        print("\n" + "="*50)
        print("ADVANCED SYSTEMS DEMO")
        print("="*50)
        print("Physics: Enabled (PyMunk)")
        print("Networking: Ready (WebSocket)")
        print("Scripting: Enabled (Lua)")
        print("="*50 + "\n")
        
        while self.running:
            dt = self.clock.tick(60) / 1000.0
            
            self.handle_events()
            self.update(dt)
            self.render()
        
        self.cleanup()
    
    def cleanup(self):
        """Cleanup resources"""
        self.network.shutdown()
        pygame.quit()
        print("\nDemo ended. Thank you for testing Omega Engine!")

if __name__ == "__main__":
    try:
        demo = AdvancedGameDemo()
        demo.run()
    except Exception as e:
        print(f"Error running demo: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
