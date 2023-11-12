// Jerzy "LifeOverflow" W © All Rights Reserved 2023-2024

// Includes
#include "ProcessFunctions.h"
#include "LogoFunctions.h"
#include "VersionCtrl.h"

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cctype>
#include <locale>
#include <codecvt>
#include <conio.h>
#include <algorithm>
#include <vector>
#include <thread>

// Current version string
std::string VERSION = "0.9.1";

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

            if (processName == "windowsterminal.exe" || processName == "powershell.exe" || processName == "cmd.exe") {
                SetConsoleTextAttribute(getHConsole(), 14);
                std::cout << "Process is terminal. Checking if it got spawned by unwanted process." << std::endl;
                SetConsoleTextAttribute(getHConsole(), 3);
                //EnumerateProcesses(processesToKill);
                GetAllProcesses(processesToKill);
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

    while (true) {
        GetAllProcesses(processesToKill);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}