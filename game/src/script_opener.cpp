#include <windows.h>
#include <shellapi.h>
#include "script_opener.h"
#include <filesystem>

void shell_open_file(const std::string &path)
{
    namespace fs = std::filesystem;
    fs::path absPath = fs::absolute(path);

    if (!std::filesystem::exists(path))
    {
        MessageBoxA(nullptr, "File does not exist", "Debug", MB_OK);
        return;
    }

    if (!fs::exists(absPath))
    {
        MessageBoxA(nullptr, "File does not exist", "Debug", MB_OK);
        return;
    }

    // visual studio
    HINSTANCE result = ShellExecuteA(
        nullptr,
        "open",
        absPath.string().c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL);

    if ((INT_PTR)result <= 32)
    {
        MessageBoxA(
            nullptr,
            "ShellExecute failed",
            "Error",
            MB_OK | MB_ICONERROR);
    }
}

void shell_open_file_vscode(const std::string &path)
{
    namespace fs = std::filesystem;
    fs::path absPath = fs::absolute(path);

    // 1. Get the path to Code.exe (use your actual path)
    std::string exePath = "C:\\Users\\tagir\\AppData\\Local\\Programs\\Microsoft VS Code\\Code.exe";

    // 2. Build the command line: "exe" --reuse-window "file"
    // Note: The first argument must be the executable name itself.
    std::string commandLine = "\"" + exePath + "\" --reuse-window \"" + absPath.string() + "\"";

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // CreateProcess is the lowest-level way to launch an app on Windows
    if (CreateProcessA(
        exePath.c_str(), 
        &commandLine[0], // Must be a modifiable buffer
        nullptr, nullptr, FALSE, 
        0, nullptr, nullptr, 
        &si, &pi)) 
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}
