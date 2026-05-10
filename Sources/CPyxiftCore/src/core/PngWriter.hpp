#ifndef PYXIFT_CORE_PNG_WRITER_HPP
#define PYXIFT_CORE_PNG_WRITER_HPP

#include <cstdint>
#include <string>

namespace pyxift {

bool save_indexed_png(const std::string &path,
                      const uint8_t *index_buffer,
                      int32_t width,
                      int32_t height,
                      const uint32_t *palette_rgb,
                      int32_t scale);

} // namespace pyxift

#endif
