"""
Physics System using PyMunk (Chipmunk2D physics engine)
Provides 2D physics simulation with bodies, shapes, and collision detection
"""

import pymunk
from typing import List, Tuple, Optional, Callable

# Optional pygame support for debug drawing
try:
    import pymunk.pygame_util
    PYGAME_AVAILABLE = True
except ImportError:
    PYGAME_AVAILABLE = False

class PhysicsBody:
    """Wrapper for pymunk bodies with game-friendly interface"""
    def __init__(self, body: pymunk.Body, shape: pymunk.Shape):
        self.body = body
        self.shape = shape
        self.user_data = {}
    
    @property
    def position(self) -> Tuple[float, float]:
        return self.body.position.x, self.body.position.y
    
    @position.setter
    def position(self, pos: Tuple[float, float]):
        self.body.position = pos
    
    @property
    def velocity(self) -> Tuple[float, float]:
        return self.body.velocity.x, self.body.velocity.y
    
    @velocity.setter
    def velocity(self, vel: Tuple[float, float]):
        self.body.velocity = vel
    
    @property
    def angle(self) -> float:
        return self.body.angle
    
    @angle.setter
    def angle(self, angle: float):
        self.body.angle = angle
    
    def apply_force(self, force: Tuple[float, float], local_point: Tuple[float, float] = (0, 0)):
        self.body.apply_force_at_local_point(force, local_point)
    
    def apply_impulse(self, impulse: Tuple[float, float], local_point: Tuple[float, float] = (0, 0)):
        self.body.apply_impulse_at_local_point(impulse, local_point)

class PhysicsSystem:
    """Main physics system managing the physics world"""
    def __init__(self, gravity: Tuple[float, float] = (0, -980)):
        self.space = pymunk.Space()
        self.space.gravity = gravity
        self.bodies: List[PhysicsBody] = []
        self.collision_handlers = {}
        self.debug_draw = None
    
    def set_gravity(self, gravity: Tuple[float, float]):
        """Set world gravity (x, y)"""
        self.space.gravity = gravity
    
    def create_static_body(self, shape_type: str, size: Tuple[float, float], 
                          position: Tuple[float, float], **kwargs) -> PhysicsBody:
        """Create a static (non-moving) physics body"""
        body = pymunk.Body(body_type=pymunk.Body.STATIC)
        body.position = position
        
        shape = self._create_shape(body, shape_type, size, **kwargs)
        self.space.add(body, shape)
        
        physics_body = PhysicsBody(body, shape)
        self.bodies.append(physics_body)
        return physics_body
    
    def create_dynamic_body(self, shape_type: str, size: Tuple[float, float], 
                           position: Tuple[float, float], mass: float = 1.0, **kwargs) -> PhysicsBody:
        """Create a dynamic (moving) physics body"""
        if shape_type == 'box':
            moment = pymunk.moment_for_box(mass, size)
        elif shape_type == 'circle':
            moment = pymunk.moment_for_circle(mass, 0, size[0])
        else:
            moment = 1000
        
        body = pymunk.Body(mass, moment)
        body.position = position
        
        shape = self._create_shape(body, shape_type, size, **kwargs)
        self.space.add(body, shape)
        
        physics_body = PhysicsBody(body, shape)
        self.bodies.append(physics_body)
        return physics_body
    
    def create_kinematic_body(self, shape_type: str, size: Tuple[float, float],
                             position: Tuple[float, float], **kwargs) -> PhysicsBody:
        """Create a kinematic (controlled) physics body"""
        body = pymunk.Body(body_type=pymunk.Body.KINEMATIC)
        body.position = position
        
        shape = self._create_shape(body, shape_type, size, **kwargs)
        self.space.add(body, shape)
        
        physics_body = PhysicsBody(body, shape)
        self.bodies.append(physics_body)
        return physics_body
    
    def _create_shape(self, body: pymunk.Body, shape_type: str, 
                     size: Tuple[float, float], **kwargs) -> pymunk.Shape:
        """Create a collision shape"""
        if shape_type == 'box':
            shape = pymunk.Poly.create_box(body, size)
        elif shape_type == 'circle':
            shape = pymunk.Circle(body, size[0])
        elif shape_type == 'segment':
            shape = pymunk.Segment(body, (0, 0), size, kwargs.get('radius', 1))
        else:
            raise ValueError(f"Unknown shape type: {shape_type}")
        
        shape.friction = kwargs.get('friction', 0.5)
        shape.elasticity = kwargs.get('elasticity', 0.5)
        shape.collision_type = kwargs.get('collision_type', 0)
        
        return shape
    
    def remove_body(self, physics_body: PhysicsBody):
        """Remove a body from the simulation"""
        self.space.remove(physics_body.body, physics_body.shape)
        if physics_body in self.bodies:
            self.bodies.remove(physics_body)
    
    def add_collision_handler(self, type_a: int, type_b: int, 
                             begin: Optional[Callable] = None,
                             pre_solve: Optional[Callable] = None,
                             post_solve: Optional[Callable] = None,
                             separate: Optional[Callable] = None):
        """Add collision callbacks between two collision types"""
        handler = self.space.add_collision_handler(type_a, type_b)
        
        if begin:
            handler.begin = begin
        if pre_solve:
            handler.pre_solve = pre_solve
        if post_solve:
            handler.post_solve = post_solve
        if separate:
            handler.separate = separate
        
        self.collision_handlers[(type_a, type_b)] = handler
    
    def raycast(self, start: Tuple[float, float], end: Tuple[float, float], 
                radius: float = 0) -> Optional[pymunk.SegmentQueryInfo]:
        """Perform a raycast and return first hit"""
        query_info = self.space.segment_query_first(start, end, radius, 
                                                     pymunk.ShapeFilter())
        return query_info if query_info else None
    
    def query_point(self, point: Tuple[float, float], 
                   max_distance: float = 0) -> List[pymunk.Shape]:
        """Query all shapes at a point"""
        shapes = self.space.point_query(point, max_distance, pymunk.ShapeFilter())
        return [info.shape for info in shapes]
    
    def query_bb(self, bb: pymunk.BB) -> List[pymunk.Shape]:
        """Query all shapes in a bounding box"""
        shapes = self.space.bb_query(bb, pymunk.ShapeFilter())
        return [info.shape for info in shapes]
    
    def update(self, dt: float, substeps: int = 1):
        """Update physics simulation"""
        step_dt = dt / substeps
        for _ in range(substeps):
            self.space.step(step_dt)
    
    def enable_debug_draw(self, screen):
        """Enable visual debug drawing for pygame"""
        if PYGAME_AVAILABLE:
            try:
                self.debug_draw = pymunk.pygame_util.DrawOptions(screen)
            except:
                pass
        else:
            print("Warning: pygame not available, debug drawing disabled")
    
    def draw_debug(self):
        """Draw physics debug visualization"""
        if self.debug_draw:
            self.space.debug_draw(self.debug_draw)
    
    def clear(self):
        """Remove all bodies from the simulation"""
        for body in list(self.bodies):
            self.remove_body(body)
        self.bodies.clear()

# Example usage
if __name__ == "__main__":
    physics = PhysicsSystem(gravity=(0, -980))
    
    # Create ground
    ground = physics.create_static_body('segment', (800, 0), (400, 50))
    
    # Create dynamic box
    box = physics.create_dynamic_body('box', (50, 50), (400, 400), mass=1.0)
    box.apply_impulse((100, 0))
    
    # Simulate
    for _ in range(100):
        physics.update(1/60)
        print(f"Box position: {box.position}, velocity: {box.velocity}")
