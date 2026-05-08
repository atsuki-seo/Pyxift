#include "pyxift_audio_test_c.h"

#include "core/AudioMixer.hpp"
#include "core/Music.hpp"
#include "core/Sound.hpp"

#include <cstring>
#include <vector>

namespace {

template <class T>
int32_t copy_to_buf(const std::vector<T> &v, T *out_buf, int32_t capacity) {
    if (out_buf != nullptr && capacity > 0) {
        const int32_t n = static_cast<int32_t>(v.size());
        const int32_t to_copy = n < capacity ? n : capacity;
        for (int32_t i = 0; i < to_copy; ++i) out_buf[i] = v[i];
    }
    return static_cast<int32_t>(v.size());
}

} // namespace

struct PyxiftAudioMixerHandle {
    pyxift::AudioMixer mixer;
};

extern "C" {

int32_t pyxift_audio_sound_parse_notes(const char *s, int8_t *out_buf, int32_t capacity) {
    if (s == nullptr) return 0;
    const auto v = pyxift::Sound::parse_notes(s);
    return copy_to_buf(v, out_buf, capacity);
}

int32_t pyxift_audio_sound_parse_tones(const char *s, uint8_t *out_buf, int32_t capacity) {
    if (s == nullptr) return 0;
    const auto v = pyxift::Sound::parse_tones(s);
    return copy_to_buf(v, out_buf, capacity);
}

int32_t pyxift_audio_sound_parse_volumes(const char *s, uint8_t *out_buf, int32_t capacity) {
    if (s == nullptr) return 0;
    const auto v = pyxift::Sound::parse_volumes(s);
    return copy_to_buf(v, out_buf, capacity);
}

int32_t pyxift_audio_sound_parse_effects(const char *s, uint8_t *out_buf, int32_t capacity) {
    if (s == nullptr) return 0;
    const auto v = pyxift::Sound::parse_effects(s);
    return copy_to_buf(v, out_buf, capacity);
}

PyxiftAudioMixerHandle *pyxift_audio_mixer_create(void) {
    return new PyxiftAudioMixerHandle{};
}

void pyxift_audio_mixer_destroy(PyxiftAudioMixerHandle *h) {
    delete h;
}

void pyxift_audio_mixer_set_sound(PyxiftAudioMixerHandle *h,
                                  int32_t sound_index,
                                  const char *notes,
                                  const char *tones,
                                  const char *volumes,
                                  const char *effects,
                                  int32_t speed) {
    if (h == nullptr) return;
    pyxift::Sound s;
    s.set(notes != nullptr ? notes : "",
          tones != nullptr ? tones : "",
          volumes != nullptr ? volumes : "",
          effects != nullptr ? effects : "",
          speed);
    h->mixer.set_sound(sound_index, s);
}

void pyxift_audio_mixer_play(PyxiftAudioMixerHandle *h,
                             int32_t channel, int32_t sound_index, bool loop) {
    if (h == nullptr) return;
    h->mixer.play(channel, sound_index, loop);
}

void pyxift_audio_mixer_stop(PyxiftAudioMixerHandle *h, int32_t channel) {
    if (h == nullptr) return;
    h->mixer.stop(channel);
}

bool pyxift_audio_mixer_is_playing(const PyxiftAudioMixerHandle *h, int32_t channel) {
    if (h == nullptr) return false;
    return h->mixer.is_playing(channel);
}

void pyxift_audio_mixer_render(PyxiftAudioMixerHandle *h, int16_t *out, int32_t frames) {
    if (h == nullptr || out == nullptr || frames <= 0) return;
    h->mixer.render(out, frames);
}

static std::vector<int32_t> audio_test_to_vec(const int32_t *p, int32_t n) {
    if (p == nullptr || n <= 0) return {};
    return std::vector<int32_t>(p, p + n);
}

void pyxift_audio_mixer_music_set(PyxiftAudioMixerHandle *h,
                                  int32_t music_index,
                                  const int32_t *ch0, int32_t ch0_len,
                                  const int32_t *ch1, int32_t ch1_len,
                                  const int32_t *ch2, int32_t ch2_len,
                                  const int32_t *ch3, int32_t ch3_len) {
    if (h == nullptr) return;
    pyxift::Music m;
    m.set(audio_test_to_vec(ch0, ch0_len),
          audio_test_to_vec(ch1, ch1_len),
          audio_test_to_vec(ch2, ch2_len),
          audio_test_to_vec(ch3, ch3_len));
    h->mixer.set_music(music_index, m);
}

void pyxift_audio_mixer_play_music(PyxiftAudioMixerHandle *h,
                                   int32_t music_index, bool loop) {
    if (h == nullptr) return;
    h->mixer.play_music(music_index, loop);
}

bool pyxift_audio_mixer_play_pos(const PyxiftAudioMixerHandle *h,
                                 int32_t channel,
                                 int32_t *out_sound_index,
                                 float *out_sec) {
    if (h == nullptr) return false;
    auto pos = h->mixer.play_pos(channel);
    if (!pos) return false;
    if (out_sound_index != nullptr) *out_sound_index = pos->first;
    if (out_sec != nullptr) *out_sec = pos->second;
    return true;
}

} // extern "C"
