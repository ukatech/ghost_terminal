### ghost_terminal  
![プレビュー]( ./preview.png )  
元は試供品ですが、今はちょっと使える感じです(  

### 使用方法  
ghost_terminalをシステム端末のように使用する  
上下切り替えコマンド、マウスの右ボタンで素早く貼り付け、タブ補完のサポート（ゴーストがサポートしている場合のみ）  
ゴーストに支持された表現を入力して、評価しよう  
ゴーストの開発を促進する  

#### コマンドラインパラメーター(Ver10以降)  
```bat
ghost_terminal.exe -g ghost_name -c command
# or
ghost_terminal.exe -gh ghost_hwnd -c command
# or (Ver11以降)
ghost_terminal.exe [-gh | -g] ghost -s sakura-script
```
例えば：  
```bat
//...
..\saori\ghost_terminal.exe -g Taromati2 -c reload
@echo on
```
ghost_name には、さくら（`\0`）側の名前、または `ShioriEcho.GetName` が返す `GhostName` 、（Ver11以降）またはdescript.txtのゴースト名のいずれかを指定できる。  

### 必要なもの 
ゴーストは `ShioriEcho` と `ShioriEcho.GetResult` をサポートしています。  
例えば[Taromati2](https://github.com/Taromati2/Taromati2 )など。  
Ps: `ShioriEcho.GetName`, `ShioriEcho.End`, `ShioriEcho.TabPress`, `ShioriEcho.Begin` はオプションです。  

ghost_terminalは`X-SSTP-PassThru-*`（[ドキュメント]( http://ssp.shillest.net/ukadoc/manual/spec_shiori3.html )参照）でゴーストと通信する。 
関連する規約や例については、以下をご参照ください。  

- `ShioriEcho`  
  コマンド入力後のイベント  
  * `Reference0`  
	端末で収集したコマンド  
  * 戻り値  
	無視する、**sakuraスクリプトは正常に実行される**。  
  * 例  
	```
	// request
	GET SHIORI/3.0
	Charset: UTF-8
	Sender: Ghost Terminal
	SenderType: external,sstp
	SecurityLevel: local
	Status: balloon(0=0)
	ID: ShioriEcho
	Reference0: 1000-7

	// response (Execution time : 0[ms])
	SHIORI/3.0 200 OK
	Sender: AYA
	Charset: UTF-8
	Value: \0\s[0]という式の実行結果は、"\_q1000-7\_q"です：\n\_q993\nタイプ：整数\_q\n\q[◇コピー結果,OnCopy,"993"] \n\q[◇コピー表現について,OnCopy,"1000-7"]\n\q[◇結果をさくらスクリプトとして実行する,OnSakuraScript,"993"]\n\n\q[◇次の評価,OnCalculateVar]\n\q[◇キャンセル,Cancel]\n\eb25jZSBzbyBkaXNwb3NhYmxl
	```
- `ShioriEcho.GetResult`  
  クエリ評価結果イベント  
  * 戻り値 1 を返すことがあります。  
	- `X-SSTP-PassThru-Result`。  
	  内容を表示し、次のコマンドの取得を入力します。  
	- `X-SSTP-PassThru-Type` (オプション)  
	  補足：バリュータイプ  
	- 例  
	  ```
	  // request
	  GET SHIORI/3.0
	  Charset: UTF-8
	  Sender: Ghost Terminal
	  SenderType: external,sstp
	  SecurityLevel: local
	  Status: talking,balloon(0=0)
	  ID: ShioriEcho.GetResult

	  // response (Execution time : 0[ms])
	  SHIORI/3.0 200 OK
	  Sender: AYA
	  Charset: UTF-8
	  Value: 
	  X-SSTP-PassThru-Result: 993
	  X-SSTP-PassThru-Type: 整数
	  ```
  * 返り値が 2 になる可能性があります。  
	- `X-SSTP-PassThru-Special`（エックス-エスティーピー-パススルー-スペシャル）。  
	  内容を表示し、次のコマンドの取得を入力します。  
	  Ver8以降では、この戻り値は単純にエスケープされます。  
	  * `\n` は改行に変換されます。  
	  * `\t` はタブに変換されます。  
	  * `\\` は `\` に変換される。  
	- 例  
	  ```
	  // request
	  GET SHIORI/3.0
	  Charset: UTF-8
	  Sender: Ghost Terminal
	  SenderType: external,sstp
	  SecurityLevel: local
	  Status: balloon(0=0)
	  ID: ShioriEcho.GetResult

	  // response (Execution time : 0[ms])
	  SHIORI/3.0 200 OK
	  Sender: AYA
	  Charset: UTF-8
	  Value: 
	  X-SSTP-PassThru-Special: 評価中止
	  ```
  * 返り値が3である可能性があります。  
	- ヌル  
	  1秒待って、`ShioriEcho.GetResult`を再実行する。  
  * 返り値が4である可能性があります。  
	- **`SHIORI/3.0 400 Bad Request`**  
	  警告情報を表示し、次のコマンドの取得に入る  
	  Ver9以降：ゴーストが `ShioriEcho.GetResult` のリクエストに正常に応答した場合、ghost_terminal は `返り値が3である可能性があります` として、そのようなリターンメッセージを無視して1秒待ちます。  
- `ShioriEcho.GetName`  
  ghost_terminalがゴーストから名前を取得したときのイベント  
  Ver10以前：ghost_terminalが起動したときのイベント  
  * `Reference0` (Ver10以降)  
	ターミナルバージョン  
  * 戻り値  
	- `X-SSTP-PassThru-GhostName` (オプション)  
	  ゴーストの名前を表示する  
	- `X-SSTP-PassThru-UserName` (オプション)  
	  ユーザー名を表示する  
  * 例  
	```
	// request
	GET SHIORI/3.0
	Charset: UTF-8
	Sender: Ghost Terminal
	SenderType: external,sstp
	SecurityLevel: local
	Status: balloon(0=0)
	ID: ShioriEcho.GetName

	// response (Execution time : 0[ms])
	SHIORI/3.0 200 OK
	Sender: AYA
	Charset: UTF-8
	Value: 
	X-SSTP-PassThru-GhostName: Taromati2
	X-SSTP-PassThru-UserName: steve
	```
- `ShioriEcho.TabPress`  
  コマンドを完了するときのイベント  
  * `Reference0`  
	端末が収集したコマンド（カーソルより前の部分のみ、カーソルより後の部分は渡しません。）  
  * `Reference1`  
	ユーザーがタブを連続して数回押した場合（開始値0）  
  * 戻り値  
	- `X-SSTP-PassThru-Command` (オプション)  
	  カーソルの前のコマンドをこの内容に置き換えます。  
  * 例  
	```
	// request
	GET SHIORI/3.0
	Charset: UTF-8
	Sender: Ghost Terminal
	SenderType: external,sstp
	SecurityLevel: local
	Status: balloon(0=0)
	ID: ShioriEcho.TabPress
	Reference0: On
	Reference1: 0

	// response (Execution time : 0[ms])
	SHIORI/3.0 200 OK
	Sender: AYA
	Charset: UTF-8
	Value: 
	X-SSTP-PassThru-Command: OnNoMatchingEvent.DumpedList


	// request
	GET SHIORI/3.0
	Charset: UTF-8
	Sender: Ghost Terminal
	SenderType: external,sstp
	SecurityLevel: local
	Status: balloon(0=0)
	ID: ShioriEcho.TabPress
	Reference0: On
	Reference1: 1

	// response (Execution time : 0[ms])
	SHIORI/3.0 200 OK
	Sender: AYA
	Charset: UTF-8
	Value: 
	X-SSTP-PassThru-Command: OnNoMatchingEvent.IgnoreList


	// request
	GET SHIORI/3.0
	Charset: UTF-8
	Sender: Ghost Terminal
	SenderType: external,sstp
	SecurityLevel: local
	Status: balloon(0=0)
	ID: ShioriEcho.TabPress
	Reference0: 'Just a '+use 
	Reference1: 0

	// response (Execution time : 0[ms])
	SHIORI/3.0 200 OK
	Sender: AYA
	Charset: UTF-8
	Value: 
	X-SSTP-PassThru-Command: 'Just a '+username
	```
- `ShioriEcho.Begin` (Ver10以降)  
  このゴーストのghost_terminal起動時のイベント。  
  * `Reference0`  
	端末バージョン  
  * 戻り値  
	無視する、**sakuraスクリプトは正常に実行される**。  
- `ShioriEcho.End`  
  exitと入力してghost_terminalが終了したときのイベント  
  Ver9以降：通常のプログラム終了時に発生します。  
  * 戻り値  
	無視する、**sakuraスクリプトは正常に実行される**。  

### 仮想端末のシーケンス  
Ver13以降、出力されたものは通常のテキストではなく、仮想端末のシーケンスで描画されるようになります。 
参考 [バーチャル・ターミナル・シーケンス](https://learn.microsoft.com/ja-jp/windows/console/console-virtual-terminal-sequences)  
文字色、背景色、フォントなど、端末の表示を制御するために使用できます。

### 例  
Taromati2からのサンプルコード抜粋  
改変・コピー・使用自由  
```aya
SHIORI_EV.On_Has_Event : void {
	SHIORI_FW.Make_X_SSTP_PassThru('Result',ISFUNC(reference0)||ISFUNC('On_'+reference0)||ISFUNC('SHIORI_EV.'+reference0)||ISFUNC('SHIORI_EV.On_'+reference0))
}
```
```aya
On_ShioriEcho.GetName:void {
	SHIORI_FW.Make_X_SSTP_PassThru('GhostName',ghostname)
	SHIORI_FW.Make_X_SSTP_PassThru('UserName',username)
}
On_ShioriEcho {
	ClearShioriEchoVar
	reference0 = reference.raw[0]
	case CUTSPACE(reference0){
		when 'reload'{
			ReloadFromTerminal=1
			OnReloadShiori
		}
		when 'errorlog'{
			OnErrorLog
			ShioriEcho.Result=GETERRORLOG
		}
		others{
			if RE_GREP(reference0,'^\s*(help|openfunc)\s+'){
				if RE_GREP(reference0,'^\s*help\s+'){
					_funcname=RE_REPLACE(reference0,'^\s*help\s+','')
					ShioriEcho.Special=Get_AYA_Function_Info(_funcname)
					if ShioriEcho.Special{
						'あやのじてん\n'+ShioriEcho.Special+'\n/
						\q[◇オンラインドキュメントを開く,OnUrlOpen,'+Get_AYA_Function_Doc(_funcname)+']\n/
						\q[◇キャンセル,Cancel]\n/
						'
						--
						IgnoreChoiceTimeout
					}
					else
						ShioriEcho.Special='システム機能ではありません'
				}
				elseif RE_GREP(reference0,'^\s*openfunc\s+'){
					_funcname=RE_REPLACE(reference0,'^\s*openfunc\s+','')
					_info=GETFUNCINFO(_funcname)
					_path=SPLITPATH(_info[0])
					if _path
						_path=_path[2]+_path[3]
					else
						_path=_info[0]
					if _info!=-1{
						OnOpenDicWithLineNum(_info[0],_info[1])
						ShioriEcho.Special="Open %(_path) line %(_info[1])"
					}
					else
						ShioriEcho.Special='ユーザー機能ではありません'
				}
			}
			else{
				OnCalculateVar
				--
				IgnoreChoiceTimeout
			}
		}
	}
}
On_ShioriEcho.TabPress{
	_lastname=RE_REPLACE(reference0,'^[\s\S]*[\[\]\(\)\+\-\*\/\=\'+"'"+'\" ]','')
	_possible_names=IARRAY
	if !reference0[1,' ']
		_possible_names,=ARRAY.BeginAs(_lastname,'reload','errorlog','openfunc','help')
	if reference0[0,' '] == 'help'
		_possible_names=GETSYSTEMFUNCLIST(_lastname)
	elseif reference0[0,' '] == 'openfunc'
		_possible_names=GETFUNCLIST(_lastname)
	else
		_possible_names,=(GETVARLIST(_lastname),GETFUNCLIST(_lastname),GETSYSTEMFUNCLIST(_lastname))
	if ARRAYSIZE(_possible_names){
		_name_after_tab=_possible_names[reference1%ARRAYSIZE(_possible_names)]
		SHIORI_FW.Make_X_SSTP_PassThru('Command',RE_REPLACE(reference0,_lastname+'$',_name_after_tab))
	}
}
On_ShioriEcho.GetResult {
	if ISVAR('ShioriEcho.Special'){
		SHIORI_FW.Make_X_SSTP_PassThru('Special',ShioriEcho.Special)
		if !ShioriEcho.Special
			BUGNow('ShioriEcho.Specialのコンテンツは空です。')
	}
	else{
		if ISVAR('ShioriEcho.Result'){
			SHIORI_FW.Make_X_SSTP_PassThru('Result',ValueTOstring(ShioriEcho.Result))
			SHIORI_FW.Make_X_SSTP_PassThru('Type',GETTYPE.string((ShioriEcho.Result)))
		}
	}
	ClearShioriEchoVar
}
ClearShioriEchoVar:void {
	ERASEALLVARBEGINAS('ShioriEcho.')
}
```
