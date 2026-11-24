#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <unordered_map>
#include <unordered_set>

constexpr int VK_NUMPAD_ENTER = 0x1000;

inline const std::unordered_map<int, int> padMap = {
    { VK_NUMPAD0,      36 },
    { VK_DECIMAL,      37 }, 
    { VK_NUMPAD_ENTER, 38 }, 
    { VK_NUMPAD1,      39 },
    { VK_NUMPAD2,      40 }, 
    { VK_NUMPAD3,      41 },
    { VK_NUMPAD4,      42 },
    { VK_NUMPAD5,      43 },
    { VK_NUMPAD6,      44 },
    { VK_ADD,          45 }, 
    { VK_NUMPAD7,      46 },
    { VK_NUMPAD8,      47 }, 
    { VK_NUMPAD9,      48 }, 
    { VK_NUMLOCK,      49 },
    { VK_DIVIDE,       50 },
    { VK_MULTIPLY,     51 },
    { VK_SUBTRACT,     52 } 
};

extern HMIDIOUT hMidiOut;
extern HHOOK hHook;
extern std::unordered_set<int> pressedKeys;

void sendMidi(BYTE status, BYTE note, BYTE velocity);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
BOOL WINAPI ConsoleHandler(DWORD signal);