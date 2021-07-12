#include "..\SSTP_linker\include\SSTP.hpp"
#include <iostream>
#ifdef _WIN32
	#include <fcntl.h>
	#include <io.h>
#endif

#define floop while(1)

using namespace SSTP_link_n;
using namespace std;

SSTP_link_t linker({{L"Charset",L"UTF-8"},{L"Sender",L"Ghost Terminal"}});

void before_login(){
	#ifdef _WIN32
		void(_setmode(_fileno(stdout), _O_U16TEXT));
		void(_setmode(_fileno(stdin), _O_U16TEXT));
	#else
		wcin.imbue(locale(""));
		wcout.imbue(locale(""));
	#endif
}
void terminal_login(){
	auto names = linker.NOTYFY({ { L"Event", L"ShioriEcho.GetName" } });
	wcout << "terminal login\n";
	if(names.has(L"GhostName"))
		wcout << "ghost: " << names[L"GhostName"] << '\n';
	if(names.has(L"UserName"))
		wcout << "User: " << names[L"UserName"] << '\n';
}
wstring terminal_tab_press(const wstring&command,size_t tab_num){
	auto&Result=linker.NOTYFY({ { L"Event", L"ShioriEcho.TabPress" },
								{ L"Reference0", command },
								{ L"Reference1", to_wstring(tab_num) }
							});
	if(Result.has(L"Command"))
		return Result[L"Command"];
	else
		return command;
}
void terminal_run(const wstring&command){
	if (command != L"exit") {
		if(!linker.Has_Event(L"ShioriEcho"))
			wcout << "Event Has_Event or ShioriEcho Not define.\n";
		linker.NOTYFY({ { L"Event", L"ShioriEcho" },
						{ L"Reference0", command }
					  });
		floop{
			auto&Result=linker.NOTYFY({ { L"Event", L"ShioriEcho.GetResult" } });
			if(Result.get_code()==400){//Bad Request
				wcout << "Event ShioriEcho.GetResult Not define.\n";
				break;
			}else if(Result.has(L"Special")){
				wcout << Result[L"Special"] << endl;
				break;
			}else if(Result.has(L"Result")){
				wcout << Result[L"Result"] << endl;
				if(Result.has(L"Type"))
					wcout << "Type: " << Result[L"Type"] << endl;
				break;
			}
			else if(Result.has(L"Type")){
				wcout << "has Type but no Result here:\n"
					  << Result << endl;
				break;
			}
			else{
				Sleep(1000);
				continue;
			}
		}
	}
}
void terminal_exit(){
	linker.NOTYFY({ { L"Event", L"ShioriEcho.End" } });
}
