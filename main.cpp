// Jerzy Wciseł © All Rights Reserved 2023-2024

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <tlhelp32.h>
#include <tchar.h>
#include <cctype>
#include <locale>
#include <codecvt>
#include <conio.h>
#include <algorithm>

const std::string VERSION = "0.5.1";

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

std::string processesToKill[25] = {};

void TerminateProcessByFileName(const std::string& fileName) {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating process snapshot (" << GetLastError() << ")\n";
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        std::cerr << "Error retrieving process information (" << GetLastError() << ")\n";
        CloseHandle(hProcessSnap);
        return;
    }

    do {
        if (_stricmp(pe32.szExeFile, reinterpret_cast<const char *>(fileName.c_str())) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess != nullptr) {
                if (TerminateProcess(hProcess, 0)) {
                    std::cout << "Process " << fileName << " terminated successfully\n";
                } else {
                    std::cerr << "Error terminating process (" << GetLastError() << ")\n";
                }

                CloseHandle(hProcess);
            } else {
                std::cerr << "Error opening process (" << GetLastError() << ")\n";
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

std::wstring GetTopLevelParentProcessName(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess) {
        char processName[MAX_PATH];
        DWORD size = sizeof(processName) / sizeof(processName[0]);

        if (QueryFullProcessImageNameA(hProcess, 0, processName, &size)) {
            CloseHandle(hProcess);
            return std::wstring(processName, processName + size / sizeof(char));
        }

        CloseHandle(hProcess);
    }
    return L"";
}

std::wstring GetParentProcessName(DWORD processId) {
    while (processId != 0) {
        std::wstring parentProcessName = GetTopLevelParentProcessName(processId);

        if (parentProcessName.empty() || parentProcessName.find(L"System32\\") == std::wstring::npos) {
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

void CALLBACK WinEventProc(
        HWINEVENTHOOK hWinEventHook,
        DWORD event,
        HWND hwnd,
        LONG idObject,
        LONG idChild,
        DWORD dwEventThread,
        DWORD dwmsEventTime
) {
    if (event == EVENT_SYSTEM_FOREGROUND) {
        DWORD processId;
        GetWindowThreadProcessId(hwnd, &processId);

        std::wstring parentProcessName = GetParentProcessName(processId);

        if (!parentProcessName.empty()) {
            const std::wstring& widePath(parentProcessName);

            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::string utf8Path = converter.to_bytes(widePath);

            std::filesystem::path filePath(utf8Path);
            std::wcout << L"New process spawned: " << filePath.filename().wstring() << L" (" << filePath.wstring() << L")\n";

            for (std::string& i : processesToKill) {
                std::string processName = filePath.filename().string();

                std::transform(processName.begin(), processName.end(), processName.begin(), [](char c) {
                    return std::tolower(c);
                });

                std::transform(i.begin(), i.end(), i.begin(), [](char c) {
                    return std::tolower(static_cast<unsigned char>(c));
                });

                // Remove trailing whitespace characters
                processName.erase(std::remove_if(processName.begin(), processName.end(), [](char c) {
                    return std::isspace(static_cast<unsigned char>(c));
                }), processName.end());

                i.erase(std::remove_if(i.begin(), i.end(), [](char c) {
                    return std::isspace(static_cast<unsigned char>(c));
                }), i.end());

                if (processName == i) {
                    TerminateProcessByFileName(i);
                    std::cout << processName << " should be terminated by now. Enjoy ;)" << std::endl;
                }
            }
        }
    }
}

// For logo ;p
int getTerminalWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.dwSize.X;
}

void bigLogo() {
    std::cout << R"(


          JJJJJJJJJJJ                                                                                     WWWWWWWW                           WWWWWWWW
          J:::::::::J                                                                                     W::::::W                           W::::::W
          J:::::::::J                                                                                     W::::::W                           W::::::W
          JJ:::::::JJ                                                                                     W::::::W                           W::::::W
            J:::::J    eeeeeeeeeeee    rrrrr   rrrrrrrrr   zzzzzzzzzzzzzzzzzyyyyyyy           yyyyyyy      W:::::W           WWWWW           W:::::W
            J:::::J  ee::::::::::::ee  r::::rrr:::::::::r  z:::::::::::::::z y:::::y         y:::::y        W:::::W         W:::::W         W:::::W
            J:::::J e::::::eeeee:::::eer:::::::::::::::::r z::::::::::::::z   y:::::y       y:::::y          W:::::W       W:::::::W       W:::::W
            J:::::Je::::::e     e:::::err::::::rrrrr::::::rzzzzzzzz::::::z     y:::::y     y:::::y            W:::::W     W:::::::::W     W:::::W
            J:::::Je:::::::eeeee::::::e r:::::r     r:::::r      z::::::z       y:::::y   y:::::y              W:::::W   W:::::W:::::W   W:::::W
JJJJJJJ     J:::::Je:::::::::::::::::e  r:::::r     rrrrrrr     z::::::z         y:::::y y:::::y                W:::::W W:::::W W:::::W W:::::W
J:::::J     J:::::Je::::::eeeeeeeeeee   r:::::r                z::::::z           y:::::y:::::y                  W:::::W:::::W   W:::::W:::::W
J::::::J   J::::::Je:::::::e            r:::::r               z::::::z             y:::::::::y                    W:::::::::W     W:::::::::W
J:::::::JJJ:::::::Je::::::::e           r:::::r              z::::::zzzzzzzz        y:::::::y                      W:::::::W       W:::::::W
 JJ:::::::::::::JJ  e::::::::eeeeeeee   r:::::r             z::::::::::::::z         y:::::y                        W:::::W         W:::::W
   JJ:::::::::JJ     ee:::::::::::::e   r:::::r            z:::::::::::::::z        y:::::y                          W:::W           W:::W
     JJJJJJJJJ         eeeeeeeeeeeeee   rrrrrrr            zzzzzzzzzzzzzzzzz       y:::::y                            WWW             WWW
                                                                                  y:::::y
                                                                                 y:::::y
                                                                                y:::::y
                                                                               y:::::y
                                                                              yyyyyyy

                                                                                                                                                      )" << std::endl;
}

void mediumLogo() {
    std::cout << "                                                                                     \n";
    std::cout << "         ,---._                                                                      \n";
    std::cout << "       .-- -.' \\                                                               .---. \n";
    std::cout << "       |    |   :                                                             /. ./| \n";
    std::cout << "       :    ;   |            __  ,-.       ,----,                         .--'.  ' ; \n";
    std::cout << "       :        |          ,' ,'/ /|     .'   .`|                        /__./ \\ : | \n";
    std::cout << "       |    :   :   ,---.  '  | |' |  .'   .'  .'      .--,          .--'.  '   \\  . \n";
    std::cout << "       :           /     \\ |  |   ,',---, '   ./     /_ ./|         /___/ \\ |    ' ' \n";
    std::cout << "       |    ;   | /    /  |'  :  /  ;   | .'  /   , ' , ' :         ;   \\  \\;      : \n";
    std::cout << "   ___ l         .    ' / ||  | '   `---' /  ;--,/___/ \\: |          \\   ;  `      | \n";
    std::cout << " /    /\\    J   :'   ;   /|;  : |     /  /  / .`| .  \\  ' |           .   \\    .\\  ; \n";
    std::cout << "/  ../  `..-    ,'   |  / ||  , ;   ./__;     .'   \\  ;   :            \\   \\   ' \\ | \n";
    std::cout << "\\    \\         ; |   :    | ---'    ;   |  .'       \\  \\  ;             :   '  |--\"  \n";
    std::cout << " \\    \\      ,'   \\   \\  /          `---'            :  \\  \\             \\   \\ ;     \n";
    std::cout << "  \"---....--'      `----'                             \\  ' ;              '---\"      \n";
    std::cout << "                                                       `--`                          \n" << std::endl;
}

void smallLogo() {
    std::cout << "   ___                       _    _ \n";
    std::cout << "  |_  |                     | |  | |\n";
    std::cout << "    | | ___ _ __ _____   _  | |  | |\n";
    std::cout << "    | |/ _ \\ '__|_  / | | | | |\\| |\n";
    std::cout << "/\\__/ /  __/ |   / /| |_| | \\  /\\  /\n";
    std::cout << "\\____/ \\___|_|  /___|\\__, |  \\/  \\/ \n";
    std::cout << "                      __/ |         \n";
    std::cout << "                     |___/          \n" << std::endl;
}

// Actual main function

int main() {
    SetConsoleTextAttribute(hConsole, 6);
    int terminalWidth = getTerminalWidth();
    std::cout << "Terminal Width: " << terminalWidth << " columns" << std::endl;
    SetConsoleTextAttribute(hConsole, 5);

    if (terminalWidth >= 149) bigLogo();
    else if (terminalWidth >= 85) mediumLogo();
    else smallLogo();

    SetConsoleTextAttribute(hConsole, 3); // Reset color to default
    // Reading properties from file
    const std::string fileName = "blocked.properties";
    std::ifstream inputFile(fileName, std::ios::in | std::ios::binary);

    if (!inputFile.is_open()) {
        SetConsoleTextAttribute(hConsole, 12);
        std::cout << "blocked.properties configuration file doesn't exist. Creating one ;P\n";
        std::ofstream outputFile(fileName);

        if (!outputFile.is_open()) {
            std::cerr << "Error creating the file.\n";
            return 1;
        }

        outputFile << "# Write here file names of processes you want to kill, separated by a new line\n";
        std::cout << "Write to it file names of processes you want to kill, separated by a new line\n";
        outputFile.close();
        std::cout << "Press ANY key to exit...\n";
        _getch();
        SetConsoleTextAttribute(hConsole, 7);
        return 0;
    } else {
        std::cout << "Found configuration file :)\n";
        std::string line;
        int i = 0;

        while (std::getline(inputFile, line)) {
            if (!line.empty() && line[0] == '#') {
                continue;
            }

            if (!line.empty()) {
                processesToKill[i] = line;
                i++;
            }
        }

        inputFile.close();
    }

    int len = 0;
    for (const std::string& process : processesToKill) {
        if (!process.empty()) len++;
    }

    if (len <= 0) {
        SetConsoleTextAttribute(hConsole, 12);
        std::cout << "You need to write program binary names in the .properties file!\n";
        std::cout << "Press ANY key to exit...\n";
        _getch();
        SetConsoleTextAttribute(hConsole, 7);
        return 0;
    }

    std::cout << "Processes to kill:\n";
    for (const std::string& process : processesToKill) {
        if (!process.empty()) std::cout << process << '\n';
    }

    if (!SetWinEventHook(
            EVENT_SYSTEM_FOREGROUND,
            EVENT_SYSTEM_FOREGROUND,
            nullptr,
            reinterpret_cast<WINEVENTPROC>(WinEventProc),
            0,
            0,
            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
    )) {
        SetConsoleTextAttribute(hConsole, 12);
        std::cerr << "SetWinEventHook failed.\n";
        SetConsoleTextAttribute(hConsole, 7);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWinEvent(nullptr);
    SetConsoleTextAttribute(hConsole, 7);
    return 0;
}