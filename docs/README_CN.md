# ghost_terminal  

![预览图]( ./preview.png )  
原试做品，现觉得有点用（  

## 用法  

如同系统终端般使用ghost_terminal  
up/down切换命令，鼠标右键快速粘贴，支持tab补全（如果人格支持）  
键入你的人格所支持的表达式随后对其求值！  
便于人格开发  

## 命令行参数  

```text
ghost terminal v13.9.0.1

ghost_terminal [options]
选项：
  -h, --help                           : 显示此帮助信息并退出。
  -v, --version                        : shows the version number and exits.
  -c, --command <command>              : 运行指定的命令并退出。
  -s, --sakura-script <script>         : 运行指定的Sakura脚本并退出。
  -g, --ghost <ghost>                  : 通过名字链接到指定的ghost。
  -gh, --ghost-hwnd <hwnd>             : 通过HWND链接到指定的ghost。
  -gp, --ghost-folder-path <path>      : 按文件夹路径链接到指定的ghost。
  -r, --run-ghost                      : 如果（它/她/他/他们/其他代称）当前没有运行，则运行该ghost。
  -rwt, --register-to-windows-terminal : 注册到Windows终端（需要 -g <ghost name> 或 -gp <ghost文件夹路径>）。
        -rwt-name <name>               : 以指定的名字注册到Windows终端（只与-rwt一起工作）。
        -rwt-icon <icon>               : 用指定的图标（PNG或ICO路径）注册到Windows终端（只适用于-rwt）。
  --disable-text <text types>|all      : disable some unnecessary text(split by ',') or all of them.
        root                           : disables the easter egg text when running terminal as root.
        event                          : disables the warning text when your ghost not having some events.
        WindowsTerminal                : disables the text telling you to install Windows Terminal or run this exe with -rwt (-g|-gp).
        FiraCode                       : disables the text telling you try Fira Code font.
example:
  ghost-terminal -g "Taromati2" -rwt --disable-text event,WindowsTerminal,FiraCode
```

比如：  

```bat
//...
..\saori\ghost_terminal.exe -g Taromati2 -c reload
@echo on
```

ghost_name 可以是Sakura（`\0`）端名称，或`ShioriEcho.GetName`返回的`GhostName`，或descript.txt中的ghost名称  

## 事件列表  

