# ghost_terminal  

![preview image]( https://github.com/ukatech/ghost_terminal/assets/31927825/bc02d62d-3a51-43e8-971f-45556aa97d3b )  
Original trial product, now finding it somewhat useful (  

## Usage  

Use ghost_terminal like a system terminal  
up/down toggle command, right mouse button for quick paste, support tab completion (if supported by ghost)  
Type in the expressions supported by your ghost and then evaluate them!  
Easy to use for ghost development  

## Command line arguments  

```text
ghost terminal v13.9.0.1

ghost_terminal [options]
options:
  -h, --help                            : shows this help message and exits.
  -v, --version                         : shows the version number and exits.
  -c, --command <command>               : runs the specified command and exits.
  -s, --sakura-script <script>          : runs the specified Sakura script and exits.
  -g, --ghost <ghost>                   : links to the specified ghost by name.
  -gh, --ghost-hwnd <hwnd>              : links to the specified ghost by HWND.
  -gp, --ghost-folder-path <path>       : links to the specified ghost by folder path.
  -r, --run-ghost                       : runs the ghost if (it/she/he/them/other pronouns) is not currently running.
  -rwt, --register-to-windows-terminal  : registers to the Windows terminal (requires -g <ghost name> or -gp <ghost folder path>).
        -rwt-name <name>                : registers to the Windows terminal with the specified name (only works with -rwt).
        -rwt-icon <icon>                : registers to the Windows terminal with the specified icon (PNG or ICO path) (only works with -rwt).
  --disable-text <text types>|all       : disable some unnecessary text(split by ',') or all of them.
        root                            : disables the easter egg text when running terminal as root.
        event                           : disables the warning text when your ghost is not having events.
        WindowsTerminal                 : disables the text telling you to install Windows Terminal or run this exe with -rwt (-g|-gp).
        FiraCode                        : disables the text telling you to try the Fira Code font.
example:
  ghost-terminal -g "Taromati2" -rwt --disable-text event,WindowsTerminal,FiraCode
```

For example:  

```bat
//...
..\saori\ghost_terminal.exe -g Taromati2 -c reload
@echo on
```

This command is used in this script file to reload the shiori function  
ghost terminal can be used to hook [functions in other programs](https://github.com/Taromati2/yaya-shiori/blob/b6b21b514bcb55aacd3e0df881869e3f9cc1d7e3/aya5.vcxproj#L189).

ghost_name can be the name of the Sakura (`\0`) side, or the `GhostName` returned by `ShioriEcho.GetName`, or the ghost name in the `descript.txt`  

## Event list  

ghost_terminal communicates information with ghost via `X-SSTP-PassThru-*` (see [documentation]( http://ssp.shillest.net/ukadoc/manual/spec_shiori3.html ))  

### Virtual terminal sequence  

Any ghost_terminal output will be rendered by a virtual terminal sequence, rather than plain text.  
Reference: [Console Virtual Terminal Sequences](https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences)  
You can use it to control the display of the terminal, such as text colour, background colour, font, etc.  
For example, Taroamti2 [use this output to clear the screen](https://github.com/Taromati2/ghost/blob/b6c6d66acd2ba91c3b843b6dd562bdf656d55796/master/dic/system/Debug.dic#L482L484):  

```c
when 'cls'{
	ShioriEcho.Special=CHR(27)+'[f'+CHR(27)+'[2J'
}
```

![Example image](https://github.com/ukatech/ghost_terminal/assets/31927825/402d1145-378a-4001-bebb-714afb028432)

### Default behaviour  

Most of the events in the following events will use the default behaviour if not implemented by ghost  
In other words, **if you want your ghost to support the terminal, you only need to implement the `ShioriEcho` event.**  
The rest of the events should only be defined if you want to change the default behaviour or provide a richer experience  

### Start/End

- `ShioriEcho.Begin`  
  ghost_terminal event when this ghost starts.  
  - `Reference0`  
    Terminal version  
  - `Reference1`  
    Terminal session type  
    `SakuraScript` if terminal start with `-s`  
    `Command` if terminal start with `-c`  
    `Common` if terminal start commonly  
  - Return value  
    - `X-SSTP-PassThru-Tittle` (optional)  
      Set the terminal title  
    - `X-SSTP-PassThru-Icon` (optional)  
      Set the terminal icon (PNG or ICO path)  
    - `X-SSTP-PassThru-SmallIcon` (optional)  
      Set the terminal small icon (PNG or ICO path)  
      If not set, it will be consistent with `X-SSTP-PassThru-Icon`  
    - `X-SSTP-PassThru-CustomLoginInfo` (optional)  
      If set up, the terminal will not display the default login information, but will display the content of this  
      This return value will simply escape:  
      - `\n` will be converted to a newline  
      - `\t` will be converted to tabs  
      - `\\` will be converted to `\`  
- `ShioriEcho.End`  
  ghost_terminal event on normal program exit  
  - return value  
    Ignored, **but sakura spirits execute normally**  
- `ShioriEcho.GetName`  
  Event when ghost_terminal gets a name  
  - Return value  
    - `X-SSTP-PassThru-GhostName` (optional)  
      Show ghost name  
    - `X-SSTP-PassThru-UserName` (optional)  
      Show username  

### Execute the command  

- `ShioriEcho`  
  Event after the command has been typed  
  - `Reference0`  
    Commands collected by the terminal  
  - Return Value  
    If you have not defined `ShioriEcho.GetResult`, you can return any of the return values supported by `ShioriEcho.GetResult` here, which will also be correctly processed by the terminal (except for return value 3, which will enter the next command acquisition when `ShioriEcho` does not return anything without defining `ShioriEcho.GetResult`).
    If you have defined `ShioriEcho.GetResult`, the return value here will be ignored (**but Sakura Script will execute normally**).  
- `ShioriEcho.GetResult`  
  Query for value result event  
  - Possible return value 1  
    - `X-SSTP-PassThru-Result`  
      Display the content and go to the next command to fetch  
    - `X-SSTP-PassThru-Type` (optional)  
      Additional info: value type  
  - Possible return value 2  
    - `X-SSTP-PassThru-Special`  
      Show content and go to the next command fetch  
      This return value will simply escape:  
      - `\n` will be converted to a newline  
      - `\t` will be converted to tabs  
      - `\\` will be converted to `\`  
  - Possible return value 3  
    - Empty  
      Wait 1 second and re-initiate `ShioriEcho.GetResult`  
  - Other
    - `X-SSTP-PassThru-State`  
      The return value can be superimposed on the above return value  
      If it is `End`, the terminal terminates.  
      If it is `Continue`, the terminal does not display any content and goes to the next command acquisition  

### Other  

- `ShioriEcho.TabPress`  
  command completes the event in tab  
  - `Reference0`  
    Commands collected by the terminal  
  - `Reference1`  
    The first character of the command where the cursor was when the tab was pressed (starting value 0)  
  - `Reference2`  
    The first consecutive time the user pressed the tab (starting value 0)  
  - `Reference3`  
    The first character of the command in this series of tab complements where the cursor was when the tab was first pressed (starting value 0)  
  - Return value  
    - `X-SSTP-PassThru-Command` (optional)  
      Replace the command with this content  
    - `X-SSTP-PassThru-InsertIndex` (optional)  
      Move the cursor to this position (or leave it unchanged if not provided)  
    - `X-SSTP-PassThru-OldInsertIndex` (optional)  
      Update the subsequent `Reference3` to this content, valid until the end of the series of tab completions (or leave it unchanged if not provided)  
- `ShioriEcho.CommandUpdate`  
  Event when the command is updated  
  - `Reference0`  
    Commands collected by the terminal  
  - Return value  
    - `X-SSTP-PassThru-CommandForDisplay` (optional)  
      Replace the displayed command with this content  
      ghost can use this event to implement masks on password entry, syntax highlighting on normal reads, and other functions  
- `ShioriEcho.CommandPrompt`  
  Events when the command prompt is updated  
  - Return value  
    - `X-SSTP-PassThru-Prompt` (optional)  
      Replace the displayed command prompt with this content  
      ghost can implement dynamic changes to the command prompt with this event  
- `ShioriEcho.CommandComplete`  
  Event when the user presses `→` at the far right of the command line  
  - `Reference0`  
    Commands collected by the terminal  
  - `Reference1`  
    The first character of the command is where the cursor was when `→` was pressed (starting value 0)  
  - Return value  
    - `X-SSTP-PassThru-Command` (optional)  
      Replace the command with this content  
      Ghosts can auto-complete commands with this event  
    - `X-SSTP-PassThru-InsertIndex` (optional)  
      Move the cursor to this position (or leave it unchanged if not provided)  

### Command History  

- `ShioriEcho.CommandHistory.New`  
  Event when an empty command is added at the end of the command history  
  - Return value  
    Ignored, **but sakura scripts are executed normally**
- `ShioriEcho.CommandHistory.Update`  
   Event when command history is updated  
  - `Reference0`  
    The contents of the history command  
  - `Reference1`  
    Index of the history command (in reverse order, starting value 0)  
  - Return value  
    Ignored, **but sakura scripts are executed normally**
- `ShioriEcho.CommandHistory.Get`  
  Event when command history is fetched  
  - `Reference0`  
    Index of the history command (in reverse order, starting value 0)
  - Return value
    - `X-SSTP-PassThru-Command` (optional)  
      Replace the command with this content  
- `ShioriEcho.CommandHistory.ForwardIndex`  
  Event when the index is updated when the user presses `↑`  
  - `Reference0`
    Index of the history command (in reverse order, starting value 0)  
  - `Reference1`
    num of the value by which `index` is expected to increase  
  - Return value
    - `X-SSTP-PassThru-Index` (optional)  
      Update the index to this value  
