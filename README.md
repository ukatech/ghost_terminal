### ghost_terminal  
![预览图]( ./preview.png )
[SSTP_linker]( https://github.com/Taromati2/SSTP_linker )试做品  

### 用法  
如同系统终端般使用ghost_terminal  
up/down切换命令，不支持tab补全  
键入你的人格所支持的表达式随后对其求值！  
便于人格开发（实际上是相当没用的东西我知道了别骂了本就是试做品我知道现在的人格都有拖入表达式进行操作的功能哪用得到什么终端啊）  

### 需求  
支持`ShioriEcho`、`ShioriEcho.GetResult`的人格  
如[Taromati2]( https://github.com/Taromati2/Taromati2 )  
Ps：`ShioriEcho.GetName`、`ShioriEcho.End`可选  

ghost_terminal通过`X-SSTP-PassThru-*`进行与人格间的信息沟通（见[文档]( http://ssp.shillest.net/ukadoc/manual/spec_shiori3.html )）  
相关约定与范例见下  

- `ShioriEcho`  
  命令键入完毕后事件  
  * `Reference0`  
    终端所收集到的命令  
  * 返值  
    **忽略**  
- `ShioriEcho.GetResult`  
  查询求值结果事件  
  * 可能返值1  
    - `X-SSTP-PassThru-Result`  
      显示内容并进入下一行内容的获取  
    - `X-SSTP-PassThru-Type`（可选）  
      补充信息：值类型  
  * 可能返值2  
    - `X-SSTP-PassThru-Special`  
      显示内容并进入下一行内容的获取  
  * 可能返值3  
    - 空  
      等待1秒后重新发起`ShioriEcho.GetResult`  
- `ShioriEcho.GetName`  
  ghost_terminal启动时事件  
  * 返值  
    - `X-SSTP-PassThru-GhostName`（可选）  
      显示人格名  
    - `X-SSTP-PassThru-UserName`（可选）  
      显示用户名  
- `ShioriEcho.End`  
  ghost_terminal通过键入exit退出时事件  
  * 返值  
    **忽略**  

