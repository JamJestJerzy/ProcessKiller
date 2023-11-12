// Jerzy "LifeOverflow" W © All Rights Reserved 2023-2024

// Includes
#include <winsock2.h>
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
#include <vector>
#include <locale>
#include <codecvt>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

// Current version string
std::string VERSION = "0.7.3";

// Idk. It enables access to terminal colors tho :)
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

// Initializes empty array
std::string processesToKill[25] = {};

// Namespaces or smth
using std::vector;
using std::cout;
using std::endl;

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

void EnumerateProcesses() {
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
            const std::wstring &widePath(parentProcessName);

            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::string utf8Path = converter.to_bytes(widePath);

            std::filesystem::path filePath(utf8Path);
            SetConsoleTextAttribute(hConsole, 9);
            std::wcout << L"New process spawned: " << filePath.filename().wstring() << L" (" << filePath.wstring()
                       << L")" << L" PID: " << (int) processId << std::endl;
            SetConsoleTextAttribute(hConsole, 3);

            // Print parent process information
            DWORD parentProcessId = GetProcessId(GetParentProcess(hwnd));
            if (parentProcessId != 0) {
                std::wstring parentProcessPath = GetTopLevelParentProcessName(parentProcessId);
                if (!parentProcessPath.empty()) {
                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                    std::string utf8ParentPath = converter.to_bytes(parentProcessPath);
                    std::cout << "Parent process: " << utf8ParentPath << std::endl;
                }
            }

            // Just transformin processName
            std::string processName = filePath.filename().string();
            std::transform(processName.begin(), processName.end(), processName.begin(), [](char c) {
                return std::tolower(c);
            });
            // Remove trailing whitespace characters
            processName.erase(std::remove_if(processName.begin(), processName.end(), [](char c) {
                return std::isspace(static_cast<unsigned char>(c));
            }), processName.end());

            if (processName == "windowsterminal.exe") {
                SetConsoleTextAttribute(hConsole, 14);
                std::cout << "Process is terminal. Checking if it got spawned by unwanted process." << std::endl;
                SetConsoleTextAttribute(hConsole, 3);
                EnumerateProcesses();
            }

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

                                                                                                                                                      )"
              << std::endl;
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

std::string getLatestVersion() {
    /* Request (begining) */
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
    BOOL bResults = FALSE;
    HINTERNET hSession = NULL,
            hConnect = NULL,
            hRequest = NULL;
    std::string extractedVersion;

    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(L"WinHTTP SESSIONKILLER",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect(hSession, L"api.j3rzy.dev",
                                  INTERNET_DEFAULT_HTTP_PORT, 0);

    // Create an HTTP request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/versions/processkiller",
                                      NULL, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES,
                                      0);

    // Send a request.
    if (hRequest)
        bResults = WinHttpSendRequest(hRequest,
                                      WINHTTP_NO_ADDITIONAL_HEADERS,
                                      0, WINHTTP_NO_REQUEST_DATA, 0,
                                      0, 0);

    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);

    std::string version;

    // Keep checking for data until there is nothing left.
    if (bResults) {
        do {
            // Check for available data.
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                printf("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());

            // Allocate space for the buffer.
            pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer)
            {
                printf("Out of memory\n");
                dwSize = 0;
            }
            else
            {
                // Read the Data.
                ZeroMemory(pszOutBuffer, dwSize + 1);

                if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
                                     dwSize, &dwDownloaded))
                    printf("Error %u in WinHttpReadData.\n", GetLastError());
                else
                    version += pszOutBuffer;

                // Free the memory allocated to the buffer.
                delete[] pszOutBuffer;
            }

        } while (dwSize > 0);

        // Extract version information from the response
        size_t startPos = version.find("\"version\":\"");
        if (startPos != std::string::npos) {
            startPos += 11; // length of "\"version\":\""
            size_t endPos = version.find("\"", startPos);
            if (endPos != std::string::npos) {
                extractedVersion = version.substr(startPos, endPos - startPos);
            }
        }
    }

    // Report any errors.
    if (!bResults)
        printf("Error %d has occurred.\n", GetLastError());

    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    /* ----- Request (end) ------ */

    return extractedVersion;
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
        // seperator
        for (int i = 0; i < terminalWidth; ++i) {
            std::cout << "-";
        }
        std::cout << std::endl << std::endl;
        // end

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
    for (const std::string &process: processesToKill) {
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
    for (const std::string &process: processesToKill) {
        if (!process.empty()) std::cout << "- " << process << '\n';
    }

    // seperator
    for (int i = 0; i < terminalWidth; ++i) {
        std::cout << "-";
    }
    std::cout << std::endl << std::endl;
    // end

    std::string latestRelease = getLatestVersion();
    std::string latestReleaseTag = latestRelease;

    if (!latestRelease.empty()) {
        latestRelease.erase(std::remove(latestRelease.begin(), latestRelease.end(), '.'), latestRelease.end());
        VERSION.erase(std::remove(VERSION.begin(), VERSION.end(), '.'), VERSION.end());
        int latestVersion = std::stoi(latestRelease, nullptr, 10);
        int currentVersion = std::stoi(VERSION, nullptr, 10);

        if (currentVersion == latestVersion) {
            SetConsoleTextAttribute(hConsole, 2);
            std::cout << "You are running the latest release!" << std::endl;
            SetConsoleTextAttribute(hConsole, 3);
        } else if (currentVersion > latestVersion) {
            SetConsoleTextAttribute(hConsole, 6);
            std::cout << "You are running a newer version than is released. Somehow" << std::endl;
            SetConsoleTextAttribute(hConsole, 3);
        } else {
            SetConsoleTextAttribute(hConsole, 4);
            std::cout << "You are running an outdated version of the program. Download the latest release at: \nhttps://github.com/JamJestJerzy/ProcessKiller/releases/tag/" << latestReleaseTag << std::endl;
            SetConsoleTextAttribute(hConsole, 3);
        }
    } else {
        std::cout << "Failed to check version." << std::endl;
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