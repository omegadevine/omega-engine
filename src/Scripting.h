#ifndef OMEGA_SCRIPTING_H
#define OMEGA_SCRIPTING_H

#include "ECS.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

// Forward declaration for Lua state (would be lua_State in real implementation)
typedef void* LuaState;

// Script component for entities
struct ScriptComponent {
    std::string scriptFile;
    LuaState luaState;
    bool initialized;
    
    ScriptComponent() : luaState(nullptr), initialized(false) {}
};

// Lua function wrapper
using LuaFunction = std::function<int(LuaState)>;

// Scripting engine
class ScriptEngine {
public:
    static ScriptEngine& getInstance();
    
    bool initialize();
    void shutdown();
    
    // Script execution
    bool loadScript(const std::string& filename);
    bool executeScript(const std::string& code);
    bool callFunction(const std::string& functionName, const std::vector<int>& args = {});
    
    // Registration
    void registerFunction(const std::string& name, LuaFunction func);
    void registerClass(const std::string& className);
    void registerVariable(const std::string& name, int value);
    void registerVariable(const std::string& name, float value);
    void registerVariable(const std::string& name, const std::string& value);
    
    // Stack operations
    void pushInt(int value);
    void pushFloat(float value);
    void pushString(const std::string& value);
    void pushBool(bool value);
    
    int popInt();
    float popFloat();
    std::string popString();
    bool popBool();
    
    // Error handling
    bool hasError() const { return !m_lastError.empty(); }
    std::string getLastError() const { return m_lastError; }
    void clearError() { m_lastError.clear(); }
    
    LuaState getLuaState() const { return m_luaState; }

private:
    ScriptEngine() = default;
    ~ScriptEngine() { shutdown(); }
    ScriptEngine(const ScriptEngine&) = delete;
    ScriptEngine& operator=(const ScriptEngine&) = delete;
    
    LuaState m_luaState;
    std::string m_lastError;
    std::unordered_map<std::string, LuaFunction> m_functions;
};

// Script system for ECS
class ScriptSystem {
public:
    ScriptSystem();
    ~ScriptSystem() = default;
    
    void update(ECS& ecs, float deltaTime);
    
    // Bind scripts to entities
    Entity createScriptedEntity(ECS& ecs, const std::string& scriptFile);
    void attachScript(ECS& ecs, Entity entity, const std::string& scriptFile);
    void detachScript(ECS& ecs, Entity entity);
    
    // Script lifecycle
    void initializeScripts(ECS& ecs);
    void callScriptUpdate(ECS& ecs, Entity entity, float deltaTime);
    
    // Event system
    void sendEvent(const std::string& eventName, const std::vector<int>& args = {});
    void registerEventHandler(const std::string& eventName, LuaFunction handler);

private:
    bool loadEntityScript(ECS& ecs, Entity entity);
    
    std::unordered_map<std::string, LuaFunction> m_eventHandlers;
};

// Script utility functions
namespace ScriptUtil {
    // Register engine API to Lua
    void registerEngineAPI(ScriptEngine& engine);
    
    // Helper functions for binding C++ to Lua
    void bindVector2(ScriptEngine& engine);
    void bindColor(ScriptEngine& engine);
    void bindTransform(ScriptEngine& engine);
    void bindInput(ScriptEngine& engine);
    void bindAudio(ScriptEngine& engine);
    
    // Example Lua bindings
    int lua_CreateEntity(LuaState L);
    int lua_DestroyEntity(LuaState L);
    int lua_GetPosition(LuaState L);
    int lua_SetPosition(LuaState L);
    int lua_PlaySound(LuaState L);
    int lua_Log(LuaState L);
}

#endif // OMEGA_SCRIPTING_H
