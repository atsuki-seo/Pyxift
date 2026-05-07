---
name: pyxel-sync
description: Pyxel 本家 (kitao/pyxel) との API/数値仕様の差分を検出し、Pyxift 側 SSOT (`docs/pyxel-reference.md`) との同期を支援する。ユーザーが `/pyxel-sync` と打ったとき、またはリリースタグ前チェックリスト実行時に呼ばれる。
allowed-tools: Bash(git -C ../pyxel:*) Bash(git clone:*) Bash(grep:*)
---

# pyxel-sync

Pyxift は Pyxel の API 互換を目指す独立実装。このスキルは本家リポジトリ (`../pyxel/`) を追跡し、追跡対象ファイルに変更があれば検出して人間に提示する。

## 追跡対象ファイル（SSOT）

追跡対象は `docs/pyxel-reference.md` の「追跡対象テーブル」が SSOT。本スキルは表をパースして bash 配列を構築する。表はマーカー `<!-- pyxel-tracked-files:start -->` ～ `<!-- pyxel-tracked-files:end -->` で範囲特定する。

```bash
# 本家側ファイル全件（sparse-checkout・差分検出に使用）
mapfile -t TRACKED_FILES < <(awk '/pyxel-tracked-files:start/,/pyxel-tracked-files:end/' docs/pyxel-reference.md \
    | awk -F'|' 'NF>=6 && $3 !~ /^[ -]*$/ && $3 !~ /本家側/ {gsub(/^ +| +$/,"",$3); print $3}')

# diff 提示対象（diff 提示列が ○ の行のみ）
mapfile -t TRACKED_DIFF_FILES < <(awk '/pyxel-tracked-files:start/,/pyxel-tracked-files:end/' docs/pyxel-reference.md \
    | awk -F'|' 'NF>=6 && $3 !~ /^[ -]*$/ && $3 !~ /本家側/ {gsub(/^ +| +$/,"",$3); gsub(/^ +| +$/,"",$5); if ($5 == "○") print $3}')

if [ ${#TRACKED_FILES[@]} -eq 0 ]; then
    echo "ERROR: docs/pyxel-reference.md の追跡対象テーブルが抽出できません。マーカー <!-- pyxel-tracked-files:start/end --> を確認してください。" >&2
    exit 1
fi
```

## 起動時の挙動

### 1. 参照リポジトリの準備

親ディレクトリの `../pyxel/` を kitao/pyxel の最新 main に揃える。

```bash
if [ -d ../pyxel/.git ]; then
    # origin が kitao/pyxel であることを確認
    REMOTE_URL=$(git -C ../pyxel remote get-url origin 2>/dev/null || echo "")
    case "$REMOTE_URL" in
        *github.com[:/]kitao/pyxel*) ;;
        *)
            echo "ERROR: ../pyxel の origin が kitao/pyxel ではありません: $REMOTE_URL"
            exit 1
            ;;
    esac
    # 表更新が sparse-checkout に反映されるよう毎回 set し直す（冪等）
    git -C ../pyxel sparse-checkout set "${TRACKED_FILES[@]}"
    git -C ../pyxel fetch origin
    git -C ../pyxel reset --hard origin/main
else
    # 必要パスのみ取得（履歴メタは保持）
    git clone --filter=blob:none --no-checkout https://github.com/kitao/pyxel.git ../pyxel
    git -C ../pyxel sparse-checkout init --no-cone
    git -C ../pyxel sparse-checkout set "${TRACKED_FILES[@]}"
    git -C ../pyxel checkout main
fi
```

参照ディレクトリでの作業は想定しないため、ローカル変更チェックは省略（問答無用で上書き）。

### 2. Tracked SHA との差分検出

`docs/pyxel-reference.md` の `<!-- pyxel-upstream-sync -->` ブロックから現在の Tracked SHA を読み取り、以降に追跡ファイルへの変更があるかを確認する。

```bash
TRACKED_SHA=$(grep -oP 'Tracked SHA\*\*: `\K[a-f0-9]+' docs/pyxel-reference.md)
CURRENT_SHA=$(git -C ../pyxel rev-parse HEAD)

if [ "$TRACKED_SHA" = "$CURRENT_SHA" ]; then
    echo "上流に変更なし（Tracked SHA = $TRACKED_SHA）"
    # 年表記チェックは続行
else
    git -C ../pyxel log --oneline "$TRACKED_SHA..HEAD" -- "${TRACKED_FILES[@]}"
fi
```

### 3. 変更ファイルごとの diff 提示

変更があった場合、ユーザーに各ファイルの diff を提示し、Pyxift 側に取り込むかどうか判断を仰ぐ。

```bash
for f in "${TRACKED_DIFF_FILES[@]}"; do
    git -C ../pyxel diff "$TRACKED_SHA..HEAD" -- "$f"
done
```

判断は人間が行う:

- **取り込む**: 該当する Pyxift 側ファイル（`Sources/...`、`docs/pyxel-reference.md` 等）を更新。コミットメッセージは `chore(pyxel-sync): ...` 形式
- **取り込まない**: 取り込まない理由を `docs/decisions.md` または `docs/status.md` に記録

判断完了後、`docs/pyxel-reference.md` の Tracked SHA と Last synced を更新するコミットを単独で打つ:

```
chore(pyxel-sync): Tracked SHA を <new-sha> に更新
```

### 4. 著作権年表記の自動同期

著作権年表記の同期は `copyright-sync` スキルへ委譲する。Skill ツールで `copyright-sync` を引数なしで呼び出し、本家 (Takashi Kitao) と Pyxift 自身 (atsuki.seo) の年範囲をそれぞれ SSOT に揃える。詳細仕様（対象ファイル・コミット粒度・検出ロジック）は `.claude/skills/copyright-sync/SKILL.md` を参照。

## スコープ外（やらないこと）

- Pyxift 実装が SSOT に従っているかの双方向チェック（API シグネチャ抽出など）
  - v0.1.0 タグ後の検討事項。`docs/status.md`「将来検討事項」を参照
- MIT ライセンス全文の同期
  - MIT はバージョンレスかつ過去の許諾は不変なので、初回コピー後は追従不要
- 追跡対象テーブルに登録されていない本家ファイル
  - 表に無いものは sparse-checkout も diff 提示も対象外。追跡したくなったら表に追加する

## 関連ファイル

- SSOT 台帳: `docs/pyxel-reference.md`
- リスペクト表明: `ACKNOWLEDGMENTS.md`
- ライセンス全文: `THIRD_PARTY_LICENSES/pyxel-MIT.txt`
- リリースタグ前チェックリスト: `docs/status.md`
- 出典コメント hook: `.claude/hooks/check-source-comment.sh`
- タグ前検査 hook: `.claude/hooks/check-release-tag.sh`
