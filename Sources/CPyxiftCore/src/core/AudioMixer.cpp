#include "AudioMixer.hpp"

#include <algorithm>
#include <cmath>

namespace pyxift {

namespace {

constexpr float kPi = 3.14159265358979323846f;

// 15-bit LFSR with taps at bits 0 and 1; mask 0x7FFF.
// The pre-advanced seed 0x0201 is taken from upstream Pyxel and reproduces its noise texture.
uint16_t advance_lfsr(uint16_t state) {
    const uint16_t bit = ((state ^ (state >> 1)) & 1u);
    return static_cast<uint16_t>((state >> 1) | (bit << 14)) & 0x7FFFu;
}

} // namespace

AudioMixer::AudioMixer() : tones_(make_default_tones()) {
    samples_per_tick_ = kAudioSampleRate / kSoundTicksPerSecond;
}

void AudioMixer::set_tone(int32_t index, const Tone &tone) {
    if (index < 0 || index >= kNumTones) return;
    std::lock_guard<std::mutex> lock(mutex_);
    tones_[index] = tone;
}

Tone AudioMixer::get_tone(int32_t index) const {
    if (index < 0 || index >= kNumTones) return Tone{};
    std::lock_guard<std::mutex> lock(mutex_);
    return tones_[index];
}

void AudioMixer::set_sound(int32_t index, const Sound &sound) {
    if (index < 0 || index >= kNumSounds) return;
    std::lock_guard<std::mutex> lock(mutex_);
    sounds_[index] = sound;
}

Sound AudioMixer::get_sound(int32_t index) const {
    if (index < 0 || index >= kNumSounds) return Sound{};
    std::lock_guard<std::mutex> lock(mutex_);
    return sounds_[index];
}

void AudioMixer::play(int32_t channel, int32_t sound_index, bool loop) {
    if (channel < 0 || channel >= kNumChannels) return;
    if (sound_index < 0 || sound_index >= kNumSounds) return;
    std::lock_guard<std::mutex> lock(mutex_);
    auto &ch = channels_[channel];
    ch.sound = sounds_[sound_index];
    if (ch.sound.empty()) {
        ch.playing = false;
        return;
    }
    ch.loop = loop;
    ch.note_index = 0;
    ch.ticks_in_note = 0;
    ch.ticks_per_note = ch.sound.speed > 0 ? ch.sound.speed : 1;
    ch.phase = 0.0f;
    ch.lfsr = 0x0201;
    ch.prev_note = kRestNote;
    ch.playing = true;
    start_note_locked(ch, 0);
}

void AudioMixer::stop(int32_t channel) {
    if (channel < 0 || channel >= kNumChannels) return;
    std::lock_guard<std::mutex> lock(mutex_);
    channels_[channel].playing = false;
}

void AudioMixer::stop_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &ch : channels_) ch.playing = false;
}

bool AudioMixer::is_playing(int32_t channel) const {
    if (channel < 0 || channel >= kNumChannels) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return channels_[channel].playing;
}

void AudioMixer::start_note_locked(ChannelState &ch, int32_t note_pos) {
    const auto &snd = ch.sound;
    if (note_pos < 0 || note_pos >= static_cast<int32_t>(snd.notes.size())) {
        ch.playing = false;
        return;
    }
    ch.prev_note = ch.current_note;
    ch.current_note = snd.notes[note_pos];
    ch.current_tone = snd.tones.empty()
        ? 0
        : snd.tones[static_cast<size_t>(note_pos) % snd.tones.size()];
    ch.current_volume = snd.volumes.empty()
        ? kMaxVolume
        : snd.volumes[static_cast<size_t>(note_pos) % snd.volumes.size()];
    ch.current_effect = snd.effects.empty()
        ? 0
        : snd.effects[static_cast<size_t>(note_pos) % snd.effects.size()];
    ch.ticks_in_note = 0;
    if (ch.current_tone < kNumTones && tones_[ch.current_tone].mode == ToneMode::Wavetable) {
        ch.phase = 0.0f;
    }
}

