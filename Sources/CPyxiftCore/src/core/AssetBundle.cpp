// Source: kitao/pyxel crates/pyxel-core/src/resource_data.rs
// Source: kitao/pyxel crates/pyxel-core/src/utils.rs
// License: MIT (Copyright (c) 2018-2026 Takashi Kitao)
//
// JSON schema and the trailing-zero compaction (compress_vec1 / compress_vec2)
// are adapted from upstream Pyxel; the code below is an independent C++
// implementation that does not copy upstream source.

#include "AssetBundle.hpp"

#include "AudioMixer.hpp"
#include "Image.hpp"
#include "Music.hpp"
#include "Sound.hpp"
#include "Tilemap.hpp"

#include "../../vendor/miniz.h"
#include "../../vendor/json.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace pyxift {

namespace {

constexpr int32_t kFormatVersion = 1;
constexpr const char *kArchiveEntryName = "pyxift_resource.json";

using json = nlohmann::json;

[[noreturn]] void fatal(const char *fmt, const std::string &arg) {
    std::fprintf(stderr, "[pyxift] asset bundle: ");
    std::fprintf(stderr, fmt, arg.c_str());
    std::fprintf(stderr, "\n");
    std::abort();
}

json encode_image(const Image &img) {
    int32_t last_row = -1;
    json rows = json::array();
    for (int32_t y = 0; y < img.height(); ++y) {
        int32_t last_col = -1;
        for (int32_t x = img.width() - 1; x >= 0; --x) {
            if (img.pget(x, y) != 0) { last_col = x; break; }
        }
        json row = json::array();
        for (int32_t x = 0; x <= last_col; ++x) {
            row.push_back(static_cast<int>(img.pget(x, y)));
        }
        rows.push_back(std::move(row));
        if (last_col >= 0) last_row = y;
    }
    while (static_cast<int32_t>(rows.size()) > last_row + 1) rows.erase(rows.size() - 1);
    return json{
        {"width", img.width()},
        {"height", img.height()},
        {"data", rows},
    };
}

void decode_image(const json &j, Image &img) {
    if (!j.contains("data") || !j["data"].is_array()) return;
    const auto &rows = j["data"];
    for (size_t y = 0; y < rows.size() && static_cast<int32_t>(y) < img.height(); ++y) {
        const auto &row = rows[y];
        if (!row.is_array()) continue;
        for (size_t x = 0; x < row.size() && static_cast<int32_t>(x) < img.width(); ++x) {
            img.pset(static_cast<int32_t>(x), static_cast<int32_t>(y),
                     static_cast<uint8_t>(row[x].get<int>()));
        }
    }
}

json encode_tilemap(const Tilemap &tm) {
    const int32_t total = tm.width() * tm.height();
    int32_t last_cell = -1;
    for (int32_t i = 0; i < total; ++i) {
        uint8_t tx = 0, ty = 0;
        tm.get_cell(i % tm.width(), i / tm.width(), tx, ty);
        if (tx != 0 || ty != 0) last_cell = i;
    }
    json flat = json::array();
    for (int32_t i = 0; i <= last_cell; ++i) {
        uint8_t tx = 0, ty = 0;
        tm.get_cell(i % tm.width(), i / tm.width(), tx, ty);
        flat.push_back(static_cast<int>(tx));
        flat.push_back(static_cast<int>(ty));
    }
    return json{
        {"width", tm.width()},
        {"height", tm.height()},
        {"imgsrc", tm.image_bank()},
        {"data", flat},
    };
}

void decode_tilemap(const json &j, Tilemap &tm) {
    if (j.contains("imgsrc") && j["imgsrc"].is_number_integer()) {
        tm.set_image_bank(j["imgsrc"].get<int32_t>());
    }
    if (!j.contains("data") || !j["data"].is_array()) return;
    const auto &flat = j["data"];
    const int32_t cells = static_cast<int32_t>(flat.size() / 2);
    for (int32_t i = 0; i < cells; ++i) {
        const int32_t cx = i % tm.width();
        const int32_t cy = i / tm.width();
        if (cy >= tm.height()) break;
        tm.set_cell(cx, cy,
                    static_cast<uint8_t>(flat[i * 2].get<int>()),
                    static_cast<uint8_t>(flat[i * 2 + 1].get<int>()));
    }
}

json encode_sound(const Sound &s) {
    json notes = json::array();
    for (int8_t v : s.notes) notes.push_back(static_cast<int>(v));
    json tones = json::array();
    for (uint8_t v : s.tones) tones.push_back(static_cast<int>(v));
    json volumes = json::array();
    for (uint8_t v : s.volumes) volumes.push_back(static_cast<int>(v));
    json effects = json::array();
    for (uint8_t v : s.effects) effects.push_back(static_cast<int>(v));
    return json{
        {"notes", notes},
        {"tones", tones},
        {"volumes", volumes},
        {"effects", effects},
        {"speed", s.speed},
    };
}

void decode_sound(const json &j, Sound &s) {
    s.notes.clear();
    s.tones.clear();
    s.volumes.clear();
    s.effects.clear();
    s.speed = 30;
    if (j.contains("notes") && j["notes"].is_array()) {
        for (const auto &v : j["notes"]) s.notes.push_back(static_cast<int8_t>(v.get<int>()));
    }
    if (j.contains("tones") && j["tones"].is_array()) {
        for (const auto &v : j["tones"]) s.tones.push_back(static_cast<uint8_t>(v.get<int>()));
    }
    if (j.contains("volumes") && j["volumes"].is_array()) {
        for (const auto &v : j["volumes"]) s.volumes.push_back(static_cast<uint8_t>(v.get<int>()));
    }
    if (j.contains("effects") && j["effects"].is_array()) {
        for (const auto &v : j["effects"]) s.effects.push_back(static_cast<uint8_t>(v.get<int>()));
    }
    if (j.contains("speed") && j["speed"].is_number_integer()) {
        s.speed = j["speed"].get<int32_t>();
    }
}

json encode_music(const Music &m) {
    int32_t last_ch = -1;
    for (int32_t c = 0; c < kNumMusicChannels; ++c) {
        if (!m.seqs[c].empty()) last_ch = c;
    }
    json seqs = json::array();
    for (int32_t c = 0; c <= last_ch; ++c) {
        json ch = json::array();
        for (int32_t v : m.seqs[c]) ch.push_back(v);
        seqs.push_back(std::move(ch));
    }
    return json{{"seqs", seqs}};
}

void decode_music(const json &j, Music &m) {
    for (auto &ch : m.seqs) ch.clear();
    if (!j.contains("seqs") || !j["seqs"].is_array()) return;
    const auto &seqs = j["seqs"];
    for (size_t c = 0; c < seqs.size() && c < static_cast<size_t>(kNumMusicChannels); ++c) {
        if (!seqs[c].is_array()) continue;
        for (const auto &v : seqs[c]) m.seqs[c].push_back(v.get<int32_t>());
    }
}

bool read_zip_entry_to_string(const std::string &path, const char *entry,
                              std::string &out) {
    mz_zip_archive zip;
    std::memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, path.c_str(), 0)) return false;
    int file_index = mz_zip_reader_locate_file(&zip, entry, nullptr, 0);
    if (file_index < 0) {
        mz_zip_reader_end(&zip);
        return false;
    }
    mz_zip_archive_file_stat stat;
    if (!mz_zip_reader_file_stat(&zip, static_cast<mz_uint>(file_index), &stat)) {
        mz_zip_reader_end(&zip);
        return false;
    }
    out.resize(static_cast<size_t>(stat.m_uncomp_size));
    bool ok = mz_zip_reader_extract_to_mem(&zip, static_cast<mz_uint>(file_index),
                                           out.data(), out.size(), 0);
    mz_zip_reader_end(&zip);
    return ok;
}

