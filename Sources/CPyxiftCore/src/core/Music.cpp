#include "Music.hpp"

namespace pyxift {

void Music::set(const std::vector<int32_t> &ch0,
                const std::vector<int32_t> &ch1,
                const std::vector<int32_t> &ch2,
                const std::vector<int32_t> &ch3) {
    seqs[0] = ch0;
    seqs[1] = ch1;
    seqs[2] = ch2;
    seqs[3] = ch3;
}

bool Music::empty() const {
    for (const auto &s : seqs) {
        if (!s.empty()) return false;
    }
    return true;
}

} // namespace pyxift
