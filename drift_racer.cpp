/**
 * DRIFT RACER - A Console-Based Driving Game Demo
 * Part of the Omega Engine Project
 * 
 * This is a basic top-down driving game that can be expanded into
 * a full drift simulator. Features:
 * - Car physics with acceleration, braking, and steering
 * - Drift mechanics with momentum and grip simulation
 * - Track with boundaries and checkpoints
 * - Lap timing system
 * - Console-based rendering at 60 FPS
 */

#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>
#include <thread>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace std;

// Console colors
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define YELLOW 6
#define GRAY 7
#define DARK_GRAY 8
#define BRIGHT_BLUE 9
#define BRIGHT_GREEN 10
#define BRIGHT_CYAN 11
#define BRIGHT_RED 12
#define BRIGHT_MAGENTA 13
#define BRIGHT_YELLOW 14
#define BRIGHT_WHITE 15

const double PI = 3.14159265359;

// ============================================================
// ULTRA-FAST CONSOLE RENDERER
// ============================================================

class Console {
private:
    static HANDLE hConsole;
    static char screenBuffer[40][121];
    static int colorBuffer[40][121];
    static atomic<bool> dirty;
    static mutex bufferMutex;
    static bool initialized;
    static CHAR_INFO writeBuffer[40 * 120];
    
public:
    static const int WIDTH = 120;
    static const int HEIGHT = 40;
    
    static void init() {
        if (initialized) return;
        
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        
        // Hide cursor
        CONSOLE_CURSOR_INFO cursorInfo;
        cursorInfo.dwSize = 1;
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        // Set console size
        SMALL_RECT windowSize = {0, 0, (SHORT)(WIDTH - 1), (SHORT)(HEIGHT - 1)};
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
        COORD bufferSize = {WIDTH, HEIGHT};
        SetConsoleScreenBufferSize(hConsole, bufferSize);
        
        clearBuffer();
        initialized = true;
    }
    
    static void clearBuffer() {
        lock_guard<mutex> lock(bufferMutex);
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                screenBuffer[y][x] = ' ';
                colorBuffer[y][x] = GRAY;
            }
        }
        dirty = true;
    }
    
    static void setChar(int x, int y, char c, int color) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
        lock_guard<mutex> lock(bufferMutex);
        screenBuffer[y][x] = c;
        colorBuffer[y][x] = color;
        dirty = true;
    }
    
    static void drawString(int x, int y, const string& str, int color) {
        lock_guard<mutex> lock(bufferMutex);
        for (size_t i = 0; i < str.length() && x + (int)i < WIDTH; i++) {
            if (y >= 0 && y < HEIGHT && x + (int)i >= 0) {
                screenBuffer[y][x + i] = str[i];
                colorBuffer[y][x + i] = color;
            }
        }
        dirty = true;
    }
    
    static void fillRect(int x, int y, int w, int h, char c, int color) {
        lock_guard<mutex> lock(bufferMutex);
        for (int dy = 0; dy < h; dy++) {
            for (int dx = 0; dx < w; dx++) {
                int px = x + dx;
                int py = y + dy;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    screenBuffer[py][px] = c;
                    colorBuffer[py][px] = color;
                }
            }
        }
        dirty = true;
    }
    
    static void render() {
        if (!dirty) return;
        
        lock_guard<mutex> lock(bufferMutex);
        
        int bufferIndex = 0;
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                writeBuffer[bufferIndex].Char.AsciiChar = screenBuffer[y][x];
                writeBuffer[bufferIndex].Attributes = colorBuffer[y][x];
                bufferIndex++;
            }
        }
        
        COORD bufferSize = {WIDTH, HEIGHT};
        COORD bufferCoord = {0, 0};
        SMALL_RECT writeRegion = {0, 0, (SHORT)(WIDTH - 1), (SHORT)(HEIGHT - 1)};
        
        WriteConsoleOutput(hConsole, writeBuffer, bufferSize, bufferCoord, &writeRegion);
        
        dirty = false;
    }
};