ghost_terminal通过`X-SSTP-PassThru-*`进行与人格间的信息沟通（见[文档]( http://ssp.shillest.net/ukadoc/manual/spec_shiori3.html )）  

### 虚拟终端序列  

任何ghost_terminal的输出都将经过虚拟终端序列渲染，而不是普通的文本。  
参考：[虚拟终端序列](https://learn.microsoft.com/zh-cn/windows/console/console-virtual-terminal-sequences)  
你可以使用它来控制终端的显示效果，如文字颜色、背景色、字体等。  

### 默认行为  

在以下事件中大多数事件如果ghost没有实现，terminal将使用默认行为  
换言之，**如果你想你的ghost支持terminal，你只需要实现`ShioriEcho`事件即可**  
其余事件只在你想要更改默认行为或者提供更为丰富的体验时才需要进行定义  

### 启动、终止

- `ShioriEcho.Begin`  
  ghost_terminal对此ghost启动时事件  
  - `Reference0`  
    终端版本  
  - `Reference1`
    终端启动方式  
    当terminal以`-s`启动时，为`SakuraScript`  
    当terminal以`-c`启动时，为`Command`  
    否则为`Common`  
  - 返值  
    - `X-SSTP-PassThru-Tittle`（可选）  
      设置终端标题  
    - `X-SSTP-PassThru-Icon`（可选）  
      设置终端图标（PNG或ICO路径）  
    - `X-SSTP-PassThru-SmallIcon`（可选）  
      设置终端小图标（PNG或ICO路径）  
      如果不设置，将与`X-SSTP-PassThru-Icon`保持一致  
    - `X-SSTP-PassThru-CustomLoginInfo`（可选）
      如果设置了，终端将不显示默认的登录信息，但会显示这个的内容。  
      这个返回值将经过简单转义：  
      - `\n` 将转换为换行  
      - `\t` 将转变为制表符  
      - `\\` 将转化为 `\`  
- `ShioriEcho.End`  
  ghost_terminal正常程序退出时事件  
  - 返值  
    忽略，**但言灵正常执行**  
- `ShioriEcho.GetName`  
  ghost_terminal获取名称时事件  
  - 返值  
    - `X-SSTP-PassThru-GhostName`（可选）  
      显示人格名  
    - `X-SSTP-PassThru-UserName`（可选）  
      显示用户名  

### 执行命令  

- `ShioriEcho`  
  命令键入完毕后事件  
  - `Reference0`  
    终端所收集到的命令  
  - 返值  
    如果你没有定义`ShioriEcho.GetResult`，你可以在这里返回`ShioriEcho.GetResult`所支持的任意返回值，它们同样会被terminal正确处理（除了可能返回值3，在没有定义`ShioriEcho.GetResult`的情况下`ShioriEcho`中什么都不返回只会进入下一个命令的获取）  
    若你定义了`ShioriEcho.GetResult`，则这里的返回值将被忽略（**但言灵正常执行**）  
- `ShioriEcho.GetResult`  
  查询求值结果事件  
  - 可能返值1  
    - `X-SSTP-PassThru-Result`  
      显示内容并进入下一命令的获取  
    - `X-SSTP-PassThru-Type`（可选）  
      补充信息：值类型  
  - 可能返值2  
    - `X-SSTP-PassThru-Special`  
      显示内容并进入下一命令的获取  
      这个返回值将经过简单转义：  
      - `\n` 将转换为换行  
      - `\t` 将转变为制表符  
      - `\\` 将转化为 `\`  
  - 可能返值3  
    - 空  
      等待1秒后重新发起`ShioriEcho.GetResult`  
  - 其他
    - `X-SSTP-PassThru-State`  
      此返值可叠加在上述返值中  
      若其为`End`，终端终止  
      若其为`Continue`，终端不显示任何内容并进入下一命令的获取  

### 其他  

- `ShioriEcho.TabPress`  
  命令通过tab补全时事件  
  - `Reference0`  
    终端所收集到的命令  
  - `Reference1`  
    按下tab时光标所在命令的第几个字符（起始值0）  
  - `Reference2`  
    用户连续第几次按下tab（起始值0）  
  - `Reference3`  
    这一系列tab补全中初次按下tab时光标所在命令的第几个字符（起始值0）  
  - 返值  
    - `X-SSTP-PassThru-Command`（可选）  
      将命令替换为此内容  
    - `X-SSTP-PassThru-InsertIndex`（可选）  
      将光标移动到此位置（若不提供则保持不变）  
    - `X-SSTP-PassThru-OldInsertIndex`（可选）  
      将后续的`Reference3`更新为此内容，有效性直到这一系列的tab补全结束（若不提供则保持不变）  
- `ShioriEcho.CommandUpdate`  
  命令更新时事件  
  - `Reference0`  
    终端所收集到的命令  
  - 返值  
    - `X-SSTP-PassThru-CommandForDisplay`（可选）  
      将显示的命令替换为此内容  
      ghost可以通过此事件实现密码输入时的掩码、正常读入时的语法高亮、以及其他功能  
- `ShioriEcho.CommandPrompt`  
  命令提示符更新时事件  
  - 返值  
    - `X-SSTP-PassThru-Prompt`（可选）  
      将显示的命令提示符替换为此内容  
      ghost可以通过此事件实现命令提示符的动态变化  
- `ShioriEcho.CommandComplete`  
  用户于命令行最右端按下`→`时事件  
  - `Reference0`  
    终端所收集到的命令  
  - `Reference1`  
    按下`→`时光标所在命令的第几个字符（起始值0）  
  - 返值  
    - `X-SSTP-PassThru-Command`（可选）  
      将命令替换为此内容  
      ghost可以通过此事件实现命令的自动补全  
    - `X-SSTP-PassThru-InsertIndex`（可选）  
      将光标移动到此位置（若不提供则保持不变）  

### 命令历史  

- `ShioriEcho.CommandHistory.New`  
  命令历史末尾新增空命令时事件  
  - 返值  
    忽略，**但言灵正常执行**
- `ShioriEcho.CommandHistory.Update`  
   命令历史更新时事件  
  - `Reference0`  
    历史命令的内容  
  - `Reference1`  
    历史命令的索引（倒序，起始值0）  
  - 返值  
    忽略，**但言灵正常执行**
- `ShioriEcho.CommandHistory.Get`  
  命令历史获取时事件  
  - `Reference0`
    历史命令的索引（倒序，起始值0）  
  - 返值
    - `X-SSTP-PassThru-Command`（可选）  
      将命令替换为此内容  
- `ShioriEcho.CommandHistory.ForwardIndex`  
  用户按压`↑`时更新索引时事件  
  - `Reference0`
    历史命令的索引（倒序，起始值0）  
  - `Reference1`
    `index`被期望增加的值  
  - 返值
    - `X-SSTP-PassThru-Index`（可选）  
      将索引更新为此数值  
