#pragma once
// =============================================================================
// Adaptive Sampling Policy
// Adjusts read interval based on current boiler temperature.
// =============================================================================

#include <cstdint>
#include "config.h"

namespace sampling {

/// Compute the next sampling interval (in seconds) based on current temperature.
/// - temp < TEMP_LOW_THRESHOLD: boiler idle → slow polling (SAMPLE_INTERVAL_IDLE_S)
/// - temp >= TEMP_HIGH_THRESHOLD: boiler active → fast polling (SAMPLE_INTERVAL_ACTIVE_S)
/// - between thresholds: linear interpolation
/// In DEBUG_MODE, uses shorter intervals for quick testing.
inline uint32_t computeIntervalS(float tempC) {
    // In debug mode, use fixed short interval for quick testing
    if constexpr (config::DEBUG_ENABLED) {
        return config::DEBUG_SAMPLE_INTERVAL_S;
    }

    if (tempC < config::TEMP_LOW_THRESHOLD) {
        return config::SAMPLE_INTERVAL_IDLE_S;
    }
    if (tempC >= config::TEMP_HIGH_THRESHOLD) {
        return config::SAMPLE_INTERVAL_ACTIVE_S;
    }

    // Linear interpolation between idle and active intervals
    const float range = config::TEMP_HIGH_THRESHOLD - config::TEMP_LOW_THRESHOLD;
    const float ratio = (tempC - config::TEMP_LOW_THRESHOLD) / range;  // 0.0 → 1.0

    const float intervalRange = static_cast<float>(config::SAMPLE_INTERVAL_IDLE_S)
                              - static_cast<float>(config::SAMPLE_INTERVAL_ACTIVE_S);

    const float interval = config::SAMPLE_INTERVAL_IDLE_S - (ratio * intervalRange);
    return static_cast<uint32_t>(interval);
}

/// State tracker with hysteresis to avoid thrashing between idle/active modes.
class SamplingPolicy {
public:
    enum class Mode { Idle, Active };

    SamplingPolicy() = default;

    /// Update internal mode based on new temperature reading.
    /// Returns the recommended sampling interval in seconds.
    uint32_t update(float tempC) {
        switch (mode_) {
            case Mode::Idle:
                // Transition to Active when temp exceeds low threshold + hysteresis
                if (tempC >= config::TEMP_LOW_THRESHOLD + config::TEMP_HYSTERESIS) {
                    mode_ = Mode::Active;
                }
                break;

            case Mode::Active:
                // Transition back to Idle when temp falls below low threshold - hysteresis
                if (tempC < config::TEMP_LOW_THRESHOLD - config::TEMP_HYSTERESIS) {
                    mode_ = Mode::Idle;
                }
                break;
        }

        return computeIntervalS(tempC);
    }

    Mode currentMode() const { return mode_; }

private:
    Mode mode_ = Mode::Idle;
};

}  // namespace sampling
