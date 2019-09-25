/*****************************************************************************/
/* MatrixWnd.cpp : CMatrixWnd implementation                                 */
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
#include "blusb_gui.h"

#include "KbdWnd.h"
#include "MatrixWnd.h"

#ifndef wxHAS_IMAGES_IN_RESOURCES
// nothing yet.
//#include "res/Application.xpm"
#endif

#ifndef ONE_CHOICE_EDITOR
// would be much more efficient, but doesn't work satisfactorily
#define ONE_CHOICE_EDITOR 0
#endif
#ifndef WITH_MATRIX_COMBOBOX
#define WITH_MATRIX_COMBOBOX 1
#endif

// Start Version that allows specific key definitions:
#define STV_ANY   0         // any version
#define STV_EXTK  0x0102    // extended keys start version
#define STV_MEDIA 0x0103    // media keys start version
#define STV_MACRO 0x0103    // macro start version
#define STV_BOOT  0x0105    // Boot Loader start version
#define STV_L67   0x0105    // layer 6/7 start version

/*===========================================================================*/
/* Global Data                                                               */
/*===========================================================================*/

static struct
  {
  int hid;
  const wxChar *txt;
  wxUint16 minver;  // minimum firmware version needed
  } hidtxts[] =
  {
    //TYPE 255: Miscellaneous
    { JMP_BOOT,        wxT("Boot"),               STV_BOOT },  // Jump to bootloader
    { BR_UP,           wxT("Brightness+"),        STV_ANY  },  // Increase LED brightness
    { BR_DOWN,         wxT("Brightness-"),        STV_ANY  },  // Decrease LED brightness
    { MACRO_RECORD,    wxT("Macro Record"),       STV_MACRO }, // Macro Record Start/Stop
    { MACRO_CLEAR_RECORDED, wxT("Macro Cancel Rec"), STV_MACRO },  // Macro Cancel Recording

    // TYPE 7: Macro keys
    { MACRO_1,         wxT("Macro  1"),           STV_MACRO },
    { MACRO_2,         wxT("Macro  2"),           STV_MACRO },
    { MACRO_3,         wxT("Macro  3"),           STV_MACRO },
    { MACRO_4,         wxT("Macro  4"),           STV_MACRO },
    { MACRO_5,         wxT("Macro  5"),           STV_MACRO },
    { MACRO_6,         wxT("Macro  6"),           STV_MACRO },
    { MACRO_7,         wxT("Macro  7"),           STV_MACRO },
    { MACRO_8,         wxT("Macro  8"),           STV_MACRO },
    { MACRO_9,         wxT("Macro  9"),           STV_MACRO },
    { MACRO_10,        wxT("Macro 10"),           STV_MACRO },
    { MACRO_11,        wxT("Macro 11"),           STV_MACRO },
    { MACRO_12,        wxT("Macro 12"),           STV_MACRO },
    { MACRO_13,        wxT("Macro 13"),           STV_MACRO },
    { MACRO_14,        wxT("Macro 14"),           STV_MACRO },
    { MACRO_15,        wxT("Macro 15"),           STV_MACRO },
    { MACRO_16,        wxT("Macro 16"),           STV_MACRO },
    { MACRO_17,        wxT("Macro 17"),           STV_MACRO },
    { MACRO_18,        wxT("Macro 18"),           STV_MACRO },
    { MACRO_19,        wxT("Macro 19"),           STV_MACRO },
    { MACRO_20,        wxT("Macro 20"),           STV_MACRO },
    { MACRO_21,        wxT("Macro 21"),           STV_MACRO },
    { MACRO_22,        wxT("Macro 22"),           STV_MACRO },
    { MACRO_23,        wxT("Macro 23"),           STV_MACRO },
    { MACRO_24,        wxT("Macro 24"),           STV_MACRO },

    // TYPE 6: System Control
    { SYSCTRL_POWERDOWN, wxT("Power Down"),       STV_ANY },
    { SYSCTRL_SLEEP,   wxT("Sleep"),              STV_ANY },
    { SYSCTRL_WAKEUP,  wxT("Wakeup"),             STV_ANY },

    // TYPE 5: Layer Toggle
    { TLAYER_0,        wxT("Toggle Layer 0"),     STV_ANY },
    { TLAYER_1,        wxT("Toggle Layer 1"),     STV_ANY },
    { TLAYER_2,        wxT("Toggle Layer 2"),     STV_ANY },
    { TLAYER_3,        wxT("Toggle Layer 3"),     STV_ANY },
    { TLAYER_4,        wxT("Toggle Layer 4"),     STV_ANY },
    { TLAYER_5,        wxT("Toggle Layer 5"),     STV_ANY },
    { TLAYER_6,        wxT("Toggle Layer 6"),     STV_L67 },
    { TLAYER_7,        wxT("Toggle Layer 7"),     STV_L67 },

    //TYPE 4: Momentary Layer Actuation
    { MLAYER_0,        wxT("Momentary Layer 0"),  STV_ANY },
    { MLAYER_1,        wxT("Momentary Layer 1"),  STV_ANY },
    { MLAYER_2,        wxT("Momentary Layer 2"),  STV_ANY },
    { MLAYER_3,        wxT("Momentary Layer 3"),  STV_ANY },
    { MLAYER_4,        wxT("Momentary Layer 4"),  STV_ANY },
    { MLAYER_5,        wxT("Momentary Layer 5"),  STV_ANY },
    { MLAYER_6,        wxT("Momentary Layer 6"),  STV_L67 },
    { MLAYER_7,        wxT("Momentary Layer 7"),  STV_L67 },

    //TYPE 3: Reserved for Mouse keys

    //TYPE 2: Media keys
    { MEDIA_PLAY,      wxT("Media Play"),         STV_MEDIA },
    { MEDIA_PAUSE,     wxT("Media Pause"),        STV_MEDIA },
    { MEDIA_NEXT,      wxT("Media Next"),         STV_MEDIA },
    { MEDIA_PREVIOUS,  wxT("Media Prev"),         STV_MEDIA },
    { MEDIA_STOP,      wxT("Media Stop"),         STV_MEDIA },
    { MEDIA_MUTE,      wxT("Media Mute"),         STV_MEDIA },
    { MEDIA_VOLUP,     wxT("Media Vol.Up"),       STV_MEDIA },
    { MEDIA_VOLDOWN,   wxT("Media Vol.Down"),     STV_MEDIA },
    { MEDIA_EMAIL,     wxT("Media Email"),        STV_MEDIA },
    { MEDIA_CALC,      wxT("Media Calc"),         STV_MEDIA },
    { MEDIA_BROWSER,   wxT("Media Browser"),      STV_MEDIA },
    { MEDIA_SEARCH,    wxT("Media Search"),       STV_MEDIA },
    { MEDIA_HOME,      wxT("Media Home"),         STV_MEDIA },
    { MEDIA_BACK,      wxT("Media Back"),         STV_MEDIA },
    { MEDIA_FORWARD,   wxT("Media Forward"),      STV_MEDIA },
    { MEDIA_REFRESH,   wxT("Media Refresh"),      STV_MEDIA },

    //TYPE 1: Keyboard modifiers
    { KB_LCTRL,        wxT("Left Control"),       STV_ANY },
    { KB_LSHFT,        wxT("Left Shift"),         STV_ANY },
    { KB_LALT,         wxT("Left Alt"),           STV_ANY },
    { KB_LGUI,         wxT("Left GUI"),           STV_ANY },
    { KB_RCTRL,        wxT("Right Control"),      STV_ANY },
    { KB_RSHFT,        wxT("Right Shift"),        STV_ANY },
    { KB_RALT,         wxT("Right Alt"),          STV_ANY },
    { KB_RGUI,         wxT("Right GUI"),          STV_ANY },

    //TYPE 0: Keyboard keycodes
    { KB_UNUSED,       wxT("Unused"),             STV_ANY },
    { KB_A,            wxT("A"),                  STV_ANY },
    { KB_B,            wxT("B"),                  STV_ANY },
    { KB_C,            wxT("C"),                  STV_ANY },
    { KB_D,            wxT("D"),                  STV_ANY },
    { KB_E,            wxT("E"),                  STV_ANY },
    { KB_F,            wxT("F"),                  STV_ANY },
    { KB_G,            wxT("G"),                  STV_ANY },
    { KB_H,            wxT("H"),                  STV_ANY },
    { KB_I,            wxT("I"),                  STV_ANY },
    { KB_J,            wxT("J"),                  STV_ANY },
    { KB_K,            wxT("K"),                  STV_ANY },
    { KB_L,            wxT("L"),                  STV_ANY },
    { KB_M,            wxT("M"),                  STV_ANY },
    { KB_N,            wxT("N"),                  STV_ANY },
    { KB_O,            wxT("O"),                  STV_ANY },
    { KB_P,            wxT("P"),                  STV_ANY },
    { KB_Q,            wxT("Q"),                  STV_ANY },
    { KB_R,            wxT("R"),                  STV_ANY },
    { KB_S,            wxT("S"),                  STV_ANY },
    { KB_T,            wxT("T"),                  STV_ANY },
    { KB_U,            wxT("U"),                  STV_ANY },
    { KB_V,            wxT("V"),                  STV_ANY },
    { KB_W,            wxT("W"),                  STV_ANY },
    { KB_X,            wxT("X"),                  STV_ANY },
    { KB_Y,            wxT("Y"),                  STV_ANY },
    { KB_Z,            wxT("Z"),                  STV_ANY },
    { KB_1,            wxT("1"),                  STV_ANY },
    { KB_2,            wxT("2"),                  STV_ANY },
    { KB_3,            wxT("3"),                  STV_ANY },
    { KB_4,            wxT("4"),                  STV_ANY },
    { KB_5,            wxT("5"),                  STV_ANY },
    { KB_6,            wxT("6"),                  STV_ANY },
    { KB_7,            wxT("7"),                  STV_ANY },
    { KB_8,            wxT("8"),                  STV_ANY },
    { KB_9,            wxT("9"),                  STV_ANY },
    { KB_0,            wxT("0"),                  STV_ANY },
    { KB_ENTER,        wxT("Enter"),              STV_ANY },
    { KB_ESC,          wxT("Escape"),             STV_ANY },
    { KB_BKSPC,        wxT("Backspace"),          STV_ANY },
    { KB_TAB,          wxT("Tab"),                STV_ANY },
    { KB_SPACE,        wxT("Space"),              STV_ANY },
    { KB_MINUS,        wxT("-"),                  STV_ANY },
    { KB_EQUAL,        wxT("="),                  STV_ANY },
    { KB_LBRCE,        wxT("["),                  STV_ANY },
    { KB_RBRCE,        wxT("]"),                  STV_ANY },
    { KB_BSLSH,        wxT("\\"),                 STV_ANY },
    { KB_NUMBER,       wxT("# (Non-US)"),         STV_ANY },
    { KB_SMCLN,        wxT(";"),                  STV_ANY },
    { KB_QUOTE,        wxT("\'"),                 STV_ANY },
    { KB_TILDE,        wxT("~"),                  STV_ANY },
    { KB_COMMA,        wxT(","),                  STV_ANY },
    { KB_DOT,          wxT("."),                  STV_ANY },
    { KB_SLASH,        wxT("/"),                  STV_ANY },
    { KB_CAPLK,        wxT("Caps Lock"),          STV_ANY },
    { KB_F1,           wxT("F1"),                 STV_ANY },
    { KB_F2,           wxT("F2"),                 STV_ANY },
    { KB_F3,           wxT("F3"),                 STV_ANY },
    { KB_F4,           wxT("F4"),                 STV_ANY },
    { KB_F5,           wxT("F5"),                 STV_ANY },
    { KB_F6,           wxT("F6"),                 STV_ANY },
    { KB_F7,           wxT("F7"),                 STV_ANY },
    { KB_F8,           wxT("F8"),                 STV_ANY },
    { KB_F9,           wxT("F9"),                 STV_ANY },
    { KB_F10,          wxT("F10"),                STV_ANY },
    { KB_F11,          wxT("F11"),                STV_ANY },
    { KB_F12,          wxT("F12"),                STV_ANY },
    { KB_PSCRN,        wxT("PrintScreen"),        STV_ANY },
    { KB_SCRLK,        wxT("Scroll Lock"),        STV_ANY },
    { KB_PAUSE,        wxT("Pause"),              STV_ANY },
    { KB_INS,          wxT("Insert"),             STV_ANY },
    { KB_HOME,         wxT("Home"),               STV_ANY },
    { KB_PGUP,         wxT("PageUp"),             STV_ANY },
    { KB_DEL,          wxT("Delete"),             STV_ANY },
    { KB_END,          wxT("End"),                STV_ANY },
    { KB_PGDN,         wxT("PageDown"),           STV_ANY },
    { KB_RIGHT,        wxT("Right"),              STV_ANY },
    { KB_LEFT,         wxT("Left"),               STV_ANY },
    { KB_DOWN,         wxT("Down"),               STV_ANY },
    { KB_UP,           wxT("Up"),                 STV_ANY },
    { KB_NUMLK,        wxT("Num Lock"),           STV_ANY },
    { KP_SLASH,        wxT("Keypad /"),           STV_ANY },
    { KP_ASTRX,        wxT("Keypad *"),           STV_ANY },
    { KP_MINUS,        wxT("Keypad -"),           STV_ANY },
    { KP_PLUS,         wxT("Keypad +"),           STV_ANY },
    { KP_ENTER,        wxT("Keypad Enter"),       STV_ANY },
    { KP_1,            wxT("Keypad 1"),           STV_ANY },
    { KP_2,            wxT("Keypad 2"),           STV_ANY },
    { KP_3,            wxT("Keypad 3"),           STV_ANY },
    { KP_4,            wxT("Keypad 4"),           STV_ANY },
    { KP_5,            wxT("Keypad 5"),           STV_ANY },
    { KP_6,            wxT("Keypad 6"),           STV_ANY },
    { KP_7,            wxT("Keypad 7"),           STV_ANY },
    { KP_8,            wxT("Keypad 8"),           STV_ANY },
    { KP_9,            wxT("Keypad 9"),           STV_ANY },
    { KP_0,            wxT("Keypad 0"),           STV_ANY },
    { KP_DOT,          wxT("Keypad ."),           STV_ANY },
    { KB_PIPE,         wxT("| (Non-US)"),         STV_ANY },
    { KB_APP,          wxT("Application"),        STV_ANY },

    { KB_POWER,        wxT("Power"),              STV_EXTK },
    { KP_EQUAL,        wxT("Keypad ="),           STV_EXTK },
    { KB_F13,          wxT("F13"),                STV_EXTK },
    { KB_F14,          wxT("F14"),                STV_EXTK },
    { KB_F15,          wxT("F15"),                STV_EXTK },
    { KB_F16,          wxT("F16"),                STV_EXTK },
    { KB_F17,          wxT("F17"),                STV_EXTK },
    { KB_F18,          wxT("F18"),                STV_EXTK },
    { KB_F19,          wxT("F19"),                STV_EXTK },
    { KB_F20,          wxT("F20"),                STV_EXTK },
    { KB_F21,          wxT("F21"),                STV_EXTK },
    { KB_F22,          wxT("F22"),                STV_EXTK },
    { KB_F23,          wxT("F23"),                STV_EXTK },
    { KB_F24,          wxT("F24"),                STV_EXTK },
    { KB_EXECUTE,      wxT("Execute"),            STV_EXTK },
    { KB_HELP,         wxT("Help"),               STV_EXTK },
    { KB_MENU,         wxT("Menu"),               STV_EXTK },
    { KB_SELECT,       wxT("Select"),             STV_EXTK },
    { KB_STOP,         wxT("Stop"),               STV_EXTK },
    { KB_AGAIN,        wxT("Again"),              STV_EXTK },
    { KB_UNDO,         wxT("Undo"),               STV_EXTK },
    { KB_CUT,          wxT("Cut"),                STV_EXTK },
    { KB_COPY,         wxT("Copy"),               STV_EXTK },
    { KB_PASTE,        wxT("Paste"),              STV_EXTK },
    { KB_FIND,         wxT("Find"),               STV_EXTK },
    { KB_MUTE,         wxT("Mute"),               STV_EXTK },
    { KB_VOLUMEUP,     wxT("Volume Up"),          STV_EXTK },
    { KB_VOLUMEDOWN,   wxT("Volume Down"),        STV_EXTK },
    { KB_LCAPLK,       wxT("Locking Caps Lock"),  STV_EXTK },
    { KB_LNUMLK,       wxT("Locking Num Lock"),   STV_EXTK },
    { KB_LSCRLK,       wxT("Locking Scroll Lock"), STV_EXTK },
    { KP_COMMA,        wxT("Keypad ,"),           STV_EXTK },   // "special key on Brazilian keyboard" (HUT Note 27)
    { KP_EQUALSIGN,    wxT("Keypad Equal Sign"),  STV_EXTK },
    { KB_INTL1,        wxT("International 1"),    STV_EXTK },
    { KB_INTL2,        wxT("International 2"),    STV_EXTK },
    { KB_INTL3,        wxT("International 3"),    STV_EXTK },
    { KB_INTL4,        wxT("International 4"),    STV_EXTK },
    { KB_INTL5,        wxT("International 5"),    STV_EXTK },
    { KB_INTL6,        wxT("International 6"),    STV_EXTK },
    { KB_INTL7,        wxT("International 7"),    STV_EXTK },
    { KB_INTL8,        wxT("International 8"),    STV_EXTK },
    { KB_INTL9,        wxT("International 9"),    STV_EXTK },
    { KB_LANG1,        wxT("Lang 1"),             STV_EXTK },  // Hangul/English
    { KB_LANG2,        wxT("Lang 2"),             STV_EXTK },  // Hanja Conversion
    { KB_LANG3,        wxT("Lang 3"),             STV_EXTK },  // Katakana
    { KB_LANG4,        wxT("Lang 4"),             STV_EXTK },  // Hiragana
    { KB_LANG5,        wxT("Lang 5"),             STV_EXTK },  // Zenkaku/Hankaku
    { KB_LANG6,        wxT("Lang 6"),             STV_EXTK },
    { KB_LANG7,        wxT("Lang 7"),             STV_EXTK },
    { KB_LANG8,        wxT("Lang 8"),             STV_EXTK },
    { KB_LANG9,        wxT("Lang 9"),             STV_EXTK },
    { KB_ALT_ERA,      wxT("Alt Erase"),          STV_EXTK },
    { KB_SYSREQ,       wxT("SysReq/Attention"),   STV_EXTK },
    { KB_CANCEL,       wxT("Cancel"),             STV_EXTK },
    { KB_CLEAR,        wxT("Clear"),              STV_EXTK },
    { KB_PRIOR,        wxT("Prior"),              STV_EXTK },
    { KB_RETURN,       wxT("Return"),             STV_EXTK },
    { KB_SEP,          wxT("Separator"),          STV_EXTK },
    { KB_OUT,          wxT("Out"),                STV_EXTK },
    { KB_OPER,         wxT("Oper"),               STV_EXTK },
    { KB_CLR_AGN,      wxT("Clear/Again"),        STV_EXTK },
    { KB_CRSEL,        wxT("CrSel/Props"),        STV_EXTK },
    { KB_EXSEL,        wxT("ExSel"),              STV_EXTK },
    { KP_00,           wxT("Keypad 00"),          STV_EXTK },
    { KP_000,          wxT("Keypad 000"),         STV_EXTK },
    { KB_THOU_SEP,     wxT("Thousands Separator"), STV_EXTK },
    { KB_DEC_SEP,      wxT("Decimal Separator"),  STV_EXTK },
    { KB_CUR_UNIT,     wxT("Currency Unit"),      STV_EXTK },
    { KB_CUR_SUBU,     wxT("Currency Sub-unit"),  STV_EXTK },
    { KP_LPAREN,       wxT("Keypad ("),           STV_EXTK },
    { KP_RPAREN,       wxT("Keypad )"),           STV_EXTK },
    { KP_LBRACE,       wxT("Keypad {"),           STV_EXTK },
    { KP_RBRACE,       wxT("Keypad }"),           STV_EXTK },
    { KP_TAB,          wxT("Keypad Tab"),         STV_EXTK },
    { KP_BACKSPACE,    wxT("Keypad Backspace"),   STV_EXTK },
    { KP_A,            wxT("Keypad A"),           STV_EXTK },
    { KP_B,            wxT("Keypad B"),           STV_EXTK },
    { KP_C,            wxT("Keypad C"),           STV_EXTK },
    { KP_D,            wxT("Keypad D"),           STV_EXTK },
    { KP_E,            wxT("Keypad E"),           STV_EXTK },
    { KP_F,            wxT("Keypad F"),           STV_EXTK },
    { KP_XOR,          wxT("Keypad XOR"),         STV_EXTK },
    { KP_POWER,        wxT("Keypad ^"),           STV_EXTK },
    { KP_MOD,          wxT("Keypad %"),           STV_EXTK },
    { KP_LESS,         wxT("Keypad <"),           STV_EXTK },
    { KP_GREATER,      wxT("Keypad >"),           STV_EXTK },
    { KP_BIT_AND,      wxT("Keypad &"),           STV_EXTK },
    { KP_AND,          wxT("Keypad &&"),          STV_EXTK },
    { KP_BIT_OR,       wxT("Keypad |"),           STV_EXTK },
    { KP_OR,           wxT("Keypad ||"),          STV_EXTK },
    { KP_COLON,        wxT("Keypad :"),           STV_EXTK },
    { KP_NUMBER,       wxT("Keypad #"),           STV_EXTK },
    { KP_SPACE,        wxT("Keypad Space"),       STV_EXTK },
    { KP_AT,           wxT("Keypad @"),           STV_EXTK },
    { KP_NOT,          wxT("Keypad !"),           STV_EXTK },
    { KP_MEM_STR,      wxT("Keypad Mem Store"),   STV_EXTK },
    { KP_MEM_RCL,      wxT("Keypad Mem Recall"),  STV_EXTK },
    { KP_MEM_CLR,      wxT("Keypad Mem Clear"),   STV_EXTK },
    { KP_MEM_ADD,      wxT("Keypad Mem Add"),     STV_EXTK },
    { KP_MEM_SUB,      wxT("Keypad Mem Subtract"), STV_EXTK },
    { KP_MEM_MUL,      wxT("Keypad Mem Multiply"), STV_EXTK },
    { KP_MEM_DIV,      wxT("Keypad Mem Divide"),  STV_EXTK },
    { KP_PLSMNS,       wxT("Keypad +/-"),         STV_EXTK },
    { KP_CLEAR,        wxT("Keypad Clear"),       STV_EXTK },
    { KP_CLR_ENT,      wxT("Keypad Clear Entry"), STV_EXTK },
    { KP_BINARY,       wxT("Keypad Binary"),      STV_EXTK },
    { KP_OCTAL,        wxT("Keypad Octal"),       STV_EXTK },
    { KP_DECIMAL,      wxT("Keypad Decimal"),     STV_EXTK },
    { KP_HEXADEC,      wxT("Keypad Hexadecimal"), STV_EXTK },

#if 0  // not yet. If ever.
    // replaced by Type 2 above ... just not completely.
    { KB_MEDIA_NTRK,   wxT("Next Track"),         STV_MEDIA },
    { KB_MEDIA_PTRK,   wxT("Previous Track"),     STV_MEDIA },
    { KB_MEDIA_STOP,   wxT("Stop"),               STV_MEDIA },
    { KB_MEDIA_PLPS,   wxT("Play/Pause"),         STV_MEDIA },
    { KB_MEDIA_PLPS,   wxT("Mute"),               STV_MEDIA },
    { KB_MEDIA_PLPS,   wxT("Bass Boost"),         STV_MEDIA },
    { KB_MEDIA_LDNS,   wxT("Loudness"),           STV_MEDIA },
    { KB_MEDIA_VOLUP,  wxT("Volume+"),            STV_MEDIA },
    { KB_MEDIA_VOLDN,  wxT("Volume-"),            STV_MEDIA },
    { KB_MEDIA_BASSUP, wxT("Bass+"),              STV_MEDIA },
    { KB_MEDIA_BASSDN, wxT("Bass-"),              STV_MEDIA },
    { KB_MEDIA_TREBUP, wxT("Treble+"),            STV_MEDIA },
    { KB_MEDIA_TREBDN, wxT("Treble-"),            STV_MEDIA },
    { KB_MEDIA_MSEL,   wxT("Media Select"),       STV_MEDIA },
    { KB_MEDIA_MAIL,   wxT("Mail"),               STV_MEDIA },
    { KB_MEDIA_CALC,   wxT("Calculator"),         STV_MEDIA },
    { KB_MEDIA_MYCMP,  wxT("My Computer"),        STV_MEDIA },
    { KB_MEDIA_WWWSRC, wxT("WWW Search"),         STV_MEDIA },
    { KB_MEDIA_WWWHOM, wxT("WWW Home"),           STV_MEDIA },
    { KB_MEDIA_WWWBK,  wxT("WWW Back"),           STV_MEDIA },
    { KB_MEDIA_WWWFW,  wxT("WWW Forward"),        STV_MEDIA },
    { KB_MEDIA_WWWSTP, wxT("WWW Stop"),           STV_MEDIA },
    { KB_MEDIA_WWWRFR, wxT("WWW Refresh"),        STV_MEDIA },
    { KB_MEDIA_WWWFAV, wxT("WWW Favorites"),      STV_MEDIA },
#endif
  };

