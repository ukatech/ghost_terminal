#include "my-gists/ukagaka/SSTP.hpp"
//#include "my-gists/ukagaka/SFMO.hpp"
#include "my-gists/STL/replace_all.hpp"
#include "my-gists/windows/Cursor.hpp"//saveCursorPos、resetCursorPos
#include <iostream>
#ifdef _WIN32
	#include <fcntl.h>
	#include <io.h>
#endif

#define floop while(1)

using namespace SSTP_link_n;
using namespace std;

SSTP_link_t linker({{L"Charset",L"UTF-8"},{L"Sender",L"Ghost Terminal"}});

wstring&do_transfer(wstring &a) {
	replace_all(a, L"\\n", L"\n");
	replace_all(a, L"\\\n", L"\\\\n");

	replace_all(a, L"\\t", L"\t");
	replace_all(a, L"\\\t", L"\\\\t");

	replace_all(a, L"\\\\", L"\\");
	return a;
}
bool has_GetResult;
void before_login(){
	has_GetResult=0;
	#ifdef _WIN32
		void(_setmode(_fileno(stdout), _O_U16TEXT));
		void(_setmode(_fileno(stdin), _O_U16TEXT));
	#else
		wcin.imbue(locale(""));
		wcout.imbue(locale(""));
	#endif
}
size_t GetStrWide(const wstring& str, size_t begin = 0, size_t end = wstring::npos);
void putchar_x_time(wchar_t the_char, size_t time);
void terminal_login(){
	/*
	SFMO_t fmobj;
	if (fmobj.Update_info()) {
		auto ghostnum= fmobj.info_map.size();
		HWND ghost_hwnd = NULL;
		if(ghostnum ==1){
			wcout << "Only one ghost was running.\n";
			ghost_hwnd=(HWND)wcstoll(fmobj.info_map.begin()->second.map[L"hwnd"].c_str(),nullptr,10);
		}
		else if (ghostnum == 0) {
			wcerr << "None of ghost was running.\n";
			exit(1);
		}
		else {
			wcout << "Select the ghost you want to log into.[Up/Down/Enter]\n";
			saveCursorPos();
			auto pbg = fmobj.info_map.begin();
			auto ped = fmobj.info_map.end();
			auto p = pbg;
			while(!ghost_hwnd){
				wcout << p->second[L"name"];
				auto c=_getwch();
				resetCursorPos();
				auto size = GetStrWide(p->second[L"name"]);
				putchar_x_time(' ', size);
				resetCursorPos();
				switch(c){
					case 27://esc
						exit(0);
					case WEOF:
					case 13://enter
						ghost_hwnd=(HWND)wcstoll(p->second.map[L"hwnd"].c_str(),nullptr,10);
						break;
					case 9:{//tab
						p++;
						if(p==ped)
							p=pbg;
						break;
					}
					case 0xE0:{//方向字符先导字符
						switch(_getwch()){
						case 72://up
						case 83://delete
						case 75://left
							if(p==pbg)
								p=ped;
							p--;
							break;
						case 77://right
						case 80://down
							p++;
							if(p==ped)
								p=pbg;
							break;
						}
						break;
					}
				}
			}
		}
		#ifdef _DEBUG
			wcout << "ghost_hwnd: "<< (size_t)ghost_hwnd <<"\n";
		#endif // _DEBUG
		if(!linker.link_to_ghost(ghost_hwnd)){
			wcerr<< "Failed to link to ghost, trying to use socket...\n";
			if(!linker.Socket_link()){
				wcerr<< "Failed.\n";
				exit(1);
			}
		}
	}
	else {
		wcerr << "Can\'t read FMO info, trying to use socket...\n";
		if(!linker.Socket_link()){
			wcerr<< "Failed.\n";
			exit(1);
		}
	}
	*/
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
	if(!linker.Has_Event(L"ShioriEcho"))
		wcout << "Event Has_Event or ShioriEcho Not define.\n";
	linker.NOTYFY({ { L"Event", L"ShioriEcho" },
					{ L"Reference0", command }
				  });
	floop{
		auto&Result=linker.NOTYFY({ { L"Event", L"ShioriEcho.GetResult" } });
		if(!has_GetResult && Result.get_code()==400){//Bad Request
			wcout << "Event ShioriEcho.GetResult Not define.\n";
			break;
		}else
			has_GetResult=1;
		if(Result.has(L"Special")){
			wcout << do_transfer(Result[L"Special"]) << endl;
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
void terminal_exit(){
	linker.NOTYFY({ { L"Event", L"ShioriEcho.End" } });
}
void terminal_args(size_t argc, std::vector<std::wstring>&argv) {
	if(argc==1)
		return;
	if(argv[1]==L"-c"){
		size_t tmp=1;
		wstring command;
		while(tmp++!=argc-1){
			command+=L' ';
			command+=argv[tmp];
		}
		command.erase(0,1);
		terminal_login();
		terminal_run(command);
		terminal_exit();
		exit(0);
	}
}