// Static member definitions
HANDLE Console::hConsole = nullptr;
char Console::screenBuffer[40][121] = {};
int Console::colorBuffer[40][121] = {};
atomic<bool> Console::dirty(false);
mutex Console::bufferMutex;
bool Console::initialized = false;
CHAR_INFO Console::writeBuffer[40 * 120] = {};

// ============================================================
// VECTOR2 HELPER
// ============================================================

struct Vec2 {
    double x, y;
    
    Vec2(double x = 0, double y = 0) : x(x), y(y) {}
    
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(double s) const { return Vec2(x * s, y * s); }
    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
    
    double length() const { return sqrt(x * x + y * y); }
    Vec2 normalized() const {
        double len = length();
        if (len > 0.0001) return Vec2(x / len, y / len);
        return Vec2(0, 0);
    }
    double dot(const Vec2& other) const { return x * other.x + y * other.y; }
};

// ============================================================
// CAR PHYSICS
// ============================================================

class Car {
public:
    // Position and orientation
    Vec2 position;
    double angle;           // Radians, 0 = pointing right
    
    // Velocity
    Vec2 velocity;
    double angularVelocity;
    
    // Physics properties
    double mass;
    double enginePower;
    double brakePower;
    double maxSpeed;
    double grip;            // Tire grip coefficient
    double driftGrip;       // Grip while drifting (lower = more drift)
    double steerSpeed;      // How fast car can turn
    double dragCoeff;       // Air resistance
    double rollingResist;   // Rolling friction
    
    // State
    bool isDrifting;
    double driftAngle;      // Angle difference between car direction and velocity
    double throttle;        // -1 to 1
    double steering;        // -1 to 1
    double handbrake;       // 0 to 1
    
    // Stats
    int lap;
    double lapTime;
    double bestLapTime;
    int checkpointsPassed;
    double totalTime;
    
    // Display character based on angle
    char getDisplayChar() const {
        // 8 directional characters
        double normalizedAngle = fmod(angle + PI * 2, PI * 2);
        int dir = (int)((normalizedAngle + PI / 8) / (PI / 4)) % 8;
        const char* chars = ">v<^>v<^";  // Simple arrows
        return chars[dir];
    }
    
    int getColor() const {
        if (isDrifting) return BRIGHT_YELLOW;
        if (handbrake > 0.5) return BRIGHT_RED;
        return BRIGHT_CYAN;
    }
    
    Car() {
        position = Vec2(60, 20);
        angle = 0;
        velocity = Vec2(0, 0);
        angularVelocity = 0;
        
        mass = 1200.0;      // kg
        enginePower = 8000.0;
        brakePower = 12000.0;
        maxSpeed = 50.0;
        grip = 0.95;
        driftGrip = 0.6;
        steerSpeed = 3.5;
        dragCoeff = 0.4;
        rollingResist = 0.01;
        
        isDrifting = false;
        driftAngle = 0;
        throttle = 0;
        steering = 0;
        handbrake = 0;
        
        lap = 0;
        lapTime = 0;
        bestLapTime = 999999.0;
        checkpointsPassed = 0;
        totalTime = 0;
    }
    