/*===========================================================================*/
/* CMatrixComboBox class declaration and members                             */
/*===========================================================================*/

#if WITH_MATRIX_COMBOBOX
class CMatrixComboBox : public wxComboBox
{
public:
    CMatrixComboBox()
      : m_pGrid(NULL)
      { }
	CMatrixComboBox(wxWindow *parent,
                    wxWindowID id,
                    const wxString& value,
                    const wxPoint& pos,
                    const wxSize& size,
                    const wxArrayString& choices,
                    long style = 0,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = wxComboBoxNameStr)
        : m_pGrid(NULL)
      {
      Create(parent, id, value, pos, size, choices, style, validator, name);
      }
	void SetCellData(int nRow, int nCol, wxGrid* pGrid)
	  {
      m_nRow = nRow;
      m_nCol = nCol;
      m_pGrid = pGrid;
      }
private:
	DECLARE_EVENT_TABLE()
	void OnChange(wxCommandEvent& event);
	int m_nRow, m_nCol; 
	wxGrid* m_pGrid;
};

/*****************************************************************************/
/* CMatrixComboBox event table                                               */
/*****************************************************************************/

BEGIN_EVENT_TABLE(CMatrixComboBox, wxComboBox)
   EVT_COMBOBOX(wxID_ANY, CMatrixComboBox::OnChange)