bool write_string_to_zip(const std::string &path, const char *entry,
                         const std::string &content) {
    mz_zip_archive zip;
    std::memset(&zip, 0, sizeof(zip));
    if (!mz_zip_writer_init_file(&zip, path.c_str(), 0)) return false;
    bool ok = mz_zip_writer_add_mem(&zip, entry, content.data(), content.size(),
                                    MZ_DEFAULT_COMPRESSION);
    if (!ok) {
        mz_zip_writer_end(&zip);
        return false;
    }
    if (!mz_zip_writer_finalize_archive(&zip)) ok = false;
    mz_zip_writer_end(&zip);
    return ok;
}

} // namespace

bool load_asset_bundle(const std::string &path,
                       const AssetBundleSlots &slots,
                       const AssetBundleOptions &opts) {
    std::string raw;
    if (!read_zip_entry_to_string(path, kArchiveEntryName, raw)) {
        fatal("failed to open '%s' or missing pyxift_resource.json", path);
    }
    json doc;
    try {
        doc = json::parse(raw);
    } catch (const std::exception &) {
        fatal("malformed JSON in '%s'", path);
    }
    if (!doc.contains("format_version") ||
        doc["format_version"].get<int>() > kFormatVersion) {
        fatal("unsupported format_version in '%s'", path);
    }

    if (!opts.exclude_images && doc.contains("images") && doc["images"].is_array() &&
        slots.images != nullptr) {
        const auto &arr = doc["images"];
        for (size_t i = 0; i < arr.size() && static_cast<int32_t>(i) < slots.image_count; ++i) {
            decode_image(arr[i], slots.images[i]);
        }
    }
    if (!opts.exclude_tilemaps && doc.contains("tilemaps") && doc["tilemaps"].is_array() &&
        slots.tilemaps != nullptr) {
        const auto &arr = doc["tilemaps"];
        for (size_t i = 0; i < arr.size() && static_cast<int32_t>(i) < slots.tilemap_count; ++i) {
            decode_tilemap(arr[i], slots.tilemaps[i]);
        }
    }
    if (!opts.exclude_sounds && doc.contains("sounds") && doc["sounds"].is_array() &&
        slots.audio_mixer != nullptr) {
        const auto &arr = doc["sounds"];
        for (size_t i = 0; i < arr.size() && static_cast<int32_t>(i) < kNumSounds; ++i) {
            Sound s;
            decode_sound(arr[i], s);
            slots.audio_mixer->set_sound(static_cast<int32_t>(i), s);
        }
    }
    if (!opts.exclude_musics && doc.contains("musics") && doc["musics"].is_array() &&
        slots.audio_mixer != nullptr) {
        const auto &arr = doc["musics"];
        for (size_t i = 0; i < arr.size() && static_cast<int32_t>(i) < kNumMusics; ++i) {
            Music m;
            decode_music(arr[i], m);
            slots.audio_mixer->set_music(static_cast<int32_t>(i), m);
        }
    }
    return true;
}

bool save_asset_bundle(const std::string &path,
                       const AssetBundleSlots &slots) {
    json doc;
    doc["format_version"] = kFormatVersion;

    if (slots.images != nullptr) {
        json arr = json::array();
        for (int32_t i = 0; i < slots.image_count; ++i) arr.push_back(encode_image(slots.images[i]));
        doc["images"] = arr;
    }
    if (slots.tilemaps != nullptr) {
        json arr = json::array();
        for (int32_t i = 0; i < slots.tilemap_count; ++i) arr.push_back(encode_tilemap(slots.tilemaps[i]));
        doc["tilemaps"] = arr;
    }
    if (slots.audio_mixer != nullptr) {
        json sounds = json::array();
        for (int32_t i = 0; i < kNumSounds; ++i) sounds.push_back(encode_sound(slots.audio_mixer->get_sound(i)));
        doc["sounds"] = sounds;
        json musics = json::array();
        for (int32_t i = 0; i < kNumMusics; ++i) musics.push_back(encode_music(slots.audio_mixer->get_music(i)));
        doc["musics"] = musics;
    }

    std::string text = doc.dump();
    return write_string_to_zip(path, kArchiveEntryName, text);
}

} // namespace pyxift
