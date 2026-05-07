# Pyxift — Claude 向け運用ルール

このリポジトリで作業する Claude Code のための、プロジェクト固有の指針。
`docs/decisions.md`（設計判断）と `docs/implementation-plan.md`（実装計画）を必ず最初に確認してから作業する。

## プロジェクト概要

Pyxel風レトロ2Dゲームエンジン（Swift製・C++コア・SDL3バックエンド）。詳細な設計判断（対象OS・言語・配布形式・ライセンス）は `docs/decisions.md` を参照。

## マイルストーン区切り規約

実装計画は `docs/implementation-plan.md` に M1〜M13 として記載。各マイルストーン完了時の手順:

1. `docs/implementation-plan.md` の該当マイルストーンのチェックボックスを `[x]` に更新するコミットを打つ
2. **完了基準を `docs/implementation-plan.md` から逐語確認**してから完了宣言
3. 完了マーカーとして以下を実行:
   ```
   git commit --allow-empty -m "M1: 垂直スライス完了"
   ```
   コミットメッセージ先頭は `M<番号>: ` 固定（例: `M1:`、`M2a:`、`M13:`）
4. v0.1.0 / v0.2.0 / v0.3.0 のリリース版のみ `git tag` を打つ。中間タグは打たない

## ブランチ運用

- **main 直接コミット**で進める（1人開発・速度優先）
- PR は使わない
- 論理的に最小単位で段階的にコミット（既存の `~/.claude/rules/git-commit.md` 通り）

## Pyxel 本家リポジトリの参照

本家コードは `~/ドキュメント/pyxel` に clone 済み。**Rust 製**（Python ではない）。

- エンジン本体: `~/ドキュメント/pyxel/crates/pyxel-core/`
- 数値定数: `~/ドキュメント/pyxel/crates/pyxel-core/src/settings.rs`
  - `DEFAULT_COLORS`（16色既定パレット）
  - `FONT_DATA`（4×6 ビットマップフォント、ASCII 0x20〜0x7F）
  - `DEFAULT_TONE_*`（v0.2 で使用、4波形のトーンテーブル）
- 公開API表面: `~/ドキュメント/pyxel/python/pyxel/__init__.pyi`
- 描画ロジック参考: `~/ドキュメント/pyxel/crates/pyxel-core/src/canvas.rs`

`editor` / `screencast` / `wasm` / `web` / `scripts` は v0.1 スコープ外なので無視。

## 本家流用コード — 出典コメント必須

`FONT_DATA` / `DEFAULT_COLORS` / `DEFAULT_TONE_*` のいずれかを含むファイルは**冒頭に出典コメント必須**:

```cpp
// Source: kitao/pyxel crates/pyxel-core/src/settings.rs
// License: MIT (Copyright (c) 2018-2026 Takashi Kitao)
```

`.claude/hooks/check-source-comment.sh` が編集前にチェックして警告する。

## C++ コア層の SDL3 import 禁止ルール

C++ 実装は2層に分ける:

- **コア層** (`Sources/CPyxiftCore/src/core/`): pure C++、テスト容易性優先
  - `InputState`、`AudioMixer`、`Canvas`（描画ロジック）など
  - `#include <SDL3/...>` は**禁止**
- **アダプタ層** (`Sources/CPyxiftCore/src/platform/`): SDL3 依存
  - `SDL_Event` → `VirtualEvent` 変換、`SDL_AudioStream` ラッパー、ウィンドウ管理
  - `#include <SDL3/...>` OK

`.claude/hooks/check-cpp-layer.sh` がコア層への SDL3 import を検出して警告する。

## Swift 6 strict concurrency

- すべての Swift コードを strict concurrency で書く（`swift-language-mode: 6`）
- ゲームループは `@MainActor` 単一実行
- C関数橋渡しは `nonisolated(unsafe)` で明示
- ファイル単位で `swift build` を毎回走らせるのは遅いので **マイルストーン完了時に手動で** `swift build -Xswiftc -warnings-as-errors` を実行して警告ゼロを確認

## v0.1.0 タグ前のチェックリスト

完了基準は `docs/implementation-plan.md` の M5「v0.1.0 タグ前のチェックリスト」を一次台帳として参照。すべての項目が `[x]` になっていることを確認してからタグを打つ。

## 作業前に必ず読むファイル

新しいセッションでこのリポジトリに入った Claude は、以下を順に読むこと:

1. `docs/decisions.md` — 設計判断
2. `docs/implementation-plan.md` — 実装ロードマップ・現在のマイルストーン
3. `docs/open-questions.md` — 未解決事項
4. `docs/api-sketch.md` — Swift API ラフスケッチ
5. `docs/pyxel-reference.md` — 本家から流用する数値・データ
