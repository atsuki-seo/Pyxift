# Pyxift — Claude 向け運用ルール

このリポジトリで作業する Claude Code のための、プロジェクト固有の指針。
`docs/decisions.md`（設計判断）と `docs/status.md`（未達タスク・未解決事項）を必ず最初に確認してから作業する。

## プロジェクト概要

Pyxel風レトロ2Dゲームエンジン（Swift製・C++コア・SDL3バックエンド）。詳細な設計判断（対象OS・言語・配布形式・ライセンス）は `docs/decisions.md` を参照。

## マイルストーン区切り規約

各マイルストーン完了時の手順:

1. 完了マーカーとして空コミットを打つ:
   ```
   git commit --allow-empty -m "M1: 垂直スライス完了"
   ```
   コミットメッセージ先頭は `M<番号>: ` 固定（例: `M1:`、`M2a:`、`M13:`）
2. リリース版のみ `git tag` を打つ。中間タグは打たない。タグを打つ前には `docs/status.md` の「リリースタグ前チェックリスト」を完了させる（`.claude/hooks/check-release-tag.sh` が機械的に検査する）

完了済みマイルストーンの確認は `git log --grep '^M[0-9]'`。

## ブランチ運用

- **main 直接コミット**で進める（1人開発・速度優先）
- PR は使わない
- 論理的に最小単位で段階的にコミット（既存の `~/.claude/rules/git-commit.md` 通り）

## Pyxel 本家リポジトリの参照

本家コードは Pyxift リポジトリの**親ディレクトリ**の `../pyxel/` に置く前提（**Rust 製**）。
`/pyxel-sync` スキルが起動時に存在確認・無ければ clone・あれば `git fetch && git reset --hard origin/main` で同期する。

追跡対象ファイル（本家のどのパスを Pyxift のどのファイルが参照しているか）は `docs/pyxel-reference.md` の「追跡対象テーブル」が SSOT。本家を参考にした実装を追加する際は同表へのエントリ登録が必須（`.claude/hooks/check-source-comment.sh` が出典コメントを持つファイルの表登録を機械的に検査する）。
追跡 SHA と最終同期日は同ファイルの「上流追従ステータス」ブロックが SSOT。

API 互換と数値仕様の上流追従は `/pyxel-sync` スキル（`.claude/skills/pyxel-sync/SKILL.md`）に集約。

## コメント方針

ソースコードのコメント方針は `.claude/rules/comment-style.md` に従う。

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

## Pyxel 本家との API/数値同期

Pyxift は Pyxel API 互換を目指す独立実装。本家追従の起動契機・検出範囲・差分処理方針は `/pyxel-sync` スキル（`.claude/skills/pyxel-sync/SKILL.md`）に集約。著作権年範囲の同期だけは独立スキル `/copyright-sync`（`.claude/skills/copyright-sync/SKILL.md`）に切り出されており、`/pyxel-sync` 実行時に内部委譲される。

- リスペクト表明: `ACKNOWLEDGMENTS.md`、ライセンス全文: `THIRD_PARTY_LICENSES/pyxel-MIT.txt`

## 作業前に必ず読むファイル

新しいセッションでこのリポジトリに入った Claude は、以下を順に読むこと:

1. `docs/decisions.md` — 設計判断
2. `docs/status.md` — 未達タスク・未解決事項・ロードマップ・リリースタグ前チェックリスト
3. `docs/pyxel-reference.md` — 本家から流用する数値・データ・上流追従ステータス
