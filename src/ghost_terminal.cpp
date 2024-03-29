﻿#define SMALLIO_OUT_ALWAYS_CONSOLE
#define SMALLIO_DISABLE_INPUT

#include "my-gists/windows/shell_base.hpp"
#include "my-gists/windows/small_io.hpp"
#include "my-gists/windows/InWindowsTerminal.hpp"
#include "my-gists/windows/IsElevated.hpp"
#include "my-gists/windows/SetIcon.h"

#include "my-gists/ukagaka/SSTP.hpp"
#include "my-gists/ukagaka/SFMO.hpp"
#include "my-gists/ukagaka/SSP_Runner.hpp"
#include "my-gists/ukagaka/ghost_path.hpp"
#include "my-gists/ukagaka/from_ghost_path.hpp"

#include "my-gists/codepage.hpp"
#include "my-gists/ansi_color.hpp"

#include "my-gists/STL/replace_all.hpp"
#include "my-gists/STL/to_json_X.hpp"

#include <string>
#include <string_view>
#include <ranges>

#include <shlwapi.h>		   //PathFileExistsW
#include <shlobj_core.h>	   //SHCreateDirectoryEx

//lib of PathFileExists & SHCreateDirectoryEx
#pragma comment(lib, "shlwapi.lib")

#define floop while(1)

#include "Version.h"

using namespace std;
using namespace SSTP_link_n;
using namespace terminal_n;

wstring to_command_path_string(wstring str) noexcept {
	if(str.ends_with('\\'))		  // 考虑"path\"，很明显\"会构成转义序列
		str.pop_back();
	return str;
}

wstring do_transfer(wstring a) {
	replace_all(a, L"\\n"sv, L"\n"sv);
	replace_all(a, L"\\\n"sv, L"\\\\n"sv);

	replace_all(a, L"\\t"sv, L"\t"sv);
	replace_all(a, L"\\\t"sv, L"\\\\t"sv);

	replace_all(a, L"\\\\"sv, L"\\"sv);
	return a;
}

class ghost_terminal final: public simple_terminal {
private:
	//data valueable until terminal login
	struct args_info_t {
		wstring ghost_link_to;
		wstring ghost_path;
		bool	run_ghost  = 0;
		HWND	ghost_hwnd = NULL;
		wstring command;
		wstring sakurascript;
		bool	has_command		 = 0;
		bool	has_sakurascript = 0;
		bool	register2wt		 = 0;

		bool disable_root_text			  = 0;
		bool disable_event_text			  = 0;
		bool disable_WindowsTerminal_text = 0;
		bool disable_FiraCode_text		  = 0;
	};
	struct data_until_login_t {		  //保持直到终端登录的数据
		args_info_t args_info;
		bool		fira_code_font_found = 0;
		wstring		LOCALAPPDATA		 = _wgetenv(L"LOCALAPPDATA");
	};
	struct data_until_login_saver_t {
		data_until_login_t* data_until_login_ptr;
		void				take_new() {
			   data_until_login_ptr = new data_until_login_t;
		}
		data_until_login_t& get() noexcept {
			return *data_until_login_ptr;
		}
		struct destroy_flag_t {
			data_until_login_t* data_until_login_ptr;
			~destroy_flag_t() {
				delete data_until_login_ptr;
			}
		};
		destroy_flag_t make_destroy_flag() noexcept {
			return destroy_flag_t{data_until_login_ptr};
		}
	} data_until_login_saver;

	//ghost info & linker
	SSTP_link_t linker{{{L"Charset", L"UTF-8"},
						{L"Sender", L"Ghost Terminal"},
						{L"SecurityLevel", L"local"}}};
	wstring		ghost_uid;

	//old terminal info for restore
	wstring		old_title;
	ICON_INFO_t old_icon_info;