END_EVENT_TABLE()

/*****************************************************************************/
/* OnChange : called when the combo box contents change                      */
/*****************************************************************************/

void CMatrixComboBox::OnChange(wxCommandEvent& event)
{
if (m_pGrid) 
  {
  m_pGrid->GetTable()->SetValue(m_nRow, m_nCol, event.GetString());
  wxGridEvent gridEvt(m_pGrid->GetId(),
                      wxEVT_GRID_CELL_CHANGED,
                      m_pGrid, m_nRow, m_nCol);
  gridEvt.SetString(event.GetString());
  GetEventHandler()->ProcessEvent(gridEvt);
  }
event.Skip();
}
#endif


/*===========================================================================*/
/* CMatrixChoiceEditor class : overlay for the cell choice editor            */
/*===========================================================================*/

class CMatrixChoiceEditor : public wxGridCellChoiceEditor
{
public:
    CMatrixChoiceEditor(size_t count = 0,
                        const wxString choices[] = NULL,
                        bool allowOthers = FALSE) :
      wxGridCellChoiceEditor(count, choices, allowOthers)
        { }

    virtual wxGridCellEditor *Clone() const wxOVERRIDE
      {
      // ATTENTION: this is a copy of wxGridCellChoiceEditor::Clone().
      // When switching to a new wxWidgets version, this needs to be
      // verified!
      CMatrixChoiceEditor *editor = new CMatrixChoiceEditor;
      editor->m_allowOthers = m_allowOthers;
      editor->m_choices = m_choices;
      return editor;
      }
    virtual void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler) wxOVERRIDE
      {
#if WITH_MATRIX_COMBOBOX
      // ATTENTION: this is a copy of wxGridCellChoiceEditor::Create().
      // When switching to a new wxWidgets version, this needs to be
      // verified!
      int style = wxTE_PROCESS_ENTER |
                  wxTE_PROCESS_TAB |
                  wxBORDER_NONE;
      if (!m_allowOthers)
        style |= wxCB_READONLY;
      m_control = new CMatrixComboBox(parent, id, wxEmptyString,
                                      wxDefaultPosition, wxDefaultSize,
                                      m_choices,
                                      style);
      // since we already did it, skip wxGridCellChoiceEditor::Create()
      wxGridCellEditor::Create(parent, id, evtHandler);
#else
      wxGridCellChoiceEditor::Create(parent, id, evtHandler);
#endif
      }
	virtual void BeginEdit(int row, int col, wxGrid* grid) wxOVERRIDE
      {
      wxGridCellChoiceEditor::BeginEdit(row, col, grid);
#if WITH_MATRIX_COMBOBOX
      // our combo box needs to know about the environment!
      ((CMatrixComboBox*)Combo())->SetCellData(row, col, grid);
#endif
      // do the popup immediately. Saves one click.
      Combo()->Popup();
      }

};


