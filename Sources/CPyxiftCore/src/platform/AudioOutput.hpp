#ifndef PYXIFT_PLATFORM_AUDIO_OUTPUT_HPP
#define PYXIFT_PLATFORM_AUDIO_OUTPUT_HPP

#include <SDL3/SDL.h>

#include <cstdint>

namespace pyxift {
class AudioMixer;
}

namespace pyxift::platform {

class AudioOutput {
public:
    AudioOutput();
    ~AudioOutput();

    AudioOutput(const AudioOutput &) = delete;
    AudioOutput &operator=(const AudioOutput &) = delete;

    bool open(AudioMixer *mixer);
    void close();
    bool valid() const { return device_ != 0 && stream_ != nullptr; }

private:
    static void SDLCALL stream_callback(void *userdata,
                                        SDL_AudioStream *stream,
                                        int additional_amount,
                                        int total_amount);

    AudioMixer *mixer_ = nullptr;
    SDL_AudioDeviceID device_ = 0;
    SDL_AudioStream *stream_ = nullptr;
};

} // namespace pyxift::platform

#endif
