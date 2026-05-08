#include "Sound.hpp"

#include <cctype>

namespace pyxift {

namespace {

bool is_skippable(char c) {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

} // namespace

std::vector<int8_t> Sound::parse_notes(const std::string &s) {
    std::vector<int8_t> out;
    const size_t n = s.size();
    for (size_t i = 0; i < n; ) {
        const char c = s[i];
        if (is_skippable(c)) { ++i; continue; }
        const char lc = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (lc == 'r') {
            out.push_back(kRestNote);
            ++i;
            continue;
        }
        if (lc < 'a' || lc > 'g') {
            ++i;
            continue;
        }
        static constexpr int kSemitoneOf[7] = {9, 11, 0, 2, 4, 5, 7}; // A B C D E F G
        int semitone = kSemitoneOf[lc - 'a'];
        ++i;
        if (i < n) {
            char a = s[i];
            if (a == '#' || a == '+') { semitone += 1; ++i; }
            else if (a == '-') { semitone -= 1; ++i; }
        }
        while (i < n && is_skippable(s[i])) ++i;
        int octave = 0;
        if (i < n && std::isdigit(static_cast<unsigned char>(s[i]))) {
            octave = s[i] - '0';
            ++i;
        }
        const int note = octave * kNoteOctaveSpan + semitone;
        if (note >= 0 && note <= kMaxNote) {
            out.push_back(static_cast<int8_t>(note));
        } else {
            out.push_back(kRestNote);
        }
    }
    return out;
}

std::vector<uint8_t> Sound::parse_tones(const std::string &s) {
    std::vector<uint8_t> out;
    for (char c : s) {
        if (is_skippable(c)) continue;
        switch (std::tolower(static_cast<unsigned char>(c))) {
            case 't': out.push_back(0); break;
            case 's': out.push_back(1); break;
            case 'p': out.push_back(2); break;
            case 'n': out.push_back(3); break;
            default:
                if (std::isdigit(static_cast<unsigned char>(c))) {
                    out.push_back(static_cast<uint8_t>(c - '0'));
                }
                break;
        }
    }
    return out;
}

std::vector<uint8_t> Sound::parse_volumes(const std::string &s) {
    std::vector<uint8_t> out;
    for (char c : s) {
        if (is_skippable(c)) continue;
        if (c >= '0' && c <= '7') {
            out.push_back(static_cast<uint8_t>(c - '0'));
        }
    }
    return out;
}

std::vector<uint8_t> Sound::parse_effects(const std::string &s) {
    std::vector<uint8_t> out;
    for (char c : s) {
        if (is_skippable(c)) continue;
        switch (std::tolower(static_cast<unsigned char>(c))) {
            case 'n': out.push_back(static_cast<uint8_t>(Effect::None)); break;
            case 's': out.push_back(static_cast<uint8_t>(Effect::Slide)); break;
            case 'v': out.push_back(static_cast<uint8_t>(Effect::Vibrato)); break;
            case 'f': out.push_back(static_cast<uint8_t>(Effect::Fadeout)); break;
            case 'h': out.push_back(static_cast<uint8_t>(Effect::HalfFadeout)); break;
            case 'q': out.push_back(static_cast<uint8_t>(Effect::QuarterFadeout)); break;
            default: break;
        }
    }
    return out;
}

void Sound::set(const std::string &notes_s,
                const std::string &tones_s,
                const std::string &volumes_s,
                const std::string &effects_s,
                int32_t speed_v) {
    notes = parse_notes(notes_s);
    tones = parse_tones(tones_s);
    volumes = parse_volumes(volumes_s);
    effects = parse_effects(effects_s);
    speed = speed_v > 0 ? speed_v : 1;
}

} // namespace pyxift