    void update(double dt) {
        // Get forward and right vectors
        Vec2 forward(cos(angle), sin(angle));
        Vec2 right(cos(angle + PI/2), sin(angle + PI/2));
        
        // Current speed in forward direction
        double forwardSpeed = velocity.dot(forward);
        double sideSpeed = velocity.dot(right);
        
        // Calculate drift angle
        if (velocity.length() > 0.5) {
            double velAngle = atan2(velocity.y, velocity.x);
            driftAngle = angle - velAngle;
            while (driftAngle > PI) driftAngle -= 2 * PI;
            while (driftAngle < -PI) driftAngle += 2 * PI;
            isDrifting = fabs(driftAngle) > 0.3 && velocity.length() > 5;
        } else {
            driftAngle = 0;
            isDrifting = false;
        }
        
        // Choose grip based on drift state and handbrake
        double currentGrip = grip;
        if (handbrake > 0.5) {
            currentGrip = driftGrip * 0.5;
        } else if (isDrifting) {
            currentGrip = driftGrip;
        }
        
        // Apply engine force
        Vec2 engineForce = forward * (throttle * enginePower);
        
        // Apply braking
        if (throttle < 0 && forwardSpeed > 0) {
            engineForce = forward * (-brakePower * fabs(throttle));
        }
        
        // Lateral friction (grip)
        Vec2 lateralVel = right * sideSpeed;
        Vec2 lateralFriction = lateralVel * (-currentGrip * mass * 9.81 / dt);
        
        // Limit lateral friction
        double maxLateralForce = mass * 9.81 * currentGrip;
        if (lateralFriction.length() > maxLateralForce) {
            lateralFriction = lateralFriction.normalized() * maxLateralForce;
        }
        
        // Air drag
        Vec2 dragForce = velocity * (-dragCoeff * velocity.length());
        
        // Rolling resistance
        Vec2 rollForce = velocity.normalized() * (-rollingResist * mass * 9.81);
        
        // Total force
        Vec2 totalForce = engineForce + lateralFriction + dragForce + rollForce;
        
        // Apply forces (F = ma)
        Vec2 acceleration = totalForce * (1.0 / mass);
        velocity += acceleration * dt;
        
        // Limit max speed
        if (velocity.length() > maxSpeed) {
            velocity = velocity.normalized() * maxSpeed;
        }
        
        // Steering
        double speedFactor = min(1.0, velocity.length() / 10.0);
        double turnRate = steering * steerSpeed * speedFactor;
        
        // Reduce turn rate at high speed
        if (velocity.length() > 20) {
            turnRate *= 20.0 / velocity.length();
        }
        
        // Apply handbrake effect on steering
        if (handbrake > 0.5) {
            turnRate *= 1.5;  // Can turn tighter with handbrake
        }
        
        angularVelocity = turnRate;
        angle += angularVelocity * dt;
        
        // Update position
        position += velocity * dt;
        
        // Update timers
        lapTime += dt;
        totalTime += dt;
    }
    
    void reset(Vec2 startPos, double startAngle) {
        position = startPos;
        angle = startAngle;
        velocity = Vec2(0, 0);
        angularVelocity = 0;
        throttle = 0;
        steering = 0;
        handbrake = 0;
        isDrifting = false;
        lapTime = 0;
        checkpointsPassed = 0;
    }
};

// ============================================================
// TRACK
// ============================================================

class Track {
public:
    static const int WIDTH = 120;
    static const int HEIGHT = 35;
    
    // Track layout: 0=grass, 1=road, 2=barrier, 3=checkpoint, 4=finish
    int layout[HEIGHT][WIDTH];
    
    // Checkpoint positions
    vector<pair<int, int>> checkpoints;
    pair<int, int> finishLine;
    
