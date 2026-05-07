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
