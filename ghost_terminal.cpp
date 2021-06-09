#include "..\SSTP_linker\include\SSTP.hpp"
#include <iostream>
int main(){
	using namespace SSTP_link_n;
	using namespace std;
	SSTP_link_t linker({{L"Charset",L"UTF-8"},{L"Sender",L"Ghost Terminal"}});
	if(linker.Has_Event(L"ShioriEcho.GetName")){
		auto names = linker.NOTYFY({ { L"Event", L"ShioriEcho.GetName" } });
		wcout << "terminal login\n";
		if(names.has(L"GhostName"))
			wcout << "ghost: " << names[L"GhostName"] << '\n';
		if(names.has(L"UserName"))
			wcout << "User: " << names[L"UserName"] << '\n';
	}
	{
		wstring commad;
		SSTP_ret_t Result;
		wcout << ">> ";
		while (wcin >> commad && commad != L"exit") {
			if(!linker.Has_Event(L"ShioriEcho"))
				wcout << "Event Has_Event or ShioriEcho Not define.\n";
			linker.NOTYFY({ { L"Event", L"ShioriEcho" },
							{ L"Reference0", commad }
						  });
			if(linker.Has_Event(L"ShioriEcho.GetResult")){
				do{
					Result=linker.NOTYFY({ { L"Event", L"ShioriEcho.GetResult" } });
					if(Result.has(L"Special")){
						wcout << Result[L"Special"] << endl;
						break;
					}else if(Result.has(L"Result")){
						wcout << Result[L"Result"] << endl;
						if(Result.has(L"Type"))
							wcout << "Type: " << Result[L"Type"] << endl;
						break;
					}
					else{
						Sleep(1000);
						continue;
					}
				}while(1);
			}
			else
				wcout << "Event Has_Event or ShioriEcho.GetResult Not define.\n";
			wcout << ">> ";
		}
	}
	{
		linker.NOTYFY({ { L"Event", L"ShioriEcho.End" } });
	}
}
