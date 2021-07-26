### ghost_terminal  
![Preview]( ./preview.png )  
The original trial product but now feels a bit useful (  
Based on [SSTP_linker]( https://github.com/Taromati2/SSTP_linker ) and [shell_base]( https://github.com/steve02081504/shell_base )  

### Usage  
Use ghost_terminal like a system terminal  
Up/down switch command, right mouse button to quickly paste, support tab completion (if the ghost supports it)  
Type an expression supported by your ghost and then evaluate it!  
Facilitate ghost development  

### need  
The ghost support `ShioriEcho` and `ShioriEcho.GetResult`  
Such as [Taromati2](https://github.com/Taromati2/Taromati2 )  
Ps: `ShioriEcho.GetName`, `ShioriEcho.End`, `ShioriEcho.TabPress` are optional  

The ghost_terminal communicates with the ghost through `X-SSTP-PassThru-*` (see [document]( http://ssp.shillest.net/ukadoc/manual/spec_shiori3.html ))  
See below for relevant conventions and examples  

-`ShioriEcho`  
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
	Value: \0\s[0] The execution result of the expression "\_q1000-7\_q" is:\n\_q993\nType: integer\_q\n\q[◇Copy result,OnCopy,"993"] \n\q[◇Copy expression,OnCopy,"1000-7"]\n\q[◇Execute Result as Sakura Script,OnSakuraScript,"993"]\n\n\q[◇Next Evaluation,OnCalculateVar ]\n\q[◇Cancel,Cancel]\n\eb25jZSBzbyBkaXNwb3NhYmxl
	```
-`ShioriEcho.GetResult`  
  Query evaluation result event  
  * May return value 1  
	-`X-SSTP-PassThru-Result`  
	  Display the content and enter the acquisition of the next command  
	-`X-SSTP-PassThru-Type` (optional)  
	  Supplementary information: value type  
	-Example  
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
	-`X-SSTP-PassThru-Special`  
	  Display the content and enter the acquisition of the next command  
	-Example  
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
	-**`SHIORI/3.0 400 Bad Request`**  
	  Display warning information and enter the acquisition of the next command  
-`ShioriEcho.GetName`  
  Event when ghost_terminal starts  
  * Return value  
	-`X-SSTP-PassThru-GhostName` (optional)  
	  Display ghost name  
	-`X-SSTP-PassThru-UserName` (optional)  
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
-`ShioriEcho.TabPress`  
  Event when the command is to be completed  
  * `Reference0`  
	Commands collected by the terminal (Just the part before the cursor, part after it is not passed)  
  * `Reference1`  
	The user presses tab several times in a row (starting value 0)  
  * Return value  
	-`X-SSTP-PassThru-Command` (optional)  
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
-`ShioriEcho.End`  
  Event when ghost_terminal exits by typing exit  
  * Return value  
	Ignore, **but sakura script executes normally**  

### Examples  
Sample code excerpt from Taromati2  
Feel free to modify/copy/use  
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
	case CUTSPACE(reference0){
		when 'reload'{
			OnReloadShiori
			ShioriEcho.Special='Overloading'
		}
		when 'errorlog'{
			OnErrorLog
			ShioriEcho.Result=GETERRORLOG
		}
		others{
			if RE_GREP(reference0,'^\s*help\s+'){
				ShioriEcho.Special=Get_AYA_Function_Info(RE_REPLACE(reference0,'^\s*help\s+',''))
				if !ShioriEcho.Special
					ShioriEcho.Special='Not a system function'
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
	_possible_names=(GETVARLIST(_lastname),GETFUNCLIST(_lastname),GETSYSTEMFUNCLIST(_lastname),ARRAY.BeginAs(_lastname,'reload','errorlog'))
	if ARRAYSIZE(_possible_names){
		_name_after_tab=_possible_names[reference1%ARRAYSIZE(_possible_names)]
		SHIORI_FW.Make_X_SSTP_PassThru('Command',RE_REPLACE(reference0,_lastname+'$',_name_after_tab))
	}
}
On_ShioriEcho.GetResult:void {
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
