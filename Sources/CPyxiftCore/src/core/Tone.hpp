#ifndef PYXIFT_CORE_TONE_HPP
#define PYXIFT_CORE_TONE_HPP

// Source: kitao/pyxel crates/pyxel-core/src/settings.rs
//   (DEFAULT_TONE_TRIANGLE / DEFAULT_TONE_SQUARE / DEFAULT_TONE_PULSE / DEFAULT_TONE_NOISE)
// License: MIT (Copyright (c) 2018-2026 Takashi Kitao)

#include <array>
#include <cstdint>
#include <vector>

namespace pyxift {

enum class ToneMode : uint8_t {
    Wavetable = 0,
    Noise = 1,
};

struct Tone {
    ToneMode mode = ToneMode::Wavetable;
    uint32_t sample_bits = 4;
    std::vector<uint8_t> wavetable;
    float gain = 1.0f;

    float sample_at(uint32_t index) const {
        if (wavetable.empty()) return 0.0f;
        const float max_sample = static_cast<float>((1u << sample_bits) - 1u);
        const float raw = static_cast<float>(wavetable[index % wavetable.size()]);
        return (raw / max_sample) * 2.0f - 1.0f;
    }

    uint32_t length() const { return static_cast<uint32_t>(wavetable.size()); }
};

inline Tone make_default_triangle() {
    return Tone{
        ToneMode::Wavetable,
        4,
        {8, 9, 10, 11, 12, 13, 14, 15, 15, 14, 13, 12, 11, 10, 9, 8,
         7, 6,  5,  4,  3,  2,  1,  0,  0,  1,  2,  3,  4,  5, 6, 7},
        1.0f,
    };
}

inline Tone make_default_square() {
    return Tone{ToneMode::Wavetable, 1, {1, 0}, 0.3f};
}

inline Tone make_default_pulse() {
    return Tone{ToneMode::Wavetable, 1, {1, 0, 0, 0}, 0.3f};
}

inline Tone make_default_noise() {
    return Tone{ToneMode::Noise, 0, {}, 0.6f};
}

inline std::array<Tone, 4> make_default_tones() {
    return {make_default_triangle(), make_default_square(), make_default_pulse(), make_default_noise()};
}

} // namespace pyxift

#endif
