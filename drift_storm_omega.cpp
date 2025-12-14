/**
 * DRIFT STORM OMEGA - GPU-Accelerated Racing Game
 * Part of the Omega Engine Project
 * 
 * A Shutoko/Initial D inspired first-person driving experience with:
 * - SDL2 hardware-accelerated rendering
 * - Advanced drift physics with weight transfer
 * - Open-world highway and touge (mountain pass) sections
 * - Traffic system with forgiving collisions
 * - Drift scoring system
 * 
 * This is the Omega Engine port of the original console-based drift_storm.
 */

#include "engine/Omega.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

using namespace Omega;

// ============================================================
// CONSTANTS
// ============================================================

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int ROAD_VIEW_HEIGHT = 620;  // Leave room for HUD at bottom

// Toggle features (for development/settings)
const bool TRAFFIC_ENABLED = false;  // Set to true to re-enable traffic

// Colors for the game
namespace GameColors {
    Color sky(100, 150, 255);
    Color skyMountain(80, 100, 140);
    Color skyTunnel(20, 20, 30);
    
    Color roadLight(80, 80, 90);
    Color roadDark(60, 60, 70);
    Color roadTunnel(50, 50, 60);
    
    Color grass(50, 120, 50);
    Color grassDark(40, 90, 40);
    Color dirt(100, 80, 60);
    
    Color lineWhite(255, 255, 255);
    Color lineYellow(255, 220, 50);
    Color guardrail(200, 200, 200);
    
    Color hudBg(20, 20, 30, 200);
    Color hudText(255, 255, 255);
    Color hudHighlight(255, 220, 50);
    Color hudSpeed(100, 255, 100);
    Color hudRpmLow(100, 255, 100);
    Color hudRpmMid(255, 255, 100);
    Color hudRpmHigh(255, 100, 100);
}

// ============================================================
// ROAD/TRACK SYSTEM
// ============================================================

struct RoadSegment {
    double z;
    double curve;
    double hill;
    double width;
    int type;  // 0=highway, 1=touge, 2=tunnel
    bool hasTraffic;
    
    RoadSegment(double z = 0, double curve = 0, double hill = 0, double width = 1.0, int type = 0)
        : z(z), curve(curve), hill(hill), width(width), type(type), hasTraffic(true) {}
};

class Road {
public:
    std::vector<RoadSegment> segments;
    double totalLength;
    double segmentLength;
    int numSegments;
    
    Road() : segmentLength(200.0), numSegments(0), totalLength(0) {
        generateOpenWorld();
    }
    
