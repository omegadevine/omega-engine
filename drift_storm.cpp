/**
 * DRIFT STORM - First-Person Console Racing Game
 * Part of the Omega Engine Project
 * 
 * A Shutoko/Initial D inspired first-person driving experience with:
 * - Raycasting pseudo-3D rendering (cockpit view)
 * - Advanced drift physics with weight transfer
 * - Open-world highway and touge (mountain pass) sections
 * - Traffic system
 * - Drift scoring system
 * 
 * Inspired by: Genki's Shutoko Battle series, Initial D Arcade Stage,
 *              and the spirit of Japanese street racing culture.
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
#include <deque>
#include <random>

using namespace std;

// ============================================================
// CONSTANTS AND COLORS
// ============================================================

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
const double DEG_TO_RAD = PI / 180.0;
const double RAD_TO_DEG = 180.0 / PI;

// ============================================================
// CONSOLE RENDERER
// ============================================================

class Console {
private:
    static HANDLE hConsole;
    static CHAR_INFO* writeBuffer;
    static int width, height;
    static bool initialized;
    static mutex bufferMutex;

public:
    static int WIDTH;
    static int HEIGHT;
    
    static void init(int w = 160, int h = 50) {
        if (initialized) return;
        
        WIDTH = w;
        HEIGHT = h;
        width = w;
        height = h;
        
        writeBuffer = new CHAR_INFO[w * h];
        
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        
        // Set console font to small for higher resolution
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = 6;
        cfi.dwFontSize.Y = 10;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        wcscpy_s(cfi.FaceName, L"Consolas");
        SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);
        
        // Set buffer and window size
        COORD bufferSize = {(SHORT)w, (SHORT)h};
        SetConsoleScreenBufferSize(hConsole, bufferSize);
        
        SMALL_RECT windowSize = {0, 0, (SHORT)(w - 1), (SHORT)(h - 1)};
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
        
        // Hide cursor
        CONSOLE_CURSOR_INFO cursorInfo = {1, FALSE};
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        clearBuffer();
        initialized = true;
    }
    
    static void cleanup() {
        if (writeBuffer) {
            delete[] writeBuffer;
            writeBuffer = nullptr;
        }
    }
    
    static void clearBuffer(int color = BLACK) {
        lock_guard<mutex> lock(bufferMutex);
        for (int i = 0; i < width * height; i++) {
            writeBuffer[i].Char.AsciiChar = ' ';
            writeBuffer[i].Attributes = color;
        }
    }
    
    static void setChar(int x, int y, char c, int color) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        int idx = y * width + x;
        writeBuffer[idx].Char.AsciiChar = c;
        writeBuffer[idx].Attributes = color;
    }
    
    static void drawString(int x, int y, const string& str, int color) {
        for (size_t i = 0; i < str.length(); i++) {
            if (x + (int)i >= 0 && x + (int)i < width && y >= 0 && y < height) {
                int idx = y * width + (x + i);
                writeBuffer[idx].Char.AsciiChar = str[i];
                writeBuffer[idx].Attributes = color;
            }
        }
    }
    
    static void drawStringCentered(int y, const string& str, int color) {
        int x = (width - (int)str.length()) / 2;
        drawString(x, y, str, color);
    }
    
    static void fillRect(int x, int y, int w, int h, char c, int color) {
        for (int dy = 0; dy < h; dy++) {
            for (int dx = 0; dx < w; dx++) {
                setChar(x + dx, y + dy, c, color);
            }
        }
    }
    
    static void drawBox(int x, int y, int w, int h, int color) {
        // Corners
        setChar(x, y, '+', color);
        setChar(x + w - 1, y, '+', color);
        setChar(x, y + h - 1, '+', color);
        setChar(x + w - 1, y + h - 1, '+', color);
        // Edges
        for (int i = 1; i < w - 1; i++) {
            setChar(x + i, y, '-', color);
            setChar(x + i, y + h - 1, '-', color);
        }
        for (int i = 1; i < h - 1; i++) {
            setChar(x, y + i, '|', color);
            setChar(x + w - 1, y + i, '|', color);
        }
    }
    
    static void render() {
        COORD bufferSize = {(SHORT)width, (SHORT)height};
        COORD bufferCoord = {0, 0};
        SMALL_RECT writeRegion = {0, 0, (SHORT)(width - 1), (SHORT)(height - 1)};
        WriteConsoleOutput(hConsole, writeBuffer, bufferSize, bufferCoord, &writeRegion);
    }
};

// Static members
HANDLE Console::hConsole = nullptr;
CHAR_INFO* Console::writeBuffer = nullptr;
int Console::width = 160;
int Console::height = 50;
int Console::WIDTH = 160;
int Console::HEIGHT = 50;
bool Console::initialized = false;
mutex Console::bufferMutex;

// ============================================================
// VECTOR AND MATH UTILITIES
// ============================================================

struct Vec2 {
    double x, y;
    Vec2(double x = 0, double y = 0) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(double s) const { return Vec2(x * s, y * s); }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    double length() const { return sqrt(x * x + y * y); }
    Vec2 normalized() const { double l = length(); return l > 0.001 ? Vec2(x/l, y/l) : Vec2(); }
    double dot(const Vec2& o) const { return x * o.x + y * o.y; }
    Vec2 rotate(double angle) const {
        double c = cos(angle), s = sin(angle);
        return Vec2(x * c - y * s, x * s + y * c);
    }
};

double clamp(double v, double lo, double hi) { return v < lo ? lo : (v > hi ? hi : v); }
double lerp(double a, double b, double t) { return a + (b - a) * t; }
double smoothstep(double t) { return t * t * (3 - 2 * t); }

// ============================================================
// ROAD/TRACK SYSTEM
// ============================================================

struct RoadSegment {
    double z;           // Distance along road
    double curve;       // Curvature (-1 to 1, negative = left, positive = right)
    double hill;        // Elevation change
    double width;       // Road width multiplier
    int type;           // 0=highway, 1=touge, 2=tunnel
    bool hasTraffic;
    
    RoadSegment(double z = 0, double curve = 0, double hill = 0, double width = 1.0, int type = 0)
        : z(z), curve(curve), hill(hill), width(width), type(type), hasTraffic(true) {}
};

class Road {
public:
    vector<RoadSegment> segments;
    double totalLength;
    double segmentLength;
    int numSegments;
    
    // Track zones for variety
    enum Zone { HIGHWAY, TOUGE_ENTRY, TOUGE_HAIRPINS, TOUGE_DOWNHILL, TUNNEL, HIGHWAY_JUNCTION };
    
    Road() : segmentLength(200.0), numSegments(0), totalLength(0) {
        generateOpenWorld();
    }
    
    void generateOpenWorld() {
        segments.clear();
        
        // Create a large open world loop with distinct sections
        double z = 0;
        
        // === SECTION 1: Highway Start (2km) ===
        addStraight(z, 30, 1.2, 0);  // Wide highway
        addEasyCurve(z, 20, 0.3, 1.2, 0);
        addStraight(z, 20, 1.2, 0);
        addEasyCurve(z, 20, -0.25, 1.2, 0);
        
        // === SECTION 2: Highway to Mountain Transition ===
        addStraight(z, 15, 1.0, 0);
        addHillClimb(z, 25, 0.1, 1.0, 1);  // Start climbing
        
        // === SECTION 3: Touge Entry - Gentle curves ===
        addTougeCurve(z, 15, 0.4, 0.02, 0.85, 1);
        addTougeCurve(z, 12, -0.35, 0.01, 0.85, 1);
        addStraight(z, 8, 0.85, 1);
        
        // === SECTION 4: Touge Hairpins - Technical section ===
        addHairpin(z, true, 0.8, 1);   // Right hairpin
        addStraight(z, 6, 0.8, 1);
        addHairpin(z, false, 0.8, 1);  // Left hairpin
        addTougeCurve(z, 10, 0.5, -0.01, 0.8, 1);
        addHairpin(z, true, 0.75, 1);
        addStraight(z, 5, 0.8, 1);
        addHairpin(z, false, 0.75, 1);
        
        // === SECTION 5: Mountain Summit & Tunnel ===
        addStraight(z, 10, 0.9, 1);
        addTunnel(z, 30);
        
        // === SECTION 6: Touge Downhill - Fast and flowy ===
        addTougeCurve(z, 15, -0.45, -0.02, 0.85, 1);
        addStraight(z, 8, 0.85, 1);
        addTougeCurve(z, 18, 0.55, -0.025, 0.85, 1);
        addStraight(z, 10, 0.9, 1);
        addTougeCurve(z, 12, -0.4, -0.015, 0.85, 1);
        
        // === SECTION 7: Return to Highway ===
        addHillClimb(z, 20, -0.02, 1.0, 0);  // Descend to highway level
        addStraight(z, 15, 1.1, 0);
        addEasyCurve(z, 25, -0.2, 1.2, 0);
        addStraight(z, 30, 1.2, 0);
        addEasyCurve(z, 20, 0.15, 1.2, 0);
        
        // Loop connection
        addStraight(z, 20, 1.2, 0);
        
        numSegments = segments.size();
        totalLength = z;
    }
    
    void addStraight(double& z, int count, double width, int type) {
        for (int i = 0; i < count; i++) {
            segments.push_back(RoadSegment(z, 0, 0, width, type));
            z += segmentLength;
        }
    }
    
    void addEasyCurve(double& z, int count, double curve, double width, int type) {
        for (int i = 0; i < count; i++) {
            double t = (double)i / count;
            double c = curve * sin(t * PI);
            segments.push_back(RoadSegment(z, c, 0, width, type));
            z += segmentLength;
        }
    }
    
    void addTougeCurve(double& z, int count, double curve, double hill, double width, int type) {
        for (int i = 0; i < count; i++) {
            double t = (double)i / count;
            double c = curve * sin(t * PI);
            segments.push_back(RoadSegment(z, c, hill, width, type));
            z += segmentLength;
        }
    }
    
    void addHairpin(double& z, bool rightTurn, double width, int type) {
        double dir = rightTurn ? 1.0 : -1.0;
        // Entry
        for (int i = 0; i < 5; i++) {
            double t = (double)i / 5;
            segments.push_back(RoadSegment(z, dir * 0.3 * t, 0, width, type));
            z += segmentLength;
        }
        // Apex
        for (int i = 0; i < 8; i++) {
            segments.push_back(RoadSegment(z, dir * 0.9, 0, width * 0.9, type));
            z += segmentLength;
        }
        // Exit
        for (int i = 0; i < 5; i++) {
            double t = 1.0 - (double)i / 5;
            segments.push_back(RoadSegment(z, dir * 0.3 * t, 0, width, type));
            z += segmentLength;
        }
    }
    
    void addHillClimb(double& z, int count, double hillRate, double width, int type) {
        for (int i = 0; i < count; i++) {
            double t = (double)i / count;
            double h = hillRate * sin(t * PI * 0.5);
            segments.push_back(RoadSegment(z, 0, h, width, type));
            z += segmentLength;
        }
    }
    
    void addTunnel(double& z, int count) {
        for (int i = 0; i < count; i++) {
            double curve = 0.1 * sin((double)i / count * PI * 2);
            RoadSegment seg(z, curve, 0, 0.9, 2);
            seg.hasTraffic = false;
            segments.push_back(seg);
            z += segmentLength;
        }
    }
    
    RoadSegment& getSegment(int index) {
        return segments[((index % numSegments) + numSegments) % numSegments];
    }
    
    int getSegmentIndex(double z) const {
        return (int)(z / segmentLength) % numSegments;
    }
};

// ============================================================
// TRAFFIC SYSTEM
// ============================================================

struct TrafficCar {
    double z;           // Position along road
    double x;           // Lateral position (-1 to 1)
    double speed;       // Current speed
    int lane;           // -1 = left lane, 1 = right lane
    int type;           // Visual type
    bool active;
    
    TrafficCar() : z(0), x(0), speed(0), lane(1), type(0), active(false) {}
};

class TrafficManager {
public:
    vector<TrafficCar> cars;
    double spawnDistance;
    double despawnDistance;
    mt19937 rng;
    
    TrafficManager() : spawnDistance(800), despawnDistance(500) {
        rng.seed((unsigned)time(nullptr));
        cars.resize(30);  // More traffic cars
    }
    
    void update(double playerZ, double playerSpeed, double dt, const Road& road) {
        uniform_real_distribution<double> speedDist(60, 100);  // km/h - slower traffic
        uniform_int_distribution<int> laneDist(0, 1);
        uniform_int_distribution<int> typeDist(0, 3);
        
        for (auto& car : cars) {
            if (car.active) {
                // Move traffic
                car.z += car.speed * dt;
                car.x = car.lane * 0.4;  // Stay in lane
                
                // Despawn if too far behind
                if (car.z < playerZ - despawnDistance) {
                    car.active = false;
                }
                
                // Check road type - no traffic in tunnel
                int segIdx = road.getSegmentIndex(car.z);
                if (road.segments[segIdx].type == 2) {  // Only despawn in tunnels
                    car.active = false;
                }
            } else {
                // Try to spawn new traffic ahead (closer and more frequent)
                double spawnZ = playerZ + 200 + spawnDistance * (0.3 + 0.7 * (rng() % 100) / 100.0);
                int segIdx = road.getSegmentIndex(spawnZ);
                
                // Allow traffic on highway and touge (types 0 and 1), not tunnels (type 2)
                if (road.segments[segIdx].type != 2) {
                    car.z = spawnZ;
                    car.lane = laneDist(rng) ? 1 : -1;
                    car.x = car.lane * 0.4;
                    car.speed = speedDist(rng) / 3.6;  // Convert to m/s
                    car.type = typeDist(rng);
                    car.active = true;
                }
            }
        }
    }
    
    vector<TrafficCar*> getCarsInRange(double z, double range) {
        vector<TrafficCar*> result;
        for (auto& car : cars) {
            if (car.active && abs(car.z - z) < range) {
                result.push_back(&car);
            }
        }
        return result;
    }
};

// ============================================================
// CAR PHYSICS - Advanced Drift Model
// ============================================================

class PlayerCar {
public:
    // Position
    double z;           // Distance along road
    double x;           // Lateral position (-1 to 1, 0 = center)
    double y;           // Vertical (for hills)
    
    // Velocity
    double speed;       // Forward speed (m/s)
    double lateralSpeed;// Sideways speed
    
    // Orientation
    double angle;       // Car body angle relative to road
    double steerAngle;  // Current steering angle
    
    // Engine/Drivetrain
    double rpm;
    int gear;
    double throttle;
    double brake;
    double handbrake;
    double clutch;
    
    // Drift state
    bool isDrifting;
    double driftAngle;      // Angle between car heading and velocity
    double driftTimer;      // How long current drift has lasted
    double driftScore;      // Accumulated drift score
    double currentDriftScore;
    double comboMultiplier;
    
    // Physics properties
    double mass;
    double wheelbase;
    double frontGrip;
    double rearGrip;
    double engineTorque[7];
    double gearRatios[7];
    double finalDrive;
    double wheelRadius;
    
    // Weight transfer
    double weightFront;     // 0-1, how much weight on front
    double weightTransferRate;
    
    // Tire temps (affects grip)
    double tireTempFront;
    double tireTempRear;
    
    PlayerCar() {
        // Position
        z = 0;
        x = 0;
        y = 0;
        
        // Velocity
        speed = 0;
        lateralSpeed = 0;
        
        // Orientation  
        angle = 0;
        steerAngle = 0;
        
        // Engine
        rpm = 1000;
        gear = 1;
        throttle = 0;
        brake = 0;
        handbrake = 0;
        clutch = 0;
        
        // Drift
        isDrifting = false;
        driftAngle = 0;
        driftTimer = 0;
        driftScore = 0;
        currentDriftScore = 0;
        comboMultiplier = 1.0;
        
        // Physics - Tuned for fun drifting
        mass = 1350.0;  // kg (like an AE86 or S13)
        wheelbase = 2.4;
        frontGrip = 1.1;
        rearGrip = 0.95;  // Slightly less rear grip for easier drifting
        
        // Torque curve (Nm at different RPM ranges)
        engineTorque[0] = 0;      // 0 RPM
        engineTorque[1] = 150;    // 2000 RPM
        engineTorque[2] = 200;    // 4000 RPM
        engineTorque[3] = 220;    // 5000 RPM (peak)
        engineTorque[4] = 210;    // 6000 RPM
        engineTorque[5] = 190;    // 7000 RPM
        engineTorque[6] = 160;    // 8000 RPM (redline)
        
        // Gear ratios (6-speed + reverse)
        gearRatios[0] = -3.2;     // Reverse
        gearRatios[1] = 3.6;      // 1st
        gearRatios[2] = 2.2;      // 2nd
        gearRatios[3] = 1.5;      // 3rd
        gearRatios[4] = 1.1;      // 4th
        gearRatios[5] = 0.85;     // 5th
        gearRatios[6] = 0.7;      // 6th
        
        finalDrive = 4.1;
        wheelRadius = 0.31;
        
        // Weight distribution
        weightFront = 0.52;  // Slightly front-heavy
        weightTransferRate = 3.0;
        
        // Tire temps
        tireTempFront = 80;
        tireTempRear = 80;
    }
    
    double getTorque() {
        // Interpolate torque from curve
        double rpmNorm = clamp((rpm - 1000) / 7000.0, 0, 1);
        int idx = (int)(rpmNorm * 5);
        double t = fmod(rpmNorm * 5, 1.0);
        return lerp(engineTorque[idx + 1], engineTorque[min(idx + 2, 6)], t);
    }
    
    double getGripMultiplier(double temp) {
        // Optimal temp around 85-95C
        if (temp < 60) return 0.7;
        if (temp < 80) return 0.7 + 0.3 * (temp - 60) / 20;
        if (temp < 100) return 1.0;
        return 1.0 - 0.2 * (temp - 100) / 30;
    }
    
    void update(double dt, double roadCurve, double roadHill, double roadWidth, int steering) {
        // === INPUT PROCESSING ===
        double targetSteer = steering * 35.0 * DEG_TO_RAD;  // Max 35 degrees
        
        // Smooth steering
        double steerSpeed = 4.0;
        if (isDrifting) steerSpeed = 6.0;  // Faster countersteer when drifting
        steerAngle = lerp(steerAngle, targetSteer, dt * steerSpeed);
        
        // === WEIGHT TRANSFER ===
        double accel = throttle - brake;
        double targetWeight = 0.52 - accel * 0.15;  // Accelerating shifts weight back
        weightFront = lerp(weightFront, targetWeight, dt * weightTransferRate);
        
        // Lateral weight transfer from steering/drifting
        double lateralTransfer = abs(steerAngle) * speed / 50.0;
        
        // === TIRE GRIP ===
        double frontGripMod = frontGrip * getGripMultiplier(tireTempFront);
        double rearGripMod = rearGrip * getGripMultiplier(tireTempRear);
        
        // Handbrake kills rear grip
        if (handbrake > 0.5) {
            rearGripMod *= 0.3;
        }
        
        // Weight affects grip
        frontGripMod *= 0.7 + weightFront * 0.6;
        rearGripMod *= 0.7 + (1 - weightFront) * 0.6;
        
        // === ENGINE & TRANSMISSION ===
        double wheelSpeed = speed / wheelRadius;
        double gearRatio = gearRatios[gear];
        double driveRatio = gearRatio * finalDrive;
        
        // Engine RPM from wheel speed (simplified)
        double targetRpm = abs(wheelSpeed * driveRatio * 60 / (2 * PI));
        targetRpm = clamp(targetRpm, 1000, 8500);
        
        // Smooth RPM changes
        rpm = lerp(rpm, targetRpm, dt * 5);
        
        // Clutch and throttle affect RPM
        if (clutch > 0.5 || gear == 0) {
            rpm = lerp(rpm, 1000 + throttle * 7500, dt * 3);
        }
        
        // Calculate drive force
        double torque = getTorque() * throttle;
        double driveForce = torque * driveRatio / wheelRadius;
        
        // === PHYSICS SIMULATION ===
        
        // Longitudinal forces
        double rollingResist = 150 * speed / (speed + 10);
        double airDrag = 0.4 * speed * speed;
        double brakeForce = brake * 15000;
        
        double netForce = driveForce - rollingResist - airDrag - brakeForce;
        double acceleration = netForce / mass;
        
        speed += acceleration * dt;
        speed = max(0.0, speed);
        
        // === STEERING & DRIFT PHYSICS ===
        
        // Slip angle calculation
        double slipAngleFront = 0;
        double slipAngleRear = 0;
        
        if (speed > 1) {
            // Front slip angle (affected by steering)
            slipAngleFront = steerAngle - atan2(lateralSpeed + angle * wheelbase * 0.5, speed);
            // Rear slip angle
            slipAngleRear = -atan2(lateralSpeed - angle * wheelbase * 0.5, speed);
        }
        
        // Lateral forces from tires (simplified Pacejka-ish)
        auto tireForce = [](double slipAngle, double grip) {
            double slip = slipAngle * RAD_TO_DEG;
            double force = grip * 12000 * sin(1.9 * atan(0.1 * slip));
            return clamp(force, -15000, 15000);
        };
        
        double frontLateralForce = tireForce(slipAngleFront, frontGripMod);
        double rearLateralForce = tireForce(slipAngleRear, rearGripMod);
        
        // Apply forces
        double lateralAccel = (frontLateralForce + rearLateralForce) / mass;
        lateralSpeed += lateralAccel * dt;
        
        // Yaw moment (rotation)
        double yawMoment = (frontLateralForce * wheelbase * 0.5 - rearLateralForce * wheelbase * 0.5);
        double yawAccel = yawMoment / (mass * wheelbase * wheelbase / 12);
        angle += yawAccel * dt * dt;
        
        // Road curvature affects car
        if (speed > 1) {
            double curvatureForce = roadCurve * speed * speed / 50;
            lateralSpeed += curvatureForce * dt;
            x += curvatureForce * dt * 0.001;
        }
        
        // Damping
        lateralSpeed *= 0.98;
        angle *= 0.95;
        
        // === DRIFT DETECTION ===
        driftAngle = atan2(lateralSpeed, speed) * RAD_TO_DEG;
        
        bool wasDrifting = isDrifting;
        isDrifting = abs(driftAngle) > 10 && speed > 10;
        
        // Drift scoring
        if (isDrifting) {
            driftTimer += dt;
            double angleScore = min(abs(driftAngle), 90.0) / 90.0;
            double speedScore = min(speed * 3.6, 200.0) / 200.0;
            currentDriftScore += angleScore * speedScore * 100 * dt * comboMultiplier;
            comboMultiplier = min(comboMultiplier + dt * 0.5, 5.0);
            
            // Heat up rear tires
            tireTempRear += abs(driftAngle) * 0.1 * dt;
        } else {
            if (wasDrifting && currentDriftScore > 100) {
                driftScore += currentDriftScore;
            }
            currentDriftScore = 0;
            driftTimer = 0;
            comboMultiplier = 1.0;
        }
        
        // Tire temps
        tireTempFront = lerp(tireTempFront, 70 + speed * 0.5, dt * 0.5);
        tireTempRear = lerp(tireTempRear, 70 + speed * 0.5, dt * 0.5);
        tireTempFront = clamp(tireTempFront, 20, 150);
        tireTempRear = clamp(tireTempRear, 20, 150);
        
        // === POSITION UPDATE ===
        z += speed * dt;
        x += (lateralSpeed * 0.0002 + angle * 0.01) * dt * 60;
        x = clamp(x, -roadWidth * 0.9, roadWidth * 0.9);
        
        // Hill effect
        y = lerp(y, roadHill * 1000, dt * 2);
        
        // === AUTO TRANSMISSION (simple) ===
        if (gear > 0) {
            if (rpm > 7500 && gear < 6) {
                gear++;
            } else if (rpm < 3000 && gear > 1) {
                gear--;
            }
        }
    }
    
    double getSpeedKmh() const { return speed * 3.6; }
};

// ============================================================
// RENDERER - First Person Raycasting
// ============================================================

class Renderer {
private:
    int screenWidth;
    int screenHeight;
    int roadHeight;     // Height of 3D view
    double cameraHeight;
    double cameraDepth;
    double fov;
    
    // Pre-calculated values
    vector<double> zMap;    // Z distance for each screen row
    
public:
    Renderer(int w, int h) : screenWidth(w), screenHeight(h) {
        roadHeight = h - 12;  // Leave room for HUD
        cameraHeight = 1200;
        cameraDepth = 0.8;
        fov = 100;
        
        // Pre-calculate z-buffer mapping
        zMap.resize(roadHeight);
        for (int y = 0; y < roadHeight; y++) {
            double screenY = (double)(y - roadHeight / 2) / (roadHeight / 2);
            if (screenY > 0.01) {
                zMap[y] = cameraHeight / screenY;
            } else {
                zMap[y] = 100000;
            }
        }
    }
    
    void render(const PlayerCar& car, const Road& road, TrafficManager& traffic) {
        Console::clearBuffer();
        
        int horizonY = roadHeight / 2 - 3 + (int)(car.y * 0.001);
        horizonY = clamp(horizonY, 5, roadHeight - 10);
        
        // Get current road segment for sky/environment color
        int segIdx = road.getSegmentIndex(car.z);
        const RoadSegment& currentSeg = road.segments[segIdx];
        
        // === SKY ===
        int skyColor = BLUE;
        if (currentSeg.type == 2) skyColor = BLACK;  // Tunnel
        else if (currentSeg.type == 1) skyColor = DARK_GRAY;  // Mountain (darker sky)
        
        for (int y = 0; y < horizonY; y++) {
            for (int x = 0; x < screenWidth; x++) {
                char c = ' ';
                int color = skyColor;
                
                // Stars/lights in tunnel
                if (currentSeg.type == 2) {
                    if ((x + y * 3) % 47 == 0) {
                        c = '.';
                        color = BRIGHT_YELLOW;
                    }
                }
                // Distant mountains for touge
                else if (currentSeg.type == 1 && y > horizonY - 8) {
                    int mountainHeight = horizonY - 5 + (int)(sin(x * 0.1) * 3);
                    if (y > mountainHeight) {
                        c = '^';
                        color = DARK_GRAY;
                    }
                }
                
                Console::setChar(x, y, c, color);
            }
        }
        
        // === ROAD RENDERING ===
        double baseSegmentZ = car.z;
        double cameraX = car.x;
        double cameraAngle = car.angle;
        
        // Accumulated curve for pseudo-3D
        double accumulatedCurve = 0;
        double dx = 0;
        
        for (int y = horizonY; y < roadHeight; y++) {
            double perspective = (double)(y - horizonY) / (roadHeight - horizonY);
            double z = zMap[y];
            
            if (z > 50000) continue;
            
            // Get road segment at this distance
            int segmentIndex = road.getSegmentIndex(baseSegmentZ + z);
            const RoadSegment& seg = road.segments[segmentIndex];
            
            // Accumulate curve
            accumulatedCurve += seg.curve * perspective * 0.1;
            dx += accumulatedCurve;
            
            // Road position on screen
            double roadCenterX = screenWidth / 2.0 + dx * screenWidth * 0.3 - cameraX * screenWidth * perspective * 0.5;
            double roadWidth = seg.width * screenWidth * 0.25 / (z * 0.001 + 1);
            
            // Road edges
            int leftEdge = (int)(roadCenterX - roadWidth);
            int rightEdge = (int)(roadCenterX + roadWidth);
            
            // Segment colors based on type and distance
            bool isStripe = ((int)(baseSegmentZ + z) / (int)road.segmentLength) % 2 == 0;
            
            int roadColor, edgeColor, groundColor;
            char roadChar, edgeChar, groundChar;
            
            if (seg.type == 2) {  // Tunnel
                roadColor = DARK_GRAY;
                edgeColor = BRIGHT_YELLOW;
                groundColor = BLACK;
                roadChar = ' ';
                edgeChar = '|';
                groundChar = ' ';
            } else if (seg.type == 1) {  // Touge
                roadColor = isStripe ? DARK_GRAY : GRAY;
                edgeColor = BRIGHT_WHITE;
                groundColor = GREEN;
                roadChar = ' ';
                edgeChar = isStripe ? '|' : ' ';
                groundChar = '.';
            } else {  // Highway
                roadColor = isStripe ? DARK_GRAY : GRAY;
                edgeColor = isStripe ? BRIGHT_WHITE : GRAY;
                groundColor = isStripe ? GREEN : DARK_GRAY;
                roadChar = ' ';
                edgeChar = '|';
                groundChar = '.';
            }
            
            // Draw scanline
            for (int x = 0; x < screenWidth; x++) {
                char c = groundChar;
                int color = groundColor;
                
                if (x >= leftEdge && x <= rightEdge) {
                    // On road
                    c = roadChar;
                    color = roadColor;
                    
                    // Center line
                    int center = (int)roadCenterX;
                    if (abs(x - center) < 2 && seg.type != 2) {
                        if (isStripe) {
                            c = ':';
                            color = BRIGHT_YELLOW;
                        }
                    }
                    
                    // Lane markers
                    int laneWidth = (int)(roadWidth / 2);
                    if (abs(x - (center - laneWidth)) < 1 || abs(x - (center + laneWidth)) < 1) {
                        if (isStripe && seg.type == 0) {
                            c = ':';
                            color = BRIGHT_WHITE;
                        }
                    }
                }
                
                // Edge lines
                if (abs(x - leftEdge) < 3 || abs(x - rightEdge) < 3) {
                    c = edgeChar;
                    color = edgeColor;
                }
                
                // Guardrails on touge
                if (seg.type == 1) {
                    if (abs(x - leftEdge) < 2 || abs(x - rightEdge) < 2) {
                        c = '#';
                        color = BRIGHT_WHITE;
                    }
                }
                
                // Tunnel walls
                if (seg.type == 2) {
                    if (x < leftEdge - 5 || x > rightEdge + 5) {
                        c = '#';
                        color = DARK_GRAY;
                    } else if (x < leftEdge || x > rightEdge) {
                        c = '|';
                        color = YELLOW;
                    }
                }
                
                Console::setChar(x, y, c, color);
            }
        }
        
        // === TRAFFIC RENDERING ===
        auto nearbyCars = traffic.getCarsInRange(car.z, 500);
        for (auto* tCar : nearbyCars) {
            double relZ = tCar->z - car.z;
            if (relZ < 10 || relZ > 400) continue;
            
            // Project to screen
            double scale = 150.0 / relZ;
            int screenY = horizonY + (int)((roadHeight - horizonY) * (50.0 / relZ));
            int screenX = screenWidth / 2 + (int)((tCar->x - car.x) * screenWidth * scale * 0.5);
            
            if (screenY > horizonY && screenY < roadHeight - 2) {
                // Simple car sprite
                int carWidth = max(3, (int)(12 * scale));
                int carHeight = max(2, (int)(4 * scale));
                
                int colors[] = {BRIGHT_RED, BRIGHT_BLUE, BRIGHT_WHITE, BRIGHT_YELLOW};
                int carColor = colors[tCar->type % 4];
                
                for (int dy = 0; dy < carHeight; dy++) {
                    for (int dx = -carWidth/2; dx < carWidth/2; dx++) {
                        int px = screenX + dx;
                        int py = screenY - dy;
                        if (px >= 0 && px < screenWidth && py > horizonY && py < roadHeight) {
                            char c = (dy == 0 || dy == carHeight-1) ? '=' : '|';
                            if (dx == -carWidth/2 || dx == carWidth/2-1) c = '|';
                            Console::setChar(px, py, c, carColor);
                        }
                    }
                }
                
                // Taillights
                if (relZ < 150) {
                    Console::setChar(screenX - carWidth/2, screenY, '*', BRIGHT_RED);
                    Console::setChar(screenX + carWidth/2 - 1, screenY, '*', BRIGHT_RED);
                }
            }
        }
        
        // === COCKPIT/DASHBOARD ===
        renderCockpit(car, road);
        
        // === HUD ===
        renderHUD(car, road);
    }
    
    void renderCockpit(const PlayerCar& car, const Road& road) {
        int dashY = roadHeight;
        
        // Dashboard background
        Console::fillRect(0, dashY, screenWidth, 12, ' ', BLACK);
        
        // Steering wheel indicator
        int wheelCenterX = screenWidth / 2;
        int wheelY = dashY + 2;
        double steerNorm = car.steerAngle / (35.0 * DEG_TO_RAD);
        int wheelOffset = (int)(steerNorm * 15);
        
        Console::drawString(wheelCenterX - 20, wheelY, "    _______________    ", DARK_GRAY);
        Console::drawString(wheelCenterX - 20, wheelY + 1, "   /               \\   ", DARK_GRAY);
        Console::drawString(wheelCenterX - 20, wheelY + 2, "  |                 |  ", DARK_GRAY);
        Console::setChar(wheelCenterX + wheelOffset, wheelY + 1, 'O', BRIGHT_WHITE);
        
        // Drift smoke effect when drifting
        if (car.isDrifting) {
            int smokeY = roadHeight - 3;
            string smoke = "~*~";
            if (car.driftAngle > 0) {
                Console::drawString(10, smokeY, smoke, GRAY);
                Console::drawString(15, smokeY - 1, smoke, DARK_GRAY);
            } else {
                Console::drawString(screenWidth - 20, smokeY, smoke, GRAY);
                Console::drawString(screenWidth - 25, smokeY - 1, smoke, DARK_GRAY);
            }
        }
    }
    
    void renderHUD(const PlayerCar& car, const Road& road) {
        int hudY = roadHeight;
        int hudHeight = 12;
        
        // === LEFT PANEL: Speed & Gear ===
        Console::drawBox(1, hudY, 30, hudHeight - 1, GRAY);
        
        // Digital speedometer
        int speedKmh = (int)car.getSpeedKmh();
        stringstream speedSS;
        speedSS << setw(3) << speedKmh;
        Console::drawString(5, hudY + 2, speedSS.str(), BRIGHT_WHITE);
        Console::drawString(15, hudY + 2, "km/h", GRAY);
        
        // Gear indicator
        string gearStr = car.gear == 0 ? "N" : to_string(car.gear);
        Console::drawString(5, hudY + 4, "GEAR", GRAY);
        Console::drawString(11, hudY + 4, gearStr, BRIGHT_CYAN);
        
        // Tachometer bar
        double rpmNorm = (car.rpm - 1000) / 7500;
        int rpmBarLen = (int)(rpmNorm * 20);
        Console::drawString(5, hudY + 6, "RPM [", GRAY);
        for (int i = 0; i < 20; i++) {
            char c = i < rpmBarLen ? '|' : ' ';
            int color = i < 14 ? GREEN : (i < 17 ? YELLOW : RED);
            if (i >= rpmBarLen) color = DARK_GRAY;
            Console::setChar(10 + i, hudY + 6, c, color);
        }
        Console::setChar(30 - 2, hudY + 6, ']', GRAY);
        
        // RPM number
        Console::drawString(5, hudY + 8, to_string((int)car.rpm), 
            car.rpm > 7000 ? BRIGHT_RED : GRAY);
        
        // === CENTER: Drift Info ===
        if (car.isDrifting || car.currentDriftScore > 0) {
            int centerX = screenWidth / 2;
            
            // Drift angle
            stringstream angleSS;
            angleSS << fixed << setprecision(0) << abs(car.driftAngle) << " DEG";
            Console::drawStringCentered(hudY + 1, angleSS.str(), BRIGHT_YELLOW);
            
            // Current drift score
            if (car.currentDriftScore > 0) {
                stringstream scoreSS;
                scoreSS << "DRIFT: " << (int)car.currentDriftScore;
                Console::drawStringCentered(hudY + 3, scoreSS.str(), BRIGHT_YELLOW);
                
                // Combo multiplier
                if (car.comboMultiplier > 1.1) {
                    stringstream comboSS;
                    comboSS << "x" << fixed << setprecision(1) << car.comboMultiplier;
                    Console::drawStringCentered(hudY + 4, comboSS.str(), BRIGHT_MAGENTA);
                }
            }
            
            // Direction indicator
            string dirStr = car.driftAngle > 0 ? "<< LEFT" : "RIGHT >>";
            Console::drawStringCentered(hudY + 6, dirStr, BRIGHT_CYAN);
        }
        
        // === RIGHT PANEL: Score & Zone ===
        Console::drawBox(screenWidth - 31, hudY, 30, hudHeight - 1, GRAY);
        
        // Total drift score
        Console::drawString(screenWidth - 28, hudY + 2, "SCORE", GRAY);
        Console::drawString(screenWidth - 28, hudY + 3, to_string((int)car.driftScore), BRIGHT_GREEN);
        
        // Current zone
        int segIdx = road.getSegmentIndex(car.z);
        string zoneName;
        int zoneColor;
        switch (road.segments[segIdx].type) {
            case 0: zoneName = "HIGHWAY"; zoneColor = BRIGHT_BLUE; break;
            case 1: zoneName = "TOUGE"; zoneColor = BRIGHT_GREEN; break;
            case 2: zoneName = "TUNNEL"; zoneColor = BRIGHT_YELLOW; break;
            default: zoneName = "UNKNOWN"; zoneColor = GRAY;
        }
        Console::drawString(screenWidth - 28, hudY + 5, "ZONE", GRAY);
        Console::drawString(screenWidth - 28, hudY + 6, zoneName, zoneColor);
        
        // Distance traveled
        double distKm = car.z / 1000.0;
        stringstream distSS;
        distSS << fixed << setprecision(1) << distKm << " km";
        Console::drawString(screenWidth - 28, hudY + 8, distSS.str(), GRAY);
        
        // === BOTTOM: Controls reminder ===
        Console::drawString(screenWidth / 2 - 35, Console::HEIGHT - 1,
            "W/S: Accel/Brake | A/D: Steer | SPACE: Handbrake | SHIFT: Clutch Kick | Q: Quit",
            DARK_GRAY);
    }
};

// ============================================================
// GAME CLASS
// ============================================================

class Game {
private:
    PlayerCar car;
    Road road;
    TrafficManager traffic;
    Renderer* renderer;
    bool running;
    
    int steerInput;
    
public:
    Game() : running(true), steerInput(0) {
        Console::init(160, 50);
        renderer = new Renderer(Console::WIDTH, Console::HEIGHT);
        
        car.z = 0;
        car.x = 0;
        car.gear = 1;
    }
    
    ~Game() {
        delete renderer;
        Console::cleanup();
    }
    
    void handleInput() {
        car.throttle = 0;
        car.brake = 0;
        car.handbrake = 0;
        car.clutch = 0;
        steerInput = 0;
        
        // Throttle
        if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000) {
            car.throttle = 1.0;
        }
        
        // Brake
        if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000) {
            car.brake = 1.0;
        }
        
        // Steering
        if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000) {
            steerInput = -1;
        }
        if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            steerInput = 1;
        }
        
        // Handbrake
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            car.handbrake = 1.0;
        }
        
        // Clutch kick (for initiating drifts)
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            car.clutch = 1.0;
            car.rpm = min(car.rpm + 500, 8500.0);  // Rev up
        }
        
        // Manual gear (optional)
        if (GetAsyncKeyState('E') & 0x8000) {
            if (car.gear < 6) car.gear++;
        }
        if (GetAsyncKeyState('Q') & 0x8000) {
            // Q now quits, but could be downshift
        }
        
        // Quit
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            running = false;
        }
        if (GetAsyncKeyState('Q') & 0x8000) {
            running = false;
        }
    }
    
    void update(double dt) {
        // Get current road properties
        int segIdx = road.getSegmentIndex(car.z);
        RoadSegment& seg = road.segments[segIdx];
        
        // Update car physics
        car.update(dt, seg.curve, seg.hill, seg.width, steerInput);
        
        // Update traffic
        traffic.update(car.z, car.speed, dt, road);
        
        // Collision with traffic (forgiving - nudge aside instead of hard stop)
        auto nearbyCars = traffic.getCarsInRange(car.z, 30);
        for (auto* tCar : nearbyCars) {
            double dz = abs(tCar->z - car.z);
            double dx = abs(tCar->x - car.x);
            if (dz < 6 && dx < 0.25) {
                // Soft collision - small speed penalty and nudge apart
                car.speed *= 0.92;  // Only lose 8% speed
                
                // Nudge player away from traffic car
                if (car.x < tCar->x) {
                    car.x -= 0.1;  // Push player left
                } else {
                    car.x += 0.1;  // Push player right
                }
                
                // Traffic car swerves away too
                tCar->x += (tCar->x > car.x) ? 0.15 : -0.15;
                
                // Brief invulnerability - move traffic car ahead slightly
                tCar->z += 15;
            }
        }
        
        // World loop
        if (car.z > road.totalLength) {
            car.z -= road.totalLength;
        }
    }
    
    void render() {
        renderer->render(car, road, traffic);
        Console::render();
    }
    
    void showTitleScreen() {
        Console::clearBuffer(BLACK);
        
        int centerY = Console::HEIGHT / 2;
        
        // Title art
        Console::drawStringCentered(centerY - 12, "================================================================", BRIGHT_YELLOW);
        Console::drawStringCentered(centerY - 10, "    ____  ____  ___ _____ _____   ____ _____ ___  ____  __  __", BRIGHT_CYAN);
        Console::drawStringCentered(centerY - 9,  "   |  _ \\|  _ \\|_ _|  ___|_   _| / ___|_   _/ _ \\|  _ \\|  \\/  |", BRIGHT_CYAN);
        Console::drawStringCentered(centerY - 8,  "   | | | | |_) || || |_    | |   \\___ \\ | || | | | |_) | |\\/| |", BRIGHT_CYAN);
        Console::drawStringCentered(centerY - 7,  "   | |_| |  _ < | ||  _|   | |    ___) || || |_| |  _ <| |  | |", BRIGHT_CYAN);
        Console::drawStringCentered(centerY - 6,  "   |____/|_| \\_\\___|_|     |_|   |____/ |_| \\___/|_| \\_\\_|  |_|", BRIGHT_CYAN);
        Console::drawStringCentered(centerY - 4, "================================================================", BRIGHT_YELLOW);
        
        Console::drawStringCentered(centerY - 2, "A First-Person Drift Racing Experience", BRIGHT_WHITE);
        Console::drawStringCentered(centerY - 1, "Inspired by Shutoko Battle & Initial D", GRAY);
        
        Console::drawStringCentered(centerY + 2, "--- CONTROLS ---", BRIGHT_GREEN);
        Console::drawStringCentered(centerY + 4, "W / UP      - Accelerate", GRAY);
        Console::drawStringCentered(centerY + 5, "S / DOWN    - Brake", GRAY);
        Console::drawStringCentered(centerY + 6, "A / LEFT    - Steer Left", GRAY);
        Console::drawStringCentered(centerY + 7, "D / RIGHT   - Steer Right", GRAY);
        Console::drawStringCentered(centerY + 8, "SPACE       - Handbrake (Drift!)", GRAY);
        Console::drawStringCentered(centerY + 9, "SHIFT       - Clutch Kick (Initiate Drift)", GRAY);
        Console::drawStringCentered(centerY + 10, "Q / ESC     - Quit", GRAY);
        
        Console::drawStringCentered(centerY + 13, "--- HOW TO DRIFT ---", BRIGHT_MAGENTA);
        Console::drawStringCentered(centerY + 14, "Enter corners at speed, tap SPACE or SHIFT to break traction,", GRAY);
        Console::drawStringCentered(centerY + 15, "then countersteer to maintain the slide!", GRAY);
        
        Console::drawStringCentered(centerY + 18, "Press any key to start...", BRIGHT_YELLOW);
        
        Console::render();
        
        while (!_kbhit()) Sleep(50);
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
            
            // Cap delta time
            dt = min(dt, 0.05);
            
            handleInput();
            update(dt);
            render();
            
            Sleep(8);  // ~120 physics updates/sec, smoother driving
        }
        
        // Exit screen
        Console::clearBuffer(BLACK);
        Console::drawStringCentered(Console::HEIGHT / 2 - 2, "Thanks for playing DRIFT STORM!", BRIGHT_YELLOW);
        Console::drawStringCentered(Console::HEIGHT / 2, "Final Drift Score: " + to_string((int)car.driftScore), BRIGHT_GREEN);
        Console::drawStringCentered(Console::HEIGHT / 2 + 2, "Press any key to exit...", GRAY);
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
