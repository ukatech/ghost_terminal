#include "..\SSTP_linker\include\SSTP.hpp"
#include <iostream>
int main(){
	using namespace SSTP_link_n;
	using namespace std;
	wcin.imbue(locale(""));
	wcout.imbue(locale(""));
	SSTP_link_t linker({{L"Charset",L"UTF-8"},{L"Sender",L"Ghost Terminal"}});
	{
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
		while (getline(wcin,commad) && commad != L"exit") {
			if(!linker.Has_Event(L"ShioriEcho"))
				wcout << "Event Has_Event or ShioriEcho Not define.\n";
			linker.NOTYFY({ { L"Event", L"ShioriEcho" },
							{ L"Reference0", commad }
						  });
			{
				do{
					Result=linker.NOTYFY({ { L"Event", L"ShioriEcho.GetResult" } });
					if(Result.get_code()==400)//Bad Request
						wcout << "Event ShioriEcho.GetResult Not define.\n";
					else if(Result.has(L"Special")){
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
				}while(1);
			}
			wcout << ">> ";
		}
	}
	{
		linker.NOTYFY({ { L"Event", L"ShioriEcho.End" } });
	}
}
