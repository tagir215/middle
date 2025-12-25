#include <windows.h>
#include <shellapi.h>
#include "script_opener.h"
#include <filesystem>

void shell_open_file(const std::string& path) {
    namespace fs = std::filesystem;
    fs::path absPath = fs::absolute(path);

    if (!std::filesystem::exists(path)) {
        MessageBoxA(nullptr, "File does not exist", "Debug", MB_OK);
        return;
    }

    if (!fs::exists(absPath)) {
        MessageBoxA(nullptr, "File does not exist", "Debug", MB_OK);
        return;
    }

    HINSTANCE result = ShellExecuteA(
        nullptr,
        "open",
        absPath.string().c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );


    if ((INT_PTR)result <= 32) {
        MessageBoxA(
            nullptr,
            "ShellExecute failed",
            "Error",
            MB_OK | MB_ICONERROR
        );
    }
}
