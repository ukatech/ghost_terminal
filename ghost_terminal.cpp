#include "..\SSTP_linker\include\SSTP.hpp"
#include <iostream>
#include <Windows.h>
int main(){
	using namespace SSTP_link_n;
	using namespace std;
	SSTP_link_t linker;
	{
		auto names =
		linker.NOTYFY({ { L"Event", L"On_ShioriEcho.GetName" } }).to_map();
		wcout << "terminal login\n";
		if(names.has(L"GhostName"))
			wcout << "ghost: " << names[L"GhostName"] << '\n';
		if(names.has(L"UserName"))
			wcout << "User: " << names[L"UserName"] << '\n';
	}
	{
		wstring commad;
		SSTP_link_args_t Result={};
		wcout << ">> ";
		while (wcin >> commad && commad != L"exit") {
			linker.NOTYFY({ { L"Event", L"On_ShioriEcho" },
							{ L"Reference0", commad }
						  });
			do{
				Result=linker.NOTYFY({ { L"Event", L"On_ShioriEcho.GetResult" } }).to_map();
				if(Result.has(L"Special")){
					wcout << Result[L"Special"] << endl;
					break;
				}else if(Result.has(L"Result")){
					wcout << Result[L"Result"] << endl;
					if(Result.has(L"Type"))
						wcout << L"Type: " << Result[L"Type"] << endl;
					break;
				}
				else{
					Sleep(1000);
					continue;
				}
			}while(1);
			wcout << ">> ";
		}
	}
	{
		linker.NOTYFY({ { L"Event", L"On_ShioriEcho.End" } });
	}
}
