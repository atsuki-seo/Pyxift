#ifndef PYXIFT_CORE_MUSIC_HPP
#define PYXIFT_CORE_MUSIC_HPP

#include <array>
#include <cstdint>
#include <vector>

namespace pyxift {

inline constexpr int32_t kNumMusicChannels = 4;

class Music {
public:
    std::array<std::vector<int32_t>, kNumMusicChannels> seqs{};

    void set(const std::vector<int32_t> &ch0,
             const std::vector<int32_t> &ch1,
             const std::vector<int32_t> &ch2,
             const std::vector<int32_t> &ch3);

    bool empty() const;
};

} // namespace pyxift

#endif
