// FileTag.cpp : 定义 DLL 应用程序的导出函数。
//


#include "stdafx.h"
#include <ctime>
#define fieldcount 12
#define _detectstring ""
char* fieldnames[fieldcount] = { "title","person",
"classify","grade","face",
"mosaic","female","uniform","relation","mating","ejaculation","tag" };

int fieldtypes[fieldcount] = {
	ft_string,
	ft_string,ft_string,ft_string,ft_string,
	ft_string,ft_string,ft_string,ft_string,ft_string,ft_string,ft_string };

vector<string> groups;
map<string, DESCRIPTION> map_Des;


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		char szDir[MAX_PATH];
		GetModuleFileNameA(hModule, szDir, MAX_PATH);
		TAGGROUP::init(szDir);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void pathsplite(char* pfilepath, char* pfiledir, char* pfilename) {
	strlcpy(pfiledir, pfilepath, 1024);
	strlcpy(pfilename, pfilepath, 1024);
	*(strrchr(pfiledir, '\\') + 1) = 0;
	pfilename = strrchr(pfilename, '\\');
}
int __stdcall ContentGetDetectString(char* DetectString, int maxlen)
{
	strlcpy(DetectString, _detectstring, maxlen);
	return 0;
}

int __stdcall ContentGetSupportedField(int FieldIndex, char* FieldName, char* Units, int maxlen)
{
	return TAGGROUP::_ContentGetSupportedField(FieldIndex, FieldName, Units, maxlen);
}

int __stdcall ContentGetValue(char* FileName, int FieldIndex, int UnitIndex, void* FieldValue, int maxlen, int flags)
{
	char pfiledir[1024]; char *pfilename;
	strlcpy(pfiledir, FileName, 1024);
	*(strrchr(pfiledir, '\\') + 1) = 0;
	pfilename = strrchr(FileName, '\\') + 1;
	//pathsplite(FileName, pfiledir, pfilename);

	//WritePrivateProfileStringA("test",pfilename,pfiledir,"D:\\avtag.ini");

	//在map中查找记录，判断是否读取过descript.ion文件
	map<string, DESCRIPTION>::iterator mapIter_Des = map_Des.find(pfiledir);
	if (mapIter_Des != map_Des.end())
	{

		int result = mapIter_Des->second.getFieldValue(pfilename, TAGGROUP::vGroup[FieldIndex].c_str(), FieldValue, maxlen);

		return result;

	}
	else {
		//DWORD startTime, endTime;
		//startTime = GetTickCount();
		DESCRIPTION des(pfiledir);
		map_Des[pfiledir] = des;
		int result = des.getFieldValue(pfilename, fieldnames[FieldIndex], FieldValue, maxlen);
		//endTime = GetTickCount();
		//DWORD runtime = endTime - startTime;
		//WritePrivateProfileStringA("chushi", FileName, (to_string(startTime) + "_" + to_string(endTime)).c_str(), "D:\\avtag.ini");
		return result;
	}
}

void __stdcall ContentSetDefaultParams(ContentDefaultParamStruct* dps)
{

}
