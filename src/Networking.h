#ifndef OMEGA_NETWORKING_H
#define OMEGA_NETWORKING_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <queue>
#include <memory>
#include <cstdint>

// Network message types
enum class MessageType : uint8_t {
    Connect = 0,
    Disconnect,
    PlayerJoin,
    PlayerLeave,
    GameState,
    Input,
    Chat,
    Custom
};

// Network packet
struct Packet {
    MessageType type;
    uint32_t senderId;
    std::vector<uint8_t> data;
    double timestamp;
    
    Packet() : type(MessageType::Custom), senderId(0), timestamp(0.0) {}
};

// Client info
struct ClientInfo {
    uint32_t id;
    std::string address;
    uint16_t port;
    double lastHeartbeat;
    bool connected;
    
    ClientInfo() : id(0), port(0), lastHeartbeat(0.0), connected(false) {}
};

// Network statistics
struct NetworkStats {
    uint64_t bytesSent;
    uint64_t bytesReceived;
    uint32_t packetsSent;
    uint32_t packetsReceived;
    uint32_t packetsLost;
    float ping;
    
    NetworkStats() : bytesSent(0), bytesReceived(0), packetsSent(0), 
                     packetsReceived(0), packetsLost(0), ping(0.0f) {}
};

// Message callback
using MessageCallback = std::function<void(const Packet&)>;

// Network peer (base class for client and server)
class NetworkPeer {
public:
    virtual ~NetworkPeer() = default;
    
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void update(float deltaTime) = 0;
    
    virtual bool send(const Packet& packet, uint32_t clientId = 0) = 0;
    virtual bool broadcast(const Packet& packet) = 0;
    
    virtual void registerCallback(MessageType type, MessageCallback callback) = 0;
    
    virtual NetworkStats getStats() const = 0;
    virtual bool isConnected() const = 0;
};

// Network client
class NetworkClient : public NetworkPeer {
public:
    NetworkClient();
    ~NetworkClient() override;
    
    bool initialize() override;
    void shutdown() override;
    void update(float deltaTime) override;
    
    bool connect(const std::string& address, uint16_t port);
    void disconnect();
    
    bool send(const Packet& packet, uint32_t clientId = 0) override;
    bool broadcast(const Packet& packet) override;
    
    void registerCallback(MessageType type, MessageCallback callback) override;
    
    NetworkStats getStats() const override;
    bool isConnected() const override { return m_connected; }
    
    uint32_t getClientId() const { return m_clientId; }

private:
    void processMessages();
    void sendHeartbeat();
    
    bool m_connected;
    uint32_t m_clientId;
    std::string m_serverAddress;
    uint16_t m_serverPort;
    
    std::queue<Packet> m_incomingMessages;
    std::unordered_map<MessageType, MessageCallback> m_callbacks;
    
    NetworkStats m_stats;
    double m_lastHeartbeat;
    double m_connectionTime;
    
    // Socket handle (would be actual socket in real implementation)
    void* m_socket;
};

// Network server
class NetworkServer : public NetworkPeer {
public:
    NetworkServer();
    ~NetworkServer() override;
    
    bool initialize() override;
    void shutdown() override;
    void update(float deltaTime) override;
    
    bool start(uint16_t port, int maxClients = 32);
    void stop();
    
    bool send(const Packet& packet, uint32_t clientId = 0) override;
    bool broadcast(const Packet& packet) override;
    
    void registerCallback(MessageType type, MessageCallback callback) override;
    
    void disconnectClient(uint32_t clientId);
    
    NetworkStats getStats() const override;
    bool isConnected() const override { return m_running; }
    bool isRunning() const { return m_running; }
    
    int getClientCount() const { return static_cast<int>(m_clients.size()); }
    std::vector<ClientInfo> getClients() const;

private:
    void acceptConnections();
    void processMessages();
    void checkHeartbeats();
    uint32_t generateClientId();
    
    bool m_running;
    uint16_t m_port;
    int m_maxClients;
    
    std::unordered_map<uint32_t, ClientInfo> m_clients;
    std::queue<Packet> m_incomingMessages;
    std::unordered_map<MessageType, MessageCallback> m_callbacks;
    
    NetworkStats m_stats;
    uint32_t m_nextClientId;
    
    // Socket handle
    void* m_socket;
};

// Network manager (singleton)
class NetworkManager {
public:
    static NetworkManager& getInstance();
    
    // Client operations
    bool createClient();
    NetworkClient* getClient() { return m_client.get(); }
    
    // Server operations
    bool createServer();
    NetworkServer* getServer() { return m_server.get(); }
    
    void update(float deltaTime);
    void shutdown();
    
    // Utility functions
    static void serializeInt(std::vector<uint8_t>& buffer, int32_t value);
    static void serializeFloat(std::vector<uint8_t>& buffer, float value);
    static void serializeString(std::vector<uint8_t>& buffer, const std::string& str);
    
    static int32_t deserializeInt(const std::vector<uint8_t>& buffer, size_t& offset);
    static float deserializeFloat(const std::vector<uint8_t>& buffer, size_t& offset);
    static std::string deserializeString(const std::vector<uint8_t>& buffer, size_t& offset);

private:
    NetworkManager() = default;
    ~NetworkManager() { shutdown(); }
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
    
    std::unique_ptr<NetworkClient> m_client;
    std::unique_ptr<NetworkServer> m_server;
};

// Replication system for networked objects
struct ReplicatedComponent {
    uint32_t networkId;
    bool isOwner;
    double lastUpdateTime;
    
    ReplicatedComponent() : networkId(0), isOwner(false), lastUpdateTime(0.0) {}
};

class NetworkReplication {
public:
    NetworkReplication(NetworkPeer* peer);
    ~NetworkReplication() = default;
    
    void update(float deltaTime);
    
    // Register entity for replication
    void registerEntity(uint32_t entityId, uint32_t networkId);
    void unregisterEntity(uint32_t entityId);
    
    // Replication
    void replicateTransform(uint32_t entityId, const class TransformComponent& transform);
    void replicateAll();
    
    void setUpdateRate(float rate) { m_updateRate = rate; }

private:
    NetworkPeer* m_peer;
    std::unordered_map<uint32_t, uint32_t> m_entityToNetwork;
    std::unordered_map<uint32_t, uint32_t> m_networkToEntity;
    float m_updateRate;
    double m_lastUpdate;
};

#endif // OMEGA_NETWORKING_H