/*===========================================================================*/
/* Text <-> HID hash maps / text array                                       */
/*===========================================================================*/

WX_DECLARE_STRING_HASH_MAP(wxUint16, Text2HID);
WX_DECLARE_HASH_MAP(wxUint16, wxString, wxIntegerHash, wxIntegerEqual, HID2Text);

static Text2HID text2HID;               /* Text -> HID map                   */
static HID2Text hid2Text;               /* HID -> Text map                   */
static wxSortedArrayString hidTexts;    /* array of all the texts            */
#if ONE_CHOICE_EDITOR
static  CMatrixChoiceEditor *choices = NULL;
#endif

void SetupText2HIDMapping(wxUint16 fwVer)
{
if (text2HID.size())
  return;

for (int i = 0; i < _countof(hidtxts); i++)
  {
  if (fwVer >= hidtxts[i].minver)
    {
    hid2Text[hidtxts[i].hid] = hidtxts[i].txt;
    text2HID[hidtxts[i].txt] = hidtxts[i].hid;
    hidTexts.push_back(hidtxts[i].txt);
    }
  }
hidTexts.Sort();

#if ONE_CHOICE_EDITOR
choices = new CMatrixChoiceEditor(hidTexts.size(), &hidTexts[0]);
#endif
}

void RemoveText2HIDMapping()
{
hid2Text.clear();
text2HID.clear();
hidTexts.clear();
#if ONE_CHOICE_EDITOR
int nRefCnt = choices->GetRefCount();
#endif
}