    Track() {
        // Initialize with grass
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                layout[y][x] = 0;
            }
        }
        
        createOvalTrack();
    }
    
    void createOvalTrack() {
        // Create an oval racing track
        int centerX = WIDTH / 2;
        int centerY = HEIGHT / 2;
        int outerRadiusX = 50;
        int outerRadiusY = 14;
        int innerRadiusX = 35;
        int innerRadiusY = 8;
        int roadWidth = 12;
        
        // Draw track surface
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                double dx = (x - centerX) / (double)outerRadiusX;
                double dy = (y - centerY) / (double)outerRadiusY;
                double dist = sqrt(dx * dx + dy * dy);
                
                double dxInner = (x - centerX) / (double)innerRadiusX;
                double dyInner = (y - centerY) / (double)innerRadiusY;
                double distInner = sqrt(dxInner * dxInner + dyInner * dyInner);
                
                if (dist <= 1.0 && distInner >= 1.0) {
                    layout[y][x] = 1;  // Road
                }
            }
        }
        
        // Add barriers (outer edge)
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                if (layout[y][x] == 1) {
                    // Check if on edge of track
                    bool onEdge = false;
                    for (int dy = -1; dy <= 1 && !onEdge; dy++) {
                        for (int dx = -1; dx <= 1 && !onEdge; dx++) {
                            int nx = x + dx;
                            int ny = y + dy;
                            if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
                                if (layout[ny][nx] == 0) {
                                    onEdge = true;
                                }
                            }
                        }
                    }
                    if (onEdge) {
                        layout[y][x] = 2;  // Barrier
                    }
                }
            }
        }
        
        // Add finish line (right side of track)
        finishLine = make_pair(centerX + outerRadiusX - roadWidth/2, centerY);
        for (int y = centerY - 4; y <= centerY + 4; y++) {
            if (y >= 0 && y < HEIGHT) {
                layout[y][finishLine.first] = 4;
            }
        }
        
        // Add checkpoints around the track
        checkpoints.clear();
        // Top checkpoint
        checkpoints.push_back(make_pair(centerX, centerY - outerRadiusY + 3));
        // Left checkpoint  
        checkpoints.push_back(make_pair(centerX - outerRadiusX + roadWidth/2 + 2, centerY));
        // Bottom checkpoint
        checkpoints.push_back(make_pair(centerX, centerY + outerRadiusY - 3));
        
        // Mark checkpoints on track
        for (auto& cp : checkpoints) {
            for (int y = cp.second - 2; y <= cp.second + 2; y++) {
                if (y >= 0 && y < HEIGHT && layout[y][cp.first] == 1) {
                    layout[y][cp.first] = 3;
                }
            }
        }
    }
    
    int getTile(int x, int y) const {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return 2;  // Barrier
        return layout[y][x];
    }
    
    void draw() {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                char c;
                int color;
                
                switch (layout[y][x]) {
                    case 0:  // Grass
                        c = '.';
                        color = GREEN;
                        break;
                    case 1:  // Road
                        c = ' ';
                        color = DARK_GRAY;
                        break;
                    case 2:  // Barrier
                        c = '#';
                        color = RED;
                        break;
                    case 3:  // Checkpoint
                        c = '|';
                        color = BRIGHT_CYAN;
                        break;
                    case 4:  // Finish line
                        c = '|';
                        color = BRIGHT_YELLOW;
                        break;
                    default:
                        c = '?';
                        color = BRIGHT_WHITE;
                }
                
                Console::setChar(x, y, c, color);
            }
        }
    }
    
    // Check collision with barriers
    bool checkCollision(double x, double y) const {
        int tileX = (int)x;
        int tileY = (int)y;
        int tile = getTile(tileX, tileY);
        return tile == 2 || tile == 0;  // Barrier or grass slows down
    }
    
    bool isOnGrass(double x, double y) const {
        int tile = getTile((int)x, (int)y);
        return tile == 0;
    }
    
    bool isOnBarrier(double x, double y) const {
        int tile = getTile((int)x, (int)y);
        return tile == 2;
    }
    
    int getCheckpoint(double x, double y) const {
        int tileX = (int)x;
        int tileY = (int)y;
        
        for (size_t i = 0; i < checkpoints.size(); i++) {
            if (abs(tileX - checkpoints[i].first) < 2 && abs(tileY - checkpoints[i].second) < 3) {
                return (int)i;
            }
        }
        return -1;
    }
    
    bool isAtFinish(double x, double y) const {
        int tileX = (int)x;
        int tileY = (int)y;
        return abs(tileX - finishLine.first) < 2 && abs(tileY - finishLine.second) < 5;
    }
};

// ============================================================
// GAME
// ============================================================

class Game {
private:
    Car car;
    Track track;
    bool running;
    double gameTime;
    int lastCheckpoint;
    bool passedCheckpoints[10];
    
    string formatTime(double seconds) {
        int mins = (int)(seconds / 60);
        double secs = fmod(seconds, 60.0);
        stringstream ss;
        ss << mins << ":" << fixed << setprecision(2) << (secs < 10 ? "0" : "") << secs;
        return ss.str();
    }
    
public:
    Game() : running(true), gameTime(0), lastCheckpoint(-1) {
        Console::init();
        
        // Start position (on finish line)
        car.position = Vec2(track.finishLine.first - 5, track.finishLine.second);
        car.angle = PI;  // Facing left (counter-clockwise)
        
        // Reset checkpoint tracking
        for (int i = 0; i < 10; i++) passedCheckpoints[i] = false;
    }
    