    void generateOpenWorld() {
        segments.clear();
        double z = 0;
        
        // Highway start
        addStraight(z, 30, 1.2, 0);
        addEasyCurve(z, 20, 0.3, 1.2, 0);
        addStraight(z, 20, 1.2, 0);
        addEasyCurve(z, 20, -0.25, 1.2, 0);
        
        // Highway to mountain
        addStraight(z, 15, 1.0, 0);
        addHillClimb(z, 25, 0.1, 1.0, 1);
        
        // Touge entry
        addTougeCurve(z, 15, 0.4, 0.02, 0.85, 1);
        addTougeCurve(z, 12, -0.35, 0.01, 0.85, 1);
        addStraight(z, 8, 0.85, 1);
        
        // Hairpins
        addHairpin(z, true, 0.8, 1);
        addStraight(z, 6, 0.8, 1);
        addHairpin(z, false, 0.8, 1);
        addTougeCurve(z, 10, 0.5, -0.01, 0.8, 1);
        addHairpin(z, true, 0.75, 1);
        addStraight(z, 5, 0.8, 1);
        addHairpin(z, false, 0.75, 1);
        
        // Summit and tunnel
        addStraight(z, 10, 0.9, 1);
        addTunnel(z, 30);
        
        // Downhill
        addTougeCurve(z, 15, -0.45, -0.02, 0.85, 1);
        addStraight(z, 8, 0.85, 1);
        addTougeCurve(z, 18, 0.55, -0.025, 0.85, 1);
        addStraight(z, 10, 0.9, 1);
        addTougeCurve(z, 12, -0.4, -0.015, 0.85, 1);
        
        // Return to highway
        addHillClimb(z, 20, -0.02, 1.0, 0);
        addStraight(z, 15, 1.1, 0);
        addEasyCurve(z, 25, -0.2, 1.2, 0);
        addStraight(z, 30, 1.2, 0);
        addEasyCurve(z, 20, 0.15, 1.2, 0);
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
            double c = curve * std::sin(t * PI);
            segments.push_back(RoadSegment(z, c, 0, width, type));
            z += segmentLength;
        }
    }
    
    void addTougeCurve(double& z, int count, double curve, double hill, double width, int type) {
        for (int i = 0; i < count; i++) {
            double t = (double)i / count;
            double c = curve * std::sin(t * PI);
            segments.push_back(RoadSegment(z, c, hill, width, type));
            z += segmentLength;
        }
    }
    
    void addHairpin(double& z, bool rightTurn, double width, int type) {
        double dir = rightTurn ? 1.0 : -1.0;
        for (int i = 0; i < 5; i++) {
            double t = (double)i / 5;
            segments.push_back(RoadSegment(z, dir * 0.3 * t, 0, width, type));
            z += segmentLength;
        }
        for (int i = 0; i < 8; i++) {
            segments.push_back(RoadSegment(z, dir * 0.9, 0, width * 0.9, type));
            z += segmentLength;
        }
        for (int i = 0; i < 5; i++) {
            double t = 1.0 - (double)i / 5;
            segments.push_back(RoadSegment(z, dir * 0.3 * t, 0, width, type));
            z += segmentLength;
        }
    }
    
    void addHillClimb(double& z, int count, double hillRate, double width, int type) {
        for (int i = 0; i < count; i++) {
            double t = (double)i / count;
            double h = hillRate * std::sin(t * PI * 0.5);
            segments.push_back(RoadSegment(z, 0, h, width, type));
            z += segmentLength;
        }
    }
    
    void addTunnel(double& z, int count) {
        for (int i = 0; i < count; i++) {
            double curve = 0.1 * std::sin((double)i / count * PI * 2);
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
    double z, x, speed;
    int lane, type;
    bool active;
    Color color;
    
    TrafficCar() : z(0), x(0), speed(0), lane(1), type(0), active(false) {}
};

class TrafficManager {
public:
    std::vector<TrafficCar> cars;
    std::mt19937 rng;
    
    TrafficManager() {
        rng.seed((unsigned)std::time(nullptr));
        cars.resize(30);
        
        // Preset colors
        Color carColors[] = {
            Color(255, 80, 80),   // Red
            Color(80, 80, 255),   // Blue
            Color(255, 255, 255), // White
            Color(255, 220, 50),  // Yellow
            Color(80, 255, 80),   // Green
            Color(200, 100, 255)  // Purple
        };
        
        for (auto& car : cars) {
            car.color = carColors[rng() % 6];
        }
    }
    
    void update(double playerZ, double playerSpeed, double dt, const Road& road) {
        std::uniform_real_distribution<double> speedDist(60, 100);
        std::uniform_int_distribution<int> laneDist(0, 1);
        
        for (auto& car : cars) {
            if (car.active) {
                car.z += car.speed * dt;
                car.x = car.lane * 0.4;
                
                if (car.z < playerZ - 500) {
                    car.active = false;
                }
                
                int segIdx = road.getSegmentIndex(car.z);
                if (road.segments[segIdx].type == 2) {
                    car.active = false;
                }
            } else {
                double spawnZ = playerZ + 200 + 800 * (0.3 + 0.7 * (rng() % 100) / 100.0);
                int segIdx = road.getSegmentIndex(spawnZ);
                
                if (road.segments[segIdx].type != 2) {
                    car.z = spawnZ;
                    car.lane = laneDist(rng) ? 1 : -1;
                    car.x = car.lane * 0.4;
                    car.speed = speedDist(rng) / 3.6;
                    car.active = true;
                }
            }
        }
    }
    
    std::vector<TrafficCar*> getCarsInRange(double z, double range) {
        std::vector<TrafficCar*> result;
        for (auto& car : cars) {
            if (car.active && std::abs(car.z - z) < range) {
                result.push_back(&car);
            }
        }
        return result;
    }
};

// ============================================================
// PACEJKA TIRE MODEL - "Magic Formula"
// Industry standard tire physics used in Assetto Corsa, rFactor, etc.
// Y(x) = D * sin(C * atan(B*x - E*(B*x - atan(B*x))))
// ============================================================

struct PacejkaTireParams {
    // Longitudinal (acceleration/braking) coefficients
    double Bx = 10.0;   // Stiffness factor
    double Cx = 1.65;   // Shape factor
    double Dx = 1.0;    // Peak factor (friction coefficient)
    double Ex = -0.5;   // Curvature factor
    
    // Lateral (cornering) coefficients
    double By = 8.0;
    double Cy = 1.3;
    double Dy = 1.0;    // Peak lateral friction
    double Ey = -0.2;
};

class PacejkaTire {
public:
    PacejkaTireParams params;
    double load;           // Vertical load (N)
    double slipRatio;      // Longitudinal slip (-1 to 1)
    double slipAngle;      // Lateral slip (radians)
    double Fx, Fy;         // Output forces
    double angularVel;     // Wheel rotation speed (rad/s)
    double radius;
    
    PacejkaTire() : load(3500), slipRatio(0), slipAngle(0), Fx(0), Fy(0), 
                   angularVel(0), radius(0.31) {}
    
    // The Magic Formula
    double magicFormula(double x, double B, double C, double D, double E) {
        return D * sin(C * atan(B * x - E * (B * x - atan(B * x))));
    }
    
    void calculate(double vLong, double vLat, double loadN, bool locked = false) {
        load = loadN;
        
        // Avoid division by zero
        double vx = std::max(std::abs(vLong), 0.5);
        
        // Slip ratio: difference between wheel rotation and ground speed
        // kappa = (wheel_speed - ground_speed) / ground_speed
        double wheelSpeed = angularVel * radius;
        if (locked) {
            slipRatio = -1.0;  // Fully locked wheel
        } else {
            slipRatio = (wheelSpeed - vLong) / vx;
            slipRatio = clamp(slipRatio, -1.0, 1.0);
        }
        
        // Slip angle: angle between wheel direction and velocity direction
        slipAngle = atan2(vLat, vx);
        
        // Scale peak force by load (simplified load sensitivity)
        double loadFactor = load / 3500.0;  // Normalized to nominal load
        double peakFx = params.Dx * load * loadFactor;
        double peakFy = params.Dy * load * loadFactor;
        
        // Calculate forces using Pacejka Magic Formula
        Fx = magicFormula(slipRatio, params.Bx, params.Cx, peakFx, params.Ex);
        Fy = magicFormula(slipAngle, params.By, params.Cy, peakFy, params.Ey);
        
        // Combined slip: reduce forces when both slipping longitudinally and laterally
        double combinedSlip = sqrt(slipRatio * slipRatio + slipAngle * slipAngle);
        if (combinedSlip > 0.01) {
            double combinedFactor = 1.0 / (1.0 + combinedSlip * 0.5);
            Fx *= combinedFactor;
            Fy *= combinedFactor;
        }
    }
};

// ============================================================
// CAR PHYSICS - Simulation-Based Vehicle Dynamics
// Based on bicycle model with 4-wheel extension
// ============================================================

class PlayerCar {
public:
    // Position and orientation (world space)
    double z, x, y;           // World position
    double heading;           // Car heading angle (radians)
    
    // Velocities (car local space)
    double vx, vy;            // Forward and lateral velocity (m/s)
    double yawRate;           // Angular velocity (rad/s)
    
    // Wheels
    PacejkaTire tireFrontLeft, tireFrontRight;
    PacejkaTire tireRearLeft, tireRearRight;
    
    // Display values
    double speed;             // Speed magnitude (m/s)
    double steerAngle;        // Current steering angle (rad)
    double rpm;
    int gear;
    
    // Inputs
    double throttle, brake, handbrake, clutch;
    double steerInput;
    
    // Drift tracking
    bool isDrifting;
    double driftAngle, driftTimer, driftScore, currentDriftScore, comboMultiplier;
    
    // Vehicle parameters
    double mass;              // kg
    double inertia;           // Moment of inertia (kg*m^2)
    double wheelbase;         // Distance front to rear axle (m)
    double trackWidth;        // Distance between left and right wheels (m)
    double cgToFront;         // Distance from CG to front axle (m)
    double cgToRear;          // Distance from CG to rear axle (m)
    double cgHeight;          // Center of gravity height (m)
    double wheelRadius;
    
    // Powertrain
    double engineTorque[7];
    double gearRatios[7];
    double finalDrive;
    
    // Weight distribution
    double weightFront, weightRear;   // Current weight on each axle (N)
    double staticWeightFront;         // Base weight distribution (0-1)
    
    PlayerCar() {
        // Initial state
        z = x = y = 0;
        heading = 0;
        vx = vy = 0;
        yawRate = 0;
        speed = 0;
        steerAngle = 0;
        rpm = 1000;
        gear = 1;
        
        throttle = brake = handbrake = clutch = 0;
        steerInput = 0;
        isDrifting = false;
        driftAngle = driftTimer = driftScore = currentDriftScore = 0;
        comboMultiplier = 1.0;
        
        // Vehicle specs (based on Nissan Silvia S15 / drift car)
        mass = 1300.0;
        inertia = 2500.0;
        wheelbase = 2.525;
        trackWidth = 1.48;
        cgToFront = 1.2;
        cgToRear = wheelbase - cgToFront;
        cgHeight = 0.45;
        wheelRadius = 0.31;
        
        staticWeightFront = cgToRear / wheelbase;  // ~52% front
        
        // Engine torque curve (Nm at different RPM points)
        engineTorque[0] = 0;
        engineTorque[1] = 180;   // 2000 RPM
        engineTorque[2] = 240;   // 4000 RPM
        engineTorque[3] = 280;   // 5000 RPM (peak)
        engineTorque[4] = 265;   // 6000 RPM
        engineTorque[5] = 230;   // 7000 RPM
        engineTorque[6] = 180;   // 8000 RPM
        
        // Gear ratios (6-speed + reverse)
        gearRatios[0] = -3.2;    // Reverse
        gearRatios[1] = 3.626;   // 1st
        gearRatios[2] = 2.188;   // 2nd
        gearRatios[3] = 1.541;   // 3rd
        gearRatios[4] = 1.213;   // 4th
        gearRatios[5] = 1.000;   // 5th
        gearRatios[6] = 0.794;   // 6th
        
        finalDrive = 4.083;
        
        // Initialize tires with sport tire characteristics
        PacejkaTireParams sportTire;
        sportTire.Bx = 12.0;
        sportTire.Cx = 1.65;
        sportTire.Dx = 1.1;
        sportTire.Ex = -0.3;
        sportTire.By = 9.5;
        sportTire.Cy = 1.3;
        sportTire.Dy = 1.0;
        sportTire.Ey = -0.1;
        
        tireFrontLeft.params = sportTire;
        tireFrontRight.params = sportTire;
        tireRearLeft.params = sportTire;
        tireRearRight.params = sportTire;
        
        // Rear tires slightly less grip for RWD drift setup
        tireRearLeft.params.Dy = 0.95;
        tireRearRight.params.Dy = 0.95;
    }
    
    double getTorque() {
        double rpmNorm = clamp((rpm - 1000) / 7000.0, 0.0, 1.0);
        int idx = (int)(rpmNorm * 5);
        double t = std::fmod(rpmNorm * 5, 1.0);
        return lerp(engineTorque[idx + 1], engineTorque[std::min(idx + 2, 6)], t);
    }
    
    void update(double dt, double roadCurve, double roadHill, double roadWidth, double steerInputValue) {
        steerInput = steerInputValue;
        
        // ============================================================
        // SIMULATION PHYSICS - Based on real vehicle dynamics
        // ============================================================
        
        // STEERING
        // Max steering angle - realistic range for sports car
        double maxSteerAngle = 35.0 * DEG_TO_RAD;
        double targetSteer = steerInput * maxSteerAngle;
        
        // Steering rate - quick response, faster for counter-steering
        double steerRate = isDrifting ? 12.0 : 8.0;
        steerAngle = lerp(steerAngle, targetSteer, dt * steerRate);
        
        // ACKERMANN STEERING - inner wheel turns more than outer
        double steerLeft, steerRight;
        if (std::abs(steerAngle) > 0.001) {
            double turnRadius = wheelbase / tan(std::abs(steerAngle));
            steerLeft = atan(wheelbase / (turnRadius - trackWidth / 2));
            steerRight = atan(wheelbase / (turnRadius + trackWidth / 2));
            if (steerAngle < 0) std::swap(steerLeft, steerRight);
            steerLeft *= (steerAngle > 0 ? 1 : -1);
            steerRight *= (steerAngle > 0 ? 1 : -1);
        } else {
            steerLeft = steerRight = 0;
        }
        
        // WEIGHT TRANSFER
        double gravity = 9.81;
        double totalWeight = mass * gravity;
        
        // Base weight distribution
        weightFront = totalWeight * staticWeightFront;
        weightRear = totalWeight * (1.0 - staticWeightFront);
        
        // Longitudinal weight transfer (acceleration/braking)
        double ax = (throttle - brake) * 8.0;  // Approximate longitudinal accel
        double longTransfer = (ax * cgHeight * mass) / wheelbase;
        weightFront -= longTransfer;
        weightRear += longTransfer;
        
        // Lateral weight transfer (cornering)
        double ay = vx > 1.0 ? (vy * yawRate + vx * yawRate) : 0;
        double latTransferFront = (ay * cgHeight * mass * 0.5) / trackWidth;
        double latTransferRear = (ay * cgHeight * mass * 0.5) / trackWidth;
        
        // Clamp weights to reasonable values
        weightFront = clamp(weightFront, totalWeight * 0.2, totalWeight * 0.8);
        weightRear = clamp(weightRear, totalWeight * 0.2, totalWeight * 0.8);
        
        // TIRE VELOCITIES (local to each wheel)
        // Front wheels
        double vxFL = vx;
        double vyFL = vy + yawRate * cgToFront;
        double vxFR = vx;
        double vyFR = vy + yawRate * cgToFront;
        
        // Rear wheels
        double vxRL = vx;
        double vyRL = vy - yawRate * cgToRear;
        double vxRR = vx;
        double vyRR = vy - yawRate * cgToRear;
        
        // Transform front wheel velocities to wheel frame (account for steering)
        double cosSteerL = cos(steerLeft);
        double sinSteerL = sin(steerLeft);
        double cosSteerR = cos(steerRight);
        double sinSteerR = sin(steerRight);
        
        double vxFL_wheel = vxFL * cosSteerL + vyFL * sinSteerL;
        double vyFL_wheel = -vxFL * sinSteerL + vyFL * cosSteerL;
        double vxFR_wheel = vxFR * cosSteerR + vyFR * sinSteerR;
        double vyFR_wheel = -vxFR * sinSteerR + vyFR * cosSteerR;
        
        // CALCULATE TIRE FORCES
        bool rearLocked = handbrake > 0.5;
        
        tireFrontLeft.calculate(vxFL_wheel, vyFL_wheel, weightFront / 2, false);
        tireFrontRight.calculate(vxFR_wheel, vyFR_wheel, weightFront / 2, false);
        tireRearLeft.calculate(vxRL, vyRL, weightRear / 2, rearLocked);
        tireRearRight.calculate(vxRR, vyRR, weightRear / 2, rearLocked);
        
        // ENGINE AND DRIVETRAIN (RWD)
        double gearRatio = gearRatios[gear];
        double driveRatio = std::abs(gearRatio * finalDrive);
        
        // Engine torque
        double engineTorqueNm = getTorque() * throttle;
        double wheelTorque = engineTorqueNm * driveRatio * 0.85;  // 85% drivetrain efficiency
        
        // Apply drive force to rear wheels
        double driveForce = wheelTorque / wheelRadius;
        
        // Brake force
        double brakeForce = brake * 15000.0;  // Strong brakes
        double brakeForceFront = brakeForce * 0.6;  // 60% front brake bias
        double brakeForceRear = brakeForce * 0.4;
        
        if (rearLocked) {
            brakeForceRear = weightRear * 1.0;  // Full lockup
        }
        
        // COMBINE FORCES
        // Front wheels - transform back from wheel frame to car frame
        double FxFL_car = tireFrontLeft.Fx * cosSteerL - tireFrontLeft.Fy * sinSteerL;
        double FyFL_car = tireFrontLeft.Fx * sinSteerL + tireFrontLeft.Fy * cosSteerL;
        double FxFR_car = tireFrontRight.Fx * cosSteerR - tireFrontRight.Fy * sinSteerR;
        double FyFR_car = tireFrontRight.Fx * sinSteerR + tireFrontRight.Fy * cosSteerR;
        
        // Rear wheels (no steering, already in car frame)
        double FxRL_car = tireRearLeft.Fx + driveForce / 2 - brakeForceRear / 2 * sign(vx);
        double FyRL_car = tireRearLeft.Fy;
        double FxRR_car = tireRearRight.Fx + driveForce / 2 - brakeForceRear / 2 * sign(vx);
        double FyRR_car = tireRearRight.Fy;
        
        // Front brake force
        FxFL_car -= brakeForceFront / 2 * sign(vx);
        FxFR_car -= brakeForceFront / 2 * sign(vx);
        
        // Total forces
        double totalFx = FxFL_car + FxFR_car + FxRL_car + FxRR_car;
        double totalFy = FyFL_car + FyFR_car + FyRL_car + FyRR_car;
        
        // Drag and rolling resistance
        double dragForce = 0.35 * vx * std::abs(vx);  // Quadratic air drag
        double rollingResist = 150.0 * sign(vx);
        totalFx -= dragForce + rollingResist;
        
        // YAW MOMENT (rotation around vertical axis)
        double yawMoment = 0;
        yawMoment += (FyFL_car + FyFR_car) * cgToFront;   // Front lateral forces
        yawMoment -= (FyRL_car + FyRR_car) * cgToRear;    // Rear lateral forces
        yawMoment += (FxFR_car - FxFL_car) * trackWidth / 2;  // Front longitudinal diff
        yawMoment += (FxRR_car - FxRL_car) * trackWidth / 2;  // Rear longitudinal diff
        
        // ROAD CURVE EFFECT
        // Road curvature adds a visual offset, not a physical force
        // The car should follow the road naturally through tire physics
        // Only add a gentle centering tendency
        double curveInfluence = roadCurve * vx * 0.5;
        heading += curveInfluence * dt;
        
        // INTEGRATE ACCELERATIONS
        double ax_car = totalFx / mass;
        double ay_car = totalFy / mass;
        double yawAccel = yawMoment / inertia;
        
        // Update velocities
        vx += ax_car * dt;
        vy += ay_car * dt;
        yawRate += yawAccel * dt;
        
        // Natural damping from tire scrub - only when not drifting
        if (!isDrifting) {
            yawRate *= 0.98;
        }
        
        // Speed limit and low-speed handling
        speed = sqrt(vx * vx + vy * vy);
        if (speed < 0.5 && throttle < 0.1 && brake < 0.1) {
            vx *= 0.95;
            vy *= 0.95;
            yawRate *= 0.9;
        }
        vx = clamp(vx, -10.0, 80.0);  // Max ~288 km/h forward
        vy = clamp(vy, -30.0, 30.0);  // Allow more lateral slide
        yawRate = clamp(yawRate, -4.0, 4.0);  // Allow faster rotation
        
        // UPDATE WORLD POSITION
        heading += yawRate * dt;
        
        // Transform velocity to world frame and integrate position
        double cosH = cos(heading);
        double sinH = sin(heading);
        double worldVx = vx * cosH - vy * sinH;
        double worldVy = vx * sinH + vy * cosH;
        
        z += worldVx * dt;
        x += worldVy * dt;  // Full lateral movement - no artificial scaling
        y = lerp(y, roadHill * 1000, dt * 2);
        
        // ROAD BOUNDARIES - based on actual road width in meters
        double roadHalfWidth = roadWidth * 5.0;  // roadWidth is normalized, scale to meters
        if (x > roadHalfWidth) {
            x = roadHalfWidth;
            vy = -std::abs(vy) * 0.3;  // Bounce back with energy loss
            vx *= 0.9;
        } else if (x < -roadHalfWidth) {
            x = -roadHalfWidth;
            vy = std::abs(vy) * 0.3;
            vx *= 0.9;
        }
        
        // RPM CALCULATION
        double wheelAngularVel = vx / wheelRadius;
        double targetRpm = std::abs(wheelAngularVel * driveRatio * 60 / (2 * PI));
        targetRpm = clamp(targetRpm, 1000.0, 8500.0);
        rpm = lerp(rpm, targetRpm, dt * 5);
        
        if (clutch > 0.5) {
            rpm = lerp(rpm, 1000 + throttle * 7500, dt * 3);
        }
        
        // Update wheel angular velocities for next frame
        tireFrontLeft.angularVel = vx / wheelRadius;
        tireFrontRight.angularVel = vx / wheelRadius;
        if (!rearLocked) {
            tireRearLeft.angularVel = vx / wheelRadius + (driveForce * dt) / (wheelRadius * 50);
            tireRearRight.angularVel = vx / wheelRadius + (driveForce * dt) / (wheelRadius * 50);
        } else {
            tireRearLeft.angularVel *= 0.9;
            tireRearRight.angularVel *= 0.9;
        }
        
        // DRIFT DETECTION
        bool wasDrifting = isDrifting;
        
        // Drift angle is the difference between car heading and velocity direction
        double velAngle = atan2(vy, std::max(vx, 1.0));
        driftAngle = velAngle * RAD_TO_DEG;
        
        // Consider drifting if slip angle is significant
        isDrifting = (std::abs(driftAngle) > 8.0 && speed > 10.0);
        
        // DRIFT SCORING
        if (isDrifting) {
            driftTimer += dt;
            double angleScore = std::min(std::abs(driftAngle), 60.0) / 60.0;
            double speedScore = std::min(speed * 3.6, 180.0) / 180.0;
            currentDriftScore += angleScore * speedScore * 100 * dt * comboMultiplier;
            comboMultiplier = std::min(comboMultiplier + dt * 0.3, 5.0);
        } else {
            if (wasDrifting && currentDriftScore > 100) {
                driftScore += currentDriftScore;
            }
            currentDriftScore = 0;
            driftTimer = 0;
            comboMultiplier = 1.0;
        }
        
        // AUTO GEAR
        if (gear > 0) {
            if (rpm > 7500 && gear < 6) gear++;
            else if (rpm < 3000 && gear > 1) gear--;
        }
    }
    
    double getSpeedKmh() const { return speed * 3.6; }
    
    // Helper function
    static double sign(double x) {
        return (x > 0) ? 1.0 : ((x < 0) ? -1.0 : 0.0);
    }
};

// ============================================================
// GAME CLASS
// ============================================================

class DriftStormOmega {
private:
    PlayerCar car;
    Road road;
    TrafficManager traffic;
    BitmapFont* font;
    Graphics* gfx;
    Input* input;
    
    double steerInput;  // Analog steering -1.0 to 1.0
    bool showTitle;
    
    // Pre-calculated z-buffer
    std::vector<double> zMap;
    double cameraHeight;
    
public:
    DriftStormOmega() : steerInput(0.0), showTitle(true), cameraHeight(1000) {
        gfx = getCore()->getGraphics();
        input = getCore()->getInput();
        font = new BitmapFont(gfx);
        
        // Pre-calculate z mapping - standard perspective projection
        zMap.resize(ROAD_VIEW_HEIGHT);
        for (int y = 0; y < ROAD_VIEW_HEIGHT; y++) {
            zMap[y] = 0;  // Will calculate in render based on horizon
        }
        
        car.gear = 1;
    }
    
    ~DriftStormOmega() {
        delete font;
    }
    
    void handleInput() {
        car.throttle = car.brake = car.handbrake = car.clutch = 0;
        double steerValue = 0;  // Analog steering -1.0 to 1.0
        
        if (input->isKeyDown(Key::W) || input->isKeyDown(Key::Up)) {
            car.throttle = 1.0;
        }
        if (input->isKeyDown(Key::S) || input->isKeyDown(Key::Down)) {
            car.brake = 1.0;
        }
        if (input->isKeyDown(Key::A) || input->isKeyDown(Key::Left)) {
            steerValue = -1.0;
        }
        if (input->isKeyDown(Key::D) || input->isKeyDown(Key::Right)) {
            steerValue = 1.0;
        }
        if (input->isKeyDown(Key::Space)) {
            car.handbrake = 1.0;
        }
        if (input->isKeyDown(Key::LShift) || input->isKeyDown(Key::RShift)) {
            car.clutch = 1.0;
            car.rpm = std::min(car.rpm + 500, 8500.0);
            // Clutch kick can also initiate drift - adds lateral velocity
            car.vy += car.steerInput * 2.0;
        }
        
        // Gamepad support - analog controls!
        if (input->isGamepadConnected()) {
            float accel = input->getRightTrigger();
            float brk = input->getLeftTrigger();
            Vec2 stick = input->getLeftStick();
            
            if (accel > 0.1) car.throttle = accel;
            if (brk > 0.1) car.brake = brk;
            
            // Analog steering from stick
            if (std::abs(stick.x) > 0.1) {
                steerValue = stick.x;  // Full analog range -1.0 to 1.0
            }
            
            if (input->isGamepadButtonDown(GamepadButton::A)) car.handbrake = 1.0;
            if (input->isGamepadButtonDown(GamepadButton::B)) {
                car.clutch = 1.0;
                car.rpm = std::min(car.rpm + 500, 8500.0);
                car.vy += car.steerInput * 2.0;
            }
        }
        
        steerInput = steerValue;
    }
    
    void update(double dt) {
        if (showTitle) {
            if (input->isAnyKeyPressed() || 
                (input->isGamepadConnected() && input->isGamepadButtonDown(GamepadButton::Start))) {
                showTitle = false;
            }
            return;
        }
        
        handleInput();
        
        int segIdx = road.getSegmentIndex(car.z);
        RoadSegment& seg = road.segments[segIdx];
        
        car.update(dt, seg.curve, seg.hill, seg.width, steerInput);
        
        // Traffic system (toggle with TRAFFIC_ENABLED)
        if (TRAFFIC_ENABLED) {
            traffic.update(car.z, car.speed, dt, road);
            
            // Forgiving collision
            auto nearbyCars = traffic.getCarsInRange(car.z, 30);
            for (auto* tCar : nearbyCars) {
                double dz = std::abs(tCar->z - car.z);
                double dx = std::abs(tCar->x - car.x);
                if (dz < 6 && dx < 0.25) {
                    car.speed *= 0.92;
                    if (car.x < tCar->x) car.x -= 0.1;
                    else car.x += 0.1;
                    tCar->x += (tCar->x > car.x) ? 0.15 : -0.15;
                    tCar->z += 15;
                }
            }
        }
        
        if (car.z > road.totalLength) {
            car.z -= road.totalLength;
        }
    }
    
    void render() {
        if (showTitle) {
            renderTitleScreen();
            return;
        }
        
        // Horizon at roughly 1/3 from top for chase camera view
        int horizonY = ROAD_VIEW_HEIGHT / 3;
        
        int segIdx = road.getSegmentIndex(car.z);
        const RoadSegment& currentSeg = road.segments[segIdx];
        
        // Sky gradient
        Color skyTop, skyBottom;
        if (currentSeg.type == 2) {
            skyTop = Color(10, 10, 20);
            skyBottom = Color(30, 30, 40);
        } else if (currentSeg.type == 1) {
            skyTop = Color(60, 80, 120);
            skyBottom = Color(100, 120, 160);
        } else {
            skyTop = Color(80, 130, 220);
            skyBottom = Color(150, 180, 230);
        }
        gfx->fillRectGradientV(0, 0, SCREEN_WIDTH, horizonY, skyTop, skyBottom);
        
        // Mountains for touge
        if (currentSeg.type == 1) {
            for (int x = 0; x < SCREEN_WIDTH; x += 3) {
                int mtnHeight = horizonY - 30 + (int)(std::sin(x * 0.02) * 20 + std::sin(x * 0.007) * 15);
                gfx->fillRect(x, mtnHeight, 3, horizonY - mtnHeight, Color(60, 70, 80));
            }
        }
        
        // Tunnel lights
        if (currentSeg.type == 2) {
            for (int i = 0; i < 20; i++) {
                int lx = (i * 67 + (int)(car.z * 0.1)) % SCREEN_WIDTH;
                gfx->fillCircle(lx, 30, 3, Color(255, 200, 100));
            }
        }
        
        // Road rendering - pseudo-3D perspective
        double baseSegmentZ = car.z;
        double accumulatedCurve = 0;
        double dx = 0;
        
        for (int y = horizonY; y < ROAD_VIEW_HEIGHT; y++) {
            // How far down from horizon (0 at horizon, 1 at bottom)
            double perspective = (double)(y - horizonY) / (ROAD_VIEW_HEIGHT - horizonY);
            
            // Z distance - closer at bottom of screen, far at horizon
            // Use exponential for more realistic perspective
            double z = 1.0 / (perspective + 0.01) * 10.0;
            
            if (z > 5000) continue;
            
            int segmentIndex = road.getSegmentIndex(baseSegmentZ + z);
            const RoadSegment& seg = road.segments[segmentIndex];
            
            accumulatedCurve += seg.curve * perspective * 0.8;
            dx += accumulatedCurve;
            
            // Road center - affected by curves and player X position
            double roadCenterX = SCREEN_WIDTH / 2.0 + dx * SCREEN_WIDTH * 0.4 - car.x * perspective * 300;
            
            // Road gets wider at bottom (closer), narrower at horizon
            double roadWidth = seg.width * SCREEN_WIDTH * 0.4 * perspective;
            roadWidth = std::max(roadWidth, 10.0);  // Minimum width
            
            int leftEdge = (int)(roadCenterX - roadWidth);
            int rightEdge = (int)(roadCenterX + roadWidth);
            
            bool isStripe = ((int)(baseSegmentZ + z) / (int)road.segmentLength) % 2 == 0;
            
            // Ground
            Color groundColor = (seg.type == 2) ? Color(20, 20, 25) : 
                               (isStripe ? GameColors::grass : GameColors::grassDark);
            gfx->drawLine(0, y, SCREEN_WIDTH, y, groundColor);
            
            // Road surface
            Color roadColor = (seg.type == 2) ? GameColors::roadTunnel :
                             (isStripe ? GameColors::roadLight : GameColors::roadDark);
            gfx->drawLine(leftEdge, y, rightEdge, y, roadColor);
            
            // Edge lines
            int edgeWidth = std::max(2, (int)(6 * (1 - perspective)));
            gfx->drawLine(leftEdge, y, leftEdge + edgeWidth, y, GameColors::lineWhite);
            gfx->drawLine(rightEdge - edgeWidth, y, rightEdge, y, GameColors::lineWhite);
            
            // Center line
            if (isStripe && seg.type != 2) {
                int center = (int)roadCenterX;
                gfx->drawLine(center - 2, y, center + 2, y, GameColors::lineYellow);
            }
            
            // Guardrails on touge
            if (seg.type == 1) {
                gfx->fillRect(leftEdge - 8, y, 6, 1, GameColors::guardrail);
                gfx->fillRect(rightEdge + 2, y, 6, 1, GameColors::guardrail);
            }
            
            // Tunnel walls
            if (seg.type == 2) {
                gfx->drawLine(0, y, leftEdge - 20, y, Color(40, 40, 50));
                gfx->drawLine(rightEdge + 20, y, SCREEN_WIDTH, y, Color(40, 40, 50));
                gfx->fillRect(leftEdge - 20, y, 15, 1, Color(80, 70, 50));
                gfx->fillRect(rightEdge + 5, y, 15, 1, Color(80, 70, 50));
            }
        }
        
        // Traffic rendering (toggle with TRAFFIC_ENABLED)
        if (TRAFFIC_ENABLED) {
            auto nearbyCars = traffic.getCarsInRange(car.z, 500);
            for (auto* tCar : nearbyCars) {
                double relZ = tCar->z - car.z;
                if (relZ < 20 || relZ > 400) continue;
                
                // Convert 3D position to screen position using same perspective as road
                double perspective = 10.0 / relZ;  // Matches road perspective
                int screenY = horizonY + (int)((ROAD_VIEW_HEIGHT - horizonY) * perspective);
                
                // X position relative to player, affected by road curve
                double relX = tCar->x - car.x;
                int screenX = SCREEN_WIDTH / 2 + (int)(relX * perspective * 300);
                
                if (screenY > horizonY + 10 && screenY < ROAD_VIEW_HEIGHT - 80) {
                    int carW = std::max(15, (int)(100 * perspective));
                    int carH = std::max(10, (int)(50 * perspective));
                    
                    // Car body
                    gfx->fillRect(screenX - carW/2, screenY - carH, carW, carH, tCar->color);
                    
                    // Windows
                    Color windowColor(150, 200, 255, 180);
                    gfx->fillRect(screenX - carW/3, screenY - carH + 3, carW * 2/3, carH/3, windowColor);
                    
                    // Taillights
                    if (relZ < 100) {
                        gfx->fillRect(screenX - carW/2, screenY - 4, 6, 4, Color(255, 50, 50));
                        gfx->fillRect(screenX + carW/2 - 6, screenY - 4, 6, 4, Color(255, 50, 50));
                    }
                }
            }
        }
        
        // ============================================================
        // PLAYER CAR (Chase camera view - centered at bottom of road)
        // ============================================================
        {
            // Player car is always centered horizontally, sits at bottom of road view
            int carCenterX = SCREEN_WIDTH / 2;
            int carBaseY = ROAD_VIEW_HEIGHT - 10;  // Bottom of road view
            
            // Apply visual steering offset (car sways when steering)
            int steerOffset = (int)(car.steerAngle * 20);
            carCenterX += steerOffset;
            
            // Car dimensions - large since it's closest to camera
            int carWidth = 140;
            int carHeight = 70;
            int carTopY = carBaseY - carHeight;
            
            // Drift visual rotation - skew the car when drifting
            int driftSkew = (int)(car.driftAngle * 1.0);
            
            // Car shadow
            gfx->fillRect(carCenterX - carWidth/2 + 8 + driftSkew/2, carBaseY - 3, carWidth, 8, Color(0, 0, 0, 80));
            
            // Main car body (red sports car)
            Color carBodyColor(200, 40, 40);
            Color carBodyDark(140, 25, 25);
            Color carBodyHighlight(255, 80, 80);
            
            // Rear of car (trapezoid shape - wider at bottom)
            int rearWidthBottom = carWidth;
            int rearWidthTop = carWidth - 30;
            
            // Draw car body from bottom to top with drift skew
            for (int row = 0; row < carHeight; row++) {
                double t = (double)row / carHeight;
                int rowWidth = (int)(rearWidthBottom - (rearWidthBottom - rearWidthTop) * t * 0.7);
                int rowY = carBaseY - row;
                int skewAtRow = (int)(driftSkew * (1.0 - t * 0.5));  // More skew at bottom
                int rowX = carCenterX - rowWidth/2 + skewAtRow;
                
                Color rowColor = (row < 10) ? carBodyDark : 
                                (row > carHeight - 15) ? carBodyHighlight : carBodyColor;
                gfx->drawLine(rowX, rowY, rowX + rowWidth, rowY, rowColor);
            }
            
            // Rear windshield (dark glass)
            int windowTop = carTopY + 5;
            int windowHeight = 25;
            int windowWidth = carWidth - 50;
            for (int row = 0; row < windowHeight; row++) {
                double t = (double)row / windowHeight;
                int rowWidth = (int)(windowWidth * (0.6 + 0.4 * t));
                int rowY = windowTop + row;
                int skewAtRow = (int)(driftSkew * 0.3);
                int rowX = carCenterX - rowWidth/2 + skewAtRow;
                gfx->drawLine(rowX, rowY, rowX + rowWidth, rowY, Color(30, 40, 50, 200));
            }
            
            // Rear spoiler
            int spoilerY = carTopY + 2;
            int spoilerWidth = carWidth - 20;
            gfx->fillRect(carCenterX - spoilerWidth/2 + (int)(driftSkew * 0.4), spoilerY, spoilerWidth, 4, Color(40, 40, 40));
            
            // Taillights (glow when braking)
            Color tailLightColor = (car.brake > 0.1) ? Color(255, 50, 50) : Color(150, 30, 30);
            int tailY = carBaseY - 15;
            gfx->fillRect(carCenterX - carWidth/2 + 5 + driftSkew, tailY, 15, 8, tailLightColor);
            gfx->fillRect(carCenterX + carWidth/2 - 20 + driftSkew, tailY, 15, 8, tailLightColor);
            
            // Wheels (visible from behind, with spin effect)
            int wheelWidth = 18;
            int wheelHeight = 20;
            int wheelY = carBaseY - wheelHeight + 5;
            
            // Left wheel
            int leftWheelX = carCenterX - carWidth/2 + 8 + driftSkew;
            gfx->fillRect(leftWheelX, wheelY, wheelWidth, wheelHeight, Color(30, 30, 30));
            // Wheel highlights based on speed (spinning effect)
            if ((int)(car.z * 0.5) % 4 < 2) {
                gfx->fillRect(leftWheelX + 2, wheelY + 3, wheelWidth - 4, 3, Color(60, 60, 60));
            }
            
            // Right wheel  
            int rightWheelX = carCenterX + carWidth/2 - wheelWidth - 8 + driftSkew;
            gfx->fillRect(rightWheelX, wheelY, wheelWidth, wheelHeight, Color(30, 30, 30));
            if ((int)(car.z * 0.5) % 4 >= 2) {
                gfx->fillRect(rightWheelX + 2, wheelY + 3, wheelWidth - 4, 3, Color(60, 60, 60));
            }
            
            // Drift smoke effect
            if (car.isDrifting && std::abs(car.driftAngle) > 10) {
                int smokeAlpha = std::min(150, (int)(std::abs(car.driftAngle) * 3));
                for (int i = 0; i < 5; i++) {
                    int smokeX = (car.driftAngle > 0) ? rightWheelX : leftWheelX;
                    smokeX += (rand() % 30) - 15;
                    int smokeY = carBaseY + i * 3;
                    int smokeSize = 8 + i * 3;
                    gfx->fillCircle(smokeX, smokeY, smokeSize, Color(200, 200, 200, smokeAlpha - i * 25));
                }
            }
        }
        
        // HUD background
        gfx->fillRect(0, ROAD_VIEW_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - ROAD_VIEW_HEIGHT, GameColors::hudBg);
        
        renderHUD();
        
        gfx->present();
    }
    
    void renderHUD() {
        int hudY = ROAD_VIEW_HEIGHT + 10;
        
        // Speed
        int speedKmh = (int)car.getSpeedKmh();
        font->drawText(30, hudY, std::to_string(speedKmh), GameColors::hudSpeed, 4);
        font->drawText(120, hudY + 20, "km/h", Color(150, 150, 150), 2);
        
        // Gear
        std::string gearStr = car.gear == 0 ? "N" : std::to_string(car.gear);
        font->drawText(30, hudY + 50, "GEAR", Color(150, 150, 150), 1);
        font->drawText(80, hudY + 45, gearStr, Color(100, 200, 255), 3);
        
        // RPM bar
        double rpmNorm = (car.rpm - 1000) / 7500;
        int rpmBarWidth = (int)(rpmNorm * 150);
        gfx->fillRect(30, hudY + 75, 150, 15, Color(40, 40, 50));
        
        Color rpmColor = rpmNorm < 0.7 ? GameColors::hudRpmLow : 
                        (rpmNorm < 0.85 ? GameColors::hudRpmMid : GameColors::hudRpmHigh);
        gfx->fillRect(30, hudY + 75, rpmBarWidth, 15, rpmColor);
        gfx->drawRect(30, hudY + 75, 150, 15, Color(100, 100, 100));
        
        // Drift info (center)
        if (car.isDrifting || car.currentDriftScore > 0) {
            std::stringstream angleSS;
            angleSS << std::fixed << std::setprecision(0) << std::abs(car.driftAngle) << " DEG";
            font->drawTextCentered(hudY, angleSS.str(), GameColors::hudHighlight, SCREEN_WIDTH, 3);
            
            if (car.currentDriftScore > 0) {
                std::stringstream scoreSS;
                scoreSS << "DRIFT: " << (int)car.currentDriftScore;
                font->drawTextCentered(hudY + 30, scoreSS.str(), GameColors::hudHighlight, SCREEN_WIDTH, 2);
                
                if (car.comboMultiplier > 1.1) {
                    std::stringstream comboSS;
                    comboSS << "x" << std::fixed << std::setprecision(1) << car.comboMultiplier;
                    font->drawTextCentered(hudY + 50, comboSS.str(), Color(255, 100, 255), 2);
                }
            }
            
            std::string dirStr = car.driftAngle > 0 ? "<< LEFT" : "RIGHT >>";
            font->drawTextCentered(hudY + 70, dirStr, Color(100, 200, 255), 2);
        }
        
        // Score and zone (right)
        font->drawText(SCREEN_WIDTH - 180, hudY, "SCORE", Color(150, 150, 150), 1);
        font->drawText(SCREEN_WIDTH - 180, hudY + 12, std::to_string((int)car.driftScore), Color(100, 255, 100), 2);
        
        int segIdx = road.getSegmentIndex(car.z);
        std::string zoneName;
        Color zoneColor;
        switch (road.segments[segIdx].type) {
            case 0: zoneName = "HIGHWAY"; zoneColor = Color(100, 150, 255); break;
            case 1: zoneName = "TOUGE"; zoneColor = Color(100, 255, 100); break;
            case 2: zoneName = "TUNNEL"; zoneColor = Color(255, 200, 100); break;
            default: zoneName = "UNKNOWN"; zoneColor = Color(150, 150, 150);
        }
        font->drawText(SCREEN_WIDTH - 180, hudY + 40, "ZONE", Color(150, 150, 150), 1);
        font->drawText(SCREEN_WIDTH - 180, hudY + 52, zoneName, zoneColor, 2);
        
        // Distance
        std::stringstream distSS;
        distSS << std::fixed << std::setprecision(1) << (car.z / 1000.0) << " km";
        font->drawText(SCREEN_WIDTH - 180, hudY + 75, distSS.str(), Color(150, 150, 150), 1);
        
        // Controls hint
        font->drawTextCentered(SCREEN_HEIGHT - 15, "WASD: Drive | SPACE: Handbrake | SHIFT: Clutch Kick | ESC: Quit", 
                              Color(80, 80, 80), SCREEN_WIDTH, 1);
    }
    
    void renderTitleScreen() {
        gfx->clear(Color(10, 15, 30));
        
        // Title gradient background
        gfx->fillRectGradientV(0, 150, SCREEN_WIDTH, 200, Color(30, 50, 100), Color(10, 15, 30));
        
        // Title
        font->drawTextCentered(180, "DRIFT STORM", Color(100, 200, 255), SCREEN_WIDTH, 6);
        font->drawTextCentered(250, "OMEGA", Color(255, 200, 100), SCREEN_WIDTH, 4);
        
        font->drawTextCentered(320, "A First-Person Drift Racing Experience", Color(200, 200, 200), SCREEN_WIDTH, 2);
        font->drawTextCentered(350, "Powered by Omega Engine", Color(150, 150, 150), SCREEN_WIDTH, 1);
        
        // Controls
        font->drawTextCentered(420, "--- CONTROLS ---", Color(100, 255, 100), SCREEN_WIDTH, 2);
        font->drawTextCentered(460, "W/S or Triggers - Accelerate/Brake", Color(180, 180, 180), SCREEN_WIDTH, 1);
        font->drawTextCentered(480, "A/D or Left Stick - Steer", Color(180, 180, 180), SCREEN_WIDTH, 1);
        font->drawTextCentered(500, "SPACE or A Button - Handbrake", Color(180, 180, 180), SCREEN_WIDTH, 1);
        font->drawTextCentered(520, "SHIFT or B Button - Clutch Kick", Color(180, 180, 180), SCREEN_WIDTH, 1);
        
        font->drawTextCentered(570, "--- HOW TO DRIFT ---", Color(255, 100, 255), SCREEN_WIDTH, 2);
        font->drawTextCentered(600, "Enter corners at speed, tap handbrake or clutch kick,", Color(180, 180, 180), SCREEN_WIDTH, 1);
        font->drawTextCentered(620, "then countersteer to maintain the slide!", Color(180, 180, 180), SCREEN_WIDTH, 1);
        
        font->drawTextCentered(680, "Press any key or START to begin...", Color(255, 220, 100), SCREEN_WIDTH, 2);
        
        gfx->present();
    }
};

// ============================================================
// MAIN
// ============================================================

DriftStormOmega* game = nullptr;

void gameUpdate(double dt) {
    game->update(dt);
}

void gameRender() {
    game->render();
}

int main(int argc, char* argv[]) {
    EngineConfig config;
    config.windowWidth = SCREEN_WIDTH;
    config.windowHeight = SCREEN_HEIGHT;
    config.windowTitle = "Drift Storm Omega";
    config.vsync = true;
    
    if (!getCore()->init(config)) {
        return 1;
    }
    
    game = new DriftStormOmega();
    
    getCore()->run(gameUpdate, gameRender);
    
    delete game;
    Core::destroyInstance();
    
    return 0;
}