/*===========================================================================*/
/* CMatrixWnd class members                                                  */
/*===========================================================================*/

/*****************************************************************************/
/* CMatrixWnd Event Table                                                    */
/*****************************************************************************/

wxBEGIN_EVENT_TABLE(CMatrixWnd, wxGrid)
//#if 0
// not interesting for now.
#if 1
    EVT_KEY_DOWN(CMatrixWnd::OnKeyDown)
    EVT_CHAR(CMatrixWnd::OnChar)
    EVT_KEY_UP(CMatrixWnd::OnKeyUp)
#endif
    EVT_GRID_CELL_CHANGED(CMatrixWnd::OnCellValueChanged)
wxEND_EVENT_TABLE()

/*****************************************************************************/
/* CMatrixWnd : constructor                                                  */
/*****************************************************************************/

CMatrixWnd::CMatrixWnd
    (
    wxWindow *parent,
    int numRows, int numCols,
    bool switched
    )
: wxGrid(parent, wxID_ANY, wxPoint(0,0), wxSize(700, 300))
{
CreateGrid(0, 0);
SetOrientation(switched);
SetLayout(numRows, numCols);
SetKbdWnd();
layernum = 0;
Layout();
#if 0
SetSize(GetMatrixSize());
#else
wxSize szNew(GetMatrixSize());
SetSize(szNew);
SetMinSize(szNew);
#endif
}

