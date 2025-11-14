#include <windows.h>
#include <string>

// Define an identifier for our button
#define IDC_PASTE_BUTTON 101
#define IDC_DELAY_EDIT   102
#define IDC_START_DELAY_EDIT 103
#define IDC_CLIPBOARD_PREVIEW 104
#define IDT_CLIPBOARD_TIMER 1

// Forward declarations of functions
void PerformPasteAsKeystrokes(HWND hwnd);
void TypeCharacter(char c, int delay);
void TypeSpecialKey(WORD vkKey, int delay);
void UpdateClipboardPreview(HWND hwnd);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Main entry point for a Windows GUI application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. Register the window class
    const char CLASS_NAME[] = "PasteAsKeystrokesWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // 2. Create the window
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        "Paste as Keystrokes",          // Window text
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, 260, 280, // Size and position
        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // 3. The message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Window Procedure: Handles messages for the window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Create a static label for the keystroke delay textbox
            CreateWindow(
                "STATIC", "Keystroke Delay (ms):",
                WS_VISIBLE | WS_CHILD,
                10, 20, 140, 25,
                hwnd, NULL, NULL, NULL);

            // Create an edit control (textbox) for the keystroke delay
            CreateWindow(
                "EDIT", "10",
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
                160, 20, 50, 25,
                hwnd, (HMENU)IDC_DELAY_EDIT, NULL, NULL);

            // Create a static label for the start delay textbox
            CreateWindow(
                "STATIC", "Start Delay (s):",
                WS_VISIBLE | WS_CHILD,
                10, 50, 140, 25,
                hwnd, NULL, NULL, NULL);

            // Create an edit control (textbox) for the start delay
            CreateWindow(
                "EDIT", "3",
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
                160, 50, 50, 25,
                hwnd, (HMENU)IDC_START_DELAY_EDIT, NULL, NULL);

            // Create a static label for the clipboard preview
            CreateWindow(
                "STATIC", "Clipboard:",
                WS_VISIBLE | WS_CHILD,
                10, 80, 80, 25,
                hwnd, NULL, NULL, NULL);

            // Create a static control to show the clipboard preview
            CreateWindow(
                "STATIC", "(empty)",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                90, 80, 150, 25,
                hwnd, (HMENU)IDC_CLIPBOARD_PREVIEW, NULL, NULL);

            // Create a button when the window is created
            CreateWindow(
                "BUTTON",               // Predefined class; Unicode assumed
                "Paste",                // Button text
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
                75, 160, 100, 30,       // x, y, width, height
                hwnd,                   // Parent window
                (HMENU)IDC_PASTE_BUTTON,// Button ID
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                NULL);                  // Pointer not needed
            
            SetTimer(hwnd, IDT_CLIPBOARD_TIMER, 500, NULL);
            UpdateClipboardPreview(hwnd);
            break;
        }

        case WM_TIMER: {
            if (wParam == IDT_CLIPBOARD_TIMER) {
                UpdateClipboardPreview(hwnd);
            }
            break;
        }

        case WM_COMMAND: {
            // Process button clicks
            if (LOWORD(wParam) == IDC_PASTE_BUTTON) {
                char buffer[16];
                GetWindowText(GetDlgItem(hwnd, IDC_START_DELAY_EDIT), buffer, 16);
                int start_delay = std::stoi(buffer);
                if (start_delay < 0) {
                    start_delay = 0;
                }
                // Give user a moment to switch focus after clicking
                Sleep(start_delay * 1000); 
                PerformPasteAsKeystrokes(hwnd);
            }
            break;
        }

        case WM_DESTROY: {
            KillTimer(hwnd, IDT_CLIPBOARD_TIMER);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void UpdateClipboardPreview(HWND hwnd) {
    if (!OpenClipboard(hwnd)) {
        SetDlgItemText(hwnd, IDC_CLIPBOARD_PREVIEW, "(cannot open)");
        return;
    }

    HANDLE hClipboardData = GetClipboardData(CF_TEXT);
    if (hClipboardData == NULL) {
        SetDlgItemText(hwnd, IDC_CLIPBOARD_PREVIEW, "(empty)");
        CloseClipboard();
        return;
    }

    char* pszText = static_cast<char*>(GlobalLock(hClipboardData));
    if (pszText == NULL) {
        SetDlgItemText(hwnd, IDC_CLIPBOARD_PREVIEW, "(cannot lock)");
        CloseClipboard();
        return;
    }

    std::string text(pszText, 10);
    GlobalUnlock(hClipboardData);
    CloseClipboard();

    SetDlgItemText(hwnd, IDC_CLIPBOARD_PREVIEW, text.c_str());
}

// This function contains the core logic to read the clipboard and type it out
void PerformPasteAsKeystrokes(HWND hwnd) {
    char buffer[16];
    GetWindowText(GetDlgItem(hwnd, IDC_DELAY_EDIT), buffer, 16);
    int delay = std::stoi(buffer);
    if (delay < 0) {
        delay = 0;
    }

    if (!OpenClipboard(NULL)) {
        MessageBox(NULL, "Error: Cannot open clipboard.", "Clipboard Error", MB_OK | MB_ICONERROR);
        return;
    }

    HANDLE hClipboardData = GetClipboardData(CF_TEXT);
    if (hClipboardData == NULL) {
        CloseClipboard();
        MessageBox(NULL, "Error: Cannot get clipboard data or data is not text.", "Clipboard Error", MB_OK | MB_ICONERROR);
        return;
    }

    char* pszText = static_cast<char*>(GlobalLock(hClipboardData));
    if (pszText == NULL) {
        CloseClipboard();
        return;
    }

    std::string text(pszText);

    GlobalUnlock(hClipboardData);
    CloseClipboard();

    for (char const &c : text) {
        if (c == '\n') {
            TypeSpecialKey(VK_RETURN, delay);
        } else if (c == '\r') {
            continue;
        } else if (c == '\t') {
            TypeSpecialKey(VK_TAB, delay);
        } else {
            TypeCharacter(c, delay);
        }
    }
}

// Simulates a key press and release for a given character
void TypeCharacter(char c, int delay) {
    SHORT vk = VkKeyScan(c);
    INPUT inputs[4] = {};
    int inputCount = 0;

    if (vk & 0x0100) { // Shift key
        inputs[inputCount].type = INPUT_KEYBOARD;
        inputs[inputCount].ki.wVk = VK_SHIFT;
        inputCount++;
    }

    inputs[inputCount].type = INPUT_KEYBOARD;
    inputs[inputCount].ki.wVk = vk & 0xFF;
    inputCount++;

    inputs[inputCount].type = INPUT_KEYBOARD;
    inputs[inputCount].ki.wVk = vk & 0xFF;
    inputs[inputCount].ki.dwFlags = KEYEVENTF_KEYUP;
    inputCount++;

    if (vk & 0x0100) { // Shift release
        inputs[inputCount].type = INPUT_KEYBOARD;
        inputs[inputCount].ki.wVk = VK_SHIFT;
        inputs[inputCount].ki.dwFlags = KEYEVENTF_KEYUP;
        inputCount++;
    }

    SendInput(inputCount, inputs, sizeof(INPUT));
    Sleep(delay);
}

// Simulates a press for special keys like Enter or Tab
void TypeSpecialKey(WORD vkKey, int delay) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vkKey;
    SendInput(1, &input, sizeof(INPUT));

    Sleep(delay);

    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}
