# Pyxift ステータス

未達タスクと未解決事項を管理する。完了済みは `git log --grep '^M[0-9]'` で参照。

## 未達タスク（v0.1.0 タグまで）

- [ ] `swift package init` 再実行検証（ホーム直下の素のフォルダで Getting Started 手順が通る）
- [ ] v0.1.0 タグ → Swift Package Index 登録申請

## 未解決事項

- noise (Perlin) API: v0.1 では提供せず、必要時点で再検討。詳細は `decisions.md` の数学・乱数 API 節を参照

## v0.2 以降のロードマップ

- v0.2.0: 音声合成（4ch・矩形/三角/パルス/ノイズ）
- v0.3.0: アセットバンドル `.pyxift` 形式

## リリースタグ前チェックリスト

`git tag v*.*.*` を打つ前に必ず実行する（バージョン非依存・全リリース共通）。
Claude 経由のタグ付けは `.claude/hooks/check-release-tag.sh` が機械的にブロックする。

- [ ] `/pyxel-sync` を実行し、Pyxel 本家との差分が解消済み（あるいは取り込まないと判断済み）
- [ ] `docs/pyxel-reference.md` の Tracked SHA が `../pyxel` の `origin/main` HEAD と一致
- [ ] `swift build -Xswiftc -warnings-as-errors` で警告ゼロ

各リリース固有の追加項目は、必要に応じて本ファイル内に別セクションで記載する。

## 将来検討事項

### v0.1.0 タグ後: Pyxel API 双方向チェック (β) の再検討

v0.1 開発中は Pyxift 実装の API シグネチャが流動的なため、SSOT (`docs/pyxel-reference.md` および `../pyxel/python/pyxel/__init__.pyi`) との双方向照合は導入しない。
v0.1.0 タグ完了後、API 表面が固まった段階で以下を再検討する:

- Swift 公開 API シグネチャを抽出するスクリプト
- Pyxel `__init__.pyi` との機械的照合
- `/pyxel-sync` への組込み or 独立 hook 化
