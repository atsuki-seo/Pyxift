#ifndef PYXIFT_PLATFORM_EVENT_TRANSLATOR_HPP
#define PYXIFT_PLATFORM_EVENT_TRANSLATOR_HPP

#include "../core/Input.hpp"

#include <SDL3/SDL.h>

#include <array>

namespace pyxift::platform {

// SDL_Event を VirtualEvent に翻訳して InputState に流し込むアダプタ。
// 仮想ボタン抽象（Button.left/right/.../.start）へのキー＆ゲームパッドマッピングと、
// SDL_JoystickID → プレイヤー番号 0..3 の正規化もここで行う。
class EventTranslator {
public:
    static constexpr int32_t kMaxPlayers = InputState::kMaxPlayers;

    explicit EventTranslator(SDL_Renderer *renderer = nullptr) : renderer_(renderer) {}

    void set_renderer(SDL_Renderer *renderer) { renderer_ = renderer; }

    // SDL_Event を翻訳。返り値は「ウィンドウ閉じる/ESC」が来たかどうか（呼び出し側の終了判定）。
    bool translate(SDL_Event &ev, InputState &state);

private:
    SDL_Renderer *renderer_ = nullptr;

    // SDL_JoystickID → プレイヤー番号（0..3）。空きスロットは 0。
    struct Slot {
        bool used = false;
        SDL_JoystickID id = 0;
        SDL_Gamepad *gamepad = nullptr;
    };
    std::array<Slot, kMaxPlayers> slots_{};

    int32_t assign_player(SDL_JoystickID id);
    int32_t player_for(SDL_JoystickID id) const;
    void release_player(SDL_JoystickID id);

    void emit_key_button(InputState &state, SDL_Keycode key, bool down);
    void emit_gamepad_button(InputState &state, int32_t player, uint8_t button, bool down);
    void emit_gamepad_axis(InputState &state, int32_t player, uint8_t axis, int16_t value);

    // 軸ごとの「方向ボタン押下中か」を保持。Dead zone を超えた瞬間に ButtonDown、戻った瞬間に ButtonUp を発火。
    struct AxisDir {
        bool neg = false;  // 左/上
        bool pos = false;  // 右/下
    };
    std::array<std::array<AxisDir, 2>, kMaxPlayers> axis_dirs_{}; // [player][0=LX, 1=LY]
};

} // namespace pyxift::platform

#endif
