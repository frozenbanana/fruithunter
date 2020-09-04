#pragma once
#include <iostream>
#include <Windows.h>
#include <vector>
#include <string>

using namespace std;

static std::wstring str2wstr(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

static void read_directory(const std::string& name, vector<string>& v) {
	std::string pattern(name);
	pattern.append("\\*");
	WIN32_FIND_DATA data;
	HANDLE hFind;
	if ((hFind = FindFirstFile(str2wstr(pattern).c_str(), &data)) != INVALID_HANDLE_VALUE) {
		do {
			wstring ws(data.cFileName);
			string str(ws.begin(), ws.end());
			v.push_back(str);
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
}

static void create_directory(string path) { CreateDirectory(str2wstr(path).c_str(), NULL); }