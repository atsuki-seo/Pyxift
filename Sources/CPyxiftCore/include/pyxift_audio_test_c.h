#ifndef PYXIFT_AUDIO_TEST_C_H
#define PYXIFT_AUDIO_TEST_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Sound parser tests: parse a single string and write the resulting sequence into out_buf.
// Returns the number of elements parsed (regardless of out_buf capacity).
// Pass out_buf=NULL with capacity=0 to query length only. Values that exceed the byte width
// of the output (notes are int8, others uint8) are not produced by the parser.

int32_t pyxift_sound_parse_notes(const char *s, int8_t *out_buf, int32_t capacity);
int32_t pyxift_sound_parse_tones(const char *s, uint8_t *out_buf, int32_t capacity);
int32_t pyxift_sound_parse_volumes(const char *s, uint8_t *out_buf, int32_t capacity);
int32_t pyxift_sound_parse_effects(const char *s, uint8_t *out_buf, int32_t capacity);

// Render an isolated AudioMixer for offline test (no SDL device needed).
typedef struct PyxiftAudioMixerHandle PyxiftAudioMixerHandle;

PyxiftAudioMixerHandle *pyxift_audio_mixer_create(void);
void pyxift_audio_mixer_destroy(PyxiftAudioMixerHandle *h);
void pyxift_audio_mixer_set_sound(PyxiftAudioMixerHandle *h,
                                  int32_t sound_index,
                                  const char *notes,
                                  const char *tones,
                                  const char *volumes,
                                  const char *effects,
                                  int32_t speed);
void pyxift_audio_mixer_play(PyxiftAudioMixerHandle *h,
                             int32_t channel, int32_t sound_index, bool loop);
void pyxift_audio_mixer_stop(PyxiftAudioMixerHandle *h, int32_t channel);
bool pyxift_audio_mixer_is_playing(const PyxiftAudioMixerHandle *h, int32_t channel);
void pyxift_audio_mixer_render(PyxiftAudioMixerHandle *h, int16_t *out, int32_t frames);

#ifdef __cplusplus
}
#endif

#endif
