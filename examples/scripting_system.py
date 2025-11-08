"""
Scripting System using Lua (via Lupa)
Allows game logic to be written in Lua scripts for flexibility and modding support
"""

import lupa
from lupa import LuaRuntime
from typing import Dict, Any, Optional, Callable
import os

class LuaScript:
    """Wrapper for individual Lua script instances"""
    def __init__(self, lua: LuaRuntime, filepath: str):
        self.lua = lua
        self.filepath = filepath
        self.globals = lua.globals()
        self.loaded = False
        self._load_script()
    
    def _load_script(self):
        """Load and execute Lua script"""
        try:
            with open(self.filepath, 'r') as f:
                script_content = f.read()
            self.lua.execute(script_content)
            self.loaded = True
        except Exception as e:
            print(f"Error loading Lua script {self.filepath}: {e}")
            self.loaded = False
    
    def reload(self):
        """Reload script from file"""
        self._load_script()
    
    def call_function(self, func_name: str, *args):
        """Call a Lua function by name"""
        if not self.loaded:
            return None
        
        try:
            func = self.globals[func_name]
            if func:
                return func(*args)
        except Exception as e:
            print(f"Error calling Lua function '{func_name}': {e}")
        return None
    
    def get_variable(self, var_name: str) -> Any:
        """Get a Lua global variable"""
        try:
            return self.globals[var_name]
        except:
            return None
    
    def set_variable(self, var_name: str, value: Any):
        """Set a Lua global variable"""
        try:
            self.globals[var_name] = value
        except Exception as e:
            print(f"Error setting Lua variable '{var_name}': {e}")
    
    def get_table(self, table_name: str) -> Optional[Dict]:
        """Get a Lua table as Python dict"""
        try:
            lua_table = self.globals[table_name]
            if lua_table:
                return dict(lua_table)
        except:
            pass
        return None

class ScriptingSystem:
    """Main scripting system managing Lua runtime and scripts"""
    def __init__(self):
        self.lua = LuaRuntime(unpack_returned_tuples=True)
        self.scripts: Dict[str, LuaScript] = {}
        self.global_functions: Dict[str, Callable] = {}
        self._register_base_functions()
    
    def _register_base_functions(self):
        """Register built-in Python functions for Lua"""
        lua_globals = self.lua.globals()
        
        # Basic logging
        lua_globals.print = print
        
        # Math functions
        import math
        lua_globals.math_sqrt = math.sqrt
        lua_globals.math_sin = math.sin
        lua_globals.math_cos = math.cos
        lua_globals.math_abs = abs
        
        # Utility functions
        lua_globals.python_type = type
        lua_globals.python_len = len
    
    def register_function(self, name: str, func: Callable):
        """Register a Python function to be callable from Lua"""
        self.global_functions[name] = func
        self.lua.globals()[name] = func
    
    def register_module(self, module_name: str, module_dict: Dict[str, Any]):
        """Register a Python module/table in Lua"""
        lua_table = self.lua.table_from(module_dict)
        self.lua.globals()[module_name] = lua_table
    
    def load_script(self, name: str, filepath: str) -> LuaScript:
        """Load a Lua script file"""
        if not os.path.exists(filepath):
            print(f"Lua script not found: {filepath}")
            return None
        
        script = LuaScript(self.lua, filepath)
        if script.loaded:
            self.scripts[name] = script
            print(f"Loaded Lua script: {name}")
        return script
    
    def unload_script(self, name: str):
        """Unload a script"""
        if name in self.scripts:
            del self.scripts[name]
    
    def get_script(self, name: str) -> Optional[LuaScript]:
        """Get a loaded script by name"""
        return self.scripts.get(name)
    
    def execute_string(self, lua_code: str) -> Any:
        """Execute Lua code from string"""
        try:
            return self.lua.execute(lua_code)
        except Exception as e:
            print(f"Error executing Lua code: {e}")
            return None
    
    def execute_file(self, filepath: str) -> Any:
        """Execute a Lua file directly"""
        try:
            with open(filepath, 'r') as f:
                lua_code = f.read()
            return self.lua.execute(lua_code)
        except Exception as e:
            print(f"Error executing Lua file {filepath}: {e}")
            return None
    
    def call_global_function(self, func_name: str, *args):
        """Call a global Lua function"""
        try:
            func = self.lua.globals()[func_name]
            if func:
                return func(*args)
        except Exception as e:
            print(f"Error calling global Lua function '{func_name}': {e}")
        return None
    
    def create_lua_table(self, data: Dict) -> Any:
        """Create a Lua table from Python dict"""
        return self.lua.table_from(data)
    
    def reload_all_scripts(self):
        """Reload all loaded scripts"""
        for script in self.scripts.values():
            script.reload()
    
    def clear(self):
        """Clear all loaded scripts"""
        self.scripts.clear()

# Example usage and helper functions
class ScriptedEntity:
    """Example of an entity with Lua script behavior"""
    def __init__(self, scripting: ScriptingSystem, script_name: str):
        self.scripting = scripting
        self.script_name = script_name
        self.script = scripting.get_script(script_name)
        self.data = {}
    
    def on_create(self):
        """Call Lua on_create function"""
        if self.script:
            self.script.call_function("on_create", self)
    
    def on_update(self, dt: float):
        """Call Lua on_update function"""
        if self.script:
            self.script.call_function("on_update", self, dt)
    
    def on_destroy(self):
        """Call Lua on_destroy function"""
        if self.script:
            self.script.call_function("on_destroy", self)

# Example usage
if __name__ == "__main__":
    scripting = ScriptingSystem()
    
    # Register custom functions
    def spawn_enemy(x, y):
        print(f"Spawning enemy at ({x}, {y})")
    
    scripting.register_function("spawn_enemy", spawn_enemy)
    
    # Register a module
    engine_module = {
        "version": "1.0",
        "get_time": lambda: 42.0,
        "log": lambda msg: print(f"[ENGINE] {msg}")
    }
    scripting.register_module("Engine", engine_module)
    
    # Execute Lua code
    scripting.execute_string("""
        print("Hello from Lua!")
        
        function greet(name)
            return "Hello, " .. name .. "!"
        end
        
        function calculate_damage(base, multiplier)
            return base * multiplier
        end
        
        -- Access Engine module
        Engine.log("Lua script initialized")
        print("Engine version: " .. Engine.version)
    """)
    
    # Call Lua functions
    result = scripting.call_global_function("greet", "Omega Engine")
    print(f"Lua returned: {result}")
    
    damage = scripting.call_global_function("calculate_damage", 10, 2.5)
    print(f"Calculated damage: {damage}")
    
    # Create example Lua script file
    example_script = """
-- Example entity behavior script

local entity_data = {
    health = 100,
    speed = 5.0
}

function on_create(entity)
    print("Entity created!")
    entity.data = entity_data
end

function on_update(entity, dt)
    -- Update logic here
    if entity.data then
        entity.data.health = entity.data.health - dt * 0.1
    end
end

function on_destroy(entity)
    print("Entity destroyed!")
end

return entity_data
"""
    
    with open("example_entity.lua", "w") as f:
        f.write(example_script)
    
    # Load script
    scripting.load_script("entity_behavior", "example_entity.lua")
    
    # Create scripted entity
    entity = ScriptedEntity(scripting, "entity_behavior")
    entity.on_create()
    entity.on_update(0.016)
    entity.on_destroy()
    
    print("\nScripting system test complete!")
