#include "AudioOutput.hpp"

#include "core/AudioMixer.hpp"

#include <vector>

namespace pyxift::platform {

namespace {

constexpr int kRenderChunkFrames = 512;

} // namespace

AudioOutput::AudioOutput() = default;

AudioOutput::~AudioOutput() {
    close();
}

bool AudioOutput::open(AudioMixer *mixer) {
    if (mixer == nullptr) return false;
    mixer_ = mixer;

    SDL_AudioSpec spec{};
    spec.format = SDL_AUDIO_S16;
    spec.channels = 1;
    spec.freq = kAudioSampleRate;

    stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                        &spec,
                                        &AudioOutput::stream_callback,
                                        this);
    if (stream_ == nullptr) {
        mixer_ = nullptr;
        return false;
    }

    device_ = SDL_GetAudioStreamDevice(stream_);
    SDL_ResumeAudioDevice(device_);
    return true;
}

void AudioOutput::close() {
    if (stream_ != nullptr) {
        // SDL3 does not formally specify that SDL_DestroyAudioStream waits for an in-progress
        // callback to return, so pause the device first to keep `mixer_` alive across the destroy.
        if (device_ != 0) {
            SDL_PauseAudioDevice(device_);
        }
        SDL_DestroyAudioStream(stream_);
        stream_ = nullptr;
    }
    device_ = 0;
    mixer_ = nullptr;
}

void SDLCALL AudioOutput::stream_callback(void *userdata,
                                          SDL_AudioStream *stream,
                                          int additional_amount,
                                          int /*total_amount*/) {
    auto *self = static_cast<AudioOutput *>(userdata);
    if (self == nullptr || self->mixer_ == nullptr) return;

    const int additional_frames = additional_amount / static_cast<int>(sizeof(int16_t));
    if (additional_frames <= 0) return;

    std::vector<int16_t> buffer(kRenderChunkFrames);
    int remaining = additional_frames;
    while (remaining > 0) {
        const int chunk = remaining < kRenderChunkFrames ? remaining : kRenderChunkFrames;
        self->mixer_->render(buffer.data(), chunk);
        SDL_PutAudioStreamData(stream, buffer.data(), chunk * static_cast<int>(sizeof(int16_t)));
        remaining -= chunk;
    }
}

} // namespace pyxift::platform
