#pragma once
class TAGGROUP {
public:
	static string strIniDir;
	static vector<string> vGroup;
	static vector<string> vGroupByPattern;
	static vector<string> vGroupByValue;
	static map<string, string> m_Pattern_Group;
	static map<string, string> m_Value_Group;
	TAGGROUP() {}
	static void init(char *szDir) {
		*(strrchr(szDir, '\\') + 1) = 0;

		strlcat(szDir, "FileTag.ini", 1024);
		strIniDir = szDir;
		char strGroupsByPattern[1024];
		//WritePrivateProfileStringA("groups", szDir, "", "D:\\filetag.ini");
		GetPrivateProfileStringA("groups", "pattern", "", strGroupsByPattern, 1024, szDir);
		vGroupByPattern = split(strGroupsByPattern, ",");
		for (auto itGroup = vGroupByPattern.begin(); itGroup != vGroupByPattern.end(); itGroup++) {
			if (*itGroup == "") {
				continue;
			}
			vGroup.push_back(*itGroup);
			
			char strPattern[1024];
			GetPrivateProfileStringA("pattern", (*itGroup).c_str(), "", strPattern, 1024, szDir);
			m_Pattern_Group[strPattern] = *itGroup;
		}
		char strGroupsByValue[1024];
		GetPrivateProfileStringA("groups", "value", "", strGroupsByValue, 1024, szDir);
		vGroupByValue = split(strGroupsByValue, ",");
		for (auto itGroup = vGroupByValue.begin(); itGroup != vGroupByValue.end(); itGroup++) {
			if (*itGroup == "") {
				continue;
			}
			vGroup.push_back(*itGroup);			
			char strValue[1024];
			GetPrivateProfileStringA("value", (*itGroup).c_str(), "", strValue, 1024, szDir);
			vector<string> vValue = split(strValue, ",");
			for (auto itValue = vValue.begin(); itValue != vValue.end(); itValue++) {
				if (*itValue == "") {
					continue;
				}
				m_Value_Group[*itValue] = *itGroup;
				
			}
		}
		vGroup.push_back("tag");

	};
	static int _ContentGetSupportedField(int FieldIndex, char* FieldName, char* Units, int maxlen) {
		if (FieldIndex<0 || FieldIndex >= vGroup.size())
			return ft_nomorefields;
		strlcpy(FieldName, vGroup[FieldIndex].c_str(), maxlen - 1);
		return ft_string;
	}
};

class DESCRIPTION {
	
public:
	FILETIME ftLastWriteTime;
	char pPathIon[1024] = {0};
	map<string, map<string,string>> m_file_tags;
	DESCRIPTION() {}
	DESCRIPTION(char* pfiledir) {
		strlcat(pPathIon, pfiledir, 1024);
		strlcat(pPathIon, "descript.ion", 1024);
		ftLastWriteTime.dwHighDateTime = 0;
		ftLastWriteTime.dwLowDateTime = 0;
	}
	int getFieldValue( char* pfilename, const char* FieldName, void* FieldValue, int maxlen) {
		if (isRewrited()) {
			//没有读取过或已经修改了就会进入这个流程
			//从descript.ion文件读取文件夹内每个文件的描述信息
			//
			m_file_tags.clear();
			ParseDirDes();
		}
		//已经读取过
		
		map<string, map<string,string>>::iterator mapIter_file_tags = m_file_tags.find(pfilename);
		if (mapIter_file_tags != m_file_tags.end()) {
			map<string, string> m_tags = mapIter_file_tags->second;
			map<string, string>::iterator mapIter_tags = m_tags.find(FieldName);
			if (mapIter_tags != m_tags.end()) {
				if (mapIter_tags->second=="") {
					return ft_fieldempty;
				}
				else {
					strlcpy((char*)FieldValue, mapIter_tags->second.c_str(), maxlen - 1);					
					return ft_string;
				}
				
			}			
		}

		return ft_fieldempty;
	}
	bool isRewrited() {
		//获取description文件的lastwritetime,对比，判断是否修改过
		WIN32_FIND_DATAA fd;
		HANDLE fh;
		fh = FindFirstFileA(pPathIon, &fd);
		
		if (fd.ftLastWriteTime.dwHighDateTime == ftLastWriteTime.dwHighDateTime && fd.ftLastWriteTime.dwLowDateTime == ftLastWriteTime.dwLowDateTime) {
			return FALSE;
		}
		else {
			ftLastWriteTime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime;
			ftLastWriteTime.dwLowDateTime = fd.ftLastWriteTime.dwLowDateTime;
			return TRUE;
		}
	}
	void ParseDirDes() {
		fstream hFile;
		hFile.open(pPathIon, ios::out | ios::in, _SH_DENYWR);
		string strLine;
		while (getline(hFile, strLine)) {
			
			ParseLine(strLine);
		}
		hFile.close();
	}
	void ParseLine(string strLine) {
		string name;
		string strDes;
		string strFirstChar = strLine.substr(0, 1);		
		if (strFirstChar == "\"") {
			string::size_type pos = strLine.find("\" ", 1);
			if (pos != strLine.npos) {
				name = strLine.substr(1, pos-1);
				strDes = strLine.substr(pos + 2, strLine.size() - pos);
				
				map<string, string> mapTags = ParseDesToTags(strDes);
				m_file_tags.insert(map<string, map<string, string>>::value_type(name, mapTags));
			}
		}
		else {

			string::size_type pos = strLine.find_first_of(" ");			
			if (pos != strLine.npos) {
				name= strLine.substr(0, pos);
				strDes=strLine.substr(pos+1, strLine.size() - pos);
				
				map<string, string> mapTags = ParseDesToTags(strDes);
				m_file_tags.insert(map<string, map<string, string>>::value_type(name, mapTags));
			}
		}						
	}
	map<string,string> ParseDesToTags(string strDes) {
		char strDelimit[MAX_PATH];
		GetPrivateProfileStringA("delimit", "pattern", ";", strDelimit, MAX_PATH, TAGGROUP::strIniDir.c_str());
		map<string, string> mapTags;
		vector<string> vTagPairs = split(strDes, strDelimit);
		for (auto itTagPairs = vTagPairs.begin(); itTagPairs != vTagPairs.end(); itTagPairs++) {
			if (*itTagPairs == "") {
				continue;
			}
			
			//string strGroup = vGroupAndTag[0];
			string strTag = *itTagPairs;
			string strGroup = getGroup(strTag);
			map<string, string>::iterator mitTags = mapTags.find(strGroup);
			if (mitTags != mapTags.end()) {
				mitTags->second = mitTags->second + ";" + strTag;
			}
			else {
				mapTags[strGroup] = strTag;			
				
			}
		}
		return mapTags;
	}
	string getGroup(string strTag) {
		string::size_type position;
		for (auto it = TAGGROUP::m_Pattern_Group.begin(); it != TAGGROUP::m_Pattern_Group.end(); it++) {
			position = strTag.find(it->first);
			if (position != strTag.npos) {
				return it ->second;
			}
		}
		map<string, string>::iterator itTags = TAGGROUP::m_Value_Group.find(strTag);
		if (itTags != TAGGROUP::m_Value_Group.end()) {
			return itTags->second;
		}
		return "tag";
	}
};

