#include "my-gists/windows/shell_base.hpp"

#include "my-gists/ukagaka/SSTP.hpp"
#include "my-gists/ukagaka/SFMO.hpp"
#include "my-gists/ukagaka/SSP_Runner.hpp"

#include "my-gists/codepage.hpp"
#include "my-gists/ansi_color.hpp"

#include "my-gists/STL/replace_all.hpp"
#include "my-gists/STL/CutSpace.hpp"

#include "my-gists/windows/Cursor.hpp"		 //saveCursorPos、resetCursorPos

#include <iostream>
#ifdef _WIN32
	#include <fcntl.h>
	#include <io.h>
#endif
#include <shlwapi.h>
#include <shlobj_core.h>
#include <rpcdce.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <conio.h>
//lib of PathFileExists
#pragma comment(lib, "shlwapi.lib")

#define floop while(1)

#define GT_VAR	   13.1
#define GT_VAR_STR "13.1"

using namespace SSTP_link_n;
using namespace std;

extern size_t GetStrWide(const wstring& str, size_t begin = 0, size_t end = wstring::npos);
extern void	  putchar_x_times(wchar_t the_char, size_t time);

string to_string(wstring str) {
	return CODEPAGE_n::UnicodeToMultiByte(str, CODEPAGE_n::CP_UTF8);
}

string to_json_string(string str) {
	replace_all(str, "\\", "\\\\");
	replace_all(str, "\"", "\\\"");
	return str;
}

wstring to_command_path_string(wstring str) {
	replace_all(str, L"\\", L"\\\\");
	return str;
}

string to_json_string(wstring str) {
	return to_json_string(to_string(str));
}

bool Split(wstring& str, wstring& s0, wstring& s1, wstring_view sepstr) {
	// strをs0とs1に分解
	auto begin = str.find(sepstr);
	s0		   = str.substr(0, begin);
	s1		   = str.substr(begin + sepstr.size());
	CutSpace(s0);
	CutSpace(s1);

	return begin != wstring::npos;
}

auto get_name_and_icon_path(wstring ghost_path) {
	auto descript_name = ghost_path + L"ghost\\master\\descript.txt";
	auto descript_f	   = _wfopen(descript_name.c_str(), L"rb");
	struct ret_t {
		wstring name;
		wstring icon_path;
	};
	ret_t aret;
	//
	CODEPAGE_n::CODEPAGE cp = CODEPAGE_n::CP_UTF8;
	char				 buf[2048];
	wstring				 line, s0, s1;
	if(descript_f) {
		while(fgets(buf, 2048, descript_f)) {
			line	 = CODEPAGE_n::MultiByteToUnicode(buf, cp);
			auto len = line.size();
			if(len && *line.rbegin() == L'\n')
				line.resize(--len);
			if(len && *line.rbegin() == L'\r')
				line.resize(--len);
			Split(line, s0, s1, L",");
			if(s0 == L"charset")
				cp = CODEPAGE_n::StringtoCodePage(s1.c_str());
			else if(s0 == L"icon") {
				aret.icon_path = ghost_path + L"ghost\\master\\" + s1;
				if(!PathFileExistsW(aret.icon_path.c_str()))
					aret.icon_path.clear();
				if(!aret.name.empty()) {
					fclose(descript_f);
					return aret;
				}
			}
			else if(s0 == L"name") {
				aret.name = s1;
				if(!aret.icon_path.empty()) {
					fclose(descript_f);
					return aret;
				}
			}
		}
		fclose(descript_f);
	}
	return aret;
}
wstring get_name(wstring ghost_path) {
	auto descript_name = ghost_path + L"ghost\\master\\descript.txt";
	auto descript_f	   = _wfopen(descript_name.c_str(), L"rb");
	//
	CODEPAGE_n::CODEPAGE cp = CODEPAGE_n::CP_UTF8;
	char				 buf[2048];
	wstring				 line, s0, s1;
	if(descript_f) {
		while(fgets(buf, 2048, descript_f)) {
			line	 = CODEPAGE_n::MultiByteToUnicode(buf, cp);
			auto len = line.size();
			if(len && *line.rbegin() == L'\n')
				line.resize(--len);
			if(len && *line.rbegin() == L'\r')
				line.resize(--len);
			Split(line, s0, s1, L",");
			if(s0 == L"charset")
				cp = CODEPAGE_n::StringtoCodePage(s1.c_str());
			else if(s0 == L"name") {
				fclose(descript_f);
				return s1;
			}
		}
		fclose(descript_f);
	}
	return {};
}
wstring do_transfer(wstring a) {
	replace_all(a, L"\\n", L"\n");
	replace_all(a, L"\\\n", L"\\\\n");

	replace_all(a, L"\\t", L"\t");
	replace_all(a, L"\\\t", L"\\\\t");

	replace_all(a, L"\\\\", L"\\");
	return a;
}

