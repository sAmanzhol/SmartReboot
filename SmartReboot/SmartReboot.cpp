#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <list>  

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

void ExecFilteredPrograms(HKEY hKey) {
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode;

	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	BYTE* buffer = new BYTE[cbMaxValueData];
	ZeroMemory(buffer, cbMaxValueData);
	wifstream black_list_file("black_list.txt");
	wstring str;
	list<wstring> black_list;
	bool isFiltered = false;
	while (getline(black_list_file, str)) {
		black_list.push_back(str);
	}
	if (cValues) {
		for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++) {
			cchValue = MAX_VALUE_NAME;
			achValue[0] = '\0';
			retCode = RegEnumValue(hKey, i,
				achValue,
				&cchValue,
				NULL,
				NULL,
				NULL,
				NULL);

			if (retCode == ERROR_SUCCESS) {
				DWORD lpData = cbMaxValueData;
				buffer[0] = '\0';
				LONG dwRes = RegQueryValueEx(hKey, achValue, 0, NULL, buffer, &lpData);
				wstring key(&achValue[0]);
				wstring value(reinterpret_cast<wchar_t*>(buffer), cbMaxValueData / sizeof(wchar_t));

				for (auto key_word : black_list) {
					if (key.find(key_word) == string::npos && value.find(key_word) == string::npos) {
						isFiltered = true;
					} else {
						isFiltered = false;
						break;
					}
				}
				
				if (isFiltered) {
					BOOL result;
					SHELLEXECUTEINFO shellExecuteInfo = { 0 };
					shellExecuteInfo.cbSize = sizeof(shellExecuteInfo);
					shellExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
					shellExecuteInfo.hwnd = NULL;
					shellExecuteInfo.lpVerb = NULL;
					shellExecuteInfo.lpFile = value.c_str();
					shellExecuteInfo.lpParameters = NULL;
					shellExecuteInfo.lpDirectory = NULL;
					shellExecuteInfo.nShow = SW_SHOWNORMAL;
					shellExecuteInfo.hInstApp = NULL;
					result = ShellExecuteEx(&shellExecuteInfo);
				}
				
			}
		}
	}

	delete[] buffer;
}


int _tmain(int argc, _TCHAR* argv[]){
	HKEY hKey_1;
	LONG dwRegOPenKey_1 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\"), 0, KEY_READ, &hKey_1);
	HKEY hKey_2;
	LONG dwRegOPenKey_2 = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run\\"), 0, KEY_READ, &hKey_2);
	if (dwRegOPenKey_1 == ERROR_SUCCESS || dwRegOPenKey_2 == ERROR_SUCCESS) {
		ExecFilteredPrograms(hKey_1);
		ExecFilteredPrograms(hKey_2);
	} else if (dwRegOPenKey_1 != ERROR_SUCCESS) {
		printf("RegOpenKeyEx failed, error code %d\n", dwRegOPenKey_1);
	} else if (dwRegOPenKey_2 != ERROR_SUCCESS) {
		printf("RegOpenKeyEx failed, error code %d\n", dwRegOPenKey_2);
	}
	RegCloseKey(hKey_1);
	RegCloseKey(hKey_2);
	return 0;
}