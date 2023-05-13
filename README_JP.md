# ゴースト・ターミナル  

![プレビュー画像]( ./preview.png )  
もともとの試供品、今はある程度使えると思います（  

## 使用方法  

ghost_terminalをシステム端末のように使用する  
上下切り替えコマンド、マウスの右ボタンでクイックペースト、タブ補完に対応（ゴーストでサポートされている場合）。
ゴーストがサポートする表現を入力し、評価することができます！
ゴーストの開発に使いやすい  

## コマンドライン引数  

テキスト

```text
ghost terminal v13.4

ghost_terminal [オプション]を指定します。
オプションで指定します。
  -h, --help                           : ヘルプメッセージを表示し、終了する。
  -v, --version                        : はバージョン番号を表示し、終了する。
  -c, --command <command>              : 指定されたコマンドを実行し、終了する。
  -s, --sakura-script <script>         : 指定されたサクラスクリプトを実行し、終了する。
  -g, --ghost <ghost>                  : 指定されたゴーストに名前でリンクする。
  -gh, --ghost-hwnd <hwnd>             : 指定されたゴーストにHWNDでリンクする。
  -gp, --ghost-folder-path <path>      : 指定されたゴーストにフォルダーパスでリンクする。
  -r, --run-ghost                      : (それ/それ/それ/それ/それらの代名詞)が実行中でない場合、ゴーストを実行する。
  -rwt, --register-to-windows-terminal : Windows端末に登録する（-g <ghost name> または -gp <ghost folder path> が必要）．
        -rwt-name <name>               : 指定された名前のWindows端末に登録する（-rwtとの組み合わせでのみ動作）。
        -rwt-icon <icon>               : 指定されたアイコン（PNGまたはICOのパス）でWindows端末に登録します（-rwtとの組み合わせのみ）。
  --disable-text <text types>|all      : 不要なテキスト（','で分割されたもの）またはそのすべてを無効にする。
        root                           : は、rootでterminalを実行したときのイースターエッグテキストを無効にします。
        event                          : ゴーストのイベントがないときの警告文を表示しないようにします。
        WindowsTerminal                : Windows Terminalをインストールするか、このexeを-rwt (-g|-gp)で実行するように指示するテキストを無効化します。
        FiraCode                       : Fira Codeフォントを試していることを示すテキストを表示しないようにします。
example:
  ghost-terminal -g "Taromati2" -rwt --disable-text event,WindowsTerminal,FiraCode
```

例えば、こんな感じです：  

```bat
//...
..\saori\ghost_terminal.exe -g Taromati2 -c reload
@echo on
```

ghost_name には、さくら（`0`）側の名前、または `ShioriEcho.GetName` が返す `GhostName` 、または descript.txt に記述されたゴースト名を指定できます。  

## イベントリスト  

