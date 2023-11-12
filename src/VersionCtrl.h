#ifndef VERSIONCTRL_H
#define VERSIONCTRL_H

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <tchar.h>
#include <cctype>
#include <locale>
#include <codecvt>
#include <conio.h>
#include <algorithm>
#include <vector>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

std::string getLatestVersion();

#endif