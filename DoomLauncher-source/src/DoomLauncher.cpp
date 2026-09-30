#include <windows.h>
#include <commdlg.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

// Minimal subset of the stable Notepad++ plug-in ABI used by this plug-in.
struct NppData {
    HWND _nppHandle;
    HWND _scintillaMainHandle;
    HWND _scintillaSecondHandle;
};

using PFUNCPLUGINCMD = void(__cdecl*)();

struct ShortcutKey {
    bool _isCtrl;
    bool _isAlt;
    bool _isShift;
    UCHAR _key;
};

struct FuncItem {
    wchar_t _itemName[64];
    PFUNCPLUGINCMD _pFunc;
    int _cmdID;
    bool _init2Check;
    ShortcutKey* _pShKey;
};

struct SCNotification;

namespace {
constexpr UINT NPPMSG = WM_USER + 1000;
constexpr UINT NPPM_GETPLUGINSCONFIGDIR = NPPMSG + 46;
constexpr wchar_t kPluginName[] = L"Doom Launcher";
constexpr wchar_t kIniName[] = L"DoomLauncher.ini";
constexpr wchar_t kIniSection[] = L"DoomLauncher";
constexpr wchar_t kEngineKey[] = L"EnginePath";

HINSTANCE g_instance = nullptr;
NppData g_npp{};

void launchDoom();
void chooseEngine();
void showAbout();

std::array<FuncItem, 3> g_commands{};

void showError(const std::wstring& message) {
    MessageBoxW(g_npp._nppHandle, message.c_str(), kPluginName, MB_OK | MB_ICONERROR);
}

std::wstring selectFile(const wchar_t* title, const wchar_t* filter) {
    std::array<wchar_t, 32768> path{};
    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = g_npp._nppHandle;
    dialog.lpstrFilter = filter;
    dialog.lpstrFile = path.data();
    dialog.nMaxFile = static_cast<DWORD>(path.size());
    dialog.lpstrTitle = title;
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetOpenFileNameW(&dialog) ? std::wstring(path.data()) : std::wstring{};
}

fs::path configPath() {
    const LRESULT length = SendMessageW(g_npp._nppHandle, NPPM_GETPLUGINSCONFIGDIR, 0, 0);
    if (length > 0) {
        std::vector<wchar_t> buffer(static_cast<size_t>(length) + 1);
        if (SendMessageW(g_npp._nppHandle, NPPM_GETPLUGINSCONFIGDIR,
                         static_cast<WPARAM>(buffer.size()),
                         reinterpret_cast<LPARAM>(buffer.data()))) {
            return fs::path(buffer.data()) / kIniName;
        }
    }

    std::array<wchar_t, MAX_PATH> appData{};
    const DWORD count = GetEnvironmentVariableW(L"APPDATA", appData.data(),
                                                 static_cast<DWORD>(appData.size()));
    if (count > 0 && count < appData.size()) {
        return fs::path(appData.data()) / L"Notepad++" / L"plugins" / L"Config" / kIniName;
    }
    return fs::temp_directory_path() / kIniName;
}

void saveEngine(const fs::path& engine) {
    const fs::path ini = configPath();
    std::error_code error;
    fs::create_directories(ini.parent_path(), error);
    if (!WritePrivateProfileStringW(kIniSection, kEngineKey, engine.c_str(), ini.c_str())) {
        showError(L"Не удалось сохранить путь к движку Doom.");
    }
}

fs::path loadEngine() {
    std::array<wchar_t, 32768> value{};
    GetPrivateProfileStringW(kIniSection, kEngineKey, L"", value.data(),
                             static_cast<DWORD>(value.size()), configPath().c_str());
    return fs::path(value.data());
}

fs::path moduleDirectory() {
    std::array<wchar_t, 32768> path{};
    const DWORD length = GetModuleFileNameW(g_instance, path.data(),
                                            static_cast<DWORD>(path.size()));
    return length > 0 && length < path.size() ? fs::path(path.data()).parent_path() : fs::path{};
}

fs::path discoverEngine() {
    const fs::path saved = loadEngine();
    if (!saved.empty() && fs::is_regular_file(saved)) {
        return saved;
    }

    const fs::path folder = moduleDirectory();
    constexpr std::array candidates{L"gzdoom.exe", L"chocolate-doom.exe", L"crispy-doom.exe",
                                    L"dsda-doom.exe", L"woof.exe"};
    for (const wchar_t* name : candidates) {
        const fs::path candidate = folder / name;
        if (fs::is_regular_file(candidate)) {
            saveEngine(candidate);
            return candidate;
        }
    }
    return {};
}

enum class WadKind { IWad, PWad, Invalid };

WadKind inspectWad(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    std::array<char, 4> signature{};
    if (!stream.read(signature.data(), signature.size())) {
        return WadKind::Invalid;
    }
    const std::string_view value(signature.data(), signature.size());
    if (value == "IWAD") return WadKind::IWad;
    if (value == "PWAD") return WadKind::PWad;
    return WadKind::Invalid;
}

std::wstring quoteArgument(const std::wstring& value) {
    std::wstring quoted = L"\"";
    size_t slashes = 0;
    for (const wchar_t ch : value) {
        if (ch == L'\\') {
            ++slashes;
        } else if (ch == L'\"') {
            quoted.append(slashes * 2 + 1, L'\\');
            quoted.push_back(ch);
            slashes = 0;
        } else {
            quoted.append(slashes, L'\\');
            slashes = 0;
            quoted.push_back(ch);
        }
    }
    quoted.append(slashes * 2, L'\\');
    quoted.push_back(L'\"');
    return quoted;
}

bool startEngine(const fs::path& engine, const fs::path& wad) {
    std::wstring command = quoteArgument(engine.wstring()) + L" -iwad " + quoteArgument(wad.wstring());
    std::vector<wchar_t> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back(L'\0');

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    const std::wstring workingDirectory = engine.parent_path().wstring();
    const BOOL started = CreateProcessW(engine.c_str(), mutableCommand.data(), nullptr, nullptr, FALSE,
                                        CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP,
                                        nullptr, workingDirectory.c_str(), &startup, &process);
    if (!started) {
        const DWORD code = GetLastError();
        showError(L"Не удалось запустить движок Doom. Код ошибки Windows: " + std::to_wstring(code));
        return false;
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return true;
}

void chooseEngine() {
    const std::wstring selected = selectFile(
        L"Выберите Doom-совместимый движок",
        L"Doom engines (*.exe)\0*.exe\0Все файлы (*.*)\0*.*\0\0");
    if (!selected.empty()) {
        saveEngine(selected);
        MessageBoxW(g_npp._nppHandle, L"Путь к движку сохранён.", kPluginName,
                    MB_OK | MB_ICONINFORMATION);
    }
}

void launchDoom() {
    fs::path engine = discoverEngine();
    if (engine.empty()) {
        const std::wstring selected = selectFile(
            L"Сначала выберите Doom-совместимый движок",
            L"Doom engines (*.exe)\0*.exe\0Все файлы (*.*)\0*.*\0\0");
        if (selected.empty()) return;
        engine = selected;
        saveEngine(engine);
    }

    const std::wstring selectedWad = selectFile(
        L"Выберите WAD игры Doom",
        L"Doom WAD (*.wad)\0*.wad\0Все файлы (*.*)\0*.*\0\0");
    if (selectedWad.empty()) return;

    const fs::path wad(selectedWad);
    switch (inspectWad(wad)) {
        case WadKind::IWad:
            startEngine(engine, wad);
            break;
        case WadKind::PWad:
            showError(L"Выбран PWAD — это мод, а не самостоятельная игра. Выберите базовый IWAD, "
                      L"например DOOM.WAD или DOOM1.WAD.");
            break;
        case WadKind::Invalid:
            showError(L"Файл не является корректным Doom WAD: отсутствует сигнатура IWAD/PWAD.");
            break;
    }
}

void showAbout() {
    MessageBoxW(g_npp._nppHandle,
        L"Doom Launcher 1.0\n\nЗапускает выбранный IWAD через установленный "
        L"Doom-совместимый движок в отдельном графическом окне.\n\n"
        L"Плагин не содержит Doom, WAD-файлы или игровой движок.",
        kPluginName, MB_OK | MB_ICONINFORMATION);
}

void setCommand(size_t index, const wchar_t* name, PFUNCPLUGINCMD action) {
    wcscpy_s(g_commands[index]._itemName, name);
    g_commands[index]._pFunc = action;
    g_commands[index]._cmdID = 0;
    g_commands[index]._init2Check = false;
    g_commands[index]._pShKey = nullptr;
}
} // namespace

BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_instance = instance;
        DisableThreadLibraryCalls(instance);
    }
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }

extern "C" __declspec(dllexport) void setInfo(NppData data) {
    g_npp = data;
    setCommand(0, L"Запустить Doom...", launchDoom);
    setCommand(1, L"Выбрать движок...", chooseEngine);
    setCommand(2, L"О плагине", showAbout);
}

extern "C" __declspec(dllexport) const wchar_t* getName() { return kPluginName; }

extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* count) {
    *count = static_cast<int>(g_commands.size());
    return g_commands.data();
}

extern "C" __declspec(dllexport) void beNotified(SCNotification*) {}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT, WPARAM, LPARAM) { return TRUE; }