/*****************************************************************************/
/* ~CMatrixWnd : destructor                                                  */
/*****************************************************************************/

CMatrixWnd::~CMatrixWnd(void)
{
HideCellEditControl();  // make sure no editor is in use

#if ONE_CHOICE_EDITOR
for (int i = 0; i < GetNumberRows(); i++)
  for (int j = 0; j < GetNumberCols(); j++)
    SetCellEditor(i, j, NULL);
#endif
}

/*****************************************************************************/
/* SetOrientation : sets the matrix orientation                              */
/*****************************************************************************/

void CMatrixWnd::SetOrientation(bool switched)
{
int oldRows = GetNumberRows();
int oldCols = GetNumberCols();
RC2Internal(oldRows, oldCols);
SetLayout(0, 0);
bColsRowsSwitched = switched;
SetLayout(oldRows, oldCols);
}

/*****************************************************************************/
/* SetLayout : defines the matrix layout                                     */
/*****************************************************************************/

void CMatrixWnd::SetLayout(int numRows, int numCols)
{
int i, j;
wxString sNone = hid2Text[KB_NONE];

RC2Internal(numRows, numCols);

#if 1
int oldRows = GetNumberRows();
int oldCols = GetNumberCols();

#if ONE_CHOICE_EDITOR
// unlink all superfluous choice editors
for (i = numRows; i < oldRows; i++)
  for (j = numCols; j < oldCols; j++)
    SetCellEditor(i, j, NULL);
#endif

// reduce if necessary
if (numRows < oldRows)
  DeleteRows(numRows, oldRows - numRows);
if (numCols < oldCols)
  DeleteCols(numCols, oldCols - numCols);
if (numCols > oldCols)
  AppendCols(numCols - oldCols);
if (numRows > oldRows)
  AppendRows(numRows - oldRows);
#else
int oldRows = 0;
int oldCols = 0;
CreateGrid(numRows, numCols);
#endif

if (numCols > oldCols)
  {
  for (i = oldCols; i < numCols; i++)
    {
    SetColSize(i, GetColSize(i) * 13 / 8);
    SetColLabelValue(i,
                     wxString::Format(bColsRowsSwitched ? "R%d" : "C%d", i));
    }
  }
if (numRows > oldRows)
  {
  for (i = oldRows; i < numRows; i++)
    SetRowLabelValue(i,
                     wxString::Format(bColsRowsSwitched ? "C%d" : "R%d", i));
  }

// Needs to be done in two loops as row AND column count may have changed
int minCols = min(oldCols, numCols);
for (i = oldRows; i < numRows; i++)
  for (j = 0; j < minCols; j++)
    {
    SetCellValue(i, j, sNone);
#if ONE_CHOICE_EDITOR
    SetCellEditor(i, j, choices);
    choices->IncRef();  // necessary, as SetCellEditor() doesn't do it.
#else
    SetCellEditor(i, j,
                  new CMatrixChoiceEditor(hidTexts.size(), &hidTexts[0]));
#endif
    }
if (numCols > oldCols)
  for (i = 0; i < numRows; i++)
    for (j = oldCols; j < numCols; j++)
      {
      SetCellValue(i, j, sNone);
#if ONE_CHOICE_EDITOR
      SetCellEditor(i, j, choices);
      choices->IncRef();  // necessary, as SetCellEditor() doesn't do it.
#else
      SetCellEditor(i, j,
                    new CMatrixChoiceEditor(hidTexts.size(), &hidTexts[0]));
#endif
      }
}

