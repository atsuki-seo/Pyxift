#ifndef PYXIFT_CORE_AUDIO_MIXER_HPP
#define PYXIFT_CORE_AUDIO_MIXER_HPP

#include "Sound.hpp"
#include "Tone.hpp"

#include <array>
#include <cstdint>
#include <mutex>

namespace pyxift {

inline constexpr int32_t kAudioSampleRate = 22050;
inline constexpr int32_t kNumChannels = 4;
inline constexpr int32_t kNumTones = 4;
inline constexpr int32_t kNumSounds = 64;
inline constexpr int32_t kSoundTicksPerSecond = 120;
inline constexpr float kMasterGain = 0.125f;

struct ChannelState {
    bool playing = false;
    bool loop = false;
    Sound sound{};
    int32_t note_index = -1;
    int32_t ticks_in_note = 0;
    int32_t ticks_per_note = 30;
    float phase = 0.0f;
    uint16_t lfsr = 0x0201;
    int8_t current_note = kRestNote;
    int8_t prev_note = kRestNote;
    uint8_t current_tone = 0;
    uint8_t current_volume = 0;
    uint8_t current_effect = 0;
};

class AudioMixer {
public:
    AudioMixer();

    void set_tone(int32_t index, const Tone &tone);
    Tone get_tone(int32_t index) const;

    void set_sound(int32_t index, const Sound &sound);
    Sound get_sound(int32_t index) const;

    void play(int32_t channel, int32_t sound_index, bool loop);
    void stop(int32_t channel);
    void stop_all();

    bool is_playing(int32_t channel) const;

    void render(int16_t *out, int32_t frame_count);

private:
    void start_note_locked(ChannelState &ch, int32_t note_pos);
    float synth_sample_locked(ChannelState &ch);
    static float midi_freq(int32_t midi_note);

    mutable std::mutex mutex_;
    std::array<Tone, kNumTones> tones_;
    std::array<Sound, kNumSounds> sounds_{};
    std::array<ChannelState, kNumChannels> channels_{};
    int32_t samples_per_tick_ = 0;
    uint32_t frame_counter_ = 0;
};

} // namespace pyxift

#endif