	//ables
	bool able_get_result	   = 1;
	bool able_tab_press		   = 1;
	bool able_command_update   = 1;
	bool able_command_complete = 1;
	bool able_command_history  = 1;
	bool able_command_prompt   = 1;

private:
	static int CALLBACK FiraCode_Finder(const LOGFONTW* lpelfe, [[maybe_unused]] const TEXTMETRICW* lpntme, [[maybe_unused]] DWORD FontType, LPARAM lParam) noexcept {
		auto&			   self		 = *(ghost_terminal*)lParam;
		const wstring_view font_name = lpelfe->lfFaceName;
		if(font_name.find(L"Fira Code") == 0) {
			self.data_until_login_saver.get().fira_code_font_found = true;
			return 0;		//stop enum
		}
		return 1;
	}

protected:
	//before terminal run
	virtual void before_terminal_login() override {
		simple_terminal::before_terminal_login();
		old_title.resize(MAX_PATH);
		old_title.resize(GetConsoleTitleW(old_title.data(), old_title.size()));
		old_icon_info = GetConsoleIcon();
	}
	virtual void terminal_args(size_t argc, std::vector<std::wstring>& argv) override {
		data_until_login_saver.take_new();

		auto& data_until_login = data_until_login_saver.get();
		auto& args_info		   = data_until_login.args_info;
		//const auto& fira_code_font_found = data_until_login.fira_code_font_found;
		const auto& LOCALAPPDATA = data_until_login.LOCALAPPDATA;

		auto& ghost_path	   = args_info.ghost_path;
		auto& run_ghost		   = args_info.run_ghost;
		auto& ghost_link_to	   = args_info.ghost_link_to;
		auto& ghost_hwnd	   = args_info.ghost_hwnd;
		auto& command		   = args_info.command;
		auto& sakurascript	   = args_info.sakurascript;
		auto& register2wt	   = args_info.register2wt;
		auto& has_command	   = args_info.has_command;
		auto& has_sakurascript = args_info.has_sakurascript;
		//disables
		auto& disable_root_text			   = args_info.disable_root_text;
		auto& disable_event_text		   = args_info.disable_event_text;
		auto& disable_WindowsTerminal_text = args_info.disable_WindowsTerminal_text;
		auto& disable_FiraCode_text		   = args_info.disable_FiraCode_text;

		wstring wt_name;
		wstring wt_icon;
		if(argc != 1) {
			size_t i = 1;
			while(i < argc) {
				if(argv[i] == L"-c" || argv[i] == L"--command") {		//command
					i++;
					has_command = 1;
					if(i < argc)
						command = argv[i];
					else {
						err << RED_OUTPUT("No command after " << argv[i - 1] << ".") << endline;
						exit(1);
					}
				}
				else if(argv[i] == L"-s" || argv[i] == L"--sakura-script") {	   //sakura script
					i++;
					has_sakurascript = 1;
					if(i < argc)
						sakurascript = argv[i];
					else {
						err << RED_OUTPUT("No sakura script after " << argv[i - 1] << ".") << endline;
						exit(1);
					}
				}
				else if(argv[i] == L"-g" || argv[i] == L"--ghost") {	   //ghost
					i++;
					if(i < argc)
						ghost_link_to = argv[i];
					else {
						err << RED_OUTPUT("No ghost name after " << argv[i - 1] << ".") << endline;
						exit(1);
					}
				}
				else if(argv[i] == L"-gh" || argv[i] == L"--ghost-hwnd") {		 //ghost hwnd
					i++;
					if(i < argc)
						ghost_hwnd = (HWND)wcstoll(argv[i].c_str(), nullptr, 10);
					else {
						err << RED_OUTPUT("No ghost hwnd after " << argv[i - 1] << ".") << endline;
						exit(1);
					}
				}
				else if(argv[i] == L"-gp" || argv[i] == L"--ghost-folder-path") {
					i++;
					if(i < argc)
						ghost_path = make_ghost_path(argv[i]);
					else {
						err << RED_OUTPUT("No ghost folder path after " << argv[i - 1] << ".") << endline;
						exit(1);
					}
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
				else if(argv[i] == L"--disable-text") {
					i++;
					if(i >= argc) {
						err << RED_OUTPUT("No disable text types after " << argv[i - 1] << ".") << endline;
						exit(1);
					}
					const wstring& disable_text = argv[i];
					if(disable_text == L"all")
						disable_root_text = disable_event_text = disable_WindowsTerminal_text = disable_FiraCode_text = true;
					else
						//for each disable text split by ','
						for(const auto& word: views::split(disable_text, L',')) {
							const wstring_view disable_text_type{word.begin(), word.end()};
							if(disable_text_type == L"root")
								disable_root_text = true;
							else if(disable_text_type == L"event")
								disable_event_text = true;
							else if(disable_text_type == L"WindowsTerminal")
								disable_WindowsTerminal_text = true;
							else if(disable_text_type == L"FiraCode")
								disable_FiraCode_text = true;
							else if(disable_text_type.size())
								err << SET_GRAY "Ignore unknown disable text: " SET_PURPLE << disable_text_type << RESET_COLOR << endline;
						}
				}
				else if(argv[i] == L"-h" || argv[i] == L"--help") {		  //help
					out << "ghost terminal " VERSION_STRING_WITH_V "\n\n"
						<< LIGHT_YELLOW_OUTPUT(argv[0]) << SET_CYAN " [options]\n" RESET_COLOR
						"options:\n"
						"  " SET_GREEN "-h" SET_YELLOW "," SET_GREEN " --help" SET_WHITE "                            : " SET_GRAY "shows this help message " SET_RED "and exits.\n"
						"  " SET_GREEN "-v" SET_YELLOW "," SET_GREEN " --version" SET_WHITE "                         : " SET_GRAY "shows the version number " SET_RED "and exits.\n"
						"  " SET_GREEN "-c" SET_YELLOW "," SET_GREEN " --command " SET_PURPLE "<command>" SET_WHITE "               : " SET_GRAY "runs the specified command " SET_RED "and exits.\n"
						"  " SET_GREEN "-s" SET_YELLOW "," SET_GREEN " --sakura-script " SET_PURPLE "<script>" SET_WHITE "          : " SET_GRAY "runs the specified Sakura script " SET_RED "and exits.\n"
						"  " SET_GREEN "-g" SET_YELLOW "," SET_GREEN " --ghost " SET_PURPLE "<ghost>" SET_WHITE "                   : " SET_GRAY "links to the specified ghost by name.\n"
						"  " SET_GREEN "-gh" SET_YELLOW "," SET_GREEN " --ghost-hwnd " SET_PURPLE "<hwnd>" SET_WHITE "              : " SET_GRAY "links to the specified ghost by HWND.\n"
						"  " SET_GREEN "-gp" SET_YELLOW "," SET_GREEN " --ghost-folder-path " SET_PURPLE "<path>" SET_WHITE "       : " SET_GRAY "links to the specified ghost by folder path.\n"
						"  " SET_GREEN "-r" SET_YELLOW "," SET_GREEN " --run-ghost" SET_WHITE "                       : " SET_GRAY "runs the ghost if (it/she/he/them/other pronouns) is not currently running.\n"
						"  " SET_GREEN "-rwt" SET_YELLOW "," SET_GREEN " --register-to-windows-terminal" SET_WHITE "  : " SET_GRAY "registers to the Windows terminal (requires " SET_GREEN "-g" SET_PURPLE " <ghost name>" SET_GRAY " or " SET_GREEN "-gp" SET_PURPLE " <ghost folder path>" SET_GRAY ").\n"
						"        " SET_GREEN "-rwt-name " SET_PURPLE "<name>" SET_WHITE "                : " SET_GRAY "registers to the Windows terminal with the specified name (only works with " SET_GREEN "-rwt" SET_GRAY ").\n"
						"        " SET_GREEN "-rwt-icon " SET_PURPLE "<icon>" SET_WHITE "                : " SET_GRAY "registers to the Windows terminal with the specified icon (PNG or ICO path) (only works with " SET_GREEN "-rwt" SET_GRAY ").\n"
						"  " SET_GREEN "--disable-text " SET_PURPLE "<text types>" SET_GRAY "|" SET_PURPLE "all" SET_WHITE "       : " SET_GRAY "disable some unnecessary text(split by '" SET_PURPLE "," SET_GRAY "') or all of them.\n"
						"        " SET_PURPLE "root" SET_WHITE "                            : " SET_GRAY "disables the " BLINK_TEXT("easter egg") " text when running terminal as root.\n"
						"        " SET_PURPLE "event" SET_WHITE "                           : " SET_GRAY "disables the warning text when your ghost not having some events.\n"
						"        " SET_PURPLE "WindowsTerminal" SET_WHITE "                 : " SET_GRAY "disables the text telling you to install " UNDERLINE_TEXT("Windows Terminal") " or run this exe with " SET_GREEN "-rwt " SET_GRAY "(" SET_GREEN "-g" SET_GRAY "|" SET_GREEN "-gp" SET_GRAY ").\n"
						"        " SET_PURPLE "FiraCode" SET_WHITE "                        : " SET_GRAY "disables the text telling you try " UNDERLINE_TEXT("Fira Code") " font.\n" RESET_COLOR
						"example:\n"
						"  " SET_LIGHT_YELLOW "ghost-terminal " SET_GREEN "-g " SET_PURPLE "\"Taromati2\" " SET_GREEN "-rwt --disable-text " SET_PURPLE "event,WindowsTerminal,FiraCode" RESET_COLOR "\n" RESET_COLOR;
					exit(0);
				}
				else if(argv[i] == L"-v" || argv[i] == L"--version") {
					out << SET_GRAY "ghost terminal " VERSION_STRING_WITH_V "\n" RESET_COLOR;
					exit(0);
				}
				else {
					err << SET_RED "unknown option: " SET_GRAY << argv[i] << RESET_COLOR << endline;
					exit(1);
				}
				i++;
			}
		}
		if(register2wt) {
			//尝试通过fmo获取ghost的文件夹路径
			if(ghost_path.empty() && !ghost_link_to.empty()) {
				SFMO_t fmobj;
				if(fmobj.Update_info()) {
					out << SET_GRAY "Read FMO info success\n" RESET_COLOR;
					for(auto& i: fmobj.info_map) {
						if(i.second[L"fullname"] != ghost_link_to)
							continue;
						ghost_path = i.second[L"ghostpath"];
						break;
					}
					if(ghost_path.empty())
						err << SET_RED "Can't find ghost path by name: " SET_BLUE << ghost_link_to << RESET_COLOR << endline;
					else
						out << SET_GRAY "Get ghost path by name: " SET_CYAN << ghost_path << RESET_COLOR << endline;
					ghost_path += L"ghost\\master\\";
				}
			}
			//处理ghost_path，获得ghost的name 和 icon
			auto ghost_info = from_ghost_path::get_name_and_icon_path(ghost_path);
			if(wt_name.empty())
				if(ghost_link_to.empty() && ghost_path.empty()) {
					err << SET_RED "Registering to Windows Terminal requires a ghost name or folder path. Use" SET_GREEN "-g" SET_RED " or " SET_GREEN "-gp" SET_RED " to set it." RESET_COLOR << endline;
					exit(1);
				}
				else {
					if(ghost_link_to.empty())
						if(!ghost_info.name.empty()) {
							ghost_link_to = ghost_info.name;
							out << SET_GRAY "Got ghost name from ghost folder: " SET_BLUE << ghost_link_to << RESET_COLOR << endline;
						}
						else {
							err << SET_RED "Unable to get ghost name from ghost folder. Use " SET_GREEN "-g" SET_RED " to set it." RESET_COLOR << endline;
							exit(1);
						}
					wt_name = ghost_link_to + L" terminal";
				}
			if(wt_icon.empty())
				if(!ghost_path.empty())
					if(ghost_info.icon_path.empty())
						err << SET_RED "Can't get ghost icon from ghost folder. Use " SET_GREEN "-rwt-icon" SET_RED " to set it." RESET_COLOR << endline;
					else {
						wchar_t full_path[MAX_PATH];
						GetFullPathNameW(ghost_info.icon_path.c_str(), MAX_PATH, full_path, nullptr);
						wt_icon = full_path;
						out << SET_GRAY "Got ghost icon from ghost folder: " SET_CYAN << wt_icon << RESET_COLOR << endline;
					}
			constexpr auto default_icon = L"ms-appx:///ProfileIcons/{0caa0dad-35be-5f56-a8ff-afceeeaa6101}.png"sv;
			if(wt_icon.empty())
				wt_icon = default_icon;
			auto& my_name = argv[0];
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
				err << SET_RED "Can't find exe file (" SET_YELLOW << my_name << SET_RED ")\n" RESET_COLOR;
				exit(1);
			}
			//register to windows terminal
			//C:\Users\<user>\AppData\Local\Microsoft\Windows Terminal\Fragments\{app-name}\{file-name}.json
			wstring wt_json_dir_path = LOCALAPPDATA + L"\\Microsoft\\Windows Terminal\\Fragments\\Ghost Terminal\\";
			//+wt_name+L".json";
			wstring wt_json_path = wt_json_dir_path + ghost_link_to + L".json";
			//创建文件夹和文件（并递归父文件夹直到成功）
			{
				const auto res = SHCreateDirectoryExW(nullptr, wt_json_dir_path.c_str(), nullptr);
				if(res != ERROR_SUCCESS && res != ERROR_FILE_EXISTS && res != ERROR_ALREADY_EXISTS) {
					err << SET_RED "Can't create directory: " SET_CYAN << wt_json_dir_path << RESET_COLOR << endline;
					exit(1);
				}
			}
			//read wt_json as utf-8
			string wt_json;
			{
				HANDLE hFile = CreateFileW(wt_json_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if(hFile != INVALID_HANDLE_VALUE) {
					const DWORD dwFileSize = GetFileSize(hFile, nullptr);
					if(dwFileSize != INVALID_FILE_SIZE) {
						wt_json.resize(dwFileSize);
						DWORD dwRead;
						if(!ReadFile(hFile, wt_json.data(), dwFileSize, &dwRead, nullptr)) {
							err << SET_RED "Can't read file: " SET_CYAN << wt_json_path << SET_RED ", overwriting it." RESET_COLOR << endline;
						}
					}
					CloseHandle(hFile);
				}
			}
			{
				wstring start_command;
				if(ghost_path.empty())
					start_command = L'"' + to_command_path_string(my_name) + L"\" -g \"" + ghost_link_to + L"\" -r";
				else {
					auto simplified_ghost_path = ghost_path.substr(0, ghost_path.size() - 14);		 //"\\ghost\\master\\"
					start_command			   = L'"' + to_command_path_string(my_name) + L"\" -gp \"" + simplified_ghost_path + L"\" -r";
				}
				if(disable_root_text | disable_event_text | disable_WindowsTerminal_text | disable_FiraCode_text) {
					start_command += L" --disable-text ";
					if(disable_root_text)
						start_command += L"root,";
					if(disable_event_text)
						start_command += L"event,";
					if(disable_WindowsTerminal_text)
						start_command += L"WindowsTerminal,";
					if(disable_FiraCode_text)
						start_command += L"FiraCode,";
					start_command.pop_back();
				}
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
					HANDLE hFile = CreateFileW(wt_json_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
					if(hFile == INVALID_HANDLE_VALUE) {
						err << SET_RED "Can't open file: " SET_CYAN << wt_json_path << RESET_COLOR << endline;
						exit(1);
					}
					DWORD dwWrite;
					if(!WriteFile(hFile, new_wt_json.data(), new_wt_json.size(), &dwWrite, nullptr)) {
						CloseHandle(hFile);
						err << SET_RED "Can't write file: " SET_CYAN << wt_json_path << RESET_COLOR << endline;
						exit(1);
					}
					CloseHandle(hFile);
				}
			}
			//run wt with arg
			//wt.exe -p "wt_name"
			wstring wt_path = LOCALAPPDATA + L"\\Microsoft\\WindowsApps\\wt.exe";
			if(!PathFileExistsW(wt_path.c_str())) {
				err << SET_PURPLE "Can't find Windows Terminal(" << wt_path << L")\n"
								  "You can download it from <" UNDERLINE_TEXT("https://aka.ms/terminal") ">." RESET_COLOR
					<< endline;
			}
			else {
				//run wt
				ShellExecuteW(nullptr, L"open", wt_path.c_str(), (L"-p \"" + wt_name + L"\"").c_str(), nullptr, SW_SHOW);
				exit(0);
			}
		}
	}
	virtual void terminal_login() override {
		SFMO_t fmobj;

		auto&		data_until_login	 = data_until_login_saver.get();
		auto&		args_info			 = data_until_login.args_info;
		const auto& fira_code_font_found = data_until_login.fira_code_font_found;
		const auto& LOCALAPPDATA		 = data_until_login.LOCALAPPDATA;

		//make destroy_flag as data is no longer needed after this function
		auto destroy_flag = data_until_login_saver.make_destroy_flag();

		auto&		ghost_link_to	 = args_info.ghost_link_to;
		auto&		ghost_path		 = args_info.ghost_path;
		const auto& run_ghost		 = args_info.run_ghost;
		auto&		ghost_hwnd		 = args_info.ghost_hwnd;
		const auto& register2wt		 = args_info.register2wt;
		const auto& command			 = args_info.command;
		const auto& sakurascript	 = args_info.sakurascript;
		const auto& has_command		 = args_info.has_command;
		const auto& has_sakurascript = args_info.has_sakurascript;
		//disables
		const auto& disable_root_text			 = args_info.disable_root_text;
		const auto& disable_event_text			 = args_info.disable_event_text;
		const auto& disable_WindowsTerminal_text = args_info.disable_WindowsTerminal_text;
		const auto& disable_FiraCode_text		 = args_info.disable_FiraCode_text;

		//处理ghost_path，获得ghost_link_to
		if(!ghost_path.empty() && ghost_link_to.empty()) {
			ghost_link_to = from_ghost_path::get_name(ghost_path);
			if(ghost_link_to.empty()) {
				err << SET_RED "Can't find ghost name from " SET_BLUE << ghost_path << RESET_COLOR << endline;
				exit(1);
			}
		}
		auto waiter = [&](auto condition, wstring condition_str, size_t timeout = 30) {
			if(!condition()) {
				out << LIGHT_YELLOW_OUTPUT("Waiting for " << condition_str << "...") << flush;
				reprinter_t reprinter;
				size_t		time = 0;
				floop {
					reprinter(L"" YELLOW_TEXT("(" + to_wstring(time) + L'/' + to_wstring(timeout) + L"s)"));
					Sleep(1000);
					if(timeout == time) {
						out << endline;
						err << RED_TEXT("timeout.") << endline;
						exit(1);
					}
					time++;
					if(condition())
						break;
				}
				out << endline;
			}
		};
		auto start_ghost = [&] {
			out << LIGHT_YELLOW_TEXT("Trying to start ghost...\n");
			if(ghost_link_to.empty() && ghost_path.empty())		  //?
				err << SET_CYAN "You can use " SET_GREEN "-g" SET_CYAN " or " SET_GREEN "-gp" SET_CYAN " to specify the ghost (by name or path).\n" RESET_COLOR;
			{
				SSP_Runner SSP;
				if(!SSP.IsInstalled()) {
					err << RED_TEXT("SSP is not installed.") << endline;
					exit(1);
				}
				if(ghost_path.empty())
					SSP.run_ghost(ghost_link_to);
				else if(ghost_link_to.empty())
					SSP();
				else
					SSP.run_ghost(ghost_path);
			}
			fmobj.Update_info();
			waiter([&] {
				return fmobj.Update_info() && fmobj.info_map.size() > 0;
			},L"FMO initialized");
			if(!ghost_link_to.empty())
				waiter([&] {
					if(fmobj.Update_info())
						for(auto& i: fmobj.info_map) {
							HWND tmp_hwnd = (HWND)wcstoll(i.second[L"hwnd"].c_str(), nullptr, 10);
							if(i.second[L"fullname"] == ghost_link_to)
								return ghost_hwnd = tmp_hwnd;
						}
					return HWND{0};
				},L"ghost hwnd created");
		};
		if(ghost_hwnd)
			goto link_to_ghost;
		if(fmobj.Update_info()) {
			{
				const auto ghostnum = fmobj.info_map.size();
				if(ghostnum == 0) {
					if(!run_ghost) {
						err << RED_TEXT("None of ghost was running.") << endline;
						exit(1);
					}
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
						err << SET_RED "Target ghost: " SET_BLUE << ghost_link_to << SET_RED " not found." RESET_COLOR << endline;
						exit(1);
					}
				}
				else if(ghostnum == 1) {
					out << GRAY_TEXT("Only one ghost was running.\n");
					ghost_hwnd = (HWND)wcstoll(fmobj.info_map.begin()->second.map[L"hwnd"].c_str(), nullptr, 10);
				}
				else {
					out << LIGHT_YELLOW_TEXT("Select the ghost you want to log into. [Up/Down/Enter]\n");
					HANDLE		 hInput = GetStdHandle(STD_INPUT_HANDLE);
					INPUT_RECORD ir;
					DWORD		 cNumRead;
					reprinter_t	 reprinter;
					const auto	 pbg   = fmobj.info_map.begin();
					const auto	 ped   = fmobj.info_map.end();
					const auto	 psize = fmobj.info_map.size();
					auto		 p	   = pbg;
					while(!ghost_hwnd) {
						reprinter(p->second[L"name"]);
						ReadConsoleInputW(hInput, &ir, 1, &cNumRead);
						switch(ir.EventType) {
						case KEY_EVENT:
							break;
						default:
							//ignore
							continue;
						}
						const auto& key = ir.Event.KeyEvent;
						if(!key.bKeyDown)
							continue;
						//esc
						if(key.wVirtualKeyCode == VK_ESCAPE)
							exit(0);
						//enter
						if(key.wVirtualKeyCode == VK_RETURN) {
							out << L'\n';
							ghost_hwnd = (HWND)wcstoll(p->second[L"hwnd"].c_str(), nullptr, 10);
						}
						//tab|right|down
						if(key.wVirtualKeyCode == VK_TAB || key.wVirtualKeyCode == VK_RIGHT || key.wVirtualKeyCode == VK_DOWN) {
							auto size = key.wRepeatCount % psize;
							while(size--) {
								p++;
								if(p == ped)
									p = pbg;
							}
						}
						//up|delete|left
						if(key.wVirtualKeyCode == VK_UP || key.wVirtualKeyCode == VK_DELETE || key.wVirtualKeyCode == VK_LEFT) {
							auto size = key.wRepeatCount % psize;
							while(size--) {
								if(p == pbg)
									p = ped;
								p--;
							}
						}
					}
				}
			}
		link_to_ghost:
			if(ghost_uid.empty()) {
				if(fmobj.Update_info()) {
					for(auto& i: fmobj.info_map) {
						const HWND tmp_hwnd = (HWND)wcstoll(i.second[L"hwnd"].c_str(), nullptr, 10);
						if(ghost_hwnd == tmp_hwnd) {
							ghost_uid = i.first;
							break;
						}
					}
				}
				if(ghost_uid.empty()) {
					err << RED_TEXT("Can\'t get ghost_uid.") << endline;
					exit(1);
				}
			}
			linker.link_to_ghost(ghost_hwnd);
		}
		else {
			if(!run_ghost) {
				err << RED_TEXT("Can\'t read FMO info.") << endline;
				exit(1);
			}
			start_ghost();
			goto link_to_ghost;
		}
		if(!has_sakurascript)	   //发送ss不需要shiori就绪
			waiter([&] {
				return fmobj.Update_info() && fmobj.info_map[ghost_uid].get_modulestate(L"shiori") == L"running";
			},L"ghost's shiori ready");

		if(!disable_WindowsTerminal_text && !InWindowsTerminal()) {
			if(!register2wt) {
				const wstring wt_path = LOCALAPPDATA + L"\\Microsoft\\WindowsApps\\wt.exe";
				if(!PathFileExistsW(wt_path.c_str()))
					out << SET_GRAY "Terminal can look more sleek if you have Windows Terminal installed.\n"
									"Download it from <" UNDERLINE_TEXT("https://aka.ms/terminal") "> and run this exe with " SET_GREEN "-rwt " SET_GRAY "(" SET_GREEN "-g" SET_GRAY "|" SET_GREEN "-gp" SET_GRAY ")." RESET_COLOR
						<< endline;
				else
					out << SET_GRAY "You can run this exe with " SET_GREEN "-rwt " SET_GRAY "(" SET_GREEN "-g" SET_GRAY "|" SET_GREEN "-gp" SET_GRAY ") for a better experience under Windows Terminal." RESET_COLOR << endline;
			}
		}
		if(!disable_FiraCode_text) {
			//通过EnumFontFamiliesEx遍历字体，找到一个以Fira Code开头的字体就不提示了
			//如果找不到，就提示一下
			{
				LOGFONTW lf;	   //可以掠过0初始化：以下3个必须项目已被正确赋值
				lf.lfCharSet		= DEFAULT_CHARSET;
				lf.lfFaceName[0]	= L'\0';
				lf.lfPitchAndFamily = 0;
				HDC hdc				= GetDC(NULL);
				if(hdc) {
					EnumFontFamiliesExW(hdc, &lf, (FONTENUMPROCW)FiraCode_Finder, (LPARAM)this, 0);
					ReleaseDC(NULL, hdc);
				}
			}
			if(!fira_code_font_found)
				out << SET_GRAY "You can use " SET_GREEN "Fira Code" SET_GRAY " font for a better experience in terminal and editor.\n"
								"Try it from <" UNDERLINE_TEXT("https://github.com/tonsky/FiraCode") "> !" RESET_COLOR
					<< endline;
		}
		{
			auto names	= linker.NOTYFY({{L"Event", L"ShioriEcho.GetName"}});
			auto result = linker.NOTYFY({
				{L"Event", L"ShioriEcho.Begin"},
				{L"Reference0", L"" VERSION_STRING},
				{L"Reference1", [&] {		//mode
					if(has_command)
						return L"Command";
					if(has_sakurascript)
						return L"SakuraScript";
					return L"Common";
				 }()},
			});
			{
				//set console title
				wstring title = L"Ghost Terminal";
				if(result.has(L"Tittle"))
					title = result[L"Tittle"];
				else if(!args_info.ghost_link_to.empty())
					title += L" - " + args_info.ghost_link_to;
				SetConsoleTitleW(title.c_str());
			}
			{
				ICON_INFO_t icon_info = old_icon_info;
				if(result.has(L"Icon")) {
					auto hIcon = LoadIconWithBasePath(ghost_path, result[L"Icon"]);
					if(!hIcon)
						err << SET_RED "Can't load icon: " SET_BLUE << result[L"Icon"] << RESET_COLOR << endline;
					else {
						icon_info.hIcon		 = hIcon;
						icon_info.hIconSmall = hIcon;
					}
				}
				if(result.has(L"SmallIcon")) {
					auto hIcon = LoadIconWithBasePath(ghost_path, result[L"SmallIcon"]);
					if(!hIcon)
						err << SET_RED "Can't load icon: " SET_BLUE << result[L"SmallIcon"] << RESET_COLOR << endline;
					else
						icon_info.hIconSmall = hIcon;
				}
				SetConsoleIcon(icon_info);
			}

			if(result.has(L"CustomLoginInfo")) {
				if(result[L"CustomLoginInfo"].size())
					out << LIGHT_YELLOW_OUTPUT(do_transfer(result[L"CustomLoginInfo"])) << '\n';
			}
			else if(!(has_command || has_sakurascript)) {
				out << CYAN_TEXT("terminal login\n");
				if(names.has(L"GhostName"))
					out << "Ghost: " << LIGHT_YELLOW_OUTPUT(names[L"GhostName"]) << '\n';
				if(names.has(L"UserName"))
					out << "User: " << LIGHT_YELLOW_OUTPUT(names[L"UserName"]) << '\n';
			}
		}

		bool need_end = 0;
		if(has_command) {
			terminal_run(command);
			need_end = 1;
		}
		if(has_sakurascript) {
			linker.SEND({{L"ID", ghost_uid}, {L"Script", sakurascript}});
			need_end = 1;
		}
		if(need_end) {
			terminal_exit();
			exit(0);
		}
		if(!disable_root_text && IsElevated())
			out << SET_GRAY "Coooool, You're running terminal with " BOLD_TEXT(UNDERLINE_TEXT("root") " access") "!\n"
							"But that won't do any good, terminal won't have any new " BLINK_TEXT("super") " cow power.\n"
							"It will just run as it always does.\n\n" RESET_COLOR;
		if(linker.Has_Event(L"Has_Event")) {
			#if defined(_MSC_VER)
				#pragma warning(push)
				#pragma warning(disable : 4459)
				#pragma warning(disable : 6244)
			#endif
			auto& err = [&]() -> base_out_t& {
				if(disable_event_text)
					return nullstream;
				return ::err;
			}();
			#if defined(_MSC_VER)
				#pragma warning(pop)
			#endif
			err << SET_GRAY;
			if(!disable_event_text && !linker.Has_Event(L"ShioriEcho"))		  //在disable_event_text时完全可以不检查ShioriEcho
				err << "Event " SET_GREEN "ShioriEcho" SET_GRAY " Not defined.\n"
					   "Your ghost may not be supporting Terminal if it can't handle ShioriEcho event.\n\n";
			//ShioriEcho.GetResult
			if(!linker.Has_Event(L"ShioriEcho.GetResult")) {
				able_get_result = 0;
				err << "Event " SET_GREEN "ShioriEcho.GetResult" SET_GRAY " Not defined.\n"
					   "Terminal will not send get result event to your ghost and will not echo the result if your ghost does not returns " SET_PURPLE "Result&Type" SET_GRAY " in the " SET_GREEN "ShioriEcho" SET_GRAY " event.\n\n";
			}
			if(!linker.Has_Event(L"ShioriEcho.CommandComplete")) {
				able_command_complete = 0;
				err << "Event " SET_GREEN "ShioriEcho.CommandComplete" SET_GRAY " Not defined.\n"
					   "Terminal will not send command complete event to your ghost.\n\n";
			}
			if(!linker.Has_Event(L"ShioriEcho.CommandUpdate")) {
				able_command_update = 0;
				err << "Event " SET_GREEN "ShioriEcho.CommandUpdate" SET_GRAY " Not defined.\n"
					   "Terminal will not send command update event to your ghost.\n\n";
			}
			if(!linker.Has_Event(L"ShioriEcho.CommandHistory.New") || !linker.Has_Event(L"ShioriEcho.CommandHistory.Get") ||
			   !linker.Has_Event(L"ShioriEcho.CommandHistory.Update") || !linker.Has_Event(L"ShioriEcho.CommandHistory.ForwardIndex")) {
				able_command_history = 0;
				err << "Your ghost needs to support all of the following events to support command history:\n" SET_GREEN
					   "ShioriEcho.CommandHistory.New\n"
					   "ShioriEcho.CommandHistory.Get\n"
					   "ShioriEcho.CommandHistory.Update\n"
					   "ShioriEcho.CommandHistory.ForwardIndex\n" SET_GRAY
					   "Terminal will use its default command history function and not send command history events to your ghost.\n\n";
			}
			if(!linker.Has_Event(L"ShioriEcho.TabPress")) {
				able_tab_press = 0;
				err << "Event " SET_GREEN "ShioriEcho.TabPress" SET_GRAY " Not defined.\n"
					   "Terminal will not send tab press event to your ghost.\n\n";
			}
			if(!linker.Has_Event(L"ShioriEcho.CommandPrompt")) {
				able_command_prompt = 0;
				err << "Event " SET_GREEN "ShioriEcho.CommandPrompt" SET_GRAY " Not defined.\n"
					   "Terminal will not send command prompt event to your ghost.\n\n";
			}
			err << RESET_COLOR;
		}
		else {
			able_tab_press		  = 0;
			able_command_update	  = 0;
			able_command_complete = 0;
			able_command_history  = 0;
			able_command_prompt	  = 0;
			able_get_result		  = 0;
			err << SET_RED "Event " SET_GREEN "Has_Event" SET_RED " Not defined.\n"
						   "You need to make your ghost support " SET_GREEN "Has_Event" SET_RED " event so that Terminal can know what events it supports.\n"
						   "Terminal will assume your ghost only supports " SET_GREEN "ShioriEcho" SET_RED " event." RESET_COLOR "\n\n";
		}
	}
	//terminal runnning
	enum terminal_result_status {
		Continue,
		Wait,
		End
	};
	terminal_result_status result_handle(const SSTP_ret_t& Result) {
		{
			const auto code = Result.get_code();
			if(code == 404 || code == -1) {
				err << RED_TEXT("Lost connection with target ghost.") << endline;
				exit(1);
			}
		}
		if(Result.has(L"Special")) {
			out << do_transfer(Result[L"Special"]) << endline;
			return Continue;
		}
		else if(Result.has(L"Result")) {
			out << Result[L"Result"] << endline;
			if(Result.has(L"Type"))
				out << "Type: " << Result[L"Type"] << endline;
			return Continue;
		}
		else if(Result.has(L"Type")) {
			out << "Has " GREEN_TEXT("Type") " but no " GREEN_TEXT("Result") " here:\n "
				<< to_ansi_colored_wstring(Result) << endline;
			return Continue;
		}
		else if(Result.has(L"Status")) {
			const auto& status = Result[L"Status"];
			if(status == L"End")
				return End;
			else if(status == L"Continue")
				return Continue;
		}
		else if(able_get_result)
			return Wait;
		return Continue;
	}
	virtual bool terminal_run(const wstring& command) override {
		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho"},
									 {L"ID", ghost_uid},
									 {L"Reference0", command}});
		if(able_get_result)
			floop {
				auto GetResult = linker.NOTYFY({{L"Event", L"ShioriEcho.GetResult"}});
				switch(result_handle(GetResult)) {
				case Continue:
					return true;
				case Wait:
					Sleep(1000);
					break;
				case End:
					return false;
				}
			}
		else {
			switch(result_handle(Result)) {
			case Continue:
				return true;
			default:
				return false;
			}
		}
	}
	//terminal exit
	virtual void terminal_exit() override {
		linker.NOTYFY({{L"Event", L"ShioriEcho.End"}});
		SetConsoleTitleW(old_title.c_str());
		SetConsoleIcon(old_icon_info);
		simple_terminal::terminal_exit();
	}
	//other terminal events

	//prompt
	virtual std::wstring terminal_command_prompt() override {
		if(able_command_prompt) {
			auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.CommandPrompt"}});
			if(Result.has(L"Prompt"))
				return Result[L"Prompt"];
		}
		return simple_terminal::terminal_command_prompt();
	}
	//completes
	virtual editting_command_t terminal_tab_press(const editting_command_t& command, size_t tab_num) override {
		if(!able_tab_press)
			return command;
		static size_t last_old_insert_index = 0;
		if(!tab_num)
			last_old_insert_index = command.insert_index;
		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.TabPress"},
									 {L"Reference0", command.command},
									 {L"Reference1", to_wstring(command.insert_index)},
									 {L"Reference2", to_wstring(tab_num)},
									 {L"Reference3", to_wstring(last_old_insert_index)}});

		editting_command_t Result_command = command;
		if(Result.has(L"Command"))
			Result_command.command = Result[L"Command"];
		if(Result.has(L"InsertIndex"))
			Result_command.insert_index = (size_t)stoull(Result[L"InsertIndex"]);
		if(Result.has(L"OldInsertIndex"))
			last_old_insert_index = (size_t)stoull(Result[L"OldInsertIndex"]);
		return Result_command;
	}
	virtual editting_command_t terminal_command_complete_by_right(const editting_command_t& command) override {
		if(!able_command_complete)
			return command;
		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.CommandComplete"},
									 {L"Reference0", command.command},
									 {L"Reference1", to_wstring(command.insert_index)}});

		editting_command_t Result_command = command;
		if(Result.has(L"Command"))
			Result_command.command = Result[L"Command"];
		if(Result.has(L"InsertIndex"))
			Result_command.insert_index = (size_t)stoull(Result[L"InsertIndex"]);
		return Result_command;
	}
	//command update
	virtual std::wstring terminal_command_update(std::wstring command) override {
		if(!able_command_update)
			return command;
		{
			//考虑到sstp极慢的速度，只在需要时更新command的色彩
			const auto next_char = terminal_n::peek_key_input();
			if(next_char == L'\r' || next_char == L'\n')
				;		//fall through
			else if(next_char)
				return command;
		}
		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.CommandUpdate"},
									 {L"Reference0", command}});
		if(Result.has(L"CommandForDisplay"))
			return Result[L"CommandForDisplay"];
		else
			return command;
	}
	//command history events
	virtual void terminal_command_history_new() override {
		if(!able_command_history) {
			simple_terminal::terminal_command_history_new();
			return;
		}
		linker.NOTYFY({{L"Event", L"ShioriEcho.CommandHistory.New"}});
	}
	virtual void terminal_command_history_update(const std::wstring& command, size_t before_num) override {
		if(!able_command_history) {
			simple_terminal::terminal_command_history_update(command, before_num);
			return;
		}
		linker.NOTYFY({{L"Event", L"ShioriEcho.CommandHistory.Update"},
					   {L"Reference0", command},
					   {L"Reference1", to_wstring(before_num)}});
	}
	virtual bool terminal_command_history_forward(size_t& index, size_t forward_num) override {
		if(!able_command_history)
			return simple_terminal::terminal_command_history_forward(index, forward_num);

		auto Result = linker.NOTYFY({{L"Event", L"ShioriEcho.CommandHistory.ForwardIndex"},
									 {L"Reference0", to_wstring(index)},
									 {L"Reference1", to_wstring(forward_num)}});
		if(Result.has(L"Index")) {
			index = (size_t)stoull(Result[L"Index"]);
			return true;
		}
		else
			return false;
	}
	virtual std::wstring terminal_get_command_history(size_t before_num) override {
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
};

int wmain(int argc, wchar_t* argv[]) {
	try {
		return ghost_terminal{}(argc, argv), 0;
	}
	catch(const std::exception& a) {
		err << RED_OUTPUT(a.what()) << endline;
		exit(1);
	}
}