/*****************************************************************************/
/* SetKbdMatrix : sets the on-screen representation of a keyboard matrix     */
/*****************************************************************************/

void CMatrixWnd::SetKbdMatrix(KbdMatrix const &kbm)
{
#if 1
// fixed 8x20 matrix layout
SetLayout(NUMROWS, NUMCOLS);
#else
// matrix layour determined by keyboard definition
SetLayout(kbm.GetRows(), kbm.GetCols());
#endif
// if controller doesn't have that many columns, mark unusable ones
int ctlrows = NUMROWS, ctlcols = NUMCOLS;
GetApp()->ReadMatrixLayout(ctlrows, ctlcols);
wxColour clrUnusable = wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK);
for (int i = 0; i < GetNumberCols(); i++)
  for (int j = 0; j < GetNumberRows(); j++)
    {
    wxUint16 key = kbm.GetKey(i, j);
    int r(i), c(j);
    RC2Internal(r, c);
    SetCellValue(r, c, hid2Text[key]);
    if (r >= ctlcols || c >= ctlrows)
      SetCellBackgroundColour(r, c, clrUnusable);
    }
wxSize szNew(GetMatrixSize());
SetSize(szNew);
SetMinSize(szNew);
// propagate size changes (potentially requires overriding parent's Layout())
if (GetParent())
  GetParent()->Layout();

}

// this is going to be a long, interesting journey
// into the abyss of OS dependencies ...

/*****************************************************************************/
/* OnKeyDown : called when a key is pressed on the window                    */
/*****************************************************************************/

void CMatrixWnd::OnKeyDown(wxKeyEvent &ev)
{
#if 1
//#if defined(__WXMSW__)

#if !defined(__WXMSW__)
wxChar uc = ev.GetKeyCode();
// printf("CMatrixWnd::OnKeyDown(%d)%s\n", uc, pKbdWnd ? " pass on" : "");
if (pKbdWnd)
  pKbdWnd->PassOnKeyDown(ev);
#endif

// ignore keys in here. The keyboard window selects the matrix position,
// so the standard grid behavior would interfere with that.
ev.Skip(false);

#else

#ifdef _DEBUG
// not for release builds until it really works!

int key = KB_UNUSED;

#if defined(wxHAS_RAW_KEY_CODES) && defined(__WXMSW__)
// get the raw key flags
wxUint32 rawFlags = ev.GetRawKeyFlags();
// Bits 16 .. 23: scancode
// Bit 24: flag whether extended key
int scancode = (rawFlags >> 16) & 0x1ff;
if (scancode & (1 << 30))
  key = KB_UNUSED;
else
  key = KbdGui::GetHIDFromScancode(scancode);

#else
int kc = ev.GetKeyCode();               /* get key code from event           */
int key = KbdGui::GetHID(kc);           /* retrieve HID for that key         */
#endif

if (key != KB_UNUSED)
  {
  int r = GetGridCursorRow(), c = GetGridCursorCol();
  if (r >= 0 && c >= 0)
    {
    SetCellValue(r, c, hid2Text[key]);
    GetApp()->GetLayout().SetKey(GetLayer(), c, r, key);
    }
  ev.Skip(false);
  }

#endif

#endif

// default: do nothing. wxGrid gets it anyway.
}

