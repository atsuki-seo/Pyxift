#ifndef PYXIFT_CORE_SOUND_HPP
#define PYXIFT_CORE_SOUND_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace pyxift {

constexpr int8_t kRestNote = -1;
constexpr int32_t kMaxVolume = 7;
constexpr int32_t kMaxNote = 59;
constexpr int32_t kNoteOctaveSpan = 12;

enum class Effect : uint8_t {
    None = 0,
    Slide = 1,
    Vibrato = 2,
    Fadeout = 3,
    HalfFadeout = 4,
    QuarterFadeout = 5,
};

class Sound {
public:
    std::vector<int8_t> notes;
    std::vector<uint8_t> tones;
    std::vector<uint8_t> volumes;
    std::vector<uint8_t> effects;
    int32_t speed = 30;

    void set(const std::string &notes_s,
             const std::string &tones_s,
             const std::string &volumes_s,
             const std::string &effects_s,
             int32_t speed);

    static std::vector<int8_t> parse_notes(const std::string &s);
    static std::vector<uint8_t> parse_tones(const std::string &s);
    static std::vector<uint8_t> parse_volumes(const std::string &s);
    static std::vector<uint8_t> parse_effects(const std::string &s);

    bool empty() const { return notes.empty(); }
};

} // namespace pyxift

#endif
