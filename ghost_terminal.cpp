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
		wcout << "terminal login\n"
			  << "ghost: " << names[L"GhostName"] << '\n'
			  << "User: " << names[L"UserName"] << '\n';
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
				if(Result.has(L"Special"))
					wcout << Result[L"Special"] << endl;
				else if(Result.has(L"Result")){
					wcout << Result[L"Result"] << endl;
					if(Result.has(L"Type"))
						wcout << L"Type: " << Result[L"Type"] << endl;
				}
				else{
					Sleep(1);
					continue;
				}
			}while(0);
			wcout << ">> ";
		}
	}
	{
		linker.NOTYFY({ { L"Event", L"On_ShioriEcho.End" } });
	}
}
