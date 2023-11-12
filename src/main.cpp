// Jerzy "LifeOverflow" W © All Rights Reserved 2023-2024

// Includes
#include "ProcessFunctions.h"
#include "LogoFunctions.h"

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

// Current version string
std::string VERSION = "0.7.5";

// Initializes empty array
std::string processesToKill[25] = {};

// Namespaces or smth
using std::vector;
using std::cout;
using std::endl;

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
            SetConsoleTextAttribute(getHConsole(), 9);
            std::wcout << L"New process spawned: " << filePath.filename().wstring() << L" (" << filePath.wstring()
                       << L")" << L" PID: " << (int) processId << std::endl;
            SetConsoleTextAttribute(getHConsole(), 3);

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
                SetConsoleTextAttribute(getHConsole(), 14);
                std::cout << "Process is terminal. Checking if it got spawned by unwanted process." << std::endl;
                SetConsoleTextAttribute(getHConsole(), 3);
                EnumerateProcesses(processesToKill);
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
                    SetConsoleTextAttribute(getHConsole(), 13);
                    std::cout << processName << " should be terminated by now. Enjoy ;)" << std::endl;
                    SetConsoleTextAttribute(getHConsole(), 3);
                }
            }
        }
    }
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
    SetConsoleTextAttribute(getHConsole(), 6);
    int terminalWidth = getTerminalWidth();
    std::cout << "Terminal Width: " << terminalWidth << " columns" << std::endl;
    SetConsoleTextAttribute(getHConsole(), 5);

    if (terminalWidth >= 149) bigLogo();
    else if (terminalWidth >= 85) mediumLogo();
    else smallLogo();

    SetConsoleTextAttribute(getHConsole(), 3); // Reset color to default
    // Reading properties from file
    const std::string fileName = "blocked.properties";
    std::ifstream inputFile(fileName, std::ios::in | std::ios::binary);

    if (!inputFile.is_open()) {
        SetConsoleTextAttribute(getHConsole(), 12);
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
        SetConsoleTextAttribute(getHConsole(), 7);
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
        SetConsoleTextAttribute(getHConsole(), 12);
        std::cout << "You need to write program binary names in the .properties file!\n";
        std::cout << "Press ANY key to exit...\n";
        _getch();
        SetConsoleTextAttribute(getHConsole(), 7);
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
            SetConsoleTextAttribute(getHConsole(), 2);
            std::cout << "You are running the latest release!" << std::endl;
            SetConsoleTextAttribute(getHConsole(), 3);
        } else if (currentVersion > latestVersion) {
            SetConsoleTextAttribute(getHConsole(), 6);
            std::cout << "You are running a newer version than is released. Somehow" << std::endl;
            SetConsoleTextAttribute(getHConsole(), 3);
        } else {
            SetConsoleTextAttribute(getHConsole(), 4);
            std::cout << "You are running an outdated version of the program. Download the latest release at: \nhttps://github.com/JamJestJerzy/ProcessKiller/releases/tag/" << latestReleaseTag << std::endl;
            SetConsoleTextAttribute(getHConsole(), 3);
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
        SetConsoleTextAttribute(getHConsole(), 12);
        std::cerr << "SetWinEventHook failed.\n";
        SetConsoleTextAttribute(getHConsole(), 7);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWinEvent(nullptr);
    SetConsoleTextAttribute(getHConsole(), 7);
    return 0;
}