ghost_terminal は `X-SSTP-PassThru-*` を介して ghost と情報通信を行う（[ドキュメント]( http://ssp.shillest.net/ukadoc/manual/spec_shiori3.html )を参照）。  

### 仮想端子配列  

ghost_terminalの出力は、プレーンテキストではなく、仮想端末のシーケンスでレンダリングされます。
参考：[コンソールの仮想ターミナル シーケンス](https://learn.microsoft.com/ja-jp/windows/console/console-virtual-terminal-sequences)  
文字色、背景色、フォントなど、端末の表示を制御するために使用できます。  

### 開始、終了

- `ShioriEcho.Begin`  
  このゴーストが起動したときのghost_terminalイベント  
  - `Reference0`  
    ターミナルバージョン  
  - 戻り値  
    - `X-SSTP-PassThru-Tittle` (オプション)  
      端末のタイトルを設定する  
    - `X-SSTP-PassThru-Icon` (オプション)  
      端末のアイコンを設定する  
    - `X-SSTP-PassThru-SmallIcon` (オプション)  
      端末の小さいアイコンを設定する  
      設定されていない場合、`X-SSTP-PassThru-Icon`と一致する  
    - `X-SSTP-PassThru-CustomLoginInfo` (optional)  
      設定された場合、端末はデフォルトのログイン情報を表示せず、この  
      この戻り値は、単純にエスケープされます：  
      - `\n` は改行に変換されます。  
      - `\t` はタブに変換されます。  
      - `\\` は `\` に変換される。  
- `ShioriEcho.End`  
  通常のプログラム終了時に発生するghost_terminalイベント  
  - 戻り値  
    無視される、**が、sakura scriptは正常に実行される**。  
- `ShioriEcho.GetName`  
  ghost_terminalが名前を取得したときのイベント  
  - 戻り値  
    - `X-SSTP-PassThru-GhostName` (オプション)  
      ゴーストの名前を表示する  
    - `X-SSTP-PassThru-UserName` (オプション)  
      ユーザー名を表示する  

### コマンドを実行する  

- `ShioriEcho`  
  コマンドが入力された後のイベント  
  - `Reference0`  
    端末で収集したコマンド  
  - 戻り値  
    無視される、**ただし、sakura scriptは正常に実行される**。  
- `ShioriEcho.GetResult`  
  バリューリザルトイベントのクエリ  
  - 可能な戻り値 1  
    - `X-SSTP-PassThru-Result`  
      コンテンツを表示し、次のコマンドに進んでフェッチする  
    - `X-SSTP-PassThru-Type` (オプション)  
      付加情報：値型  
  - 可能な戻り値 2  
    - `X-SSTP-PassThru-Special`  
      コンテンツを表示し、次のコマンドに進む fetch  
      この戻り値は、単純にエスケープされます：  
      - `\n` は改行に変換されます。  
      - `\t` はタブに変換されます。  
      - `\\` は `\` に変換される。  
  - 可能な戻り値 3  
    - エンプティ  
      1秒待って `ShioriEcho.GetResult` を再実行する。  
  - その他
    - `X-SSTP-PassThru-State`  
      この返り値は、上記の返り値に重ね合わせることができます  
      もし `End` であれば、端末は終了する。  

### その他  

- `ShioriEcho.TabPress`  
  コマンドは、タブでイベントを完了させます。  
  - `Reference0`  
    端末で収集したコマンド  
  - `Reference1`  
    タブを押したときにカーソルがあったコマンドの最初の文字（開始値0）。  
  - `Reference2`  
    ユーザーが最初に連続してタブを押した回数（開始値0）  
  - `Reference3`  
    この一連のタブ補完の中で、最初にタブを押したときにカーソルがあったコマンドの最初の文字（開始値0）。  
  - 戻り値  
    - `X-SSTP-PassThru-Command` (オプション)  
      コマンドをこの内容で置き換える  
    - `X-SSTP-PassThru-InsertIndex` (オプション)  
      カーソルをこの位置に移動させる（提供されない場合は変更しない）  
    - `X-SSTP-PassThru-OldInsertIndex`（オプション）  
      後続の `Reference3` をこの内容に更新し、この一連のタブコンプリートが終了するまで有効とする（提供されない場合は変更しない）  
- `ShioriEcho.CommandUpdate`  
  コマンド更新時のイベント  
  - `Reference0`  
    端末で収集したコマンド  
  - 戻り値  
    - `X-SSTP-PassThru-CommandForDisplay` (オプション)  
      表示されているコマンドをこの内容に置き換えます。  
      ゴーストはこのイベントを利用して、パスワード入力時のマスク、通常読み上げ時のシンタックスハイライトなどの機能を実装することができます。  
- `ShioriEcho.CommandPrompt`  
  コマンドプロンプトが更新されたときのイベント  
  - 戻り値  
    - X-SSTP-PassThru-Prompt` (オプション)  
      表示されているコマンドプロンプトをこの内容で置き換える  
      ゴーストはこのイベントでコマンドプロンプトを動的に変更することができます。  
- `ShioriEcho.CommandComplete`  
  ユーザーがコマンドラインの右端にある `→` を押したときのイベント  
  - `Reference0`  
    端末が収集したコマンド  
  - `Reference1`  
    カーソルがある位置で `→` を押したときのコマンドの最初の文字（開始値は0）。  
  - 戻り値  
    - `X-SSTP-PassThru-Command` (オプション)  
      コマンドをこの内容で置き換える  
      ゴーストは、このイベントでコマンドをオートコンプリートすることができます。  
    - `X-SSTP-PassThru-InsertIndex` (オプション)  
      カーソルをこの位置に移動させる（提供されない場合はそのままにする）  

### コマンド履歴  

- `ShioriEcho.CommandHistory.New`  
  コマンドヒストリーの末尾に空のコマンドが追加されたときのイベント  
  - 戻り値  
    無視される、**ただし、sakura scriptは正常に実行される**。
- `ShioriEcho.CommandHistory.Update`  
   コマンドヒストリーが更新されたときのイベント  
  - `Reference0`  
    historyコマンドの内容  
  - `Reference1`  
    履歴コマンドのインデックス（逆順、開始値0）  
  - 戻り値  
    無視される、**ただし、sakura scriptは正常に実行される**。
- `ShioriEcho.CommandHistory.Get`  
  コマンドヒストリーを取得したときのイベント  
  - `Reference0`  
    履歴コマンドのインデックス（逆順、開始値0）
  - 戻り値
    - `X-SSTP-PassThru-Command` (オプション)  
      コマンドをこの内容で置き換える  
- `ShioriEcho.CommandHistory.NextIndex`  
  ユーザが `↑` を押したときにインデックスが更新されるときのイベント  
  - `Reference0`
    履歴コマンドのインデックス（逆順、開始値0）  
  - 戻り値
    - `X-SSTP-PassThru-Index` (オプション)  
      インデックスをこの値に更新する  
