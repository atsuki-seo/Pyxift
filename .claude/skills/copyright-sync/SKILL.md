---
name: copyright-sync
description: Pyxel 本家 (Takashi Kitao) と Pyxift 自身 (atsuki.seo) の著作権年範囲を、各 SSOT（本家 LICENSE / 現在年）と同期する。ユーザーが `/copyright-sync` を打ったとき、または pyxel-sync スキルから委譲されたときに呼ばれる。年表記のみが対象で、ライセンス本文は同期しない。
allowed-tools: Bash(grep:*) Bash(sed:*) Bash(xargs:*) Bash(date:*) Bash(git add:*) Bash(git commit:*) Bash(git diff:*) Bash(git status:*)
---

# copyright-sync

Pyxift 内に散在する著作権年表記を、各 SSOT に対して機械的に同期する。対象は2系統:

- **本家 (Takashi Kitao)** の年範囲: SSOT は `../pyxel/LICENSE`
- **Pyxift 自身 (atsuki.seo)** の年範囲: SSOT は現在年（`date +%Y`）、開始年は 2026 でハードコード

ライセンス本文（MIT 全文）は対象外（バージョンレスかつ過去の許諾は不変なため、初回コピー後は追従不要）。

## 前提

- 本スキルは `../pyxel/` が最新 main に同期済みであることを前提とする。`pyxel-sync` から委譲される場合はその §1 で同期済み。`/copyright-sync` 単独実行時は呼び出し元が責任を持つ。
- 検出のみで何も変更不要なら、コミットを打たず「ズレなし」を1行報告して終了する。

## 起動時の挙動

### 1. 本家 (Takashi Kitao) の年範囲

`../pyxel/LICENSE` の `Copyright (c) <range> Takashi Kitao` から年範囲を抽出し、Pyxift 内の以下の箇所と比較する:

- `THIRD_PARTY_LICENSES/pyxel-MIT.txt`
- `.claude/hooks/check-source-comment.sh`（テンプレ文字列）
- `CLAUDE.md`（出典コメント例）
- `docs/pyxel-reference.md` 冒頭の出典表記
- `ACKNOWLEDGMENTS.md`
- `README.md`（本家リスペクト記述内の年範囲）
- `LICENSE`（NOTICE 部の本家年範囲）
- `Sources/` 配下（grep が `Copyright (c) <range> Takashi Kitao` で出典コメント持ちファイルのみを拾う）

ズレを検出したら、機械的に書き換えて単独コミットを打つ:

```
chore(copyright-sync): Pyxel 著作権表記を <old-range> → <new-range> に更新
```

検出例:

```bash
UPSTREAM_RANGE=$(grep -oP 'Copyright \(c\) \K[0-9]{4}-[0-9]{4}' ../pyxel/LICENSE | head -1)
LOCAL_RANGE=$(grep -oP 'Copyright \(c\) \K[0-9]{4}-[0-9]{4}' THIRD_PARTY_LICENSES/pyxel-MIT.txt | head -1)
if [ "$UPSTREAM_RANGE" != "$LOCAL_RANGE" ]; then
    grep -rlE "Copyright \(c\) $LOCAL_RANGE Takashi Kitao" \
        THIRD_PARTY_LICENSES/ ACKNOWLEDGMENTS.md CLAUDE.md README.md LICENSE \
        docs/pyxel-reference.md .claude/ Sources/ 2>/dev/null \
      | xargs -r sed -i "s|Copyright (c) $LOCAL_RANGE Takashi Kitao|Copyright (c) $UPSTREAM_RANGE Takashi Kitao|g"
fi
```

ズレなしなら何もせず次へ。

### 2. Pyxift 自身 (atsuki.seo) の年範囲

`LICENSE` の `Copyright (c) <year-or-range> atsuki.seo` を、開始年 2026 〜 現在年（`date +%Y`）の範囲表記 `2026-<current_year>` に正規化する。現在年が 2026 のままなら単年表記（`Copyright (c) 2026 atsuki.seo`）を維持する。

```bash
CURRENT_YEAR=$(date +%Y)
if [ "$CURRENT_YEAR" = "2026" ]; then
    DESIRED="Copyright (c) 2026 atsuki.seo"
else
    DESIRED="Copyright (c) 2026-$CURRENT_YEAR atsuki.seo"
fi

# 現状のいずれの形（単年 / 範囲）にもマッチさせて DESIRED に置換
sed -i -E "s|Copyright \(c\) 2026(-[0-9]{4})? atsuki\.seo|$DESIRED|g" LICENSE
```

ズレを書き換えた場合は本家年範囲とは独立に単独コミットを打つ:

```
chore(copyright-sync): Pyxift 著作権年を <old> → <new> に更新
```

ズレなしなら何もせず終了。

## コミット粒度

§1（本家年）と §2（Pyxift 自身年）は動機が独立（前者は本家追従、後者は Pyxift 自身の年経過）。検出したズレは**それぞれ単独コミット**として打ち、混ぜない。両方ズレなしならコミットなしで終了。

## スコープ外（やらないこと）

- MIT ライセンス全文の同期（`THIRD_PARTY_LICENSES/pyxel-MIT.txt` の本文）
- 出典コメントの追加・削除（`.claude/hooks/check-source-comment.sh` の責務）
- 追跡対象テーブル自体の更新（`pyxel-sync` の責務）

## 関連ファイル

- 本家年 SSOT: `../pyxel/LICENSE`
- Pyxift 自身年 SSOT: 現在年（`date +%Y`）+ 開始年 2026（本ファイル内ハードコード）
- 呼び出し元: `.claude/skills/pyxel-sync/SKILL.md` §4
