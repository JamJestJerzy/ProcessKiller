// ProcessFunctions.h
#ifndef PROCESS_FUNCTIONS_H
#define PROCESS_FUNCTIONS_H

#include <string>
#include <windows.h>

// Function declarations
void TerminateProcessByFileName(std::basic_string<char> fileName);
std::wstring GetTopLevelParentProcessName(DWORD processId);
std::wstring GetParentProcessName(DWORD processId);
HWND GetProcessWindow(DWORD processId);
HWND GetParentProcessById(DWORD processId);
HWND GetParentProcess(HWND hwnd);
void EnumerateProcesses(std::string (&processesToKill)[25]);
HANDLE getHConsole();

#endif