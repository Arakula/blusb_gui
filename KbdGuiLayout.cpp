/*****************************************************************************/
/* KbdGuiLayout.cpp : GUI Layout Definition for the keyboard                 */
/*****************************************************************************/
/* 
Copyright (C) 2019  Hermann Seib

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "wxStd.h"

#include "layout.h"
#include "KbdGuiLayout.h"

using namespace std;

/* The layout of the keyboard GUI consists of the following parts:

1) Matrix -> USB HID Keycode translation
========================================

This matrix is the thing that can be reprogrammed in the controller.

For the IBM Model M, there are 2 layouts that are in wide circulation:
ANSI (101 keys) and ISO (102 keys) layout.
Internally, the 16x8 matrix for both is the same, but different connections
are used for 2 of the possible keys.
Additional variants, such as a Brazilian layout which has
two more keys, exist; in total, 5 additional matrix connection points are
available.

The controller, in its current incarnation, can be used with 122-key
Model M keyboards, too; these have a 20x8 matrix.

While the communication with the physical controller always exchanges 20x8
matrices, it doesn't makes too much sense to display unusable columns if
the controller is embedded in a keyboard with a 16x8 matrix; therefore, the
matrix layout used in the GUI is configurable.

2) physical keyboard layout
===========================

For a GUI representation, the physical layout (i.e., number of keys,
their position, size, and form) needs to be configurable. Each of the
displayed keys is tied to a specific matrix position.

3) Text on the keys
===================

The USB HID keycodes sent by the keyboard are based on a US keyboard layout;
the additional key on European keyboards has a special code. National logical
keyboard layouts on the computer the keyboard is attached to, however,
can assign drastically different meanings to the keys (Q -> A, W -> Z etc. for
French keyboards, for example). So, the text on the keys presented in 2)
needs to be separately configurable. Each text is tied to a specific key in
the physical keyboard layout.

The complete keyboard layout, therefore, could be composed of 1 to 3 files:
.) matrix definition file (optional)
.) keyboard layout file with default texts
.) key text file (optional)
with defaults set for a US ANSI keyboard using the standard matrix layout.

The Key Text File isn't really necessary; it would allow for a completely
normalized layout, but at the cost of a much higher complexity.
Embedding the text in the keyboard layout file should be sufficent.

*/

/*****************************************************************************/
/* Enumeration for special virtual key codes                                 */
/*****************************************************************************/

enum kbdKeyCode
  {
  // start with highest possible wxWidgets key code
  WXK_LSHIFT = WXK_LAUNCH_APP2 + 1,
  WXK_RSHIFT,
  WXK_LCONTROL,
  WXK_RCONTROL,
  WXK_LMENU,
  WXK_RMENU,

  WXK_NUMCODES
  };

/*****************************************************************************/
/* Global Data                                                               */
/*****************************************************************************/

