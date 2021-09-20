### ghost_terminal  
![预览图]( ./preview.png )  
原试做品，现觉得有点用（  
基于[SSTP_linker]( https://github.com/Taromati2/SSTP_linker )与[shell_base]( https://github.com/steve02081504/shell_base )  

### 用法  
如同系统终端般使用ghost_terminal  
up/down切换命令，鼠标右键快速粘贴，支持tab补全（如果人格支持）  
键入你的人格所支持的表达式随后对其求值！  
便于人格开发  

### 需求  
支持`ShioriEcho`、`ShioriEcho.GetResult`的人格  
如[Taromati2]( https://github.com/Taromati2/Taromati2 )  
Ps：`ShioriEcho.GetName`、`ShioriEcho.End`、`ShioriEcho.TabPress`可选  

ghost_terminal通过`X-SSTP-PassThru-*`进行与人格间的信息沟通（见[文档]( http://ssp.shillest.net/ukadoc/manual/spec_shiori3.html )）  
相关约定与范例见下  

- `ShioriEcho`  
  命令键入完毕后事件  
  * `Reference0`  
	终端所收集到的命令  
  * 返值  
	忽略，**但言灵正常执行**  
  * 示例  
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
	Value: \0\s[0]表达式『\_q1000-7\_q』的执行结果为：\n\_q993\n类型：整数\_q\n\q[◇复制结果,OnCopy,"993"]\n\q[◇复制表达式,OnCopy,"1000-7"]\n\q[◇结果作Sakura Script執行,OnSakuraScript,"993"]\n\n\q[◇求值下一个,OnCalculateVar]\n\q[◇无用,Cancel]\n\eb25jZSBzbyBkaXNwb3NhYmxl
	```
- `ShioriEcho.GetResult`  
  查询求值结果事件  
  * 可能返值1  
	- `X-SSTP-PassThru-Result`  
	  显示内容并进入下一命令的获取  
	- `X-SSTP-PassThru-Type`（可选）  
	  补充信息：值类型  
	- 示例  
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
  * 可能返值2  
	- `X-SSTP-PassThru-Special`  
	  显示内容并进入下一命令的获取  
	  var8后这个返回值将经过简单转义：  
	  * `\n` 将转换为换行  
	  * `\t` 将转变为制表符  
	  * `\\` 将转化为 `\`  
	- 示例  
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
	  X-SSTP-PassThru-Special: 已取消求值
	  ```
  * 可能返值3  
	- 空  
	  等待1秒后重新发起`ShioriEcho.GetResult`  
  * 可能返值4  
	- **`SHIORI/3.0 400 Bad Request`**  
	  显示警告信息并进入下一命令的获取  
- `ShioriEcho.GetName`  
  ghost_terminal启动时事件  
  * 返值  
	- `X-SSTP-PassThru-GhostName`（可选）  
	  显示人格名  
	- `X-SSTP-PassThru-UserName`（可选）  
	  显示用户名  
  * 示例  
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
  命令待补全时事件  
  * `Reference0`  
	终端所收集到的命令（光标前部分，其后部分不做传递）  
  * `Reference1`  
	用户连续第几次按下tab（起始值0）  
  * 返值  
	- `X-SSTP-PassThru-Command`（可选）  
	  光标前的命令替换为此内容  
  * 示例  
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
- `ShioriEcho.End`  
  ghost_terminal通过键入exit退出时事件  
  * 返值  
	忽略，**但言灵正常执行**  

### 范例  
范例代码节选于Taromati2  
随意修改/复制/使用  
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
			ShioriEcho.Special='重载中'
		}
		when 'errorlog'{
			OnErrorLog
			ShioriEcho.Result=GETERRORLOG
		}
		others{
			if RE_GREP(reference0,'^\s*help\s+'){
				ShioriEcho.Special=Get_AYA_Function_Info(RE_REPLACE(reference0,'^\s*help\s+',''))
				if !ShioriEcho.Special
					ShioriEcho.Special='不是系统函数'
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
			BUGNow('ShioriEcho.Special内容为空')
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
