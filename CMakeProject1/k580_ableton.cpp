#include "k580_ableton.h"
#include <algorithm>
#include <iostream>
#include <string>

#pragma comment(lib, "winmm.lib")

HMIDIOUT hMidiOut = nullptr;
HHOOK hHook = nullptr;
std::unordered_map<int, int> activeNotes;
int transposeOffset = 0;

void sendMidi(BYTE status, BYTE note, BYTE velocity) {
    if (!hMidiOut) return;
    DWORD msg = status | (note << 8) | (velocity << 16);
    midiOutShortMsg(hMidiOut, msg);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        auto* p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        int key = static_cast<int>(p->vkCode);

        if (key == VK_UP || key == VK_DOWN || key == VK_LEFT || key == VK_RIGHT) {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                if (key == VK_UP)         transposeOffset += 1;
                else if (key == VK_DOWN)  transposeOffset -= 1;
                else if (key == VK_RIGHT) transposeOffset += 12;
                else if (key == VK_LEFT)  transposeOffset -= 12;

                transposeOffset = std::clamp(transposeOffset, -36, 72);
                std::cout << "Transpose: " << (transposeOffset >= 0 ? "+" : "")
                    << transposeOffset << " semitones\n";
            }
            return 1;
        }

        if (key == VK_RETURN) {
            if (p->flags & LLKHF_EXTENDED) {
                key = VK_NUMPAD_ENTER;
            }
            else {
                return CallNextHookEx(nullptr, nCode, wParam, lParam);
            }
        }

        auto it = padMap.find(key);
        if (it != padMap.end()) {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                if (activeNotes.find(key) == activeNotes.end()) {
                    int note = std::clamp(it->second + transposeOffset, 0, 127);
                    activeNotes[key] = note;
                    sendMidi(0x90, static_cast<BYTE>(note), 127);
                }
            }
            else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                auto activeIt = activeNotes.find(key);
                if (activeIt != activeNotes.end()) {
                    sendMidi(0x80, static_cast<BYTE>(activeIt->second), 0);
                    activeNotes.erase(activeIt);
                }
            }
            return 1;
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        if (hHook) UnhookWindowsHookEx(hHook);
        if (hMidiOut) midiOutClose(hMidiOut);
        PostQuitMessage(0);
        return TRUE;
    }
    return FALSE;
}

int main() {
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    UINT numDevs = midiOutGetNumDevs();
    if (numDevs == 0) {
        std::cerr << "MIDI devices not found.\n";
        return 1;
    }

    int autoPort = -1;
    std::cout << "Available MIDI Out ports:\n";
    for (UINT i = 0; i < numDevs; ++i) {
        MIDIOUTCAPSA caps;
        if (midiOutGetDevCapsA(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            std::string name(caps.szPname);
            std::cout << "[" << i << "] " << name << "\n";
            if (autoPort == -1 && (name.find("loopMIDI") != std::string::npos || name.find("K580") != std::string::npos)) {
                autoPort = static_cast<int>(i);
            }
        }
    }

    UINT port = 0;
    if (autoPort != -1) {
        port = static_cast<UINT>(autoPort);
        std::cout << "Auto-selected port [" << port << "]\n";
    }
    else {
        std::cout << "Select loopMIDI port index: ";
        std::cin >> port;
    }

    if (midiOutOpen(&hMidiOut, port, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "Failed to open MIDI port.\n";
        return 1;
    }

    hHook = SetWindowsHookExA(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
    if (!hHook) {
        std::cerr << "Failed to install keyboard hook.\n";
        midiOutClose(hMidiOut);
        return 1;
    }

    std::cout << "K580 Ableton DrumRack active (C1..E2).\n"
        << "Arrows: Up/Down (+/-1 semitone), Left/Right (+/-1 octave).\n"
        << "Keep NumLock ON before starting. Press Ctrl+C to exit.\n";

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hHook) UnhookWindowsHookEx(hHook);
    if (hMidiOut) midiOutClose(hMidiOut);
    return 0;
}