float AudioMixer::synth_sample_locked(ChannelState &ch) {
    if (ch.current_note < 0 || ch.current_volume == 0) {
        return 0.0f;
    }
    if (ch.current_tone >= kNumTones) {
        return 0.0f;
    }

    const Tone &tone = tones_[ch.current_tone];

    // Pyxel's legacy `Sound` API documents note 33 = A2 = 440 Hz, so we anchor frequency on that.
    float freq = 440.0f * std::pow(2.0f, static_cast<float>(ch.current_note - 33) / 12.0f);

    const float t01 = ch.ticks_per_note > 0
        ? static_cast<float>(ch.ticks_in_note) / static_cast<float>(ch.ticks_per_note)
        : 0.0f;

    const Effect eff = static_cast<Effect>(ch.current_effect);

    if (eff == Effect::Slide && ch.prev_note >= 0) {
        const float prev_freq = 440.0f * std::pow(2.0f, static_cast<float>(ch.prev_note - 33) / 12.0f);
        freq = prev_freq + (freq - prev_freq) * t01;
    } else if (eff == Effect::Vibrato) {
        const float vib = std::sin(2.0f * kPi * 6.0f * static_cast<float>(frame_counter_) /
                                    static_cast<float>(kAudioSampleRate));
        freq *= std::pow(2.0f, (25.0f / 1200.0f) * vib);
    }

    float volume_scale = static_cast<float>(ch.current_volume) / static_cast<float>(kMaxVolume);
    if (eff == Effect::Fadeout) {
        volume_scale *= std::max(0.0f, 1.0f - t01);
    } else if (eff == Effect::HalfFadeout) {
        volume_scale *= std::max(0.0f, 1.0f - std::max(0.0f, (t01 - 0.5f) * 2.0f));
    } else if (eff == Effect::QuarterFadeout) {
        volume_scale *= std::max(0.0f, 1.0f - std::max(0.0f, (t01 - 0.75f) * 4.0f));
    }

    float raw = 0.0f;
    if (tone.mode == ToneMode::Noise) {
        const float advance_per_sample = freq / 110.0f;
        ch.phase += advance_per_sample;
        while (ch.phase >= 1.0f) {
            ch.lfsr = advance_lfsr(ch.lfsr);
            ch.phase -= 1.0f;
        }
        raw = (ch.lfsr & 1u) ? 1.0f : -1.0f;
    } else {
        const uint32_t length = tone.length();
        if (length == 0) return 0.0f;
        ch.phase += freq * static_cast<float>(length) / static_cast<float>(kAudioSampleRate);
        if (ch.phase >= static_cast<float>(length)) {
            ch.phase = std::fmod(ch.phase, static_cast<float>(length));
        }
        const uint32_t idx = static_cast<uint32_t>(ch.phase) % length;
        raw = tone.sample_at(idx);
    }

    return raw * tone.gain * volume_scale;
}

void AudioMixer::render(int16_t *out, int32_t frame_count) {
    std::lock_guard<std::mutex> lock(mutex_);

    for (int32_t i = 0; i < frame_count; ++i) {
        float mix = 0.0f;
        for (auto &ch : channels_) {
            if (!ch.playing) continue;
            mix += synth_sample_locked(ch);
        }

        if (samples_per_tick_ > 0 && (frame_counter_ + 1) % static_cast<uint32_t>(samples_per_tick_) == 0) {
            for (auto &ch : channels_) {
                if (!ch.playing) continue;
                ch.ticks_in_note += 1;
                if (ch.ticks_in_note >= ch.ticks_per_note) {
                    int32_t next = ch.note_index + 1;
                    if (next >= static_cast<int32_t>(ch.sound.notes.size())) {
                        if (ch.loop) {
                            next = 0;
                        } else {
                            ch.playing = false;
                            continue;
                        }
                    }
                    ch.note_index = next;
                    start_note_locked(ch, next);
                }
            }
        }

        ++frame_counter_;

        const float scaled = mix * kMasterGain;
        const float clamped = std::clamp(scaled, -1.0f, 1.0f);
        out[i] = static_cast<int16_t>(clamped * 32767.0f);
    }
}

} // namespace pyxift
