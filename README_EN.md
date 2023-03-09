### ghost_terminal  
![Preview]( ./preview.png )  
The original trial product but now feels a bit useful (  

### Usage  
Use ghost_terminal like a system terminal  
Up/down switch command, right mouse button to quickly paste, support tab completion (if the ghost supports it)  
Type an expression supported by your ghost and then evaluate it!  
Facilitate ghost development  

#### Command line parameters (after Ver10)  
```bat
ghost_terminal.exe -g ghost_name -c command
# or
ghost_terminal.exe -gh ghost_hwnd -c command
# or (after Ver11)
ghost_terminal.exe [-gh | -g] ghost -s sakura-script
```
For example:  
```bat
//...
..\saori\ghost_terminal.exe -g Taromati2 -c reload
@echo on
```
ghost_name can be either the name of the Sakura (`\0`) side, or the `GhostName` returned by `ShioriEcho.GetName`, (after Ver11) or ghost name in descript.txt  

### need  
The ghost support `ShioriEcho` and `ShioriEcho.GetResult`  
Such as [Taromati2](https://github.com/Taromati2/Taromati2 )  
Ps: `ShioriEcho.GetName`, `ShioriEcho.End`, `ShioriEcho.TabPress`, `ShioriEcho.Begin` are optional  

The ghost_terminal communicates with the ghost through `X-SSTP-PassThru-*` (see [document]( http://ssp.shillest.net/ukadoc/manual/spec_shiori3.html ))  
See below for relevant conventions and examples  

- `ShioriEcho`  
  Event after the command entered  
  * `Reference0`  
	Commands collected by the terminal  
  * Return value  
	Ignore, **but sakura script executes normally**  
  * Example  
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
	Value: \0\s[0]The execution result of the expression "\_q1000-7\_q" is:\n\_q993\nType: integer\_q\n\q[◇Copy result,OnCopy,"993"] \n\q[◇Copy expression,OnCopy,"1000-7"]\n\q[◇Execute Result as Sakura Script,OnSakuraScript,"993"]\n\n\q[◇Next Evaluation,OnCalculateVar ]\n\q[◇Cancel,Cancel]\n\eb25jZSBzbyBkaXNwb3NhYmxl
	```
- `ShioriEcho.GetResult`  
  Query evaluation result event  
  * May return value 1  
	- `X-SSTP-PassThru-Result`  
	  Display the content and enter the acquisition of the next command  
	- `X-SSTP-PassThru-Type` (optional)  
	  Supplementary information: value type  
	- Example  
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
	  X-SSTP-PassThru-Type: integer
	  ```
  * May return value 2  
	- `X-SSTP-PassThru-Special`  
	  Display the content and enter the acquisition of the next command  
	  After Ver8 this return value will be simply escaped.  
	  * `\n` will be converted to a newline  
	  * `\t` will be converted to a tab  
	  * `\\` will be converted to `\`  
	- Example  
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
	  X-SSTP-PassThru-Special: Evaluation cancelled
	  ```
  * May return value 3  
	- null  
	  Wait for 1 second and re-initiate `ShioriEcho.GetResult`  
  * May return value 4  
	- **`SHIORI/3.0 400 Bad Request`**  
	  Display warning information and enter the acquisition of the next command  
	  After Ver9: When ghost responds normally to any `ShioriEcho.GetResult` request, ghost_terminal will ignore such a return message and wait one second, as it is `May return value 3`  
- `ShioriEcho.GetName`  
  Event when ghost_terminal get name from ghost  
  Pre-Ver10: event when ghost_terminal starts  
  * `Reference0` (after Ver10)  
	Terminal version  
  * Return value  
	- `X-SSTP-PassThru-GhostName` (optional)  
	  Display ghost name  
	- `X-SSTP-PassThru-UserName` (optional)  
	  Show username  
  * Example  
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
  Event when the command is to be completed  
  * `Reference0`  
	Commands collected by the terminal (Just the part before the cursor, part after it is not passed)  
  * `Reference1`  
	The user presses tab several times in a row (starting value 0)  
  * Return value  
	- `X-SSTP-PassThru-Command` (optional)  
	  Replace the command before the cursor with this content  
  * Example  
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
- `ShioriEcho.Begin` (after Ver10)  
  Event when ghost_terminal startup for this ghost  
  * `Reference0`  
	Terminal version  
  * Return value  
	Ignore, **but sakura script executes normally**  
- `ShioriEcho.End`  
  Event when ghost_terminal exits by typing exit  
  After Ver9: any normal program exit will trigger this  
  * Return value  
	Ignore, **but sakura script executes normally**  

### Virtual Terminal Sequences  
After Ver13, any output will be rendered by the Virtual Terminal Sequence instead of normal text.  
Reference: [Virtual Terminal Sequence](https://learn.microsoft.com/en-gb/windows/console/console-virtual-terminal-sequences)  
You can use it to control the display of the terminal, e.g. text colour, background colour, font, etc.

### Examples  
Sample code excerpt from Taromati2  
Free to modify/copy/use  
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
						'aya's own underlying functions\n'+ShioriEcho.Special+'\n/
						\q[◇Open online document,OnUrlOpen,'+Get_AYA_Function_Doc(_funcname)+']\n/
						\q[◇Cancel,Cancel]\n/
						'
						--
						IgnoreChoiceTimeout
					}
					else
						ShioriEcho.Special='Not a system function'
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
						ShioriEcho.Special='Not a user function'
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
			BUGNow('ShioriEcho.Special content is empty')
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