class ghost_terminal final: public simple_terminal {
	SSTP_link_t linker{{{L"Charset", L"UTF-8"},
						{L"Sender", L"Ghost Terminal"},
						{L"SecurityLevel", L"local"}}};
	struct args_info_t {
		wstring ghost_link_to;
		wstring ghost_path;
		bool	run_ghost  = 0;
		HWND	ghost_hwnd = NULL;
		wstring command;
		wstring sakurascript;
	} args_info;
	wstring ghost_uid;

	bool	is_windows_terminal = 0;
	wstring old_title;
	void	before_terminal_login() override {
#ifdef _WIN32
		void(_setmode(_fileno(stdout), _O_U16TEXT));
		void(_setmode(_fileno(stdin), _O_U16TEXT));
#else
		wcin.imbue(locale(""));
		wcout.imbue(locale(""));
#endif
		old_title.resize(MAX_PATH);
		old_title.resize(GetConsoleTitleW(old_title.data(), old_title.size()));
		{
			//判断是否为Windows Terminal
			//依据为当前进程和WindowsTerminal.exe的进程在同一进程树下
			{
				auto pid = GetCurrentProcessId();
				while(pid) {
					wchar_t buf[MAX_PATH];
					auto	h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
					if(!h)
						break;
					DWORD len = MAX_PATH;
					if(!QueryFullProcessImageNameW(h, 0, buf, &len)) {
						CloseHandle(h);
						break;
					}
					CloseHandle(h);
					if(wcsstr(buf, L"WindowsTerminal.exe")) {
						is_windows_terminal = 1;
						break;
					}
					PROCESSENTRY32W pe32;
					pe32.dwSize = sizeof(PROCESSENTRY32W);
					auto hSnap	= CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
					if(hSnap == INVALID_HANDLE_VALUE)
						break;
					if(!Process32FirstW(hSnap, &pe32)) {
						CloseHandle(hSnap);
						break;
					}
					do {
						if(pe32.th32ProcessID == pid) {
							pid = pe32.th32ParentProcessID;
							break;
						}
					} while(Process32NextW(hSnap, &pe32));
					CloseHandle(hSnap);
				}
			}
		}
	}

