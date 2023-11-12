#include "ProcessFunctions.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <tlhelp32.h>
#include <tchar.h>
#include <cctype>
#include <locale>
#include <codecvt>
#include <algorithm>
#pragma comment(lib, "winhttp.lib")

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

// Function to terminate process using its filename
void TerminateProcessByFileName(std::basic_string<char> fileName) {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(hConsole, 6);
        std::cerr << "Error creating process snapshot (" << GetLastError() << ")\n";
        SetConsoleTextAttribute(hConsole, 3);
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        SetConsoleTextAttribute(hConsole, 6);
        std::cerr << "Error retrieving process information (" << GetLastError() << ")\n";
        SetConsoleTextAttribute(hConsole, 3);
        CloseHandle(hProcessSnap);
        return;
    }

    do {
        if (_stricmp(pe32.szExeFile, reinterpret_cast<const char *>(fileName.c_str())) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess != nullptr) {
                if (TerminateProcess(hProcess, 0)) {
                    SetConsoleTextAttribute(hConsole, 6);
                    std::cout << "Process " << fileName << " terminated successfully\n";
                    SetConsoleTextAttribute(hConsole, 3);
                } else {
                    SetConsoleTextAttribute(hConsole, 4);
                    std::cerr << "Error terminating process (" << GetLastError() << ")\n";
                    SetConsoleTextAttribute(hConsole, 3);
                }

                CloseHandle(hProcess);
            } else {
                SetConsoleTextAttribute(hConsole, 4);
                std::cerr << "Error opening process (" << GetLastError() << ")\n";
                SetConsoleTextAttribute(hConsole, 3);
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

// Gets process name from its PID
std::wstring GetTopLevelParentProcessName(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess) {
        char processName[MAX_PATH];
        DWORD size = sizeof(processName) / sizeof(processName[0]);

        if (QueryFullProcessImageNameA(hProcess, 0, processName, &size)) {
            CloseHandle(hProcess);

            // Convert the process name to wstring using codecvt
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.from_bytes(processName);
        }

        CloseHandle(hProcess);
    }
    return L"";
}

// Returns Parent process name if process have one. (probably doesn't work) ¯\_(ツ)_/¯
std::wstring GetParentProcessName(DWORD processId) {
    while (processId != 0) {
        std::wstring parentProcessName = GetTopLevelParentProcessName(processId);

        if (parentProcessName.empty()/* || parentProcessName.find(L"System32\\") == std::wstring::npos*/) {
            return parentProcessName;
        }

        if (parentProcessName.find(L"conhost.exe") != std::wstring::npos) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
            if (hProcess) {
                GetWindowThreadProcessId(GetParent(GetConsoleWindow()), &processId);
                CloseHandle(hProcess);
            } else {
                return L"";
            }
        } else {
            return parentProcessName;
        }
    }

    return L"";
}

HWND GetProcessWindow(DWORD processId) {
    HWND hwnd = GetTopWindow(0);
    while (hwnd) {
        DWORD windowProcessId;
        GetWindowThreadProcessId(hwnd, &windowProcessId);
        if (windowProcessId == processId) {
            return hwnd;
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    return nullptr;
}

HWND GetParentProcessById(DWORD processId) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == processId) {
                CloseHandle(hSnapshot);
                return GetProcessWindow(pe32.th32ParentProcessID);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return nullptr;
}

HWND GetParentProcess(HWND hwnd) {
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    return GetParentProcessById(processId);
}

HANDLE getHConsole() {
    return hConsole;
}

void EnumerateProcesses(std::string (&processesToKill)[25]) {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(hConsole, 4);
        std::cerr << "Error creating process snapshot (" << GetLastError() << ")\n";
        SetConsoleTextAttribute(hConsole, 3);
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        SetConsoleTextAttribute(hConsole, 4);
        std::cerr << "Error retrieving process information (" << GetLastError() << ")\n";
        SetConsoleTextAttribute(hConsole, 3);
        CloseHandle(hProcessSnap);
        return;
    }

    do {
        std::wstring parentProcessName = GetParentProcessName(pe32.th32ProcessID);

        if (!parentProcessName.empty()) {
            const std::wstring &widePath(parentProcessName);

            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::string utf8Path = converter.to_bytes(widePath);

            std::filesystem::path filePath(utf8Path);

            // Just transformin processName
            std::string processName = filePath.filename().string();
            std::transform(processName.begin(), processName.end(), processName.begin(), [](char c) {
                return std::tolower(c);
            });
            // Remove trailing whitespace characters
            processName.erase(std::remove_if(processName.begin(), processName.end(), [](char c) {
                return std::isspace(static_cast<unsigned char>(c));
            }), processName.end());

            // For each
            for (std::string &i: processesToKill) {
                std::transform(i.begin(), i.end(), i.begin(), [](char c) {
                    return std::tolower(static_cast<unsigned char>(c));
                });

                i.erase(std::remove_if(i.begin(), i.end(), [](char c) {
                    return std::isspace(static_cast<unsigned char>(c));
                }), i.end());

                if (processName == i) {
                    TerminateProcessByFileName(i);
                    SetConsoleTextAttribute(hConsole, 13);
                    std::cout << processName << " should be terminated by now. Enjoy ;)" << std::endl;
                    SetConsoleTextAttribute(hConsole, 3);
                }
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

void GetAllProcesses(std::string (&processesToKill)[25]) {
    try {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            // Handle error
            return;
        }
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (!Process32First(hSnapshot, &pe32)) {
            CloseHandle(hSnapshot);
            // Handle error
            return;
        }
        do {
            // Access process information through the pe32 structure
            //_tprintf(TEXT("Process ID: %d, Name: %s\n"), pe32.th32ProcessID, pe32.szExeFile);
            std::string processName = pe32.szExeFile;
            std::transform(processName.begin(), processName.end(), processName.begin(), [](char c) {
                return std::tolower(c);
            });
            processName.erase(std::remove_if(processName.begin(), processName.end(), [](char c) {
                return std::isspace(static_cast<unsigned char>(c));
            }), processName.end());
            for (std::string &i: processesToKill) {
                std::transform(i.begin(), i.end(), i.begin(), [](char c) {
                    return std::tolower(static_cast<unsigned char>(c));
                });

                i.erase(std::remove_if(i.begin(), i.end(), [](char c) {
                    return std::isspace(static_cast<unsigned char>(c));
                }), i.end());

                if (processName == i) {
                    TerminateProcessByFileName(i);
                    SetConsoleTextAttribute(hConsole, 13);
                    std::cout << processName << " should be terminated by now. Enjoy ;)" << std::endl;
                    SetConsoleTextAttribute(hConsole, 3);
                }
            }
        } while (Process32Next(hSnapshot, &pe32));

        CloseHandle(hSnapshot);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }
};