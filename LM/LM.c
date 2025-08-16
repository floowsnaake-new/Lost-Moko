#include <windows.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "comctl32.lib")

#define ID_LISTVIEW 1001
#define ID_BUTTON_SCAN 1002
#define ID_BUTTON_RESCAN 1003
#define ID_BUTTON_ATTACH 1004
#define ID_EDIT_VALUE 1005
#define ID_CHECK_DWORD 1006
#define ID_BUTTON_CLEAR 1007
#define ID_STATIC_COUNT 1008
#define ID_EDIT_NEW_VALUE 2000
#define ID_BUTTON_WRITE 2001
#define ID_TABCONTROL 3000
#define ID_BUTTON_STARTBOT 3001
#define ID_BUTTON_STOPBOT 3002
#define ID_TARGETING_STATIC 4001
#define ID_MULTIBOX_STATIC 5001
#define ID_PROFILES_STATIC 6001
#define ID_SETTINGS_STATIC 7001
#define ID_BUTTON_COPY 1009
#define ID_BUTTON_SAVE 1010

HWND hwndListView, hwndEditValue, hwndScanButton, hwndRescanButton, hwndAttachButton;
HWND hwndCheckDword, hwndButtonClear, hwndStaticCount;
HWND hwndEditNewValue, hwndButtonWriteValue;
HWND hwndTabControl;
HWND hwndButtonStartBot, hwndButtonStopBot;
HWND hwndTargetingStatic, hwndMultiboxStatic, hwndProfilesStatic, hwndSettingsStatic;
HWND hwndButtonCopy, hwndButtonSave;

HANDLE hProcess = NULL;
unsigned char* foundResults[10000];
size_t foundCount = 0;
size_t valueSize = 2;