	void terminal_login() override {
		SFMO_t fmobj;
		auto&  ghost_link_to = args_info.ghost_link_to;
		auto&  ghost_path	 = args_info.ghost_path;
		auto&  run_ghost	 = args_info.run_ghost;
		auto&  ghost_hwnd	 = args_info.ghost_hwnd;
		auto&  command		 = args_info.command;
		auto&  sakurascript	 = args_info.sakurascript;
		//处理ghost_path，获得ghost_link_to
		if(!ghost_path.empty() && ghost_link_to.empty()) {
			ghost_link_to = get_name(ghost_path);
			if(ghost_link_to.empty()) {
				wcerr << SET_RED "Can't find ghost name from " SET_BLUE << ghost_path << RESET_COLOR << endl;
				exit(1);
			}
		}
		auto waiter = [&](auto condition, wstring condition_str, size_t timeout = 30) {
			if(!condition()) {
				wcout << LIGHT_YELLOW_OUTPUT("Waiting for " << condition_str << "...") << flush;
				terminal::reprinter_t reprinter;
				size_t				  time = 0;
				floop {
					reprinter(L"" YELLOW_TEXT("(" + to_wstring(time) + L'/' + to_wstring(timeout) + L"s)"));
					Sleep(1000);
					if(timeout == time) {
						wcout << endl;
						wcerr << RED_TEXT("timeout.") << endl;
						exit(1);
					}
					time++;
					if(condition())
						break;
				}
				wcout << endl;
			}
		};
		auto start_ghost = [&] {
			wcout << LIGHT_YELLOW_TEXT("Trying to start ghost...\n");
			if(ghost_link_to.empty() && ghost_path.empty())		  //?
				wcerr << SET_CYAN "You can use " SET_GREEN "-g" SET_CYAN " or " SET_GREEN "-gp" SET_CYAN " to specify the ghost(by name or path).\n" RESET_COLOR;
			SSP_Runner SSP;
			if(!SSP.IsInstalled()) {
				wcerr << RED_TEXT("SSP is not installed.") << endl;
				exit(1);
			}
			if(ghost_path.empty())
				SSP.run_ghost(ghost_link_to);
			else if(ghost_link_to.empty())
				SSP();
			else
				SSP.run_ghost(ghost_path);
			fmobj.Update_info();
			waiter([&] {
				return fmobj.Update_info() && fmobj.info_map.size() > 0;
			},
				   L"FMO initialized");
			if(!ghost_link_to.empty())
				waiter([&] {
					if(fmobj.Update_info())
						for(auto& i: fmobj.info_map) {
							HWND tmp_hwnd = (HWND)wcstoll(i.second[L"hwnd"].c_str(), nullptr, 10);
							if(i.second[L"fullname"] == ghost_link_to)
								return ghost_hwnd = tmp_hwnd;
						}
					return HWND(0);
				},
					   L"ghost hwnd created");
		};
		if(ghost_hwnd)
			goto link_to_ghost;
		if(fmobj.Update_info()) {
			{
				auto ghostnum = fmobj.info_map.size();
				if(ghostnum == 0) {
					wcerr << RED_TEXT("None of ghost was running.") << endl;
					if(!run_ghost)
						exit(1);
					start_ghost();
				}
				else if(!ghost_link_to.empty()) {
					for(auto& i: fmobj.info_map) {
						HWND tmp_hwnd = (HWND)wcstoll(i.second[L"hwnd"].c_str(), nullptr, 10);
						if(i.second[L"name"] == ghost_link_to || i.second[L"fullname"] == ghost_link_to) {
							ghost_hwnd = tmp_hwnd;
							ghost_uid  = i.first;
							break;
						}
						else {
							linker.link_to_ghost(tmp_hwnd);
							auto names = linker.NOTYFY({{L"Event", L"ShioriEcho.GetName"}});

							if(names[L"GhostName"] == ghost_link_to) {
								ghost_hwnd = tmp_hwnd;
								ghost_uid  = i.first;
								break;
							}
							else
								linker.link_to_ghost(NULL);
						}
					}
					if(!ghost_hwnd) {
						wcerr << SET_RED "Target ghost: " SET_BLUE << ghost_link_to << SET_RED " not found." RESET_COLOR << endl;
						exit(1);
					}
				}
				else if(ghostnum == 1) {
					wcout << GRAY_TEXT("Only one ghost was running.\n");
					ghost_hwnd = (HWND)wcstoll(fmobj.info_map.begin()->second.map[L"hwnd"].c_str(), nullptr, 10);
				}
				else {
					wcout << LIGHT_YELLOW_TEXT("Select the ghost you want to log into.[Up/Down/Enter]\n");
					terminal::reprinter_t reprinter;
					auto				  pbg = fmobj.info_map.begin();
					auto				  ped = fmobj.info_map.end();
					auto				  p	  = pbg;
					while(!ghost_hwnd) {
						reprinter(p->second[L"name"]);
						switch(_getwch()) {
						case 27:	   //esc
							exit(0);
						case WEOF:
						case 13:	   //enter
							ghost_hwnd = (HWND)wcstoll(p->second[L"hwnd"].c_str(), nullptr, 10);
							break;
						case 9: {		//tab
							p++;
							if(p == ped)
								p = pbg;
							break;
						}
						case 0xE0: {	   //方向字符先导字符
							switch(_getwch()) {
							case 72:	   //up
							case 83:	   //delete
							case 75:	   //left
								if(p == pbg)
									p = ped;
								p--;
								break;
							case 77:	   //right
							case 80:	   //down
								p++;
								if(p == ped)
									p = pbg;
								break;
							}
							break;
						}
						}
					}
				}
			}
		link_to_ghost:
			if(ghost_uid.empty()) {
				if(fmobj.Update_info()) {
					for(auto& i: fmobj.info_map) {
						HWND tmp_hwnd = (HWND)wcstoll(i.second[L"hwnd"].c_str(), nullptr, 10);
						if(ghost_hwnd == tmp_hwnd) {
							ghost_uid = i.first;
							break;
						}
					}
				}
				if(ghost_uid.empty()) {
					wcerr << RED_TEXT("Can\'t get ghost_uid.") << endl;
					exit(1);
				}
			}
		}
		else {
			wcerr << RED_TEXT("Can\'t read FMO info.") << endl;
			if(!run_ghost)
				exit(1);
			start_ghost();
			goto link_to_ghost;
		}
		waiter([&] {
			return fmobj.Update_info() && fmobj.info_map[ghost_uid].get_modulestate(L"shiori") == L"running";
		},
			   L"ghost's shiori ready");

		auto names	= linker.NOTYFY({{L"Event", L"ShioriEcho.GetName"}});
		auto result = linker.NOTYFY({{L"Event", L"ShioriEcho.Begin"},
									 {L"Reference0", L"" GT_VAR_STR}});

		//set console title
		wstring title = L"Ghost Terminal";
		if(result.has(L"Tittle"))
			title = result[L"Tittle"];
		else if(!args_info.ghost_link_to.empty())
			title += L" - " + args_info.ghost_link_to;
		SetConsoleTitleW(title.c_str());

		wcout << CYAN_TEXT("terminal login\n");
		if(names.has(L"GhostName"))
			wcout << "Ghost: " << LIGHT_YELLOW_OUTPUT(names[L"GhostName"]) << '\n';
		if(names.has(L"UserName"))
			wcout << "User: " << LIGHT_YELLOW_OUTPUT(names[L"UserName"]) << '\n';

		bool need_end = 0;
		if(!command.empty()) {
			terminal_run(command);
			need_end = 1;
		}
		if(!sakurascript.empty()) {
			linker.SEND({{L"ID", ghost_uid}, {L"Script", sakurascript}});
			need_end = 1;
		}
		if(need_end) {
			terminal_exit();
			exit(0);
		}
		if(linker.Has_Event(L"Has_Event")) {
			wcerr << SET_GRAY;
			if(!linker.Has_Event(L"ShioriEcho"))
				wcerr << "Event " SET_GREEN "ShioriEcho" SET_GRAY " Not defined.\n"
					  << "Your ghost may not be supporting Terminal if it can't handle ShioriEcho event.\n\n";
			//ShioriEcho.GetResult
			if(!linker.Has_Event(L"ShioriEcho.GetResult")) {
				able_get_result = 0;
				wcerr << "Event " SET_GREEN "ShioriEcho.GetResult" SET_GRAY " Not defined.\n"
					  << "Terminal will not send get result event to your ghost and will not echo result.\n\n";
			}
			if(!linker.Has_Event(L"ShioriEcho.CommandComplete")) {
				able_command_complete = 0;
				wcerr << "Event " SET_GREEN "ShioriEcho.CommandComplete" SET_GRAY " Not defined.\n"
					  << "Terminal will not send command complete event to your ghost.\n\n";
			}
			if(!linker.Has_Event(L"ShioriEcho.CommandUpdate")) {
				able_command_update = 0;
				wcerr << "Event " SET_GREEN "ShioriEcho.CommandUpdate" SET_GRAY " Not defined.\n"
					  << "Terminal will not send command update event to your ghost.\n\n";
			}
			if(!linker.Has_Event(L"ShioriEcho.CommandHistory.New") || !linker.Has_Event(L"ShioriEcho.CommandHistory.Get") || !linker.Has_Event(L"ShioriEcho.CommandHistory.Update")) {
				able_command_history = 0;
				wcerr << "Your ghost needs to support all of the following events to support command history:\n"
					  << SET_GREEN
					  << "ShioriEcho.CommandHistory.New\n"
					  << "ShioriEcho.CommandHistory.Get\n"
					  << "ShioriEcho.CommandHistory.Update\n"
					  << SET_GRAY
					  << "Terminal will use its default command history function and not send command history events to your ghost.\n\n";
			}
			if(!linker.Has_Event(L"ShioriEcho.TabPress")) {
				able_tab_press = 0;
				wcerr << "Event " SET_GREEN "ShioriEcho.TabPress" SET_GRAY " Not defined.\n"
					  << "Terminal will not send tab press event to your ghost.\n\n";
			}
			wcerr << RESET_COLOR;
		}
		else {
			able_tab_press		  = 0;
			able_command_update	  = 0;
			able_command_complete = 0;
			able_command_history  = 0;
			wcerr << SET_RED "Event " SET_GREEN "Has_Event" SET_RED " Not defined.\n"
				  << "You need to make your ghost support " SET_GREEN "Has_Event" SET_RED " event so that Terminal can know what events it supports.\n"
				  << "Terminal will assume your ghost only supports " SET_GREEN "ShioriEcho" SET_RED " and " SET_GREEN "ShioriEcho.GetResult" SET_RED " events." RESET_COLOR "\n\n";
		}
	}