/*****************************************************************************/
/* OnChar : called for character events                                      */
/*****************************************************************************/

void CMatrixWnd::OnChar(wxKeyEvent &ev)
{
#if 1
//#if defined(__WXMSW__)

#if !defined(__WXMSW__)
wxChar uc = ev.GetUnicodeKey();
if (uc == WXK_NONE)
  uc = ev.GetKeyCode();
// printf("CMatrixWnd::OnChar(%d)%s\n", uc, pKbdWnd ? " pass on" : "");
if (pKbdWnd)
  pKbdWnd->PassOnChar(ev);
#endif

// ignore keys in here. The keyboard window selects the matrix position,
// so the standard grid behavior would interfere with that.
ev.Skip(false);

#else

#ifdef _DEBUG

int key = KB_UNUSED;

#if defined(wxHAS_RAW_KEY_CODES) && defined(WIN32)
// get the raw key flags
wxUint32 rawFlags = ev.GetRawKeyFlags();
// Bits 16 .. 23: scancode
// Bit 24: flag whether extended key
int scancode = (rawFlags >> 16) & 0x1ff;
if (scancode & (1 << 30))
  key = KB_UNUSED;
else
  key = KbdGui::GetHIDFromScancode(scancode);

#else

// not for release builds until it really works!
int kc = ev.GetKeyCode();
int key = KbdGui::GetHID(kc);           /* retrieve HID for that key         */
#endif

if (key != KB_UNUSED)
  {
  // we already captured the key in OnKeyDown, so just ignore the CHAR event
  ev.Skip(false);
  }

#endif

#endif

// default: do nothing. wxGrid gets it anyway.
}

/*****************************************************************************/
/* OnKeyUp : called when a key is released on the window                     */
/*****************************************************************************/

void CMatrixWnd::OnKeyUp(wxKeyEvent &ev)
{
#if 1
//#if defined(__WXMSW__)

#if !defined(__WXMSW__)
wxChar uc = ev.GetKeyCode();
// printf("CMatrixWnd::OnKeyUp(%d)%s\n", uc, pKbdWnd ? " pass on" : "");
if (pKbdWnd)
  pKbdWnd->PassOnKeyUp(ev);
#endif

// ignore keys in here. The keyboard window selects the matrix position,
// so the standard grid behavior would interfere with that.
ev.Skip(false);

#else

#ifdef _DEBUG

int key = KB_UNUSED;

#if defined(wxHAS_RAW_KEY_CODES) && defined(WIN32)
// get the raw key flags
wxUint32 rawFlags = ev.GetRawKeyFlags();
// Bits 16 .. 23: scancode
// Bit 24: flag whether extended key
int scancode = (rawFlags >> 16) & 0x1ff;
key = KbdGui::GetHIDFromScancode(scancode);

// without installing a keyboard hook, Print Screen is only released
if (key != KB_PSCRN)
  key = KB_UNUSED;

#else
// not for release builds until it really works!
int kc = ev.GetKeyCode();
int key = KbdGui::GetHID(kc);           /* retrieve HID for that key         */
#endif

if (key != KB_UNUSED)
  {
  int r = GetGridCursorRow(), c = GetGridCursorCol();
  if (r >= 0 && c >= 0)
    {
    SetCellValue(r, c, hid2Text[key]);
    GetApp()->GetLayout().SetKey(GetLayer(), c, r, key);
    }
  ev.Skip(false);
  }

#endif

#endif 
// default: do nothing. wxGrid gets it anyway.
}

/*****************************************************************************/
/* OnCellValueChanged : value of a cell has changed                          */
/*****************************************************************************/

void CMatrixWnd::OnCellValueChanged( wxGridEvent& ev )
{
int mrow = ev.GetRow(),
    mcol = ev.GetCol();
wxUint16 key = text2HID[GetCellValue(mrow, mcol)];

RC2Internal(mrow, mcol);
OnMatrixChanged(mrow, mcol, key);

/*
    wxLogMessage("Value of cell at (%d, %d) changed and is now \"%s\" "
                 "(was \"%s\")",
                 row, col,
                 grid->GetCellValue(row, col), ev.GetString());
*/

ev.Skip();
}

/*****************************************************************************/
/* SelectMatrix : called to select a grid element                            */
/*****************************************************************************/

void CMatrixWnd::SelectMatrix(int row, int col)
{
if (row < 0 || col < 0)
  return;
SetFocus();
RC2Internal(row, col);
GoToCell(row, col);
}

/*****************************************************************************/
/* OnMatrixChanged : called when a matrix value has changed                  */
/*****************************************************************************/

void CMatrixWnd::OnMatrixChanged(int row, int col, wxUint16 key)
{
GetApp()->GetLayout().SetKey(GetLayer(), row, col, key);
}