// Function to get process ID by name (case-insensitive)
DWORD GetProcessIdByName(const char* processName) {
    PROCESSENTRY32 pe = {0};
    pe.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return 0;

    if (Process32First(hSnap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, processName) == 0) {
                CloseHandle(hSnap);
                return pe.th32ProcessID;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return 0;
}

// Function to open process
HANDLE OpenTargetProcess(DWORD pid) {
    return OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
}

// Function to scan process memory for 2 or 4-byte values (optimized)
void ScanMemoryForValue(UINT64 targetValue, unsigned char* results[], size_t* count, size_t valSize) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    MEMORY_BASIC_INFORMATION mbi;
    *count = 0;

    unsigned char* addr = (unsigned char*)si.lpMinimumApplicationAddress;
    unsigned char* maxAddr = (unsigned char*)si.lpMaximumApplicationAddress;

    unsigned char chunk[64 * 1024];
    SIZE_T chunkSize = sizeof(chunk);

    while (addr < maxAddr) {
        if (VirtualQueryEx(hProcess, addr, &mbi, sizeof(mbi)) == 0) {
            addr += 4096;
            continue;
        }

        if (mbi.State != MEM_COMMIT ||
            (mbi.Protect & PAGE_GUARD) ||
            (mbi.Protect & PAGE_NOACCESS) ||
            !(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) {
            addr = (unsigned char*)mbi.BaseAddress + mbi.RegionSize;
            continue;
        }

        unsigned char* regionBase = (unsigned char*)mbi.BaseAddress;
        SIZE_T regionSize = mbi.RegionSize;

        for (SIZE_T offset = 0; offset < regionSize; ) {
            SIZE_T toRead = (regionSize - offset > chunkSize) ? chunkSize : (regionSize - offset);
            SIZE_T bytesRead = 0;

            if (!ReadProcessMemory(hProcess, regionBase + offset, chunk, toRead, &bytesRead) || bytesRead == 0) {
                offset += 4096;
                continue;
            }

            SIZE_T alignMask = valSize - 1;
            SIZE_T start = (offset & alignMask);
            SIZE_T alignedStart = (start + alignMask) & ~alignMask;
            if (alignedStart >= bytesRead) {
                offset += toRead;
                continue;
            }

            for (SIZE_T i = alignedStart; i <= (bytesRead - valSize); i += valSize) {
                UINT64 value = 0;
                if (valSize == 2) {
                    value = *(WORD*)(chunk + i);
                } else if (valSize == 4) {
                    value = *(DWORD*)(chunk + i);
                } else {
                    continue;
                }

                if (value == targetValue) {
                    unsigned char* foundAddr = regionBase + offset + i;
                    if (*count < 10000) {
                        results[*count] = foundAddr;
                        (*count)++;
                    } else {
                        CloseHandle(hProcess);
                        hProcess = OpenTargetProcess(GetProcessIdByName("LOSTARK.exe"));
                        return;
                    }
                }
            }

            offset += toRead - (valSize - 1);
        }

        addr = regionBase + regionSize;
    }
}

// Helper to clear ListView
void ClearListView(HWND listView) {
    ListView_DeleteAllItems(listView);
}

// Helper to update the "results count" label
void UpdateResultsCountLabel(HWND hwndStatic, size_t totalFound, size_t displayed) {
    char buf[100];
    if (totalFound > displayed) {
        sprintf(buf, "Showing first %zu out of %zu", displayed, totalFound);
    } else {
        sprintf(buf, "Found %zu addresses", totalFound);
    }
    SetWindowText(hwndStatic, buf);
}

// Helper to populate ListView with max 100 items
void PopulateListView(HWND listView, unsigned char* results[], size_t count) {
    ClearListView(listView);

    size_t displayCount = (count < 100) ? count : 100;

    for (size_t i = 0; i < displayCount; i++) {
        char addrStr[32];
        sprintf(addrStr, "%p", (void*)results[i]);

        LVITEM lvItem = {0};
        lvItem.iItem = (int)i;
        lvItem.mask = LVIF_TEXT;
        lvItem.pszText = addrStr;
        lvItem.iSubItem = 0;
        ListView_InsertItem(listView, &lvItem);

        // Read and display the current value
        UINT64 value = 0;
        SIZE_T bytesRead = 0;
        if (ReadProcessMemory(hProcess, results[i], &value, valueSize, &bytesRead) && bytesRead == valueSize) {
            char valStr[32];
            if (valueSize == 2) {
                sprintf(valStr, "%u", (WORD)value);
            } else {
                sprintf(valStr, "%u", (DWORD)value);
            }
            ListView_SetItemText(listView, (int)i, 1, valStr);
        }
    }

    UpdateResultsCountLabel(hwndStaticCount, count, displayCount);
}

// Attach process function
void AttachLostArk() {
    if (hProcess) {
        CloseHandle(hProcess);
        hProcess = NULL;
    }

    DWORD pid = GetProcessIdByName("LOSTARK.exe");
    if (pid == 0) {
        MessageBox(NULL, "LOSTARK.exe not found!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    hProcess = OpenTargetProcess(pid);
    if (hProcess == NULL) {
        MessageBox(NULL, "Failed to open Lost ARK process!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    MessageBox(NULL, "Successfully attached to Lost ARK!", "Success", MB_OK | MB_ICONINFORMATION);
}

// Copy selected address to clipboard
void CopyAddressToClipboard(HWND hwnd) {
    int selected = ListView_GetNextItem(hwndListView, -1, LVNI_SELECTED);
    if (selected == -1) {
        MessageBox(hwnd, "No address selected!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    char addrStr[32];
    sprintf(addrStr, "%p", (void*)foundResults[selected]);

    if (OpenClipboard(hwnd)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(addrStr) + 1);
        if (hMem) {
            strcpy((char*)GlobalLock(hMem), addrStr);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
            CloseClipboard();
            MessageBox(hwnd, "Address copied to clipboard!", "Success", MB_OK | MB_ICONINFORMATION);
        }
    }
}

// Save selected address to a text file
void SaveAddressToFile(HWND hwnd) {
    int selected = ListView_GetNextItem(hwndListView, -1, LVNI_SELECTED);
    if (selected == -1) {
        MessageBox(hwnd, "No address selected!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    char addrStr[32];
    sprintf(addrStr, "%p", (void*)foundResults[selected]);

    OPENFILENAME ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        FILE* f = fopen(szFile, "w");
        if (f) {
            fprintf(f, "%s\n", addrStr);
            fclose(f);
            MessageBox(hwnd, "Address saved to file!", "Success", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(hwnd, "Failed to save file!", "Error", MB_OK | MB_ICONERROR);
        }
    }
}

// Main window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static UINT64 targetValue = 0;

    switch (msg) {
        case WM_CREATE: {
            hwndTabControl = CreateWindow(WC_TABCONTROL, NULL,
                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                5, 5, 600, 400,
                hwnd, (HMENU)ID_TABCONTROL, NULL, NULL);

            TCITEM tie = {0};
            tie.mask = TCIF_TEXT;

            tie.pszText = "Mem Logic";
            TabCtrl_InsertItem(hwndTabControl, 0, &tie);

            tie.pszText = "Pathing";
            TabCtrl_InsertItem(hwndTabControl, 1, &tie);

            tie.pszText = "Targeting";
            TabCtrl_InsertItem(hwndTabControl, 2, &tie);

            tie.pszText = "Multibox";
            TabCtrl_InsertItem(hwndTabControl, 3, &tie);

            tie.pszText = "Profiles";
            TabCtrl_InsertItem(hwndTabControl, 4, &tie);

            tie.pszText = "Settings";
            TabCtrl_InsertItem(hwndTabControl, 5, &tie);

            // Scanner Tab Controls
            hwndEditValue = CreateWindow("EDIT", "0",
                WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                10, 40, 100, 25,
                hwnd, (HMENU)ID_EDIT_VALUE, NULL, NULL);

            hwndScanButton = CreateWindow("BUTTON", "Scan",
                WS_CHILD,
                120, 40, 80, 25,
                hwnd, (HMENU)ID_BUTTON_SCAN, NULL, NULL);

            hwndRescanButton = CreateWindow("BUTTON", "Rescan",
                WS_CHILD,
                210, 40, 80, 25,
                hwnd, (HMENU)ID_BUTTON_RESCAN, NULL, NULL);

            hwndAttachButton = CreateWindow("BUTTON", "Attach",
                WS_CHILD,
                300, 40, 120, 25,
                hwnd, (HMENU)ID_BUTTON_ATTACH, NULL, NULL);

            hwndCheckDword = CreateWindow("BUTTON", "Search 4 Bytes",
                WS_CHILD | BS_AUTOCHECKBOX,
                430, 40, 150, 25,
                hwnd, (HMENU)ID_CHECK_DWORD, NULL, NULL);

            hwndButtonClear = CreateWindow("BUTTON", "Clear Results",
                WS_CHILD,
                10, 75, 120, 25,
                hwnd, (HMENU)ID_BUTTON_CLEAR, NULL, NULL);

            hwndStaticCount = CreateWindow("STATIC", "Found 0 addresses.",
                WS_CHILD,
                150, 75, 300, 25,
                hwnd, (HMENU)ID_STATIC_COUNT, NULL, NULL);

            hwndListView = CreateWindow(WC_LISTVIEW, NULL,
                WS_CHILD | LVS_REPORT | LVS_SINGLESEL | WS_BORDER,
                10, 105, 580, 220,
                hwnd, (HMENU)ID_LISTVIEW, NULL, NULL);

            {
                LVCOLUMN lvCol = {0};
                lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
                lvCol.cx = 300;
                lvCol.pszText = "Address";
                ListView_InsertColumn(hwndListView, 0, &lvCol);

                lvCol.cx = 280;
                lvCol.pszText = "Value";
                ListView_InsertColumn(hwndListView, 1, &lvCol);
            }

            CreateWindow("STATIC", "New Value:",
                WS_CHILD,
                10, 335, 70, 20,
                hwnd, NULL, NULL, NULL);

            hwndEditNewValue = CreateWindow("EDIT", "0",
                WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                80, 330, 100, 25,
                hwnd, (HMENU)ID_EDIT_NEW_VALUE, NULL, NULL);

            hwndButtonWriteValue = CreateWindow("BUTTON", "Write Value",
                WS_CHILD,
                190, 330, 100, 25,
                hwnd, (HMENU)ID_BUTTON_WRITE, NULL, NULL);

            hwndButtonCopy = CreateWindow("BUTTON", "Copy Address",
                WS_CHILD,
                300, 330, 100, 25,
                hwnd, (HMENU)ID_BUTTON_COPY, NULL, NULL);

            hwndButtonSave = CreateWindow("BUTTON", "Save Address",
                WS_CHILD,
                410, 330, 100, 25,
                hwnd, (HMENU)ID_BUTTON_SAVE, NULL, NULL);

            // Game Bot Tab Controls
            hwndButtonStartBot = CreateWindow("BUTTON", "Start Bot",
                WS_CHILD,
                10, 40, 100, 30,
                hwnd, (HMENU)ID_BUTTON_STARTBOT, NULL, NULL);

            hwndButtonStopBot = CreateWindow("BUTTON", "Stop Bot",
                WS_CHILD,
                120, 40, 100, 30,
                hwnd, (HMENU)ID_BUTTON_STOPBOT, NULL, NULL);

            hwndTargetingStatic = CreateWindow("STATIC", "Targeting Tab Content",
                WS_CHILD, 10, 40, 200, 25, hwnd, (HMENU)ID_TARGETING_STATIC, NULL, NULL);

            hwndMultiboxStatic = CreateWindow("STATIC", "Multibox Tab Content",
                WS_CHILD, 10, 40, 200, 25, hwnd, (HMENU)ID_MULTIBOX_STATIC, NULL, NULL);

            hwndProfilesStatic = CreateWindow("STATIC", "Profiles Tab Content",
                WS_CHILD, 10, 40, 200, 25, hwnd, (HMENU)ID_PROFILES_STATIC, NULL, NULL);

            hwndSettingsStatic = CreateWindow("STATIC", "Settings Tab Content",
                WS_CHILD, 10, 40, 200, 25, hwnd, (HMENU)ID_SETTINGS_STATIC, NULL, NULL);

            ShowWindow(hwndEditValue, SW_SHOW);
            ShowWindow(hwndScanButton, SW_SHOW);
            ShowWindow(hwndRescanButton, SW_SHOW);
            ShowWindow(hwndAttachButton, SW_SHOW);
            ShowWindow(hwndCheckDword, SW_SHOW);
            ShowWindow(hwndButtonClear, SW_SHOW);
            ShowWindow(hwndStaticCount, SW_SHOW);
            ShowWindow(hwndListView, SW_SHOW);
            ShowWindow(hwndEditNewValue, SW_SHOW);
            ShowWindow(hwndButtonWriteValue, SW_SHOW);
            ShowWindow(hwndButtonCopy, SW_SHOW);
            ShowWindow(hwndButtonSave, SW_SHOW);

            ShowWindow(hwndButtonStartBot, SW_HIDE);
            ShowWindow(hwndButtonStopBot, SW_HIDE);
            ShowWindow(hwndTargetingStatic, SW_HIDE);
            ShowWindow(hwndMultiboxStatic, SW_HIDE);
            ShowWindow(hwndProfilesStatic, SW_HIDE);
            ShowWindow(hwndSettingsStatic, SW_HIDE);

            MessageBox(hwnd, "Work in progress! Working with the latest Lost Ark patch", "Lost Moko", MB_OK | MB_ICONINFORMATION);
            break;
        }

        case WM_NOTIFY: {
            if (((LPNMHDR)lParam)->hwndFrom == hwndTabControl) {
                if (((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
                    int sel = TabCtrl_GetCurSel(hwndTabControl);

                    ShowWindow(hwndEditValue, SW_HIDE);
                    ShowWindow(hwndScanButton, SW_HIDE);
                    ShowWindow(hwndRescanButton, SW_HIDE);
                    ShowWindow(hwndAttachButton, SW_HIDE);
                    ShowWindow(hwndCheckDword, SW_HIDE);
                    ShowWindow(hwndButtonClear, SW_HIDE);
                    ShowWindow(hwndStaticCount, SW_HIDE);
                    ShowWindow(hwndListView, SW_HIDE);
                    ShowWindow(hwndEditNewValue, SW_HIDE);
                    ShowWindow(hwndButtonWriteValue, SW_HIDE);
                    ShowWindow(hwndButtonCopy, SW_HIDE);
                    ShowWindow(hwndButtonSave, SW_HIDE);
                    ShowWindow(hwndButtonStartBot, SW_HIDE);
                    ShowWindow(hwndButtonStopBot, SW_HIDE);
                    ShowWindow(hwndTargetingStatic, SW_HIDE);
                    ShowWindow(hwndMultiboxStatic, SW_HIDE);
                    ShowWindow(hwndProfilesStatic, SW_HIDE);
                    ShowWindow(hwndSettingsStatic, SW_HIDE);

                    switch(sel) {
                        case 0:
                            ShowWindow(hwndEditValue, SW_SHOW);
                            ShowWindow(hwndScanButton, SW_SHOW);
                            ShowWindow(hwndRescanButton, SW_SHOW);
                            ShowWindow(hwndAttachButton, SW_SHOW);
                            ShowWindow(hwndCheckDword, SW_SHOW);
                            ShowWindow(hwndButtonClear, SW_SHOW);
                            ShowWindow(hwndStaticCount, SW_SHOW);
                            ShowWindow(hwndListView, SW_SHOW);
                            ShowWindow(hwndEditNewValue, SW_SHOW);
                            ShowWindow(hwndButtonWriteValue, SW_SHOW);
                            ShowWindow(hwndButtonCopy, SW_SHOW);
                            ShowWindow(hwndButtonSave, SW_SHOW);
                            break;
                        case 1:
                            ShowWindow(hwndButtonStartBot, SW_SHOW);
                            ShowWindow(hwndButtonStopBot, SW_SHOW);
                            break;
                        case 2:
                            ShowWindow(hwndTargetingStatic, SW_SHOW);
                            break;
                        case 3:
                            ShowWindow(hwndMultiboxStatic, SW_SHOW);
                            break;
                        case 4:
                            ShowWindow(hwndProfilesStatic, SW_SHOW);
                            break;
                        case 5:
                            ShowWindow(hwndSettingsStatic, SW_SHOW);
                            break;
                    }
                }
            }
            break;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BUTTON_SCAN: {
                    char buf[32];
                    GetWindowText(hwndEditValue, buf, sizeof(buf));
                    valueSize = (IsDlgButtonChecked(hwnd, ID_CHECK_DWORD) == BST_CHECKED) ? 4 : 2;

                    if (valueSize == 4) {
                        targetValue = strtoul(buf, NULL, 0);
                    } else {
                        targetValue = (WORD)strtoul(buf, NULL, 0);
                    }

                    if (hProcess == NULL) {
                        DWORD pid = GetProcessIdByName("LOSTARK.exe");
                        if (pid == 0) {
                            MessageBox(hwnd, "LOSTARK.exe not found!", "Error", MB_OK);
                            break;
                        }
                        hProcess = OpenTargetProcess(pid);
                        if (!hProcess) {
                            MessageBox(hwnd, "Failed to open process!", "Error", MB_OK);
                            break;
                        }
                    }

                    size_t count = 0;
                    ScanMemoryForValue(targetValue, foundResults, &count, valueSize);
                    foundCount = count;
                    PopulateListView(hwndListView, foundResults, foundCount);
                    break;
                }

                case ID_BUTTON_RESCAN: {
                    if (hProcess == NULL || foundCount == 0) break;

                    char buf[32];
                    GetWindowText(hwndEditValue, buf, sizeof(buf));
                    UINT64 newTargetValue = 0;
                    if (valueSize == 4) {
                        newTargetValue = strtoul(buf, NULL, 0);
                    } else {
                        newTargetValue = (WORD)strtoul(buf, NULL, 0);
                    }

                    size_t newCount = 0;
                    unsigned char* newResults[10000];

                    for (size_t i = 0; i < foundCount; i++) {
                        UINT64 value = 0;
                        SIZE_T bytesRead = 0;
                        if (ReadProcessMemory(hProcess, foundResults[i], &value, valueSize, &bytesRead) && bytesRead == valueSize) {
                            if ((valueSize == 2 && (WORD)value == (WORD)newTargetValue) ||
                                (valueSize == 4 && (DWORD)value == (DWORD)newTargetValue)) {
                                if (newCount < 10000) {
                                    newResults[newCount++] = foundResults[i];
                                }
                            }
                        }
                    }

                    foundCount = newCount;
                    memcpy(foundResults, newResults, newCount * sizeof(unsigned char*));
                    PopulateListView(hwndListView, foundResults, foundCount);
                    break;
                }

                case ID_BUTTON_ATTACH:
                    AttachLostArk();
                    break;

                case ID_BUTTON_CLEAR:
                    ClearListView(hwndListView);
                    foundCount = 0;
                    UpdateResultsCountLabel(hwndStaticCount, 0, 0);
                    break;

                case ID_BUTTON_WRITE: {
                    int selected = ListView_GetNextItem(hwndListView, -1, LVNI_SELECTED);
                    if (selected == -1) {
                        MessageBox(hwnd, "No address selected!", "Error", MB_OK | MB_ICONERROR);
                        break;
                    }
                    if (hProcess == NULL) {
                        MessageBox(hwnd, "No process attached!", "Error", MB_OK | MB_ICONERROR);
                        break;
                    }

                    char buf[32];
                    GetWindowText(hwndEditNewValue, buf, sizeof(buf));
                    UINT64 newValue = 0;
                    if (valueSize == 4) {
                        newValue = strtoul(buf, NULL, 0);
                    } else {
                        newValue = (WORD)strtoul(buf, NULL, 0);
                    }

                    unsigned char* addr = foundResults[selected];
                    SIZE_T bytesWritten = 0;
                    if (!WriteProcessMemory(hProcess, addr, &newValue, valueSize, &bytesWritten) || bytesWritten != valueSize) {
                        MessageBox(hwnd, "Failed to write value!", "Error", MB_OK | MB_ICONERROR);
                    } else {
                        MessageBox(hwnd, "Value written successfully!", "Success", MB_OK | MB_ICONINFORMATION);
                    }
                    break;
                }

                case ID_BUTTON_COPY:
                    CopyAddressToClipboard(hwnd);
                    break;

                case ID_BUTTON_SAVE:
                    SaveAddressToFile(hwnd);
                    break;

                case ID_BUTTON_STARTBOT:
                    MessageBox(hwnd, "Bot started (stub). Implement bot logic!", "Info", MB_OK | MB_ICONINFORMATION);
                    break;

                case ID_BUTTON_STOPBOT:
                    MessageBox(hwnd, "Bot stopped (stub).", "Info", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            break;

        case WM_DESTROY:
            if (hProcess) CloseHandle(hProcess);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd) {
    INITCOMMONCONTROLSEX icex = {sizeof(INITCOMMONCONTROLSEX)};
    icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MemoryScannerClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(0, "MemoryScannerClass", "Lost Moko",
        WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, 620, 430,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, showCmd);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