	bool able_get_result	   = 1;
	bool able_tab_press		   = 1;
	bool able_command_update   = 1;
	bool able_command_complete = 1;
	bool able_command_history  = 1;

	editting_command_t terminal_command_complete_by_right(const editting_command_t& command) {
		if(!able_command_complete)
			return command;
		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.CommandComplete"},
									 {L"Reference0", command.command},
									 {L"Reference1", to_wstring(command.insert_index)}});

		editting_command_t Result_command = command;
		if(Result.has(L"Command"))
			Result_command.command = Result[L"Command"];
		if(Result.has(L"InsertIndex"))
			Result_command.insert_index = stoull(Result[L"InsertIndex"]);
		return Result_command;
	}
	std::wstring terminal_command_update(std::wstring command) {
		if(!able_command_update || _kbhit())	   //考虑到sstp极慢的速度，只在需要时更新command的色彩
			return command;
		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.CommandUpdate"},
									 {L"Reference0", command}});
		if(Result.has(L"CommandForDisplay"))
			return Result[L"CommandForDisplay"];
		else
			return command;
	}
	void terminal_command_history_new() {
		if(!able_command_history) {
			simple_terminal::terminal_command_history_new();
			return;
		}
		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.CommandHistory.New"}});
	}
	void terminal_command_history_update_last(const std::wstring& command, size_t before_num) {
		if(!able_command_history) {
			simple_terminal::terminal_command_history_update(command, before_num);
			return;
		}
		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.CommandHistory.Update"},
									 {L"Reference0", command},
									 {L"Reference1", to_wstring(before_num)}});
	}
	std::wstring terminal_get_command_history(size_t before_num) {
		if(!able_command_history) {
			return simple_terminal::terminal_get_command_history(before_num);
		}
		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.CommandHistory.Get"},
									 {L"Reference0", to_wstring(before_num)}});
		if(Result.has(L"Command"))
			return Result[L"Command"];
		else
			return {};
	}
	editting_command_t terminal_tab_press(const editting_command_t& command, size_t tab_num) override {
		if(!able_tab_press)
			return command;
		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.TabPress"},
									 {L"Reference0", command.command},
									 {L"Reference1", to_wstring(command.insert_index)},
									 {L"Reference2", to_wstring(tab_num)}});

		editting_command_t Result_command = command;
		if(Result.has(L"Command"))
			Result_command.command = Result[L"Command"];
		if(Result.has(L"InsertIndex"))
			Result_command.insert_index = stoull(Result[L"InsertIndex"]);
		return Result_command;
	}
	bool terminal_run(const wstring& command) override {
		linker.NOTYFY({{L"Event", L"ShioriEcho"},
					   {L"ID", ghost_uid},
					   {L"Reference0", command}});
		if(able_get_result)
			try {
				floop {
					auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.GetResult"}});
					if(Result.get_code() == 404) {
						wcerr << RED_TEXT("Lost connection with target ghost.") << endl;
						exit(1);
					}
					if(Result.has(L"Special")) {
						wcout << do_transfer(Result[L"Special"]) << endl;
						break;
					}
					else if(Result.has(L"Result")) {
						wcout << Result[L"Result"] << endl;
						if(Result.has(L"Type"))
							wcout << "Type: " << Result[L"Type"] << endl;
						break;
					}
					else if(Result.has(L"Type")) {
						wcout << "Has " GREEN_TEXT("Type") " but no " GREEN_TEXT("Result") " here:\n "
							  << to_ansi_colored_wstring(Result) << endl;
						break;
					}
					else {
						Sleep(1000);
					}
					if(Result.has(L"Status")) {
						if(Result[L"Status"] == L"End")
							return false;
					}
				}
			}
			catch(const std::exception& a) {
				cerr << RED_OUTPUT(a.what()) << endl;
				exit(1);
			}
		return true;
	}
	void terminal_exit() override {
		linker.NOTYFY({{L"Event", L"ShioriEcho.End"}});
		SetConsoleTitleW(old_title.c_str());
	}
	void terminal_args(size_t argc, std::vector<std::wstring>& argv) override {
		auto& ghost_path	= args_info.ghost_path;
		auto& run_ghost		= args_info.run_ghost;
		auto& ghost_link_to = args_info.ghost_link_to;
		auto& ghost_hwnd	= args_info.ghost_hwnd;
		auto& command		= args_info.command;
		auto& sakurascript	= args_info.sakurascript;

		bool	register2wt = 0;
		wstring wt_name;
		wstring wt_icon;
		if(argc != 1) {
			size_t i = 1;
			while(i < argc) {
				if(argv[i] == L"-c" || argv[i] == L"--command") {		//command
					i++;
					if(i < argc)
						command = argv[i];
				}
				else if(argv[i] == L"-s" || argv[i] == L"--sakura-script") {	   //sakura script
					i++;
					if(i < argc)
						sakurascript = argv[i];
				}
				else if(argv[i] == L"-g" || argv[i] == L"--ghost") {	   //ghost
					i++;
					if(i < argc)
						ghost_link_to = argv[i];
				}
				else if(argv[i] == L"-gh" || argv[i] == L"--ghost-hwnd") {		 //ghost hwnd
					i++;
					if(i < argc)
						ghost_hwnd = (HWND)wcstoll(argv[i].c_str(), nullptr, 10);
				}
				else if(argv[i] == L"-gp" || argv[i] == L"--ghost-folder-path") {
					i++;
					if(i < argc)
						ghost_path = argv[i];
				}
				else if(argv[i] == L"-r" || argv[i] == L"--run-ghost") {	   //run ghost if not running
					run_ghost = 1;
				}
				else if(argv[i] == L"-rwt" || argv[i] == L"--register-to-windows-terminal") {		//register to windows terminal
					register2wt = 1;
				}
				else if(argv[i] == L"-rwt-name") {		 //register to windows terminal as name
					i++;
					if(i < argc)
						wt_name = argv[i];
				}
				else if(argv[i] == L"-rwt-icon") {		 //register to windows terminal with icon
					i++;
					if(i < argc)
						wt_icon = argv[i];
				}
				else if(argv[i] == L"-h" || argv[i] == L"--help") {		  //help
					wcout << LIGHT_YELLOW_OUTPUT(argv[0]) << SET_CYAN " [options]\n" RESET_COLOR
						  << L"options:\n"
						  << L"  " SET_GREEN "-c" SET_YELLOW "," SET_GREEN " --command " SET_PURPLE "<command>" SET_WHITE "               : " SET_GRAY "run command " SET_RED "and exit\n"
						  << L"  " SET_GREEN "-s" SET_YELLOW "," SET_GREEN " --sakura-script " SET_PURPLE "<script>" SET_WHITE "          : " SET_GRAY "run sakura script " SET_RED "and exit\n"
						  << L"  " SET_GREEN "-g" SET_YELLOW "," SET_GREEN " --ghost " SET_PURPLE "<ghost>" SET_WHITE "                   : " SET_GRAY "link to ghost by name\n"
						  << L"  " SET_GREEN "-gh" SET_YELLOW "," SET_GREEN " --ghost-hwnd " SET_PURPLE "<hwnd>" SET_WHITE "              : " SET_GRAY "link to ghost by hwnd\n"
						  << L"  " SET_GREEN "-gp" SET_YELLOW "," SET_GREEN " --ghost-folder-path " SET_PURPLE "<path>" SET_WHITE "       : " SET_GRAY "link to ghost by folder path\n"
						  << L"  " SET_GREEN "-r" SET_YELLOW "," SET_GREEN " --run-ghost" SET_WHITE "                       : " SET_GRAY "run ghost if not running\n"
						  << L"  " SET_GREEN "-rwt" SET_YELLOW "," SET_GREEN " --register-to-windows-terminal" SET_WHITE "  : " SET_GRAY "register to windows terminal, needs " SET_GREEN "-g" SET_PURPLE " <ghost name>" SET_GRAY " or " SET_GREEN "-gp" SET_PURPLE " <ghost folder path>" SET_GRAY "\n"
						  << L"  " SET_GREEN "-rwt-name " SET_PURPLE "<name>" SET_WHITE "                      : " SET_GRAY "register to windows terminal as name, only work with " SET_GREEN "-rwt" SET_GRAY "\n"
						  << L"  " SET_GREEN "-rwt-icon " SET_PURPLE "<icon>" SET_WHITE "                      : " SET_GRAY "register to windows terminal with icon(png|ico path), only work with " SET_GREEN "-rwt" SET_GRAY "\n"
						  << RESET_COLOR;
					exit(0);
				}
				else {
					wcerr << SET_RED "unknown option: " SET_GRAY << argv[i] << RESET_COLOR << endl;
					exit(1);
				}
				i++;
			}
		}
		wstring LOCALAPPDATA = _wgetenv(L"LOCALAPPDATA");
		if(!is_windows_terminal) {
			if(!register2wt) {
				wstring wt_path = LOCALAPPDATA + L"\\Microsoft\\WindowsApps\\wt.exe";
				if(!PathFileExistsW(wt_path.c_str())) {
					wcout << SET_GRAY "Terminal can look more sleek if you have Windows Terminal installed." << endl
						  << "Download it from https://aka.ms/terminal and run this exe with " SET_GREEN "-rwt (-g|-gp)" RESET_COLOR " ." << endl;
				}
				else {
					wcout << SET_GRAY "You can run this exe with " SET_GREEN "-rwt (-g|-gp)" SET_GRAY " for a better experience under Windows Terminal." RESET_COLOR << endl;
				}
			}
		}
		if(register2wt) {
			//尝试通过fmo获取ghost的文件夹路径
			if(ghost_path.empty() && !ghost_link_to.empty()) {
				SFMO_t fmobj;
				if(fmobj.Update_info()) {
					wcout << SET_GRAY "Read FMO info success\n" RESET_COLOR;
					for(auto& i: fmobj.info_map) {
						if(i.second[L"fullname"] != ghost_link_to)
							continue;
						ghost_path = i.second[L"ghostpath"];
						break;
					}
					if(ghost_path.empty())
						wcerr << SET_RED "Can't find ghost path by name: " SET_BLUE << ghost_link_to << RESET_COLOR << endl;
					else
						wcout << SET_GRAY "Get ghost path by name: " SET_CYAN << ghost_path << RESET_COLOR << endl;
				}
			}
			//处理ghost_path，获得ghost的name 和 icon
			auto ghost_info = get_name_and_icon_path(ghost_path);
			if(wt_name.empty())
				if(ghost_link_to.empty() && ghost_path.empty()) {
					wcerr << SET_RED "Registering to Windows Terminal requires a ghost name or folder path. Use" SET_GREEN "-g" SET_RED " or " SET_GREEN "-gp" SET_RED " to set it." RESET_COLOR << endl;
					exit(1);
				}
				else {
					if(ghost_link_to.empty())
						if(!ghost_info.name.empty()) {
							ghost_link_to = ghost_info.name;
							wcout << SET_GRAY "Got ghost name from ghost folder: " SET_BLUE << ghost_link_to << RESET_COLOR << endl;
						}
						else {
							wcerr << SET_RED "Unable to get ghost name from ghost folder. Use " SET_GREEN "-g" SET_RED " to set it." RESET_COLOR << endl;
							exit(1);
						}
					wt_name = ghost_link_to + L" terminal";
				}
			if(wt_icon.empty()) {
				if(!ghost_path.empty()) {
					if(ghost_info.icon_path.empty()) {
						wcerr << SET_RED "Can't get ghost icon from ghost folder. Use " SET_GREEN "-rwt-icon" SET_RED " to set it.\n";
					}
					else {
						wchar_t full_path[MAX_PATH];
						GetFullPathNameW(ghost_info.icon_path.c_str(), MAX_PATH, full_path, nullptr);
						wt_icon = full_path;
						wcout << SET_GRAY "Got ghost icon from ghost folder: " SET_CYAN << wt_icon << RESET_COLOR << endl;
					}
				}
			}
			const wstring_view default_icon = L"ms-appx:///ProfileIcons/{0caa0dad-35be-5f56-a8ff-afceeeaa6101}.png";
			if(wt_icon.empty())
				wt_icon = default_icon;
			auto my_name = argv[0];
			{
				//update my name to full path
				wchar_t full_path[MAX_PATH];
				GetFullPathNameW(my_name.c_str(), MAX_PATH, full_path, nullptr);
				my_name = full_path;
				//update ghost_path to full path
				if(!ghost_path.empty()) {
					GetFullPathNameW(ghost_path.c_str(), MAX_PATH, full_path, nullptr);
					ghost_path = full_path;
				}
			}
			//if my path not exist
			if(!PathFileExistsW(my_name.c_str())) {
				wcerr << SET_RED "Can't find exe file (" SET_YELLOW << my_name << SET_RED ")\n" RESET_COLOR;
				exit(1);
			}
			//register to windows terminal
			//C:\Users\<user>\AppData\Local\Microsoft\Windows Terminal\Fragments\{app-name}\{file-name}.json
			wstring wt_json_dir_path = LOCALAPPDATA + L"\\Microsoft\\Windows Terminal\\Fragments\\Ghost Terminal\\";
			//+wt_name+L".json";
			wstring wt_json_path = wt_json_dir_path + ghost_link_to + L".json";
			//创建文件夹和文件（并递归父文件夹直到成功）
			{
				auto res = SHCreateDirectoryExW(nullptr, wt_json_dir_path.c_str(), nullptr);
				if(res != ERROR_SUCCESS && res != ERROR_FILE_EXISTS && res != ERROR_ALREADY_EXISTS) {
					wcerr << SET_RED "Can't create directory: " SET_CYAN << wt_json_dir_path << RESET_COLOR << endl;
					exit(1);
				}
			}
			//read wt_json as utf-8
			string wt_json;
			{
				//用win api读取文件
				HANDLE hFile = CreateFileW(wt_json_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if(hFile == INVALID_HANDLE_VALUE) {
					wcerr << SET_RED "Can't open file: " SET_CYAN << wt_json_path << RESET_COLOR << endl;
					exit(1);
				}
				DWORD dwFileSize = GetFileSize(hFile, nullptr);
				if(dwFileSize == INVALID_FILE_SIZE) {
					wcerr << SET_RED "Can't get file size: " SET_CYAN << wt_json_path << RESET_COLOR << endl;
					exit(1);
				}
				wt_json.resize(dwFileSize);
				DWORD dwRead;
				if(!ReadFile(hFile, wt_json.data(), dwFileSize, &dwRead, nullptr)) {
					wcerr << SET_RED "Can't read file: " SET_CYAN << wt_json_path << RESET_COLOR << endl;
					exit(1);
				}
				CloseHandle(hFile);
			}
			{
				wstring start_command;
				if(ghost_path.empty())
					start_command = L'"' + to_command_path_string(my_name) + L"\" -g \"" + ghost_link_to + L"\" -r";
				else
					start_command = L'"' + to_command_path_string(my_name) + L"\" -gp \"" + to_command_path_string(ghost_path) + L"\" -r";
				string new_wt_json = R"+({
    "profiles": [
        {
            "commandline": ")+" + to_json_string(start_command) + R"+(",
            "hidden": false,
            "icon": ")+" + to_json_string(wt_icon) + R"+(",
            "name": ")+" + to_json_string(wt_name) + R"+("
        }
    ]
}
)+";
				if(wt_json != new_wt_json) {
					//win api写文件
					HANDLE hFile = CreateFileW(wt_json_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
					if(hFile == INVALID_HANDLE_VALUE) {
						wcerr << SET_RED "Can't open file: " SET_CYAN << wt_json_path << RESET_COLOR << endl;
						exit(1);
					}
					DWORD dwWrite;
					if(!WriteFile(hFile, new_wt_json.data(), new_wt_json.size(), &dwWrite, nullptr)) {
						wcerr << SET_RED "Can't write file: " SET_CYAN << wt_json_path << RESET_COLOR << endl;
						exit(1);
					}
					CloseHandle(hFile);
				}
			}
			//run wt with arg
			//wt.exe -p "wt_name"
			wstring wt_path = LOCALAPPDATA + L"\\Microsoft\\WindowsApps\\wt.exe";
			if(!PathFileExistsW(wt_path.c_str())) {
				wcerr << SET_PURPLE "Can't find Windows Terminal(" << wt_path << L")\n"
					  << "You can download it from https://aka.ms/terminal ." RESET_COLOR << endl;
			}
			else {
				//run wt
				ShellExecuteW(nullptr, L"open", wt_path.c_str(), (L"-p \"" + wt_name + L"\"").c_str(), nullptr, SW_SHOW);
				exit(0);
			}
		}
	}
};

int wmain(int argc, wchar_t* argv[]) {
	ghost_terminal{}(argc, argv);
	return 0;
}
