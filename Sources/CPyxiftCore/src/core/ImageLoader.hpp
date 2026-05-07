#ifndef PYXIFT_CORE_IMAGE_LOADER_HPP
#define PYXIFT_CORE_IMAGE_LOADER_HPP

#include "Image.hpp"

#include <string>

namespace pyxift {

// PNG ファイルを読み、16色最近傍マッピングして Image に書き込む。
// 本家 Pyxel `Image::from_image` (crates/pyxel-core/src/image.rs:57) と同等の挙動:
//   - α は捨てて RGB のみ使用
//   - 既定パレット 16 色とのユークリッド距離（二乗）で最近傍
//   - ユニーク RGB → Color のマップをキャッシュして再計算回避
// 画像が dest より大きい場合は左上原点で範囲内のみ書き込む（dest 外は黙って捨てる）。
// 戻り値: 成功なら true。ファイルが開けない／PNG として復号できなければ false。
bool load_png_into_image(const std::string &path, Image &dest);

} // namespace pyxift

#endif
