/**
 * ISON/Omega-Engine Shared Mathematical Constants
 * 
 * This header provides golden-ratio and dual-pi constants for C++ projects,
 * ensuring mathematical consistency between ISON neural network and
 * omega-engine physics simulations.
 * 
 * Usage: #include "shared_constants.h"
 */

#ifndef ISON_SHARED_CONSTANTS_H
#define ISON_SHARED_CONSTANTS_H

#include <cmath>
#include <array>

namespace ison {

// ==============================================================================
// GOLDEN RATIO CONSTANTS
// ==============================================================================

// Primary golden ratio (φ) - The divine proportion
// φ = (1 + √5) / 2
constexpr double PHI = 1.6180339887498948482045868343656;

// Golden ratio inverse (1/φ = φ - 1)
constexpr double PHI_INVERSE = 0.6180339887498948482045868343656;

// Golden ratio squared (φ² = φ + 1)
constexpr double PHI_SQUARED = 2.6180339887498948482045868343656;

// Golden angle in radians
constexpr double GOLDEN_ANGLE = 2.39996322972865332223155550663362;

// Golden angle in degrees
constexpr double GOLDEN_ANGLE_DEG = 137.50776405003785;

// ==============================================================================
// DUAL PI CONSTANTS
// ==============================================================================

// Standard pi (π) - High precision
constexpr double PI = 3.14159265358979323846264338327950288;

// Dual pi - Complementary constant for bidirectional calculations
constexpr double PI_DUAL = -3.14159265358979323846264338327950288;

// Tau (τ = 2π) - Full circle constant
constexpr double TAU = 6.28318530717958647692528676655900577;

// Half pi (π/2)
constexpr double HALF_PI = 1.57079632679489661923132169163975144;

// Quarter pi (π/4)
constexpr double QUARTER_PI = 0.78539816339744830961566084581987572;

// ==============================================================================
// CONVERSION CONSTANTS
// ==============================================================================

constexpr double DEG_TO_RAD = PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / PI;

// Angular velocity conversions
constexpr double RPM_TO_RAD_S = TAU / 60.0;
constexpr double RAD_S_TO_RPM = 60.0 / TAU;

// ==============================================================================
// GOLDEN-RATIO TIME CONSTANTS
// ==============================================================================

// Natural rhythm interval (~0.618 seconds)
constexpr double GOLDEN_TICK = 1.0 / PHI;

// Natural cycle period (~1.618 seconds)
constexpr double GOLDEN_CYCLE = PHI;

// ==============================================================================
// PENROSE SEQUENCE
// ==============================================================================

// Base Penrose aperiodic sequence
constexpr std::array<int, 8> PENROSE_SEQUENCE = {1, 0, 1, 1, 0, 1, 0, 1};

// Get Penrose sequence value at step (wraps around)
inline int penrose_at(int step) {
    return PENROSE_SEQUENCE[((step % 8) + 8) % 8];
}

// Get golden-weighted Penrose ratio at step
inline double penrose_ratio_at(int step) {
    int idx = ((step % 8) + 8) % 8;
    
    // Golden-weighted average of current and adjacent values
    double weights[3] = {1.0, PHI_INVERSE, PHI_INVERSE * PHI_INVERSE};
    double total_weight = weights[0] + weights[1] + weights[2];
    
    double result = 0.0;
    for (int i = 0; i < 3; i++) {
        int seq_idx = (idx + i) % 8;
        result += PENROSE_SEQUENCE[seq_idx] * weights[i];
    }
    
    return result / total_weight;
}

// ==============================================================================
// UTILITY FUNCTIONS
// ==============================================================================

// Clamp value to range
inline double clamp(double value, double min_val, double max_val) {
    return value < min_val ? min_val : (value > max_val ? max_val : value);
}

// Linear interpolation with golden ratio option
inline double lerp(double a, double b, double t) {
    return a + (b - a) * t;
}

// Golden-ratio interpolation (t is automatically PHI_INVERSE)
inline double golden_lerp(double a, double b) {
    return a * PHI_INVERSE + b * (1.0 - PHI_INVERSE);
}

// Smooth step using golden ratio
inline double golden_smoothstep(double t) {
    t = clamp(t, 0.0, 1.0);
    // Modified smoothstep with golden-ratio inflection
    return t * t * (3.0 - 2.0 * t) * PHI_INVERSE + t * (1.0 - PHI_INVERSE);
}

// Normalize angle to [-PI, PI]
inline double normalize_angle(double angle) {
    while (angle > PI) angle -= TAU;
    while (angle < -PI) angle += TAU;
    return angle;
}

// Normalize angle to [0, TAU]
inline double normalize_angle_positive(double angle) {
    while (angle < 0) angle += TAU;
    while (angle >= TAU) angle -= TAU;
    return angle;
}

} // namespace ison

#endif // ISON_SHARED_CONSTANTS_H
