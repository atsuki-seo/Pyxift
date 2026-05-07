#ifndef PYXIFT_CORE_IMAGE_LOADER_HPP
#define PYXIFT_CORE_IMAGE_LOADER_HPP

#include "Image.hpp"

#include <string>

namespace pyxift {

bool load_png_into_image(const std::string &path, Image &dest);

} // namespace pyxift

#endif