    void handleInput() {
        // Reset controls each frame
        car.throttle = 0;
        car.steering = 0;
        car.handbrake = 0;
        
        // Check multiple keys simultaneously
        if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000) {
            car.throttle = 1.0;
        }
        if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000) {
            car.throttle = -1.0;
        }
        if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000) {
            car.steering = -1.0;
        }
        if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            car.steering = 1.0;
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            car.handbrake = 1.0;
        }
        if (GetAsyncKeyState('R') & 0x8000) {
            // Reset car to start
            car.reset(Vec2(track.finishLine.first - 5, track.finishLine.second), PI);
            for (int i = 0; i < 10; i++) passedCheckpoints[i] = false;
            lastCheckpoint = -1;
        }
        if (GetAsyncKeyState('Q') & 0x8000 || GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            running = false;
        }
    }
    
    void update(double dt) {
        gameTime += dt;
        
        // Update car physics
        car.update(dt);
        
        // Check grass slowdown
        if (track.isOnGrass(car.position.x, car.position.y)) {
            car.velocity = car.velocity * 0.95;  // Slow down on grass
        }
        
        // Check barrier collision
        if (track.isOnBarrier(car.position.x, car.position.y)) {
            // Bounce off barrier
            car.velocity = car.velocity * -0.3;
            car.position = car.position + car.velocity.normalized() * (-2);
        }
        
        // Keep car in bounds
        car.position.x = max(1.0, min((double)Track::WIDTH - 2, car.position.x));
        car.position.y = max(1.0, min((double)Track::HEIGHT - 2, car.position.y));
        
        // Check checkpoints
        int cp = track.getCheckpoint(car.position.x, car.position.y);
        if (cp >= 0 && !passedCheckpoints[cp]) {
            passedCheckpoints[cp] = true;
            car.checkpointsPassed++;
        }
        
        // Check finish line
        if (track.isAtFinish(car.position.x, car.position.y)) {
            // Check if all checkpoints were passed
            bool allPassed = true;
            for (size_t i = 0; i < track.checkpoints.size(); i++) {
                if (!passedCheckpoints[i]) allPassed = false;
            }
            
            if (allPassed && car.lap > 0) {  // Complete lap (not the first crossing)
                if (car.lapTime < car.bestLapTime) {
                    car.bestLapTime = car.lapTime;
                }
                car.lap++;
                car.lapTime = 0;
                // Reset checkpoints for next lap
                for (int i = 0; i < 10; i++) passedCheckpoints[i] = false;
            } else if (car.lap == 0) {
                car.lap = 1;  // Start first lap
                car.lapTime = 0;
            }
        }
    }
    
    void render() {
        Console::clearBuffer();
        
        // Draw track
        track.draw();
        
        // Draw car
        int carX = (int)car.position.x;
        int carY = (int)car.position.y;
        Console::setChar(carX, carY, car.getDisplayChar(), car.getColor());
        
        // Draw tire marks when drifting
        if (car.isDrifting && car.velocity.length() > 5) {
            Vec2 behind = car.position - Vec2(cos(car.angle), sin(car.angle)) * 1.5;
            Console::setChar((int)behind.x, (int)behind.y, '~', DARK_GRAY);
        }
        
        // Draw HUD
        int hudY = Track::HEIGHT;
        
        Console::drawString(1, hudY, "=== DRIFT RACER ===", BRIGHT_YELLOW);
        
        // Speed gauge
        double speed = car.velocity.length() * 3.6;  // Convert to km/h-ish
        string speedStr = "Speed: " + to_string((int)speed) + " km/h";
        Console::drawString(1, hudY + 1, speedStr, BRIGHT_WHITE);
        
        // Speed bar
        int speedBarLen = min(30, (int)(speed / 5));
        Console::drawString(25, hudY + 1, "[", GRAY);
        for (int i = 0; i < 30; i++) {
            char c = i < speedBarLen ? '=' : ' ';
            int color = i < 10 ? GREEN : (i < 20 ? YELLOW : RED);
            if (i >= speedBarLen) color = DARK_GRAY;
            Console::setChar(26 + i, hudY + 1, c, color);
        }
        Console::drawString(56, hudY + 1, "]", GRAY);
        
        // Drift indicator
        if (car.isDrifting) {
            Console::drawString(60, hudY + 1, "** DRIFTING **", BRIGHT_YELLOW);
        }
        
        // Lap info
        string lapStr = "Lap: " + to_string(car.lap) + "  Time: " + formatTime(car.lapTime);
        Console::drawString(1, hudY + 2, lapStr, BRIGHT_CYAN);
        
        string bestStr = "Best: " + (car.bestLapTime < 999999 ? formatTime(car.bestLapTime) : "---");
        Console::drawString(40, hudY + 2, bestStr, BRIGHT_GREEN);
        
        // Checkpoints
        string cpStr = "Checkpoints: " + to_string(car.checkpointsPassed) + "/" + to_string(track.checkpoints.size());
        Console::drawString(70, hudY + 2, cpStr, BRIGHT_MAGENTA);
        
        // Controls
        Console::drawString(1, hudY + 3, "Controls: WASD/Arrows=Drive  SPACE=Handbrake/Drift  R=Reset  Q=Quit", GRAY);
        
        Console::render();
    }
    
    void showTitleScreen() {
        Console::clearBuffer();
        
        int centerX = Console::WIDTH / 2;
        int centerY = Console::HEIGHT / 2;
        
        Console::drawString(centerX - 15, centerY - 6, "================================", BRIGHT_YELLOW);
        Console::drawString(centerX - 15, centerY - 5, "         DRIFT RACER            ", BRIGHT_YELLOW);
        Console::drawString(centerX - 15, centerY - 4, "    A Console Racing Demo       ", BRIGHT_CYAN);
        Console::drawString(centerX - 15, centerY - 3, "================================", BRIGHT_YELLOW);
        
        Console::drawString(centerX - 18, centerY - 1, "Race around the oval track!", BRIGHT_WHITE);
        Console::drawString(centerX - 22, centerY, "Use handbrake (SPACE) to initiate drifts!", BRIGHT_WHITE);
        Console::drawString(centerX - 20, centerY + 1, "Pass all checkpoints to complete a lap", BRIGHT_WHITE);
        
        Console::drawString(centerX - 12, centerY + 3, "CONTROLS:", BRIGHT_GREEN);
        Console::drawString(centerX - 16, centerY + 4, "W/Up    - Accelerate", GRAY);
        Console::drawString(centerX - 16, centerY + 5, "S/Down  - Brake/Reverse", GRAY);
        Console::drawString(centerX - 16, centerY + 6, "A/D     - Steer Left/Right", GRAY);
        Console::drawString(centerX - 16, centerY + 7, "SPACE   - Handbrake (Drift!)", GRAY);
        Console::drawString(centerX - 16, centerY + 8, "R       - Reset Position", GRAY);
        Console::drawString(centerX - 16, centerY + 9, "Q/ESC   - Quit", GRAY);
        
        Console::drawString(centerX - 12, centerY + 11, "Press any key to start!", BRIGHT_YELLOW);
        
        Console::render();
        
        // Wait for keypress
        while (!_kbhit()) {
            Sleep(50);
        }
        _getch();
    }
    
    void run() {
        showTitleScreen();
        
        LARGE_INTEGER frequency, lastTime, currentTime;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastTime);
        
        while (running) {
            QueryPerformanceCounter(&currentTime);
            double dt = (double)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
            lastTime = currentTime;
            
            // Cap delta time to avoid physics issues
            if (dt > 0.05) dt = 0.05;
            
            handleInput();
            update(dt);
            render();
            
            Sleep(16);  // ~60 FPS
        }
        
        // Show final stats
        Console::clearBuffer();
        Console::drawString(45, 15, "Thanks for playing!", BRIGHT_YELLOW);
        Console::drawString(40, 17, "Best Lap: " + (car.bestLapTime < 999999 ? formatTime(car.bestLapTime) : "No complete laps"), BRIGHT_GREEN);
        Console::drawString(42, 19, "Press any key to exit", GRAY);
        Console::render();
        
        while (!_kbhit()) Sleep(50);
        _getch();
    }
};

// ============================================================
// MAIN
// ============================================================

int main() {
    try {
        Game game;
        game.run();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
