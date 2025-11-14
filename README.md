# Paste as Keystrokes

A simple Windows utility to paste clipboard text as simulated keystrokes. This is useful for applications that don't support standard copy-paste, or for sending text to virtual machines or remote desktops.

## Features

*   Pastes clipboard text by simulating individual key presses.
*   Configurable delay between each keystroke.
*   Configurable delay before the pasting process begins, giving you time to focus the target window.
*   A simple GUI to control the settings.
*   Live preview of the first few characters on the clipboard.

## Building from Source

This project is built using C++ and the Windows API. The recommended way to build it is with the `w64devkit` toolchain.

### Prerequisites

*   [w64devkit](https://github.com/skeeto/w64devkit/releases): Download the latest release.

### Build Steps

1.  **Download and Extract w64devkit**: Download the `w64devkit` archive and extract it to a location on your computer.

2.  **Open the w64devkit Terminal**: Run `w64devkit.exe` to open a pre-configured terminal environment.

3.  **Clone the Repository**:
    ```sh
    git clone <repository-url>
    cd <repository-directory>
    ```

4.  **Compile the Application**: Use `g++` to compile the source code. The following command links against the necessary Windows libraries:
    ```sh
    g++ main.cpp -o PasteAsKeystrokes.exe -lcomctl32 -lgdi32 -luser32 -lkernel32
    ```

5.  **Run**: You will now have `PasteAsKeystrokes.exe` in the directory.
