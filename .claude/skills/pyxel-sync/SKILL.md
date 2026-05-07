---
name: pyxel-sync
description: Pyxel 本家 (kitao/pyxel) との API/数値仕様の差分を検出し、Pyxift 側 SSOT (`docs/pyxel-reference.md`) との同期を支援する。ユーザーが `/pyxel-sync` と打ったとき、またはリリースタグ前チェックリスト実行時に呼ばれる。
disable-model-invocation: true
allowed-tools: Bash(git -C ../pyxel:*) Bash(git clone:*) Bash(grep:*) Bash(sed:*) Bash(xargs:*)
---

# pyxel-sync

Pyxift は Pyxel の API 互換を目指す独立実装。このスキルは本家リポジトリ (`../pyxel/`) を追跡し、追跡対象ファイルに変更があれば検出して人間に提示する。

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
    git -C ../pyxel fetch origin
    git -C ../pyxel reset --hard origin/main
else
    # 必要パスのみ取得（履歴メタは保持）
    git clone --filter=blob:none --no-checkout https://github.com/kitao/pyxel.git ../pyxel
    git -C ../pyxel sparse-checkout init --no-cone
    git -C ../pyxel sparse-checkout set \
        python/pyxel/__init__.pyi \
        crates/pyxel-core/src/settings.rs \
        crates/pyxel-core/src/canvas.rs \
        LICENSE
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
    git -C ../pyxel log --oneline "$TRACKED_SHA..HEAD" -- \
        python/pyxel/__init__.pyi \
        crates/pyxel-core/src/settings.rs \
        crates/pyxel-core/src/canvas.rs \
        LICENSE
fi
```

### 3. 変更ファイルごとの diff 提示

変更があった場合、ユーザーに各ファイルの diff を提示し、Pyxift 側に取り込むかどうか判断を仰ぐ。

```bash
git -C ../pyxel diff "$TRACKED_SHA..HEAD" -- python/pyxel/__init__.pyi
git -C ../pyxel diff "$TRACKED_SHA..HEAD" -- crates/pyxel-core/src/settings.rs
git -C ../pyxel diff "$TRACKED_SHA..HEAD" -- crates/pyxel-core/src/canvas.rs
```

判断は人間が行う:

- **取り込む**: 該当する Pyxift 側ファイル（`Sources/...`、`docs/pyxel-reference.md` 等）を更新。コミットメッセージは `chore(pyxel-sync): ...` 形式
- **取り込まない**: 取り込まない理由を `docs/decisions.md` または `docs/status.md` に記録

判断完了後、`docs/pyxel-reference.md` の Tracked SHA と Last synced を更新するコミットを単独で打つ:

```
chore(pyxel-sync): Tracked SHA を <new-sha> に更新
```

### 4. 著作権年表記の自動同期（例外的に機械対応）

`../pyxel/LICENSE` の `Copyright (c) <range> Takashi Kitao` から年範囲を抽出し、Pyxift 内の以下の箇所と比較する:

- `THIRD_PARTY_LICENSES/pyxel-MIT.txt`
- `.claude/hooks/check-source-comment.sh`（テンプレ文字列）
- `CLAUDE.md`（出典コメント例）
- `docs/pyxel-reference.md` 冒頭の出典表記
- `ACKNOWLEDGMENTS.md`
- `Sources/` 配下（grep が `Copyright (c) <range> Takashi Kitao` で出典コメント持ちファイルのみを拾う）

ズレを検出したら、機械的に書き換えて単独コミットを打つ:

```
chore(pyxel-sync): Pyxel 著作権表記を <old-range> → <new-range> に更新
```

検出例:

```bash
UPSTREAM_RANGE=$(grep -oP 'Copyright \(c\) \K[0-9]{4}-[0-9]{4}' ../pyxel/LICENSE | head -1)
LOCAL_RANGE=$(grep -oP 'Copyright \(c\) \K[0-9]{4}-[0-9]{4}' THIRD_PARTY_LICENSES/pyxel-MIT.txt | head -1)
if [ "$UPSTREAM_RANGE" != "$LOCAL_RANGE" ]; then
    grep -rlE "Copyright \(c\) $LOCAL_RANGE Takashi Kitao" \
        THIRD_PARTY_LICENSES/ ACKNOWLEDGMENTS.md CLAUDE.md \
        docs/pyxel-reference.md .claude/ Sources/ 2>/dev/null \
      | xargs -r sed -i "s|Copyright (c) $LOCAL_RANGE Takashi Kitao|Copyright (c) $UPSTREAM_RANGE Takashi Kitao|g"
fi
```

## スコープ外（やらないこと）

- Pyxift 実装が SSOT に従っているかの双方向チェック（API シグネチャ抽出など）
  - v0.1.0 タグ後の検討事項。`docs/status.md`「将来検討事項」を参照
- MIT ライセンス全文の同期
  - MIT はバージョンレスかつ過去の許諾は不変なので、初回コピー後は追従不要
- 本家リポジトリの v0.1 スコープ外ディレクトリ
  - 対象ディレクトリは `CLAUDE.md`「Pyxel 本家リポジトリの参照」節を参照

## 関連ファイル

- SSOT 台帳: `docs/pyxel-reference.md`
- リスペクト表明: `ACKNOWLEDGMENTS.md`
- ライセンス全文: `THIRD_PARTY_LICENSES/pyxel-MIT.txt`
- リリースタグ前チェックリスト: `docs/status.md`
- 出典コメント hook: `.claude/hooks/check-source-comment.sh`
- タグ前検査 hook: `.claude/hooks/check-release-tag.sh`