// Model M US ANSI Keyboard Default Layout
static GuiKey AnsiM[] =
  {
    // Row 1
    { 0, 1, KB_ESC,     0, 13, 1.00f, 1.00f, "Esc", "Esc" },
    { 0, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    { 0, 1, KB_F1,      2, 12, 1.00f, 1.00f, "F1", "F1" },
    { 0, 1, KB_F2,      2, 11, 1.00f, 1.00f, "F2", "F2" },
    { 0, 1, KB_F3,      1, 11, 1.00f, 1.00f, "F3", "F3" },
    { 0, 1, KB_F4,      0, 11, 1.00f, 1.00f, "F4", "F4" },
    { 0, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 0, 1, KB_F5,      0,  9, 1.00f, 1.00f, "F5", "F5" },
    { 0, 1, KB_F6,      0,  7, 1.00f, 1.00f, "F6", "F6" },
    { 0, 1, KB_F7,      1,  6, 1.00f, 1.00f, "F7", "F7" },
    { 0, 1, KB_F8,      2,  6, 1.00f, 1.00f, "F8", "F8" },
    { 0, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 0, 1, KB_F9,      2,  9, 1.00f, 1.00f, "F9", "F5" },
    { 0, 1, KB_F10,     3,  9, 1.00f, 1.00f, "F10", "F10" },
    { 0, 1, KB_F11,     3,  4, 1.00f, 1.00f, "F11", "F11" },
    { 0, 1, KB_F12,     3,  3, 1.00f, 1.00f, "F12", "F12" },
    { 0, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 0, 1, KB_PSCRN,   3,  0, 1.00f, 1.00f, "Print\nScrn", "Print Screen" },
    { 0, 1, KB_SCRLK,   4,  0, 1.00f, 1.00f, "Scroll\nLock", "Scroll Lock" },
    { 0, 1, KB_PAUSE,   6,  1, 1.00f, 1.00f, "Pause", "Pause" },
    { 0, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 0, 1, LED_NUMLK, -1, -1, 1.34f, 1.34f, "NumLk", "" },
    { 0, 1, LED_CAPLK, -1, -1, 1.34f, 1.34f, "CapsLk", "" },
    { 0, 1, LED_SCRLK, -1, -1, 1.34f, 1.34f, "ScrlLk", "" },
    // Row 2
    { 1, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    // Row 3
    { 2, 1, KB_TILDE,   2, 13, 1.00f, 1.00f, "`~", "~" },
    { 2, 1, KB_1,       3, 13, 1.00f, 1.00f, "1 !", "1" },
    { 2, 1, KB_2,       3, 12, 1.00f, 1.00f, "2 @", "2" },
    { 2, 1, KB_3,       3, 11, 1.00f, 1.00f, "3 #", "3" },
    { 2, 1, KB_4,       3, 10, 1.00f, 1.00f, "4 $", "4" },
    { 2, 1, KB_5,       2, 10, 1.00f, 1.00f, "5 %", "5" },
    { 2, 1, KB_6,       2,  8, 1.00f, 1.00f, "6 ^", "6" },
    { 2, 1, KB_7,       3,  8, 1.00f, 1.00f, "7 &", "7" },
    { 2, 1, KB_8,       3,  7, 1.00f, 1.00f, "8 *", "8" },
    { 2, 1, KB_9,       3,  6, 1.00f, 1.00f, "9 (", "9" },
    { 2, 1, KB_0,       3,  5, 1.00f, 1.00f, "0 )", "0" },
    { 2, 1, KB_MINUS,   2,  5, 1.00f, 1.00f, "- _", "-" },
    { 2, 1, KB_EQUAL,   2,  7, 1.00f, 1.00f, "= +", "=" },
    { 2, 1, KB_BKSPC,   1,  9, 2.00f, 2.00f, "Backspace", "Backspace" },
    { 2, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 2, 1, KB_INS,     2,  3, 1.00f, 1.00f, "Insert", "Insert" },
    { 2, 1, KB_HOME,    2,  1, 1.00f, 1.00f, "Home", "Home" },
    { 2, 1, KB_PGUP,    2,  2, 1.00f, 1.00f, "Page\nUp", "Page Up" },
    { 2, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 2, 1, KB_NUMLK,   6,  4, 1.00f, 1.00f, "Num\nLock", "Num Lock" },
    { 2, 1, KP_SLASH,   6,  3, 1.00f, 1.00f, "/", "Num /" },
    { 2, 1, KP_ASTRX,   6,  2, 1.00f, 1.00f, "*", "Num *" },
    { 2, 1, KP_MINUS,   7,  2, 1.00f, 1.00f, "-", "Num -" },
    // Row 4
    { 3, 1, KB_TAB,     1, 13, 1.50f, 1.50f, "Tab", "Tab" },
    { 3, 1, KB_Q,       4, 13, 1.00f, 1.00f, "Q", "Q" },
    { 3, 1, KB_W,       4, 12, 1.00f, 1.00f, "W", "W" },
    { 3, 1, KB_E,       4, 11, 1.00f, 1.00f, "E", "E" },
    { 3, 1, KB_R,       4, 10, 1.00f, 1.00f, "R", "R" },
    { 3, 1, KB_T,       1, 10, 1.00f, 1.00f, "T", "T" },
    { 3, 1, KB_Y,       1,  8, 1.00f, 1.00f, "Y", "Y" },
    { 3, 1, KB_U,       4,  8, 1.00f, 1.00f, "U", "U" },
    { 3, 1, KB_I,       4,  7, 1.00f, 1.00f, "I", "I" },
    { 3, 1, KB_O,       4,  6, 1.00f, 1.00f, "O", "O" },
    { 3, 1, KB_P,       4,  5, 1.00f, 1.00f, "P", "P" },
    { 3, 1, KB_LBRCE,   1,  5, 1.00f, 1.00f, "[ {", "[" },
    { 3, 1, KB_RBRCE,   1,  7, 1.00f, 1.00f, "] }", "]" },
    { 3, 1, KB_BSLSH,   5,  9, 1.50f, 1.50f, "\\ |", "\\" },
    { 3, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 3, 1, KB_DEL,     2,  4, 1.00f, 1.00f, "Delete", "Delete" },
    { 3, 1, KB_END,     3,  1, 1.00f, 1.00f, "End", "End" },
    { 3, 1, KB_PGDN,    3,  2, 1.00f, 1.00f, "Page\nDown", "Page Down" },
    { 3, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 3, 1, KP_7,       4,  4, 1.00f, 1.00f, "7", "Num 7" },
    { 3, 1, KP_8,       4,  3, 1.00f, 1.00f, "8", "Num 8" },
    { 3, 1, KP_9,       4,  2, 1.00f, 1.00f, "9", "Num 9" },
    { 3, 2, KP_PLUS,    4,  1, 1.00f, 1.00f, "+", "Num +" },
    // Row 5
    { 4, 1, KB_CAPLK,   1, 12, 1.75f, 1.75f, "Caps Lock", "Caps Lock" },
    { 4, 1, KB_A,       5, 13, 1.00f, 1.00f, "A", "A" },
    { 4, 1, KB_S,       5, 12, 1.00f, 1.00f, "S", "S" },
    { 4, 1, KB_D,       5, 11, 1.00f, 1.00f, "D", "D" },
    { 4, 1, KB_F,       5, 10, 1.00f, 1.00f, "F", "F" },
    { 4, 1, KB_G,       0, 10, 1.00f, 1.00f, "G", "G" },
    { 4, 1, KB_H,       0,  8, 1.00f, 1.00f, "H", "H" },
    { 4, 1, KB_J,       5,  8, 1.00f, 1.00f, "J", "J" },
    { 4, 1, KB_K,       5,  7, 1.00f, 1.00f, "K", "K" },
    { 4, 1, KB_L,       5,  6, 1.00f, 1.00f, "L", "L" },
    { 4, 1, KB_SMCLN,   5,  5, 1.00f, 1.00f, "; :", ";" },
    { 4, 1, KB_QUOTE,   0,  5, 1.00f, 1.00f, "' \"", "'" },
    { 4, 1, KB_ENTER,   6,  9, 2.25f, 2.25f, "Enter", "Enter" },
    { 4, 1, -1,        -1, -1, 4.00f, 4.00f, "", "" },
    { 4, 1, KP_4,       1,  4, 1.00f, 1.00f, "4", "Num 4" },
    { 4, 1, KP_5,       1,  3, 1.00f, 1.00f, "5", "Num 5" },
    { 4, 1, KP_6,       1,  2, 1.00f, 1.00f, "6", "Num 6" },
    // Row 6
    { 5, 1, KB_LSHFT,   1, 14, 2.25f, 2.25f, "Shift", "LShift" },
    { 5, 1, KB_Z,       6, 13, 1.00f, 1.00f, "Z", "Z" },
    { 5, 1, KB_X,       6, 12, 1.00f, 1.00f, "X", "X" },
    { 5, 1, KB_C,       6, 11, 1.00f, 1.00f, "C", "C" },
    { 5, 1, KB_V,       6, 10, 1.00f, 1.00f, "V", "V" },
    { 5, 1, KB_B,       7, 10, 1.00f, 1.00f, "B", "B" },
    { 5, 1, KB_N,       7,  8, 1.00f, 1.00f, "N", "N" },
    { 5, 1, KB_M,       6,  8, 1.00f, 1.00f, "M", "M" },
    { 5, 1, KB_COMMA,   6,  7, 1.00f, 1.00f, ", <", "," },
    { 5, 1, KB_DOT,     6,  6, 1.00f, 1.00f, ". >", "." },
    { 5, 1, KB_SLASH,   7,  5, 1.00f, 1.00f, "/ ?", "/" },
    { 5, 1, KB_RSHFT,   6, 14, 2.75f, 2.75f, "Shift", "RShift" },
    { 5, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 5, 1, KB_UP,      0,  1, 1.00f, 1.00f, "Up", "Up" },
    { 5, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 5, 1, KP_1,       5,  4, 1.00f, 1.00f, "1", "1" },
    { 5, 1, KP_2,       5,  3, 1.00f, 1.00f, "2", "2" },
    { 5, 1, KP_3,       5,  2, 1.00f, 1.00f, "3", "3" },
    { 5, 2, KP_ENTER,   5,  1, 1.00f, 1.00f, "Enter", "Enter" },
    // Row 7
    { 6, 1, KB_LCTRL,   2, 15, 1.50f, 1.50f, "Ctrl", "LCtrl" },
    { 6, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    { 6, 1, KB_LALT,    0,  0, 1.50f, 1.50f, "Alt", "LAlt" },
    { 6, 1, KB_SPACE,   7,  9, 7.00f, 7.00f, "Space", "Space" },
    { 6, 1, KB_RALT,    7,  0, 1.50f, 1.50f, "AltGr", "RAlt" },
    { 6, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    { 6, 1, KB_RCTRL,   6, 15, 1.50f, 1.50f, "Ctrl", "RCtrl" },
    { 6, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 6, 1, KB_LEFT,    7,  1, 1.00f, 1.00f, "Left", "Left" },
    { 6, 1, KB_DOWN,    7,  4, 1.00f, 1.00f, "Down", "Down" },
    { 6, 1, KB_RIGHT,   7,  3, 1.00f, 1.00f, "Right", "Right" },
    { 6, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 6, 1, KP_0,       0,  3, 2.00f, 2.00f, "0", "0" },
    { 6, 1, KP_DOT,     0,  2, 1.00f, 1.00f, ".", "." },
  };

// Model M 121 US ANSI Keyboard Default Layout
static GuiKey AnsiM121[] =
  {
    // Row 0 - Function Keys F13-F24
    { 0, 1, -1,        -1, -1, 3.50f, 3.50f, "", "" },
    { 0, 1, KB_F13,     0,  7, 1.00f, 1.00f, "F13", "F13" },
    { 0, 1, KB_F14,     1,  7, 1.00f, 1.00f, "F14", "F14" },
    { 0, 1, KB_F15,     1,  8, 1.00f, 1.00f, "F15", "F15" },
    { 0, 1, KB_F16,     2,  7, 1.00f, 1.00f, "F16", "F16" },
    { 0, 1, KB_F17,     3,  7, 1.00f, 1.00f, "F17", "F17" },
    { 0, 1, KB_F18,     3,  8, 1.00f, 1.00f, "F18", "F18" },
    { 0, 1, KB_F19,     4,  7, 1.00f, 1.00f, "F19", "F19" },
    { 0, 1, KB_F20,     5,  7, 1.00f, 1.00f, "F20", "F20" },
    { 0, 1, KB_F21,     5,  8, 1.00f, 1.00f, "F21", "F21" },
    { 0, 1, KB_F22,     6,  7, 1.00f, 1.00f, "F22", "F22" },
    { 0, 1, KB_F23,     7,  7, 1.00f, 1.00f, "F23", "F23" },
    { 0, 1, KB_F24,     7,  8, 1.00f, 1.00f, "F24", "F24" },
    // Row 1
    { 1, 1, -1,        -1, -1, 3.50f, 3.50f, "", "" },
    { 1, 1, KB_F1,      0,  8, 1.00f, 1.00f, "F1", "F1" },
    { 1, 1, KB_F2,      0,  9, 1.00f, 1.00f, "F2", "F2" },
    { 1, 1, KB_F3,      1,  9, 1.00f, 1.00f, "F3", "F3" },
    { 1, 1, KB_F4,      2,  8, 1.00f, 1.00f, "F4", "F4" },
    { 1, 1, KB_F5,      2,  9, 1.00f, 1.00f, "F5", "F5" },
    { 1, 1, KB_F6,      3,  9, 1.00f, 1.00f, "F6", "F6" },
    { 1, 1, KB_F7,      4,  8, 1.00f, 1.00f, "F7", "F7" },
    { 1, 1, KB_F8,      4,  9, 1.00f, 1.00f, "F8", "F8" },
    { 1, 1, KB_F9,      5,  9, 1.00f, 1.00f, "F9", "F5" },
    { 1, 1, KB_F10,     6,  8, 1.00f, 1.00f, "F10", "F10" },
    { 1, 1, KB_F11,     6,  9, 1.00f, 1.00f, "F11", "F11" },
    { 1, 1, KB_F12,     7,  9, 1.00f, 1.00f, "F12", "F12" },
    { 1, 1, -1,        -1, -1, 6.00f, 6.00f, "", "" },
    { 1, 1, LED_NUMLK, -1, -1, 1.34f, 1.34f, "NumLk", "" },
    { 1, 1, LED_CAPLK, -1, -1, 1.34f, 1.34f, "CapsLk", "" },
    { 1, 1, LED_SCRLK, -1, -1, 1.34f, 1.34f, "ScrlLk", "" },
    // Row 2 - spacer
    { 2, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    // Row 3
    { 3, 1, KB_ESC,     3, 19, 1.00f, 1.00f, "Esc", "Esc" },
    { 3, 1, KB_SCRLK,   3,  0, 1.00f, 1.00f, "Scroll\nLock", "Scroll Lock" },
    { 3, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 3, 1, KB_TILDE,   3,  2, 1.00f, 1.00f, "`~", "~" },
    { 3, 1, KB_1,       2,  2, 1.00f, 1.00f, "1 !", "1" },
    { 3, 1, KB_2,       2,  3, 1.00f, 1.00f, "2 @", "2" },
    { 3, 1, KB_3,       2,  4, 1.00f, 1.00f, "3 #", "3" },
    { 3, 1, KB_4,       2,  5, 1.00f, 1.00f, "4 $", "4" },
    { 3, 1, KB_5,       3,  5, 1.00f, 1.00f, "5 %", "5" },
    { 3, 1, KB_6,       3,  6, 1.00f, 1.00f, "6 ^", "6" },
    { 3, 1, KB_7,       2,  6, 1.00f, 1.00f, "7 &", "7" },
    { 3, 1, KB_8,       2, 10, 1.00f, 1.00f, "8 *", "8" },
    { 3, 1, KB_9,       2, 11, 1.00f, 1.00f, "9 (", "9" },
    { 3, 1, KB_0,       2, 12, 1.00f, 1.00f, "0 )", "0" },
    { 3, 1, KB_MINUS,   3, 12, 1.00f, 1.00f, "- _", "-" },
    { 3, 1, KB_EQUAL,   3, 10, 1.00f, 1.00f, "= +", "=" },
    { 3, 1, KB_BKSPC,   3, 13, 2.00f, 2.00f, "Backspace", "Backspace" },
    { 3, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 3, 1, KB_INS,     3, 17, 1.00f, 1.00f, "Insert", "Insert" },
    { 3, 1, KB_HOME,    3, 16, 1.00f, 1.00f, "Home", "Home" },
    { 3, 1, KB_PGUP,    2, 17, 1.00f, 1.00f, "Page\nUp", "Page Up" },
    { 3, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 3, 1, KB_END,     2, 13, 1.00f, 1.00f, "End", "End" },
    { 3, 1, KB_NUMLK,   2, 16, 1.00f, 1.00f, "Num\nLock", "Num Lock" },
    { 3, 1, KP_SLASH,   2, 14, 1.00f, 1.00f, "/", "Num /" },
    { 3, 1, KP_ASTRX,   2, 18, 1.00f, 1.00f, "*", "Num *" },
    // Row 4
    { 4, 1, KB_PSCRN,   2, 19, 1.00f, 1.00f, "Print\nScrn", "Print Screen" },
    { 4, 1, KB_PAUSE,   4, 19, 1.00f, 1.00f, "Pause", "Pause" },
    { 4, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 4, 1, KB_TAB,     4,  0, 1.50f, 1.50f, "Tab", "Tab" },
    { 4, 1, KB_Q,       4,  2, 1.00f, 1.00f, "Q", "Q" },
    { 4, 1, KB_W,       4,  3, 1.00f, 1.00f, "W", "W" },
    { 4, 1, KB_E,       4,  4, 1.00f, 1.00f, "E", "E" },
    { 4, 1, KB_R,       4,  5, 1.00f, 1.00f, "R", "R" },
    { 4, 1, KB_T,       5,  5, 1.00f, 1.00f, "T", "T" },
    { 4, 1, KB_Y,       5,  6, 1.00f, 1.00f, "Y", "Y" },
    { 4, 1, KB_U,       4,  6, 1.00f, 1.00f, "U", "U" },
    { 4, 1, KB_I,       4, 10, 1.00f, 1.00f, "I", "I" },
    { 4, 1, KB_O,       4, 11, 1.00f, 1.00f, "O", "O" },
    { 4, 1, KB_P,       4, 12, 1.00f, 1.00f, "P", "P" },
    { 4, 1, KB_LBRCE,   5, 12, 1.00f, 1.00f, "[ {", "[" },
    { 4, 1, KB_RBRCE,   5, 10, 1.00f, 1.00f, "] }", "]" },
    { 4, 1, KB_BSLSH,   4, 13, 1.50f, 1.50f, "\\ |", "\\" },
    { 4, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 4, 1, KB_DEL,     1, 17, 1.00f, 1.00f, "Delete", "Delete" },
    { 4, 1, KB_END,     5, 17, 1.00f, 1.00f, "End", "End" },
    { 4, 1, KB_PGDN,    4, 17, 1.00f, 1.00f, "Page\nDown", "Page Down" },
    { 4, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 4, 1, KP_7,       4, 13, 1.00f, 1.00f, "7", "Num 7" },
    { 4, 1, KP_8,       4, 16, 1.00f, 1.00f, "8", "Num 8" },
    { 4, 1, KP_9,       4, 14, 1.00f, 1.00f, "9", "Num 9" },
    { 4, 1, KP_MINUS,   4, 18, 1.00f, 1.00f, "-", "Num -" },
    // Row 5
    { 5, 1, KB_NONE,    5, 19, 1.00f, 1.00f, "EF5", "EF5" },
    { 5, 1, KB_NONE,    5,  0, 1.00f, 1.00f, "EF6", "EF6" },
    { 5, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 5, 1, KB_CAPLK,   6,  0, 1.75f, 1.75f, "Caps Lock", "Caps Lock" },
    { 5, 1, KB_A,       1,  2, 1.00f, 1.00f, "A", "A" },
    { 5, 1, KB_S,       1,  3, 1.00f, 1.00f, "S", "S" },
    { 5, 1, KB_D,       1,  4, 1.00f, 1.00f, "D", "D" },
    { 5, 1, KB_F,       1,  5, 1.00f, 1.00f, "F", "F" },
    { 5, 1, KB_G,       0,  5, 1.00f, 1.00f, "G", "G" },
    { 5, 1, KB_H,       0,  6, 1.00f, 1.00f, "H", "H" },
    { 5, 1, KB_J,       1,  6, 1.00f, 1.00f, "J", "J" },
    { 5, 1, KB_K,       1, 10, 1.00f, 1.00f, "K", "K" },
    { 5, 1, KB_L,       1, 11, 1.00f, 1.00f, "L", "L" },
    { 5, 1, KB_SMCLN,   1, 12, 1.00f, 1.00f, "; :", ";" },
    { 5, 1, KB_QUOTE,   0, 12, 1.00f, 1.00f, "' \"", "'" },
    { 5, 1, KB_ENTER,   6, 13, 2.25f, 2.25f, "Enter", "Enter" },
    { 5, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 5, 1, KB_UP,      0, 17, 1.00f, 1.00f, "Up", "Up" },
    { 5, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 5, 1, KP_4,       1, 13, 1.00f, 1.00f, "4", "Num 4" },
    { 5, 1, KP_5,       1, 16, 1.00f, 1.00f, "5", "Num 5" },
    { 5, 1, KP_6,       1, 14, 1.00f, 1.00f, "6", "Num 6" },
    { 5, 1, KP_PLUS,    1, 18, 1.00f, 1.00f, "+", "Num +" },
    // Row 6
    { 6, 1, KB_NONE,    1, 19, 1.00f, 1.00f, "EF7", "EF7" },
    { 6, 1, KB_NONE,    1,  0, 1.00f, 1.00f, "EF8", "EF8" },
    { 6, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 6, 1, KB_LSHFT,   7,  1, 2.25f, 2.25f, "Shift", "LShift" },
    { 6, 1, KB_Z,       6,  2, 1.00f, 1.00f, "Z", "Z" },
    { 6, 1, KB_X,       6,  3, 1.00f, 1.00f, "X", "X" },
    { 6, 1, KB_C,       6,  4, 1.00f, 1.00f, "C", "C" },
    { 6, 1, KB_V,       6,  5, 1.00f, 1.00f, "V", "V" },
    { 6, 1, KB_B,       7,  5, 1.00f, 1.00f, "B", "B" },
    { 6, 1, KB_N,       7,  6, 1.00f, 1.00f, "N", "N" },
    { 6, 1, KB_M,       6,  6, 1.00f, 1.00f, "M", "M" },
    { 6, 1, KB_COMMA,   6, 10, 1.00f, 1.00f, ", <", "," },
    { 6, 1, KB_DOT,     6, 11, 1.00f, 1.00f, ". >", "." },
    { 6, 1, KB_SLASH,   7, 12, 1.00f, 1.00f, "/ ?", "/" },
    { 6, 1, KB_RSHFT,   6,  1, 2.75f, 2.75f, "Shift", "RShift" },
    { 6, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 6, 1, KB_LEFT,    6, 18, 1.00f, 1.00f, "Left", "Left" },
    { 6, 1, KB_HOME,    0, 15, 1.00f, 1.00f, "Home", "Home" },
    { 6, 1, KB_RIGHT,   1, 15, 1.00f, 1.00f, "Right", "Right" },
    { 6, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 6, 1, KP_1,       6, 19, 1.00f, 1.00f, "1", "1" },
    { 6, 1, KP_2,       6, 16, 1.00f, 1.00f, "2", "2" },
    { 6, 1, KP_3,       6, 14, 1.00f, 1.00f, "3", "3" },
    { 6, 2, KP_ENTER,   7, 15, 1.00f, 1.00f, "Enter", "Enter" },
    // Row 7
    { 7, 1, KB_LGUI,    0,  0, 1.00f, 1.00f, "Win", "LWin" },
    { 7, 1, KB_APP,     0, 19, 1.00f, 1.00f, "Menu", "Menu" },
    { 7, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 7, 1, KB_LCTRL,   0, 18, 1.50f, 1.50f, "Ctrl", "LCtrl" },
    { 7, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    { 7, 1, KB_LALT,    7, 18, 1.50f, 1.50f, "Alt", "LAlt" },
    { 7, 1, KB_SPACE,   7, 19, 7.00f, 7.00f, "Space", "Space" },
    { 7, 1, KB_RALT,    0,  1, 1.50f, 1.50f, "AltGr", "RAlt" },
    { 7, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    { 7, 1, KB_RCTRL,   7,  0, 1.50f, 1.50f, "Ctrl", "RCtrl" },
    { 7, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 7, 1, KB_DOWN,    0, 13, 1.00f, 1.00f, "Down", "Down" },
    { 7, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 7, 1, KP_0,       7, 16, 2.00f, 2.00f, "0", "0" },
    { 7, 1, KP_DOT,     7, 14, 1.00f, 1.00f, ".", "." },

  };

// Model M US ISO Keyboard Default Layout
static GuiKey IsoM[] =
  {
    // Row 1
    { 0, 1, KB_ESC,     0, 13, 1.00f, 1.00f, "Esc", "Esc" },
    { 0, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    { 0, 1, KB_F1,      2, 12, 1.00f, 1.00f, "F1", "F1" },
    { 0, 1, KB_F2,      2, 11, 1.00f, 1.00f, "F2", "F2" },
    { 0, 1, KB_F3,      1, 11, 1.00f, 1.00f, "F3", "F3" },
    { 0, 1, KB_F4,      0, 11, 1.00f, 1.00f, "F4", "F4" },
    { 0, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 0, 1, KB_F5,      0,  9, 1.00f, 1.00f, "F5", "F5" },
    { 0, 1, KB_F6,      0,  7, 1.00f, 1.00f, "F6", "F6" },
    { 0, 1, KB_F7,      1,  6, 1.00f, 1.00f, "F7", "F7" },
    { 0, 1, KB_F8,      2,  6, 1.00f, 1.00f, "F8", "F8" },
    { 0, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 0, 1, KB_F9,      2,  9, 1.00f, 1.00f, "F9", "F5" },
    { 0, 1, KB_F10,     3,  9, 1.00f, 1.00f, "F10", "F10" },
    { 0, 1, KB_F11,     3,  4, 1.00f, 1.00f, "F11", "F11" },
    { 0, 1, KB_F12,     3,  3, 1.00f, 1.00f, "F12", "F12" },
    { 0, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 0, 1, KB_PSCRN,   3,  0, 1.00f, 1.00f, "Print\nScrn", "Print Screen" },
    { 0, 1, KB_SCRLK,   4,  0, 1.00f, 1.00f, "Scroll\nLock", "Scroll Lock" },
    { 0, 1, KB_PAUSE,   6,  1, 1.00f, 1.00f, "Pause", "Pause" },
    { 0, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 0, 1, LED_NUMLK, -1, -1, 1.34f, 1.34f, "NumLk", "" },
    { 0, 1, LED_CAPLK, -1, -1, 1.34f, 1.34f, "CapsLk", "" },
    { 0, 1, LED_SCRLK, -1, -1, 1.34f, 1.34f, "ScrlLk", "" },
    // Row 2
    { 1, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    // Row 3
    { 2, 1, KB_TILDE,   2, 13, 1.00f, 1.00f, "`~", "~" },
    { 2, 1, KB_1,       3, 13, 1.00f, 1.00f, "1 !", "1" },
    { 2, 1, KB_2,       3, 12, 1.00f, 1.00f, "2 @", "2" },
    { 2, 1, KB_3,       3, 11, 1.00f, 1.00f, "3 #", "3" },
    { 2, 1, KB_4,       3, 10, 1.00f, 1.00f, "4 $", "4" },
    { 2, 1, KB_5,       2, 10, 1.00f, 1.00f, "5 %", "5" },
    { 2, 1, KB_6,       2,  8, 1.00f, 1.00f, "6 ^", "6" },
    { 2, 1, KB_7,       3,  8, 1.00f, 1.00f, "7 &", "7" },
    { 2, 1, KB_8,       3,  7, 1.00f, 1.00f, "8 *", "8" },
    { 2, 1, KB_9,       3,  6, 1.00f, 1.00f, "9 (", "9" },
    { 2, 1, KB_0,       3,  5, 1.00f, 1.00f, "0 )", "0" },
    { 2, 1, KB_MINUS,   2,  5, 1.00f, 1.00f, "- _", "-" },
    { 2, 1, KB_EQUAL,   2,  7, 1.00f, 1.00f, "= +", "=" },
    { 2, 1, KB_BKSPC,   1,  9, 2.00f, 2.00f, "Backspace", "Backspace" },
    { 2, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 2, 1, KB_INS,     2,  3, 1.00f, 1.00f, "Insert", "Insert" },
    { 2, 1, KB_HOME,    2,  1, 1.00f, 1.00f, "Home", "Home" },
    { 2, 1, KB_PGUP,    2,  2, 1.00f, 1.00f, "Page\nUp", "Page Up" },
    { 2, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 2, 1, KB_NUMLK,   6,  4, 1.00f, 1.00f, "Num\nLock", "Num Lock" },
    { 2, 1, KP_SLASH,   6,  3, 1.00f, 1.00f, "/", "Num /" },
    { 2, 1, KP_ASTRX,   6,  2, 1.00f, 1.00f, "*", "Num *" },
    { 2, 1, KP_MINUS,   7,  2, 1.00f, 1.00f, "-", "Num -" },
    // Row 4
    { 3, 1, KB_TAB,     1, 13, 1.50f, 1.50f, "Tab", "Tab" },
    { 3, 1, KB_Q,       4, 13, 1.00f, 1.00f, "Q", "Q" },
    { 3, 1, KB_W,       4, 12, 1.00f, 1.00f, "W", "W" },
    { 3, 1, KB_E,       4, 11, 1.00f, 1.00f, "E", "E" },
    { 3, 1, KB_R,       4, 10, 1.00f, 1.00f, "R", "R" },
    { 3, 1, KB_T,       1, 10, 1.00f, 1.00f, "T", "T" },
    { 3, 1, KB_Y,       1,  8, 1.00f, 1.00f, "Y", "Y" },
    { 3, 1, KB_U,       4,  8, 1.00f, 1.00f, "U", "U" },
    { 3, 1, KB_I,       4,  7, 1.00f, 1.00f, "I", "I" },
    { 3, 1, KB_O,       4,  6, 1.00f, 1.00f, "O", "O" },
    { 3, 1, KB_P,       4,  5, 1.00f, 1.00f, "P", "P" },
    { 3, 1, KB_LBRCE,   1,  5, 1.00f, 1.00f, "[ {", "[" },
    { 3, 1, KB_RBRCE,   1,  7, 1.00f, 1.00f, "] }", "]" },
    { 3, 2, KB_ENTER,   6,  9, 1.50f, 1.25f, "Enter", "Enter" },
    { 3, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 3, 1, KB_DEL,     2,  4, 1.00f, 1.00f, "Delete", "Delete" },
    { 3, 1, KB_END,     3,  1, 1.00f, 1.00f, "End", "End" },
    { 3, 1, KB_PGDN,    3,  2, 1.00f, 1.00f, "Page\nDown", "Page Down" },
    { 3, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 3, 1, KP_7,       4,  4, 1.00f, 1.00f, "7", "Num 7" },
    { 3, 1, KP_8,       4,  3, 1.00f, 1.00f, "8", "Num 8" },
    { 3, 1, KP_9,       4,  2, 1.00f, 1.00f, "9", "Num 9" },
    { 3, 2, KP_PLUS,    4,  1, 1.00f, 1.00f, "+", "Num +" },
    // Row 5
    { 4, 1, KB_CAPLK,   1, 12, 1.75f, 1.75f, "Caps Lock", "Caps Lock" },
    { 4, 1, KB_A,       5, 13, 1.00f, 1.00f, "A", "A" },
    { 4, 1, KB_S,       5, 12, 1.00f, 1.00f, "S", "S" },
    { 4, 1, KB_D,       5, 11, 1.00f, 1.00f, "D", "D" },
    { 4, 1, KB_F,       5, 10, 1.00f, 1.00f, "F", "F" },
    { 4, 1, KB_G,       0, 10, 1.00f, 1.00f, "G", "G" },
    { 4, 1, KB_H,       0,  8, 1.00f, 1.00f, "H", "H" },
    { 4, 1, KB_J,       5,  8, 1.00f, 1.00f, "J", "J" },
    { 4, 1, KB_K,       5,  7, 1.00f, 1.00f, "K", "K" },
    { 4, 1, KB_L,       5,  6, 1.00f, 1.00f, "L", "L" },
    { 4, 1, KB_SMCLN,   5,  5, 1.00f, 1.00f, "; :", ";" },
    { 4, 1, KB_QUOTE,   0,  5, 1.00f, 1.00f, "' \"", "'" },
    { 4, 1, KB_BSLSH,   6,  5, 1.00f, 1.00f, "\\ |", "\\" },
    { 4, 1, -1,        -1, -1, 4.00f, 4.00f, "", "" },
    { 4, 1, KP_4,       1,  4, 1.00f, 1.00f, "4", "Num 4" },
    { 4, 1, KP_5,       1,  3, 1.00f, 1.00f, "5", "Num 5" },
    { 4, 1, KP_6,       1,  2, 1.00f, 1.00f, "6", "Num 6" },
    // Row 6
    { 5, 1, KB_LSHFT,   1, 14, 1.25f, 1.25f, "Shift", "LShift" },
    { 5, 1, KB_PIPE,    0, 12, 1.00f, 1.00f, "\\ |", "\\" },
    { 5, 1, KB_Z,       6, 13, 1.00f, 1.00f, "Z", "Z" },
    { 5, 1, KB_X,       6, 12, 1.00f, 1.00f, "X", "X" },
    { 5, 1, KB_C,       6, 11, 1.00f, 1.00f, "C", "C" },
    { 5, 1, KB_V,       6, 10, 1.00f, 1.00f, "V", "V" },
    { 5, 1, KB_B,       7, 10, 1.00f, 1.00f, "B", "B" },
    { 5, 1, KB_N,       7,  8, 1.00f, 1.00f, "N", "N" },
    { 5, 1, KB_M,       6,  8, 1.00f, 1.00f, "M", "M" },
    { 5, 1, KB_COMMA,   6,  7, 1.00f, 1.00f, ", <", "," },
    { 5, 1, KB_DOT,     6,  6, 1.00f, 1.00f, ". >", "." },
    { 5, 1, KB_SLASH,   7,  5, 1.00f, 1.00f, "/ ?", "/" },
    { 5, 1, KB_RSHFT,   6, 14, 2.75f, 2.75f, "Shift", "RShift" },
    { 5, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 5, 1, KB_UP,      0,  1, 1.00f, 1.00f, "Up", "Up" },
    { 5, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 5, 1, KP_1,       5,  4, 1.00f, 1.00f, "1", "1" },
    { 5, 1, KP_2,       5,  3, 1.00f, 1.00f, "2", "2" },
    { 5, 1, KP_3,       5,  2, 1.00f, 1.00f, "3", "3" },
    { 5, 2, KP_ENTER,   5,  1, 1.00f, 1.00f, "Enter", "Enter" },
    // Row 7
    { 6, 1, KB_LCTRL,   2, 15, 1.50f, 1.50f, "Ctrl", "LCtrl" },
    { 6, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    { 6, 1, KB_LALT,    0,  0, 1.50f, 1.50f, "Alt", "LAlt" },
    { 6, 1, KB_SPACE,   7,  9, 7.00f, 7.00f, "Space", "Space" },
    { 6, 1, KB_RALT,    7,  0, 1.50f, 1.50f, "AltGr", "RAlt" },
    { 6, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    { 6, 1, KB_RCTRL,   6, 15, 1.50f, 1.50f, "Ctrl", "RCtrl" },
    { 6, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 6, 1, KB_LEFT,    7,  1, 1.00f, 1.00f, "Left", "Left" },
    { 6, 1, KB_DOWN,    7,  4, 1.00f, 1.00f, "Down", "Down" },
    { 6, 1, KB_RIGHT,   7,  3, 1.00f, 1.00f, "Right", "Right" },
    { 6, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 6, 1, KP_0,       0,  3, 2.00f, 2.00f, "0", "0" },
    { 6, 1, KP_DOT,     0,  2, 1.00f, 1.00f, ".", "." },
  };

// Model M 122 US ISO Keyboard Default Layout
static GuiKey IsoM122[] =
  {
    // Row 0 - Function Keys F13-F24
    { 0, 1, -1,        -1, -1, 3.50f, 3.50f, "", "" },
    { 0, 1, KB_F13,     0,  7, 1.00f, 1.00f, "F13", "F13" },
    { 0, 1, KB_F14,     1,  7, 1.00f, 1.00f, "F14", "F14" },
    { 0, 1, KB_F15,     1,  8, 1.00f, 1.00f, "F15", "F15" },
    { 0, 1, KB_F16,     2,  7, 1.00f, 1.00f, "F16", "F16" },
    { 0, 1, KB_F17,     3,  7, 1.00f, 1.00f, "F17", "F17" },
    { 0, 1, KB_F18,     3,  8, 1.00f, 1.00f, "F18", "F18" },
    { 0, 1, KB_F19,     4,  7, 1.00f, 1.00f, "F19", "F19" },
    { 0, 1, KB_F20,     5,  7, 1.00f, 1.00f, "F20", "F20" },
    { 0, 1, KB_F21,     5,  8, 1.00f, 1.00f, "F21", "F21" },
    { 0, 1, KB_F22,     6,  7, 1.00f, 1.00f, "F22", "F22" },
    { 0, 1, KB_F23,     7,  7, 1.00f, 1.00f, "F23", "F23" },
    { 0, 1, KB_F24,     7,  8, 1.00f, 1.00f, "F24", "F24" },
    // Row 1
    { 1, 1, -1,        -1, -1, 3.50f, 3.50f, "", "" },
    { 1, 1, KB_F1,      0,  8, 1.00f, 1.00f, "F1", "F1" },
    { 1, 1, KB_F2,      0,  9, 1.00f, 1.00f, "F2", "F2" },
    { 1, 1, KB_F3,      1,  9, 1.00f, 1.00f, "F3", "F3" },
    { 1, 1, KB_F4,      2,  8, 1.00f, 1.00f, "F4", "F4" },
    { 1, 1, KB_F5,      2,  9, 1.00f, 1.00f, "F5", "F5" },
    { 1, 1, KB_F6,      3,  9, 1.00f, 1.00f, "F6", "F6" },
    { 1, 1, KB_F7,      4,  8, 1.00f, 1.00f, "F7", "F7" },
    { 1, 1, KB_F8,      4,  9, 1.00f, 1.00f, "F8", "F8" },
    { 1, 1, KB_F9,      5,  9, 1.00f, 1.00f, "F9", "F5" },
    { 1, 1, KB_F10,     6,  8, 1.00f, 1.00f, "F10", "F10" },
    { 1, 1, KB_F11,     6,  9, 1.00f, 1.00f, "F11", "F11" },
    { 1, 1, KB_F12,     7,  9, 1.00f, 1.00f, "F12", "F12" },
    { 1, 1, -1,        -1, -1, 6.00f, 6.00f, "", "" },
    { 1, 1, LED_NUMLK, -1, -1, 1.34f, 1.34f, "NumLk", "" },
    { 1, 1, LED_CAPLK, -1, -1, 1.34f, 1.34f, "CapsLk", "" },
    { 1, 1, LED_SCRLK, -1, -1, 1.34f, 1.34f, "ScrlLk", "" },
    // Row 2 - spacer
    { 2, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    // Row 3
    { 3, 1, KB_ESC,     3, 19, 1.00f, 1.00f, "Esc", "Esc" },
    { 3, 1, KB_SCRLK,   3,  0, 1.00f, 1.00f, "Scroll\nLock", "Scroll Lock" },
    { 3, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 3, 1, KB_TILDE,   3,  2, 1.00f, 1.00f, "`~", "~" },
    { 3, 1, KB_1,       2,  2, 1.00f, 1.00f, "1 !", "1" },
    { 3, 1, KB_2,       2,  3, 1.00f, 1.00f, "2 @", "2" },
    { 3, 1, KB_3,       2,  4, 1.00f, 1.00f, "3 #", "3" },
    { 3, 1, KB_4,       2,  5, 1.00f, 1.00f, "4 $", "4" },
    { 3, 1, KB_5,       3,  5, 1.00f, 1.00f, "5 %", "5" },
    { 3, 1, KB_6,       3,  6, 1.00f, 1.00f, "6 ^", "6" },
    { 3, 1, KB_7,       2,  6, 1.00f, 1.00f, "7 &", "7" },
    { 3, 1, KB_8,       2, 10, 1.00f, 1.00f, "8 *", "8" },
    { 3, 1, KB_9,       2, 11, 1.00f, 1.00f, "9 (", "9" },
    { 3, 1, KB_0,       2, 12, 1.00f, 1.00f, "0 )", "0" },
    { 3, 1, KB_MINUS,   3, 12, 1.00f, 1.00f, "- _", "-" },
    { 3, 1, KB_EQUAL,   3, 10, 1.00f, 1.00f, "= +", "=" },
    { 3, 1, KB_BKSPC,   3, 13, 2.00f, 2.00f, "Backspace", "Backspace" },
    { 3, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 3, 1, KB_INS,     3, 17, 1.00f, 1.00f, "Insert", "Insert" },
    { 3, 1, KB_HOME,    3, 16, 1.00f, 1.00f, "Home", "Home" },
    { 3, 1, KB_PGUP,    2, 17, 1.00f, 1.00f, "Page\nUp", "Page Up" },
    { 3, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 3, 1, KB_END,     2, 13, 1.00f, 1.00f, "End", "End" },
    { 3, 1, KB_NUMLK,   2, 16, 1.00f, 1.00f, "Num\nLock", "Num Lock" },
    { 3, 1, KP_SLASH,   2, 14, 1.00f, 1.00f, "/", "Num /" },
    { 3, 1, KP_ASTRX,   2, 18, 1.00f, 1.00f, "*", "Num *" },
    // Row 4
    { 4, 1, KB_PSCRN,   2, 19, 1.00f, 1.00f, "Print\nScrn", "Print Screen" },
    { 4, 1, KB_PAUSE,   4, 19, 1.00f, 1.00f, "Pause", "Pause" },
    { 4, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 4, 1, KB_TAB,     4,  0, 1.50f, 1.50f, "Tab", "Tab" },
    { 4, 1, KB_Q,       4,  2, 1.00f, 1.00f, "Q", "Q" },
    { 4, 1, KB_W,       4,  3, 1.00f, 1.00f, "W", "W" },
    { 4, 1, KB_E,       4,  4, 1.00f, 1.00f, "E", "E" },
    { 4, 1, KB_R,       4,  5, 1.00f, 1.00f, "R", "R" },
    { 4, 1, KB_T,       5,  5, 1.00f, 1.00f, "T", "T" },
    { 4, 1, KB_Y,       5,  6, 1.00f, 1.00f, "Y", "Y" },
    { 4, 1, KB_U,       4,  6, 1.00f, 1.00f, "U", "U" },
    { 4, 1, KB_I,       4, 10, 1.00f, 1.00f, "I", "I" },
    { 4, 1, KB_O,       4, 11, 1.00f, 1.00f, "O", "O" },
    { 4, 1, KB_P,       4, 12, 1.00f, 1.00f, "P", "P" },
    { 4, 1, KB_LBRCE,   5, 12, 1.00f, 1.00f, "[ {", "[" },
    { 4, 1, KB_RBRCE,   5, 10, 1.00f, 1.00f, "] }", "]" },
    { 4, 2, KB_ENTER,   6, 13, 1.50f, 1.25f, "Enter", "Enter" },
    { 4, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 4, 1, KB_DEL,     1, 17, 1.00f, 1.00f, "Delete", "Delete" },
    { 4, 1, KB_END,     5, 17, 1.00f, 1.00f, "End", "End" },
    { 4, 1, KB_PGDN,    4, 17, 1.00f, 1.00f, "Page\nDown", "Page Down" },
    { 4, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 4, 1, KP_7,       4, 13, 1.00f, 1.00f, "7", "Num 7" },
    { 4, 1, KP_8,       4, 16, 1.00f, 1.00f, "8", "Num 8" },
    { 4, 1, KP_9,       4, 14, 1.00f, 1.00f, "9", "Num 9" },
    { 4, 1, KP_MINUS,   4, 18, 1.00f, 1.00f, "-", "Num -" },
    // Row 5
    { 5, 1, KB_NONE,    5, 19, 1.00f, 1.00f, "EF5", "EF5" },
    { 5, 1, KB_NONE,    5,  0, 1.00f, 1.00f, "EF6", "EF6" },
    { 5, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 5, 1, KB_CAPLK,   6,  0, 1.75f, 1.75f, "Caps Lock", "Caps Lock" },
    { 5, 1, KB_A,       1,  2, 1.00f, 1.00f, "A", "A" },
    { 5, 1, KB_S,       1,  3, 1.00f, 1.00f, "S", "S" },
    { 5, 1, KB_D,       1,  4, 1.00f, 1.00f, "D", "D" },
    { 5, 1, KB_F,       1,  5, 1.00f, 1.00f, "F", "F" },
    { 5, 1, KB_G,       0,  5, 1.00f, 1.00f, "G", "G" },
    { 5, 1, KB_H,       0,  6, 1.00f, 1.00f, "H", "H" },
    { 5, 1, KB_J,       1,  6, 1.00f, 1.00f, "J", "J" },
    { 5, 1, KB_K,       1, 10, 1.00f, 1.00f, "K", "K" },
    { 5, 1, KB_L,       1, 11, 1.00f, 1.00f, "L", "L" },
    { 5, 1, KB_SMCLN,   1, 12, 1.00f, 1.00f, "; :", ";" },
    { 5, 1, KB_QUOTE,   0, 12, 1.00f, 1.00f, "' \"", "'" },
    { 5, 1, KB_BSLSH,   6, 12, 1.00f, 1.00f, "\\ |", "\\" },
    { 5, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 5, 1, KB_UP,      0, 17, 1.00f, 1.00f, "Up", "Up" },
    { 5, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 5, 1, KP_4,       1, 13, 1.00f, 1.00f, "4", "Num 4" },
    { 5, 1, KP_5,       1, 16, 1.00f, 1.00f, "5", "Num 5" },
    { 5, 1, KP_6,       1, 14, 1.00f, 1.00f, "6", "Num 6" },
    { 5, 1, KP_PLUS,    1, 18, 1.00f, 1.00f, "+", "Num +" },
    // Row 6
    { 6, 1, KB_NONE,    1, 19, 1.00f, 1.00f, "EF7", "EF7" },
    { 6, 1, KB_NONE,    1,  0, 1.00f, 1.00f, "EF8", "EF8" },
    { 6, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 6, 1, KB_LSHFT,   7,  1, 1.25f, 1.25f, "Shift", "LShift" },
    { 6, 1, KB_PIPE,    7,  2, 1.00f, 1.00f, "\\ |", "\\" },
    { 6, 1, KB_Z,       6,  2, 1.00f, 1.00f, "Z", "Z" },
    { 6, 1, KB_X,       6,  3, 1.00f, 1.00f, "X", "X" },
    { 6, 1, KB_C,       6,  4, 1.00f, 1.00f, "C", "C" },
    { 6, 1, KB_V,       6,  5, 1.00f, 1.00f, "V", "V" },
    { 6, 1, KB_B,       7,  5, 1.00f, 1.00f, "B", "B" },
    { 6, 1, KB_N,       7,  6, 1.00f, 1.00f, "N", "N" },
    { 6, 1, KB_M,       6,  6, 1.00f, 1.00f, "M", "M" },
    { 6, 1, KB_COMMA,   6, 10, 1.00f, 1.00f, ", <", "," },
    { 6, 1, KB_DOT,     6, 11, 1.00f, 1.00f, ". >", "." },
    { 6, 1, KB_SLASH,   7, 12, 1.00f, 1.00f, "/ ?", "/" },
    { 6, 1, KB_RSHFT,   6,  1, 2.75f, 2.75f, "Shift", "RShift" },
    { 6, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 6, 1, KB_LEFT,    6, 18, 1.00f, 1.00f, "Left", "Left" },
    { 6, 1, KB_HOME,    0, 15, 1.00f, 1.00f, "Home", "Home" },
    { 6, 1, KB_RIGHT,   1, 15, 1.00f, 1.00f, "Right", "Right" },
    { 6, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 6, 1, KP_1,       6, 19, 1.00f, 1.00f, "1", "1" },
    { 6, 1, KP_2,       6, 16, 1.00f, 1.00f, "2", "2" },
    { 6, 1, KP_3,       6, 14, 1.00f, 1.00f, "3", "3" },
    { 6, 2, KP_ENTER,   7, 15, 1.00f, 1.00f, "Enter", "Enter" },
    // Row 7
    { 7, 1, KB_LGUI,    0,  0, 1.00f, 1.00f, "Win", "LWin" },
    { 7, 1, KB_APP,     0, 19, 1.00f, 1.00f, "Menu", "Menu" },
    { 7, 1, -1,        -1, -1, 0.50f, 0.50f, "", "" },
    { 7, 1, KB_LCTRL,   0, 18, 1.50f, 1.50f, "Ctrl", "LCtrl" },
    { 7, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    { 7, 1, KB_LALT,    7, 18, 1.50f, 1.50f, "Alt", "LAlt" },
    { 7, 1, KB_SPACE,   7, 19, 7.00f, 7.00f, "Space", "Space" },
    { 7, 1, KB_RALT,    0,  1, 1.50f, 1.50f, "AltGr", "RAlt" },
    { 7, 1, -1,        -1, -1, 1.00f, 1.00f, "", "" },
    { 7, 1, KB_RCTRL,   7,  0, 1.50f, 1.50f, "Ctrl", "RCtrl" },
    { 7, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 7, 1, KB_DOWN,    0, 13, 1.00f, 1.00f, "Down", "Down" },
    { 7, 1, -1,        -1, -1, 1.50f, 1.50f, "", "" },
    { 7, 1, KP_0,       7, 16, 2.00f, 2.00f, "0", "0" },
    { 7, 1, KP_DOT,     7, 14, 1.00f, 1.00f, ".", "." },
  };

/*===========================================================================*/
/* KbdLayout class members                                                   */
/*===========================================================================*/

/*****************************************************************************/
/* ReadFile : load contents from a file                                      */
/*****************************************************************************/

bool KbdLayout::ReadFile(wxString const &filename)
{
wxFile f;
if (!f.Open(filename))
  return false;

int tgtrows = NUMROWS, tgtcols = NUMCOLS;
int tgtkeys = tgtrows * tgtcols;
int layers = 0;

wxMemoryBuffer mb;
wxUint8 c;                              /* get first character for filetype  */
size_t frc;
if ((frc = f.Read(&c, sizeof(c))) != sizeof(c))
  return false;
if (c > 0 && c <= NUMLAYERS_MAX)        /* binary?                           */
  {                                     /* same format as dev write          */
  layers = c;
  mb.SetBufSize(8192);
  wxUint8 *buf = (wxUint8 *)mb.GetData();
  *buf++ = c;
  *buf++ = '\0';
  ssize_t rb = f.Read(buf, mb.GetBufSize() - sizeof(wxUint16));
  mb.SetDataLen(sizeof(wxUint16) + rb);
  }
else if (isspace(c) || isdigit(c))
  {                                     /* Joern's text format?              */
  mb.AppendByte('\0');                  /* put empty layer count             */
  mb.AppendByte('\0');
  while (frc == sizeof(c))              /* iterate through the lines         */
    {
    while (isspace(c))                  /* catch leading blanks              */
      {
      c = '\0';
      if ((frc = f.Read(&c, sizeof(c))) != sizeof(c))
        {
        if (!frc)
          break;
        return false;
        }
      }
    wxMemoryBuffer mbLine;              /* read complete line into buffer    */
    while (c != '\r' && c != '\n' && frc == sizeof(c))
      {
      mbLine.AppendByte((char)c);
      c = '\0';
      frc = f.Read(&c, sizeof(c));
      }
    mbLine.AppendByte('\0');
    wxUint16 cc = -1;
    const char *pcs = mbLine;
    int curpos = 0;

    // first, try to determine the format (dec or hex)
    bool hasHex = false, hasDec = false, bCorrFmt = true;
    for (size_t i = 0; bCorrFmt && i < mbLine.GetDataLen(); i++)
      {
      const char c = pcs[i];
      if (isdigit(c))
        hasDec = true;
      else if (isxdigit(c))
        hasHex = true;
      else if (c != ' ' && c != ',' && c != '\0')
        bCorrFmt = false;
      }
    if (!hasDec && !hasHex)
      bCorrFmt = false;
    if (!bCorrFmt)
      return false;
    // Now we know whether there's hex or dec chars in there.
    // While this is NOT enough to determine the file type with
    // absolute certainty, it SHOULD be enough. A layout which only
    // contains hex values that can also be read as decimal is
    // highly unlikely.
    const int mult = hasHex ? 16 : 10;
    for (size_t i = 0; i < mbLine.GetDataLen(); i++)
      {
      if (isdigit(pcs[i]))
        {
        if (cc == (wxUint16)-1)
          cc = 0;
        cc = cc * mult + (pcs[i] - '0');
        }
      else if (isxdigit(pcs[i]))
        {
        // this is A-F | a-f, as 0-9 has already been processed above
        if (cc == (wxUint16)-1)
          cc = 0;
        char c = pcs[i];
        if (c >= 'a' && c <= 'f')
          c -= ('a' - 'A');
        cc = cc * mult + (c - ('A' - 10));
        }
      else if (pcs[i] == ',' || pcs[i] == '\0')
        {
        if (cc != (wxUint16)-1)
          {
          // assure little-endian internal buffer format
          mb.AppendByte(cc & 0xff);
          mb.AppendByte(cc >> 8);
          cc = (wxUint16)-1;
          }
        curpos++;
        if (curpos >= tgtkeys)
          break;
        }
      else if (!isspace(pcs[i]))
        return false;
      }
    if (curpos)                         /* if the line has contents,         */
      {                                 /* assure the layer is complete      */
      for (; curpos < tgtkeys; curpos++)
        {
        mb.AppendByte('\0');
        mb.AppendByte('\0');
        }
      if (++layers >= NUMLAYERS_MAX)    /* stop if we got enough             */
        break;
      }

    while (c == '\r' || c == '\n')      /* prefetch 1st char of next line    */
      {
      c = '\0';
      if ((frc = f.Read(&c, sizeof(c))) != sizeof(c))
        {
        if (!frc)
          break;
        return false;
        }
      }
    }
  *(char *)mb.GetData() = (char)layers; /* store # layers                    */
  }
else
  return false;
                                        /* calculate layout                  */
ssize_t ks = mb.GetDataLen() - sizeof(wxUint16);
if (!layers)                            /* get size of 1 layer               */
  return false;
ssize_t layersize = ks / (ssize_t)layers;
if (ks % layersize)                     /* rb must be a multiple of that!    */
  return false;
// Okay ... we got 2 possible layouts at the moment: 16x8 and 20x8.
// So, to get the target columns, we have to divide the layersize through
// 8*2 (sinze it's 8 rows of wxUint16's).
// This might need to be adjusted if rows can be != 8
tgtcols = layersize / (tgtrows * sizeof(wxUint16));
if ((tgtcols != 16 && tgtcols != 20) ||
    (tgtcols * tgtrows * sizeof(wxUint16) != layersize))
  return false;
ssize_t sz = (ssize_t)layers * tgtrows * tgtcols;
if (ks != sz * sizeof(wxUint16))
  return false;

return Import((wxUint8 *)mb.GetData(), mb.GetDataLen(), tgtrows, tgtcols);
}

/*****************************************************************************/
/* WriteFile write contents to a file                                        */
/*****************************************************************************/

bool KbdLayout::WriteFile
    (
    wxString const &filename,
    bool bNative,
    int tgtrows,
    int tgtcols
    )
{
wxFile f;
if (!f.Open(filename, wxFile::write))
  return false;

if (bNative)                            /* write compact binary              */
  {
  wxMemoryBuffer mb;
  mb.SetBufSize(8192);
  int lbufsz = mb.GetBufSize();
  if (!Export((wxUint8 *)mb.GetData(), lbufsz, tgtrows, tgtcols))
    return false;
  if (f.Write(mb.GetData(), lbufsz) != (size_t)lbufsz)
    return false;
  }
else                                    /* write in Joern's text format      */
  {
  for (int l = 0; l < GetLayers(); l++)
    {
    wxString s;
    for (int r = 0; r < tgtrows; r++)
      for (int c = 0; c < tgtcols; c++)
        {
        if (s.size())
          s += wxT(", ");
#if 1
        // Joern switched from decimal to hexadecimal format in V1.5.
        // As I don't want to include a special switch here, just go with that.
        s += wxString::Format("%X", GetKey(l, r, c));
#else
        s += wxString::Format("%d", GetKey(l, r, c));
#endif
        }
    s += (l < GetLayers() - 1) ? wxT("\n\n") : wxT("\n");
    const char *ps = (const char *)s;
    size_t ls = s.size();
    if (f.Write(ps, ls) != ls)
      return false;
    }
  }

bool bOK = f.Close();
if (bOK)
  SetModified(false);
return bOK;
}

/*===========================================================================*/
/* KbdGui class members                                                      */
/*===========================================================================*/

GuiKey *KbdGui::pDefault = AnsiM;
int KbdGui::nDefault = _countof(AnsiM);
wxString KbdGui::sDefault = wxT("101 Key ANSI");

/*****************************************************************************/
/* KbdGui : constructor                                                      */
/*****************************************************************************/

KbdGui::KbdGui(int kbdStyle)
{
int i;

switch (kbdStyle)
  {
  case KbdANSI :
    layoutName = wxT("101 Key ANSI");
    for (i = 0; i < _countof(AnsiM); i++)
      keys.push_back(AnsiM[i]);
    break;
  case KbdISO :
    layoutName = wxT("102 Key ISO");
    for (i = 0; i < _countof(IsoM); i++)
      keys.push_back(IsoM[i]);
    break;
  case KbdANSI121 :
    layoutName = wxT("121 Key ANSI");
    for (i = 0; i < _countof(AnsiM121); i++)
      keys.push_back(AnsiM121[i]);
    break;
  case KbdISO122 :
    layoutName = wxT("122 Key ISO");
    for (i = 0; i < _countof(IsoM122); i++)
      keys.push_back(IsoM122[i]);
    break;
  default :
    layoutName = sDefault;
    for (i = 0; i < nDefault; i++)
      keys.push_back(pDefault[i]);
    break;
  }
CalcLayout();
}

/*****************************************************************************/
/* ~KbdGui : destructor                                                      */
/*****************************************************************************/

KbdGui::~KbdGui()
{
keys.clear();
}

/*****************************************************************************/
/* DoCopy : copy another keyboard layout                                     */
/*****************************************************************************/

KbdGui &KbdGui::DoCopy(KbdGui const &org)
{
layoutName = org.layoutName;
keys.assign(org.keys.begin(), org.keys.end());
CalcLayout();
return *this;
}

/*****************************************************************************/
/* SetDefault : sets GUI layout to ANSI or ISO default                       */
/*****************************************************************************/

void KbdGui::SetDefault(bool bISO, bool b122)
{
if (bISO && !b122)
  {
  pDefault = IsoM;
  nDefault = _countof(IsoM);
  sDefault = wxT("102 Key ISO");
  }
else if (bISO && b122)
  {
  pDefault = IsoM122;
  nDefault = _countof(IsoM122);
  sDefault = wxT("122 Key ISO");
  }
else if (!bISO && !b122)
  {
  pDefault = AnsiM;
  nDefault = _countof(AnsiM);
  sDefault = wxT("101 Key ANSI");
  }
else
  {
  pDefault = AnsiM121;
  nDefault = _countof(AnsiM121);
  sDefault = wxT("121 Key ANSI");
  }
}

/*****************************************************************************/
/* GetHID : convert OS-defined Virtual Key into HID                          */
/*****************************************************************************/

int KbdGui::GetHID(int vkey)
{
static int vkhids[][2] =
  {
    { WXK_BACK,             KB_BKSPC },
    { WXK_TAB,              KB_TAB },
    { WXK_RETURN,           KB_ENTER },
    { WXK_ESCAPE,           KB_ESC },
    { WXK_SPACE,            KB_SPACE },
    { WXK_DELETE,           KB_DEL },
    { '0',                  KB_0 },
    { '1',                  KB_1 },
    { '2',                  KB_2 },
    { '3',                  KB_3 },
    { '4',                  KB_4 },
    { '5',                  KB_5 },
    { '6',                  KB_6 },
    { '7',                  KB_7 },
    { '8',                  KB_8 },
    { '9',                  KB_9 },
    { 'A',                  KB_A },
    { 'B',                  KB_B },
    { 'C',                  KB_C },
    { 'D',                  KB_D },
    { 'E',                  KB_E },
    { 'F',                  KB_F },
    { 'G',                  KB_G },
    { 'H',                  KB_H },
    { 'I',                  KB_I },
    { 'J',                  KB_J },
    { 'K',                  KB_K },
    { 'L',                  KB_L },
    { 'M',                  KB_M },
    { 'N',                  KB_N },
    { 'O',                  KB_O },
    { 'P',                  KB_P },
    { 'Q',                  KB_Q },
    { 'R',                  KB_R },
    { 'S',                  KB_S },
    { 'T',                  KB_T },
    { 'U',                  KB_U },
    { 'V',                  KB_V },
    { 'W',                  KB_W },
    { 'X',                  KB_X },
    { 'Y',                  KB_Y },
    { 'Z',                  KB_Z },
    { '~',                  KB_TILDE },      // unsure
    { '-',                  KB_MINUS },      // unsure
    { '=',                  KB_EQUAL },      // unsure
    { '[',                  KB_LBRCE },      // unsure
    { ']',                  KB_RBRCE },      // unsure
    { '\\',                 KB_BSLSH },      // unsure
    { ';',                  KB_SMCLN },      // unsure
    { '\'',                 KB_QUOTE },      // unsure
    { '<',                  KB_PIPE },       // unsure
    { ',',                  KB_COMMA },      // unsure
    { '.',                  KB_DOT },        // unsure
    { '/',                  KB_SLASH },      // unsure
    { '#',                  KB_NUMBER },     // unsure
    { WXK_ALT,              KB_ALT },        // wxWindows doesn't give left/right alt
    { WXK_CONTROL,          KB_CTRL },       // wxWindows doesn't give left/right control
    { WXK_MENU,             KB_MENU },
    { WXK_PAUSE,            KB_PAUSE },
    { WXK_CAPITAL,          KB_CAPLK },
    { WXK_END,              KB_END },
    { WXK_HOME,             KB_HOME },
    { WXK_LEFT,             KB_LEFT },
    { WXK_UP,               KB_UP },
    { WXK_RIGHT,            KB_RIGHT },
    { WXK_DOWN,             KB_DOWN },
    { WXK_SELECT,           KP_6 },          // unsure
    { WXK_PRINT,            KB_PSCRN },
    { WXK_EXECUTE,          KB_EXECUTE },
    { WXK_SNAPSHOT,         KB_NONE },       // unsure
    { WXK_INSERT,           KB_INS },
    { WXK_HELP,             KB_HELP },
    { WXK_NUMPAD0,          KP_0 },
    { WXK_NUMPAD1,          KP_1 },
    { WXK_NUMPAD2,          KP_2 },
    { WXK_NUMPAD3,          KP_3 },
    { WXK_NUMPAD4,          KP_4 },
    { WXK_NUMPAD5,          KP_5 },
    { WXK_NUMPAD6,          KP_6 },
    { WXK_NUMPAD7,          KP_7 },
    { WXK_NUMPAD8,          KP_8 },
    { WXK_NUMPAD9,          KP_9 },
    { WXK_MULTIPLY,         KP_ASTRX },      // unsure
    { WXK_ADD,              KP_PLUS },       // unsure
    { WXK_SEPARATOR,        KB_SEPARATOR },  // unsure
    { WXK_SUBTRACT,         KP_MINUS },      // unsure
//  { WXK_DECIMAL,          KB_DECIMAL_SEPARATOR }, // unsure
    { WXK_DECIMAL,          KP_DOT },        // unsure
    { WXK_F1,               KB_F1 },
    { WXK_F2,               KB_F2 },
    { WXK_F3,               KB_F3 },
    { WXK_F4,               KB_F4 },
    { WXK_F5,               KB_F5 },
    { WXK_F6,               KB_F6 },
    { WXK_F7,               KB_F7 },
    { WXK_F8,               KB_F8 },
    { WXK_F9,               KB_F9 },
    { WXK_F10,              KB_F10 },
    { WXK_F11,              KB_F11 },
    { WXK_F12,              KB_F12 },
    { WXK_F13,              KB_F13 },
    { WXK_F14,              KB_F14 },
    { WXK_F15,              KB_F15 },
    { WXK_F16,              KB_F16 },
    { WXK_F17,              KB_F17 },
    { WXK_F18,              KB_F18 },
    { WXK_F19,              KB_F19 },
    { WXK_F20,              KB_F20 },
    { WXK_F21,              KB_F21 },
    { WXK_F22,              KB_F22 },
    { WXK_F23,              KB_F23 },
    { WXK_F24,              KB_F24 },
    { WXK_NUMLOCK,          KB_NUMLK },
    { WXK_SCROLL,           KB_SCRLK },
    { WXK_PAGEUP,           KB_PGUP },
    { WXK_PAGEDOWN,         KB_PGDN },
    { WXK_NUMPAD_SPACE,     KP_SPACE },      // unsure
    { WXK_NUMPAD_TAB,       KP_TAB },        // unsure
    { WXK_NUMPAD_ENTER,     KP_ENTER },      // unsure
    { WXK_NUMPAD_F1,        KB_NONE },       // unsure
    { WXK_NUMPAD_F2,        KB_NONE },       // unsure
    { WXK_NUMPAD_F3,        KB_NONE },       // unsure
    { WXK_NUMPAD_F4,        KB_NONE },       // unsure
    { WXK_NUMPAD_HOME,      KB_HOME },       // unsure
    { WXK_NUMPAD_LEFT,      KB_LEFT },       // unsure
    { WXK_NUMPAD_UP,        KB_UP },         // unsure
    { WXK_NUMPAD_RIGHT,     KB_RIGHT },      // unsure
    { WXK_NUMPAD_DOWN,      KB_DOWN },       // unsure
    { WXK_NUMPAD_PAGEUP,    KB_PGUP },       // unsure
    { WXK_NUMPAD_PAGEDOWN,  KB_PGDN },       // unsure
    { WXK_NUMPAD_END,       KB_END },        // unsure
    { WXK_NUMPAD_BEGIN,     KB_NONE },       // unsure
    { WXK_NUMPAD_INSERT,    KB_INS },        // unsure
    { WXK_NUMPAD_DELETE,    KB_DEL },        // unsure
    { WXK_NUMPAD_EQUAL,     KP_EQUAL },      // unsure
    { WXK_NUMPAD_MULTIPLY,  KP_ASTRX },      // unsure
    { WXK_NUMPAD_ADD,       KP_PLUS },       // unsure
    { WXK_NUMPAD_SEPARATOR, KB_SEPARATOR },  // unsure
    { WXK_NUMPAD_SUBTRACT,  KP_MINUS },      // unsure
    { WXK_NUMPAD_DECIMAL,   KB_INTL6 },      // unsure
    { WXK_NUMPAD_DIVIDE,    KP_SLASH },      // unsure
    { WXK_WINDOWS_LEFT,     KB_LGUI },       // only on Windows
    { WXK_WINDOWS_RIGHT,    KB_RGUI },       // only on Windows
    { WXK_WINDOWS_MENU,     KB_APP },        // only on Windows
#ifdef __WXOSX__
    { WXK_RAW_CONTROL,      KB_CTRL },       // wxWindows doesn't give left/right ctrl
#endif
    { WXK_SPECIAL1,         KB_NONE },
    { WXK_SPECIAL2,         KB_NONE },
    { WXK_SPECIAL3,         KB_NONE },
    { WXK_SPECIAL4,         KB_NONE },
    { WXK_SPECIAL5,         KB_NONE },
    { WXK_SPECIAL6,         KB_NONE },
    { WXK_SPECIAL7,         KB_NONE },
    { WXK_SPECIAL8,         KB_NONE },
    { WXK_SPECIAL9,         KB_NONE },
    { WXK_SPECIAL10,        KB_NONE },
    { WXK_SPECIAL11,        KB_NONE },
    { WXK_SPECIAL12,        KB_NONE },
    { WXK_SPECIAL13,        KB_NONE },
    { WXK_SPECIAL14,        KB_NONE },
    { WXK_SPECIAL15,        KB_NONE },
    { WXK_SPECIAL16,        KB_NONE },
    { WXK_SPECIAL17,        KB_NONE },
    { WXK_SPECIAL18,        KB_NONE },
    { WXK_SPECIAL19,        KB_NONE },
    { WXK_SPECIAL20,        KB_NONE },
#if 1
    // would need an extended HID usage table
    { WXK_BROWSER_BACK,     MEDIA_BACK },
    { WXK_BROWSER_FORWARD,  MEDIA_FORWARD },
    { WXK_BROWSER_REFRESH,  MEDIA_REFRESH },
//  { WXK_BROWSER_STOP,     KB_MEDIA_WWW_STOP },  // it's either this or WXK_MEDIA_STOP below
    { WXK_BROWSER_SEARCH,   MEDIA_SEARCH },
//  { WXK_BROWSER_FAVORITES,KB_MEDIA_WWW_FAVORITES },  // what?
    { WXK_BROWSER_HOME,     MEDIA_HOME },
    { WXK_VOLUME_MUTE,      MEDIA_MUTE },       // unsure
    { WXK_VOLUME_DOWN,      MEDIA_VOLDOWN }, // unsure
    { WXK_VOLUME_UP,        MEDIA_VOLUP },   // unsure
    { WXK_MEDIA_NEXT_TRACK, MEDIA_NEXT },
    { WXK_MEDIA_PREV_TRACK, MEDIA_PREVIOUS },
    { WXK_MEDIA_STOP,       MEDIA_STOP },
    { WXK_MEDIA_PLAY_PAUSE, MEDIA_PLAY },  // what's MEDIA_PAUSE then?
    { WXK_LAUNCH_MAIL,      MEDIA_EMAIL },
    { WXK_LAUNCH_APP1,      MEDIA_CALC }, // unsure
    { WXK_LAUNCH_APP2,      MEDIA_BROWSER }, // unsure
#endif
    // things wxWidgets doesn't originally offer ...
    { WXK_LSHIFT,           KB_LSHFT },
    { WXK_RSHIFT,           KB_RSHFT },
    { WXK_LCONTROL,         KB_LCTRL },
    { WXK_RCONTROL,         KB_RCTRL },
    { WXK_LMENU,            KB_LALT },
    { WXK_RMENU,            KB_RALT },

  };
static int vktbl[WXK_NUMCODES] = { -1 };
if (vktbl[0] == -1)                     /* initialize VKey table only once   */
  {
  int i;
  for (i = 0; i < _countof(vktbl); i++)
    vktbl[i] = KB_NONE;
  for (i = 0; i < _countof(vkhids); i++)
    vktbl[vkhids[i][0]] = vkhids[i][1];
  }

int hid = vktbl[vkey];
#if defined(wxHAS_RAW_KEY_CODES) && defined(__WXMSW__)
switch (hid)
  {
  case WXK_SHIFT :
  case WXK_CONTROL :
  case WXK_ALT :
    {
    bool left = false, right = false;
    int skcleft, skcright;
#ifdef __WXMSW__
    static struct
      {
      int vkleft, vkright;
      int skcleft, skcright;
      } defs[3] =
      {
        { VK_LSHIFT,   VK_RSHIFT,    WXK_LSHIFT,   WXK_RSHIFT },
        { VK_LCONTROL, VK_RCONTROL,  WXK_LCONTROL, WXK_RCONTROL },
        { VK_LMENU,    VK_RMENU,     WXK_LMENU,    WXK_RMENU },
      };
    int specidx = (hid == WXK_SHIFT) ? 0 :
                  (hid == WXK_CONTROL) ? 1 :
                  2;
    skcleft = defs[specidx].skcleft;
    skcright = defs[specidx].skcright;
    left = GetKeyState(defs[specidx].vkleft) < 0;
    right = GetKeyState(defs[specidx].vkright) < 0;
#endif
    if (left ^ right)                   /* if one of them is set,            */
      hid = left ? skcleft : skcright;  /* use special code instead          */
    }
    break;
  }
#endif

return hid;
}

/*****************************************************************************/
/* GetHIDFromScancode : retrieve HID from scan code (if possible)            */
/*****************************************************************************/

int KbdGui::GetHIDFromScancode(int scancode)
{
static int schids[][2] =
  {
    { 0x000, KB_NONE },
#if defined(wxHAS_RAW_KEY_CODES) && defined(__WXMSW__)
    // normal keys are in range 0x000..0x0FF
    // extended keys are in range 0x100..0x1FF
    // Keys that don't have a defined Windows scan code equivalent:
    // normal keys are in range 0x200..0x2FF
    // extended keys are in range 0x300..0x3FF
    { 0x043, KB_F9 },
    { 0x03F, KB_F5 },
    { 0x03D, KB_F3 },
    { 0x03B, KB_F1 },
    { 0x03C, KB_F2 },
    { 0x058, KB_F12 },
    { 0x208, KB_F13 },
    { 0x064, KB_F13 },
    { 0x044, KB_F10 },
    { 0x042, KB_F8 },
    { 0x040, KB_F6 },
    { 0x03E, KB_F4 },
    { 0x00F, KB_TAB },
    { 0x029, KB_TILDE },
    { 0x20F, KP_EQUAL },
    { 0x210, KB_F14 },
    { 0x065, KB_F14 },
    { 0x038, KB_LALT },
    { 0x02A, KB_LSHFT },
    { 0x213, KB_INTERNATIONAL2 },
    { 0x01D, KB_LCTRL },
    { 0x010, KB_Q },
    { 0x002, KB_1 },
    { 0x217, KP_00 },   // unsure
    { 0x218, KB_F15 },
    { 0x066, KB_F15 },
    { 0x219, KP_000 },  // unsure
    { 0x02C, KB_Z },
    { 0x01F, KB_S },
    { 0x01E, KB_A },
    { 0x011, KB_W },
    { 0x003, KB_2 },
    { 0x21F, KB_THOUSANDS_SEPARATOR },  // unsure
    { 0x220, KB_F16 },
    { 0x067, KB_F16 },
    { 0x02E, KB_C },
    { 0x02D, KB_X },
    { 0x020, KB_D },
    { 0x012, KB_E },
    { 0x005, KB_4 },
    { 0x004, KB_3 },
    { 0x227, KB_INTERNATIONAL6 },
    { 0x228, KB_F17 },
    { 0x068, KB_F17 },
    { 0x039, KB_SPACE },
    { 0x02F, KB_V },
    { 0x021, KB_F },
    { 0x014, KB_T },
    { 0x013, KB_R },
    { 0x006, KB_5 },
    { 0x22F, KP_RIGHT_PAREN },  // unsure - 122-key left hand side F1
    { 0x230, KB_F18 },
    { 0x069, KB_F18 },
    { 0x031, KB_N },
    { 0x030, KB_B },
    { 0x023, KB_H },
    { 0x022, KB_G },
    { 0x015, KB_Y },
    { 0x007, KB_6 },
    { 0x237, KP_LEFT_BRACE },  // unsure - 122-key left hand side F2
    { 0x238, KB_F19 },
    { 0x06A, KB_F19 },
    { 0x239, KB_DECIMAL_SEPARATOR },  // unsure - AT-F extra pad rhs of space
    { 0x032, KB_M },
    { 0x024, KB_J },
    { 0x016, KB_U },
    { 0x008, KB_7 },
    { 0x009, KB_8 },
    { 0x23F, KP_RIGHT_BRACE },  // unsure - 122-key left hand side F3
    { 0x240, KB_F20 },
    { 0x06B, KB_F20 },
    { 0x033, KB_COMMA },
    { 0x025, KB_K },
    { 0x017, KB_I },
    { 0x018, KB_O },
    { 0x00B, KB_0 },
    { 0x00A, KB_9 },
    { 0x247, KP_TAB },  // unsure - 122-key left hand side F4
    { 0x248, KB_F21 },
    { 0x06C, KB_F21 },
    { 0x034, KB_DOT },
    { 0x035, KB_SLASH },
    { 0x026, KB_L },
    { 0x027, KB_SMCLN },
    { 0x019, KB_P },
    { 0x00C, KB_MINUS },
    { 0x24F, KP_BACKSPACE },  // unsure - 122-key left hand side F5
    { 0x250, KB_F22 },
    { 0x06D, KB_F22 },
    { 0x251, KB_INTERNATIONAL1 },
    { 0x028, KB_QUOTE },
    { 0x253, KB_CURRENCY_UNIT },  // unsure - AT-F extra pad lhs of enter
    { 0x01A, KB_LBRCE },
    { 0x00D, KB_EQUAL },
    { 0x256, KP_A },  // unsure - 122-key left hand side F6
    { 0x257, KB_F23 },
    { 0x06E, KB_F23 },
    { 0x03A, KB_CAPLK },
    { 0x036, KB_RSHFT },
    { 0x01C, KB_ENTER },
    { 0x01B, KB_RBRCE },
    { 0x25C, KB_CURRENCY_SUBUNIT },  // unsure - AT-F extra pad top of enter
    { 0x02B, KB_BSLSH },
    { 0x25E, KP_B },  // unsure - 122-key left hand side F7
    { 0x25F, KB_F24 },
    // { 0x06F, KB_F24 },
    { 0x076, KB_F24 },
    { 0x260, KP_C },  // unsure - 122-key left hand side F8
    { 0x056, KB_PIPE },
    { 0x262, KB_LANG4 },
    { 0x263, KB_LANG3 },
    { 0x264, KB_INTERNATIONAL4 },
    { 0x265, KP_D },  // unsure - 122-key left hand side F9
    { 0x00E, KB_BKSPC },
    { 0x267, KB_INTERNATIONAL5 },
    { 0x268, KP_LEFT_PAREN },  // unsure - AT-F extra pad lhs of Insert
    { 0x04F, KP_1 },
    { 0x26A, KB_INTERNATIONAL4 },
    { 0x04B, KP_4 },
    { 0x047, KP_7 },
    { 0x26D, KP_COMMA },
    { 0x26E, KP_E },  // unsure - 122-key left hand side F10
    { 0x26F, KB_NONE },  // in Soarer's Converter, this is "KEY_FAKE_18" (an internal one)
    { 0x052, KP_0 },
    { 0x053, KP_DOT  },
    { 0x050, KP_2 },
    { 0x04C, KP_5 },
    { 0x04D, KP_6 },
    { 0x048, KP_8 },
    { 0x001, KB_ESC },
    { 0x145, KB_NUMLK },
    { 0x057, KB_F11 },
    { 0x04E, KP_PLUS },
    { 0x051, KP_3 },
    { 0x04A, KP_MINUS },
    { 0x037, KP_ASTRX },
    { 0x049, KP_9 },
    { 0x046, KB_SCRLK },
    { 0x041, KB_F7 },
    { 0x284, KP_XOR },  // unsure - Sys Req (AT 84-key)
    { 0x138, KB_RALT },
    { 0x11D, KB_RCTRL },
    { 0x15B, KB_LGUI },
    { 0x15C, KB_RGUI },
    { 0x15D, KB_APP },
    { 0x12E, KB_VOLUMEDOWN },  // unsure - it's KB_MEDIA_VOLUME_DN in Soarer's Converter
    { 0x120, KB_MUTE },  // unsure - it's KB_MEDIA_MUTE in Soarer's Converter
    { 0x130, KB_VOLUMEUP },  // unsure - it's KB_MEDIA_VOLUME_UP in Soarer's Converter
    { 0x135, KP_SLASH },
    { 0x15E, KB_POWER },
//  { 0x15F, 169 },  // that would be SYSTEM_SLEEP
//  { 0x163, 170 },  // that would be SYSTEM_WAKE in Soarer's Converter
    { 0x11C, KP_ENTER },
    { 0x14F, KB_END },
    { 0x14B, KB_LEFT },
    { 0x147, KB_HOME },
    { 0x152, KB_INS },
    { 0x153, KB_DEL },
    { 0x150, KB_DOWN },
    { 0x14D, KB_RIGHT },
    { 0x148, KB_UP },
    { 0x151, KB_PGDN },
    { 0x137, KB_PSCRN },  // Make only received in Raw Input, Break also in WM_KEYUP
    { 0x054, KB_PSCRN },  // That's Alt+Print (VK_SNAPSHOT)
    { 0x149, KB_PGUP },
    { 0x045, KB_PAUSE },
    { 0x146, KB_PAUSE },  // Special for Ctrl+Break
#if 1
// would need an extended HID usage table
    // extended keys:
    { 0x310, MEDIA_SEARCH },
    { 0x315, MEDIA_PREVIOUS },
//  { 0x318, KB_MEDIA_WWW_FAVORITES },  // what?
    { 0x320, MEDIA_REFRESH },
//  { 0x328, KB_MEDIA_WWW_STOP },    // it's either this or 0x124 below
    { 0x32B, MEDIA_CALC },
//  { 0x32F, KB_MENU }, // what's that got to do here?
    { 0x330, MEDIA_FORWARD },
    { 0x122, MEDIA_PLAY },   // what's MEDIA_PAUSE then?
    { 0x338, MEDIA_BACK },
    { 0x33A, MEDIA_HOME },
    { 0x124, MEDIA_STOP },
    { 0x340, MEDIA_BROWSER },  // unsure
    { 0x348, MEDIA_EMAIL },
    { 0x34D, MEDIA_NEXT },
//  { 0x350, KB_MEDIA_MEDIA_SELECT },
#endif
#endif
  };

static int sctbl[512] = { -1 };
if (sctbl[0] == -1)                     /* initialize scan code table once   */
  {
  int i;
  for (i = 0; i < _countof(sctbl); i++)
    sctbl[i] = KB_NONE;
  for (i = 0; i < _countof(schids); i++)
    {
    if (schids[i][0] < _countof(sctbl))
      sctbl[schids[i][0]] = schids[i][1];
    }
  }

int kc = sctbl[scancode];
return kc;
}

/******************************************************************************
GUI Layout file syntax (adapted from EK Switch Hitter):

block comments are enclosed in / * and * /
comment( line)s start with # or / /
empty lines are allowed
First non-comment line defines the layout name
Rrow[-row2], [hidcode]@[mtxrow/mtxcol], ["Text" [, "TextLog" [, width[xheight][-width2] [,align]]]]

Row can be any number in [0..maxint], as long as
  (a) following rows have a higher number and
  (b) all items in a row have the same number.

Row2, if given, indicates a multiline key. Currently, this value MUST be row+1
if given (i.e., 2 rows is the maximum height of a key).
If omitted, it is assumed to be row.

Either hidcode or matrix pos must be given; @ is only needed if both are.
If it's a blank space, "BL" is also accepted (treated like -1@-1/-1).
hidcode,mtxrow,mtxcol can be given as a hex value by preceding them with 0x.
Special hidcodes:
  0xfe00 = Num Lock LED,
  0xfe01 = Caps Lock LED,
  0xfe02 = Scroll Lock LED

Text and TextLog must be enclosed in ""s.
If omitted, tey are assumed to be "".

Width, if omitted, is assumed to be 1.

Width2, if omitted and necessary, is assumed to be the same as width.

Height only has to be specified if the key height is < 1.
If omitted, it's row2-row+1.

Align can be used to set the vertical alignment of keys with a height < 1
in the row. Possible values: Top (default), Bottom (both can be abbreviated).
Bottom-aligned buttons force the row height to 1.

Special characters can be escaped by preceding them with \. The following
special characters are defined:
  \r ... carriage return in text
  \n ... newline in text
  \t ... tabulator in text
  All other sequences of \-followed-by-another char drop the \; notably,
  \# ... which would otherwise start a comment
  \" ... which would otherwise terminate a text

******************************************************************************/

static wxString EscToken(wxString const &s)
{
wxString o;

for (size_t i = 0; i < s.size(); i++)
  {
  wxChar c = s[i];
  switch (c)
    {
    case wxT('\n') :
      c = wxT('n'); o += wxT('\\');
      break;
    case wxT('\r') :
      c = wxT('r'); o += wxT('\\');
      break;
    case wxT('\t') :
      c = wxT('t'); o += wxT('\\');
      break;
    case wxT('\\') :
    case wxT('\"') :
      o +=  wxT('\\');
      break;
    }
  o += c;
  }
return o;
}

static wxString UnescToken(wxString const &s)
{
wxString o;

for (size_t i = 0; i < s.size(); i++)
  {
  wxChar c = s[i];
  if (c == wxT('\\') &&
      i != s.size() - 1)
    {
    char c2 = s[++i];
    switch (c2)
      {
      case wxT('n') :
        c = wxT('\n');
        break;
      case wxT('r') :
        c = wxT('\r');
        break;
      case wxT('t') :
        c = wxT('\t');
        break;
      default :
        c = c2;
        break;
      }
    }
  o += c;
  }

return o;
}

static bool TokenizeText
    (
    wxString &s,
    wxArrayString &sa,
    bool &inBlockComment
    )
{
wxString sUncommented, sCur;
bool inText = false;

// walk through line, splitting at , (unless in ""s),
// terminate at end or # or // (unless in ""s)
for (size_t i = 0; i < s.size(); i++)
  {
  wxChar c = s[i];
  switch (c)
    {
    case wxT(',') :
      if (!inBlockComment)
        {
        if (!inText)
          {
          sCur.Trim(false); sCur.Trim(true);
          sa.push_back(sCur);
          sCur.clear();
          }
        else
          sCur += c;
        sUncommented += c;
        }
      break;
    case wxT('#') :
      if (!inBlockComment)
        {
        if (!inText)
          i = s.size();
        else
          {
          sCur += c;
          sUncommented += c;
          }
        }
      break;
    case wxT('/') :
      if (!inBlockComment && !inText && i < s.size() - 1 && s[i + 1] == wxT('/'))
        i = s.size();
      else if (!inText && i < s.size() - 1 && s[i + 1] == wxT('*'))
        inBlockComment = true;
      else if (!inBlockComment)
        {
        sCur += c;
        sUncommented += c;
        }
      break;
    case wxT('*') :
      if (!inText && inBlockComment && i < s.size() - 1 && s[i + 1] == wxT('/'))
        {
        i++;
        inBlockComment = false;
        }
      else if (!inBlockComment)
        {
        sCur += c;
        sUncommented += c;
        }
      break;
    case wxT('\\') :
      if (!inBlockComment)
        {
        // \ followed by anything is copied 1:1
        if (i < s.size() - 1)
          {
          sCur += c;
          sUncommented += c;
          c = s[++i];
          }
        sCur += c;
        sUncommented += c;
        }
      break;
    case wxT('\"') :
      if (!inBlockComment)
        {
        inText = !inText;
        sCur += c;
        sUncommented += c;
        }
      break;
    default :
      if (!inBlockComment)
        {
        sCur += c;
        sUncommented += c;
        }
      break;
    }
  }
// last token
sCur.Trim(false); sCur.Trim(true);
sa.push_back(sCur);
sUncommented.Trim(false);sUncommented.Trim(true);
s = sUncommented;

return !inText;
}

/*****************************************************************************/
/* ReadLayoutFile : read keyboard GUI layout file                            */
/*****************************************************************************/

bool KbdGui::ReadLayoutFile(wxString const &filename, wxString *error)
{
wxTextFile f;
if (!f.Open(filename))
  {
  if (error)
    *error = wxT("Error opening ") + filename;
  return false;
  }

wxString lname;                         /* layout name                       */
bool gotname = false;
bool inBlockComment = false;
wxVector<GuiKey> fileKeys;

size_t i;

int line = 0;                           /* loop through the file contents    */
wxString sCurLine;
for (wxString s = f.GetFirstLine(); !f.Eof(); s = f.GetNextLine())
  {
  line++;
  sCurLine = s;

  s.Trim(false); s.Trim(true);
  if (s.IsEmpty())
    continue;

                                        /* the others are key definitions    */
  GuiKey key = { 0, 0, -1, -1, -1, 1.f, 1.f, "", "" };
  wxArrayString sa;
  if (!TokenizeText(s, sa, inBlockComment))
    break;                              /* stop at badly formatted lines     */

  if (!gotname && s.size())             /* first line is the layout name.    */
    {
    lname = s;
    gotname = true;
    continue;
    }

  if (s.IsEmpty())                      /* ignore empty content              */
    continue;

  if (sa.size() < 2)                    /* at least row, matrix, text must be*/
    break;

  // parse Row: rA[-B]
  wxString row(sa[0]);
  if (row.size() < 2 || (row[0] != wxT('R') && row[0] != wxT('r')))
    break;
  row = row.substr(1);
  int delimpos = -1, delim2pos = -1;
  for (i = 0; i < row.size(); i++)
    {
    wxChar c = row[i];
    if (c == wxT('-'))
      {
      if (delimpos >= 0)
        break;
      delimpos = (int)i;
      }
    else if (c < wxT('0') || c > wxT('9'))
      break;
    }
  if (i < row.size())
    break;
  long l;
  if (delimpos >= 0)
    {
    if (delimpos < 1 || !row.substr(0, delimpos).ToCLong(&l))
      break;
    key.row = l;
    if (!row.substr(delimpos + 1).ToCLong(&l))
      break;
    if (l < key.row ||
        l > key.row + 1)
      break;
    key.height = l - key.row + 1;
    }
  else
    {
    if (!row.ToCLong(&l))
      break;
    key.row = l;
    key.height = 1;
    }

  // parse [hidcode]@[matrixpos A/B]
  wxString mtx(sa[1]);
  if (mtx.CmpNoCase(wxT("bl")))
    {
    delimpos = -1;
    delim2pos = -1;
    for (i = 0; i < mtx.size(); i++)
      {
      wxChar c = mtx[i];
      if (c == wxT('@'))
        {
        if (delimpos >= 0)
          break;
        delimpos = (int)i;
        }
      else if (c == wxT('/'))
        {
        if (delim2pos >= 0)
          break;
        delim2pos = (int)i;
        }
      else if (c == wxT('x') ||
               c == wxT('a') || c == wxT('A') ||
               c == wxT('b') || c == wxT('B') ||
               c == wxT('c') || c == wxT('C') ||
               c == wxT('d') || c == wxT('D') ||
               c == wxT('e') || c == wxT('E') ||
               c == wxT('f') || c == wxT('F') )
        continue;  // deal with "0x" stuff later
      else if (c != wxT('-') && (c < wxT('0') || c > wxT('9')))
        break;
      }
    if (mtx.size() == 0 || i < mtx.size())
      break;
    if (delimpos == 0 && mtx.size() == 1)  // only @ is not allowed
      break;
    wxString scan;
    if (delimpos < 0 && delim2pos < 0)  // no delimiter - only scan code
      {
      scan = mtx;
      mtx.clear();
      }
    else if (delimpos >= 0)
      {
      scan = mtx.substr(0, delimpos);
      mtx = mtx.substr(delimpos + 1);
      delim2pos -= delimpos + 1;
      }
    if (scan.size())
      {
      int base = 10;
      if (scan.substr(0, 2) == wxT("0x"))
        {
        base = 16;
        scan = scan.substr(2);
        }
      if (!scan.ToCLong(&l, base) || l < -1 || l > 0xffff)
        break;
      key.hidcode = l;
      }
    if (mtx.size())
      {
      if (delim2pos < 0)
        break;
      wxString row = mtx.substr(0, delim2pos);
      int base = 10;
      if (row.substr(0, 2) == wxT("0x"))
        {
        base = 16;
        row = row.substr(2);
        }
      if (!row.ToCLong(&l, base))
        break;
      key.matrixrow = l;
      wxString col = mtx.substr(delim2pos + 1);
      base = 10;
      if (col.substr(0, 2) == wxT("0x"))
        {
        base = 16;
        col = col.substr(2);
        }
      if (!col.ToCLong(&l, base))
        break;
      key.matrixcol = l;
      if (key.matrixrow < -1 || key.matrixrow >= MAXROWS ||
          key.matrixcol < -1 || key.matrixcol >= MAXCOLS)
        break;
      }
    }

  wxString text;
  if (sa.size() > 2)                    /* if there's a 3rd parameter,       */
    {                                   /* it must be text                   */
    text = sa[2];
    if (text.size() < 2 ||
        text.Left(1) != wxT("\"") ||
        text.Right(1) != wxT("\""))
      break;
    key.label[0] = UnescToken(text.substr(1, text.size() - 2));
    key.label[0].Trim();
    }

  if (sa.size() > 3)                    /* if there's a 4th parameter,       */
    {                                   /* it must be text2                  */
    text = sa[3];
    if (text.size() < 2 ||
        text.Left(1) != wxT("\"") ||
        text.Right(1) != wxT("\""))
      break;
    key.label[1] = UnescToken(text.substr(1, text.size() - 2));
    key.label[1].Trim();
    }

  if (sa.size() > 4)                    /* if there's a 5th parameter,       */
    {                                   /* it must be width[xheight][-width2]*/
    wxString width(sa[4]), height, width2;
    int delimpos = -1, delim2pos = -1;
    for (i = 0; i < width.size(); i++)
      {
      wxChar c = width[i];
      if (c == wxT('-'))
        {
        if (delimpos >= 0)
          break;
        delimpos = (int)i;
        }
      else if (c == wxT('x') || c == wxT('X'))
        {
        if (delimpos >= 0 || delim2pos >= 0)
          break;
        delim2pos = (int)i;
        }
      // 2nd row may have a negative width
      else if (c == wxT('-') && delimpos < 0)
        break;
      else if (c != wxT('.') && (c < wxT('0') || c > wxT('9')))
        break;
      }
    if (i < width.size())
      break;
    if ((delimpos >= 0 && key.height == 1) ||
        (delim2pos >= 0 && key.height != 1) ||
        (delim2pos >= 0 && delimpos >= 0 && delimpos <= delim2pos) ||
        (delimpos == 0) ||
        (delim2pos == 0))
      break;
    double d;
    if (delimpos > 0)
      {
      width2 = width.substr(delimpos + 1);
      if (!width2.ToCDouble(&d))
        break;
      key.width2 = (float)d;
      width = width.substr(0, delimpos);
      }
    if (delim2pos > 0)
      {
      height = width.substr(delim2pos + 1);
      if (!height.ToCDouble(&d))
        break;
      key.height = (float)d;
      width = width.substr(0, delim2pos);
      }
    if (!width.ToCDouble(&d))
      break;
    key.width1 = (float)d;
    if (key.height <= 1)
      key.width2 = (float)d;
#if 0
    // upper part must be larger or equal. Larger lower part is no good.
    if (fabsf(key.width2) > key.width1)
      continue;
#endif
    // allow funny keys ... but not TOO funny.
    if (key.width1 < 0.5f || 
        (key.width2 > -0.5f && key.width2 < 0.5f) ||
        key.height < 0.1 || 
        (key.height > 1.0 && key.height != 2.0))
      break;
    }

  if (sa.size() > 5)                    /* if there's a 6th parameter,       */
    {                                   /* it must be alignment              */
    text = sa[5];
    if (text.size() < 1)
      break;
    float dir = 1.0;
    wxString top(wxT("Top")), bottom(wxT("Bottom"));
    if (!text.CmpNoCase(top.substr(0, text.size())))
      dir = 1.f;
    else if (!text.CmpNoCase(bottom.substr(0, text.size())))
      dir = -1.f;
    else
      break;
    if (key.height > 1.f)
      break;
    key.height *= dir;
    }

  // whew. line parsed. now sort it into the file representation...
  for (i = 0; i < fileKeys.size(); i++)
    if (fileKeys[i].row > key.row)
      break;
  fileKeys.insert(fileKeys.begin() + i, key);
  }
if (line != f.GetLineCount())
  {
  if (error)
    *error = wxString::Format(wxT("Syntax error in line %d: "), line) + sCurLine;
  return false;
  }
// there must be at least one text and one key.
if (lname.empty() || !fileKeys.size())
  {
  if (error)
    *error = wxT("Error: at least the name and one key definition must be given");
  return false;
  }

layoutName = lname;
keys.assign(fileKeys.begin(), fileKeys.end());
CalcLayout();
return true;
}

/*****************************************************************************/
/* WriteLayoutFile : write keyboard GUI layout file                          */
/*****************************************************************************/

bool KbdGui::WriteLayoutFile(wxString const &filename)
{
wxTextFile f;
if (!f.Create(filename) &&
    !f.Open(filename))
  return false;
f.Clear();

f.AddLine(wxT("# BlUSB_GUI Keyboard GUI Layout File"));
f.AddLine(wxT(""));
f.AddLine(layoutName);

int curRow = -1, commentRow = 1;
for (size_t i = 0; i < keys.size(); i++)
  {
  GuiKey const &curKey = keys[i];
  if (curKey.row != curRow)
    {
    f.AddLine(wxT(""));
    f.AddLine(wxString::Format(wxT("# Row %d"), commentRow++));
    curRow = curKey.row;
    }

  wxString row = (curKey.height > 1) ?
      wxString::Format(wxT("R%d-%d"),
                       curKey.row, (int)(curKey.row + curKey.height - 1)) :
      wxString::Format(wxT("R%d"), curKey.row);
  wxString matrixpos;
  if (curKey.hidcode == -1 &&
      curKey.matrixrow == -1 && curKey.matrixcol == -1)
    matrixpos = wxT("BL");
  else if (curKey.hidcode != -1 ||
      (curKey.matrixrow == -1 && curKey.matrixcol == -1))
    matrixpos = wxString::Format(wxT("0x%02x"), curKey.hidcode);
  if (curKey.hidcode != -1 &&
      (curKey.matrixrow != -1 || curKey.matrixcol != -1))
    matrixpos += wxT('@');
  if (curKey.matrixrow != -1 || curKey.matrixcol != -1)
    matrixpos += wxString::Format(wxT("%d/%d"),
                                  curKey.matrixrow, curKey.matrixcol);
  wxString sText1 = wxT('\"') + EscToken(curKey.label[0]) + wxT('\"');
  wxString sText2 = wxT('\"') + EscToken(curKey.label[1]) + wxT('\"');
  wxString sWidth = wxString::Format(wxT("%f"), curKey.width1);
  wxString sHeight = wxString::Format(wxT("%f"), curKey.height);
  while (sWidth.size() > 1 && sWidth.Right(1) == wxT('0'))
    sWidth = sWidth.substr(0, sWidth.size() - 1);
  if (sWidth.Right(1) == wxT('.'))
    sWidth = sWidth.substr(0, sWidth.size() - 1);
  if (curKey.height < 1)
    {
    wxString sHeight = wxString::Format(wxT("%f"), fabsf(curKey.height));
    while (sHeight.size() > 1 && sHeight.Right(1) == wxT('0'))
      sHeight = sHeight.substr(0, sHeight.size() - 1);
    // can't be integral, so no need to check that
    sWidth += wxT("x") + sHeight;
    }
  if (curKey.width1 != curKey.width2)
    {
    wxString sWidth2 = wxString::Format(wxT("%f"), curKey.width2);
    while (sWidth2.size() > 1 && sWidth2.Right(1) == wxT('0'))
      sWidth2 = sWidth2.substr(0, sWidth2.size() - 1);
    if (sWidth2.Right(1) == wxT('.'))
      sWidth2 = sWidth2.substr(0, sWidth2.size() - 1);
    sWidth += wxT('-') + sWidth2;
    }

  wxString sRow = row + wxT(", ") + matrixpos;
  if (sText1.size() > 2 ||
      curKey.label[1].size() ||
      curKey.width1 != 1.f || curKey.width1 != curKey.width2 ||
      curKey.height < 1.)
    sRow += wxT(", ") + sText1;
  if (curKey.label[1].size() ||
      curKey.width1 != 1.f || curKey.width1 != curKey.width2 ||
      curKey.height < 1.)
    {
    sRow += wxT(", ") + sText2;
    if (curKey.width1 != 1.f || curKey.width1 != curKey.width2 ||
        curKey.height < 1.)
      sRow += wxT(", ") + sWidth;
    if (curKey.height < 0.)
      sRow += wxT(", B");
    }

#if 0
  if (curKey.width1 != curKey.width2)
    sRow += wxString::Format(wxT("   # %.2f/%.2f - %.2f/%.2f"),
                             curKey.startx1, curKey.startx2,
                             curKey.startx1 + curKey.width1, curKey.startx2 + curKey.width2);
  else
    sRow += wxString::Format(wxT("   # %.2f - %.2f"),
                             curKey.startx1, curKey.startx1 + curKey.width1);
  if (curKey.starty1 != curKey.starty2)
    sRow += wxString::Format(wxT(", %.2f/%.2f"),
                             curKey.starty1, curKey.starty2);
  else
    sRow += wxString::Format(wxT(", %.2f"),
                             curKey.starty1);

#endif
  f.AddLine(sRow);
  }

#if 0
f.AddLine(wxT(""));
f.AddLine(wxString::Format(wxT("# Total size: %.2f x %.2f units"), unitsH, unitsV));
#endif

return f.Write(wxTextFileType_Unix);
}

/*****************************************************************************/
/* CalcLayout : calculates the x positions based on the key definitions      */
/*****************************************************************************/

struct keyoverlap
  {
  float startx1;
  float width;
  keyoverlap(float s = 0.f, float e = 0.f)
    {
    startx1 = s;
    width = e;
    }
  };

void KbdGui::CalcLayout()
{
size_t i;

float fNextUpper = -1.f;
int prevStart = -1, nextStart = -1;
int curRow = -1, outRow = -1;
float rowx, rowy = -1.f, nextRowy = 1.f;
wxVector<keyoverlap> prevOver, nextOver;
int prevOverIdx;
nMaxRow = nMaxCol = 0;
unitsH = unitsV = 0.f;
for (i = 0; i < keys.size(); i++)
  {
  nMaxRow = max(nMaxRow, keys[i].matrixrow);
  nMaxCol = max(nMaxCol, keys[i].matrixcol);
  if (keys[i].row != curRow)
    {
    outRow++;
    curRow = keys[i].row;
    prevStart = nextStart;
    prevOver.assign(nextOver.begin(), nextOver.end());
    prevOverIdx = 0;
    nextOver.clear();
    nextStart = i;
    rowx = 0.f;
    if (nextRowy <= 0.f)  // only double-height keys (purely theoretical)
      nextRowy = 1.f;
    rowy += nextRowy;
    nextRowy = 0.f;
    }
  if (keys[i].row != outRow)
    keys[i].row = outRow;

  keys[i].startx1 = rowx;
  if (keys[i].width2 != keys[i].width1)
    {
    if (keys[i].width2 < 0)
      keys[i].startx2 = keys[i].startx1;
    else
      keys[i].startx2 = keys[i].startx1 + keys[i].width1 - keys[i].width2;
    }
  else
    keys[i].startx2 = rowx;
  keys[i].starty1 = rowy;
  if (keys[i].height > 1)
    {
    keys[i].starty2 = rowy + keys[i].height - 1;
    nextOver.push_back(keyoverlap(keys[i].startx2,
                                  fabsf(keys[i].width2)));
    }
  else if (keys[i].height < 0)
    {
    // bottom-aligned height < 1 in a 1-unit vertical space
    if (nextRowy < 1.f)
      nextRowy = 1.f;
    keys[i].starty1 = keys[i].starty2 = rowy + 1.f + keys[i].height;
    }
  else
    {
    keys[i].starty2 = rowy;
    if (keys[i].height > nextRowy)
      nextRowy = keys[i].height;
    }
  rowx += keys[i].width1;
  if (prevOverIdx < (int)prevOver.size() &&
      (prevOver[prevOverIdx].startx1 - rowx < 0.01f))
    rowx += prevOver[prevOverIdx++].width;
  // calculate total extension
  if (rowx > unitsH)
    unitsH = rowx;
  if (keys[i].starty1 + fabsf(keys[i].height) > unitsV)
    unitsV = keys[i].starty1 + fabsf(keys[i].height);
  }
}
