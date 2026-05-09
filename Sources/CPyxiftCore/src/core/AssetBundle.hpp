#ifndef PYXIFT_CORE_ASSET_BUNDLE_HPP
#define PYXIFT_CORE_ASSET_BUNDLE_HPP

#include <string>

namespace pyxift {

class Image;
class Tilemap;
class AudioMixer;

struct AssetBundleSlots {
    Image *images = nullptr;
    int32_t image_count = 0;
    Tilemap *tilemaps = nullptr;
    int32_t tilemap_count = 0;
    AudioMixer *audio_mixer = nullptr;
};

struct AssetBundleOptions {
    bool exclude_images = false;
    bool exclude_tilemaps = false;
    bool exclude_sounds = false;
    bool exclude_musics = false;
};

bool load_asset_bundle(const std::string &path,
                       const AssetBundleSlots &slots,
                       const AssetBundleOptions &opts);

bool save_asset_bundle(const std::string &path,
                       const AssetBundleSlots &slots);

} // namespace pyxift

#endif
