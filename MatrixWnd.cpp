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

/*===========================================================================*/
/* Global Data                                                               */
/*===========================================================================*/

static struct
  {
  int hid;
  const wxChar *txt;
  } hidtxts[] =
  {
    //TYPE 255: Miscellaneous
#if 0 // not yet.
    { JMP_BOOT,        wxT("Boot") },         // Jump to bootloader
#endif
    { BR_UP,           wxT("Brightness+") },  // Increase LED brightness
    { BR_DOWN,         wxT("Brightness-") },  // Decrease LED brightness
    { MACRO_RECORD,    wxT("Macro Record") }, // Macro Record Start/Stop
    { MACRO_CLEAR_RECORDED, wxT("Macro Cancel Rec") },  // Macro Cancel Recording

    // TYPE 7: Macro keys
    { MACRO_1,         wxT("Macro  1") },
    { MACRO_2,         wxT("Macro  2") },
    { MACRO_3,         wxT("Macro  3") },
    { MACRO_4,         wxT("Macro  4") },
    { MACRO_5,         wxT("Macro  5") },
    { MACRO_6,         wxT("Macro  6") },
    { MACRO_7,         wxT("Macro  7") },
    { MACRO_8,         wxT("Macro  8") },
    { MACRO_9,         wxT("Macro  9") },
    { MACRO_10,        wxT("Macro 10") },
    { MACRO_11,        wxT("Macro 11") },
    { MACRO_12,        wxT("Macro 12") },
    { MACRO_13,        wxT("Macro 13") },
    { MACRO_14,        wxT("Macro 14") },
    { MACRO_15,        wxT("Macro 15") },
    { MACRO_16,        wxT("Macro 16") },
    { MACRO_17,        wxT("Macro 17") },
    { MACRO_18,        wxT("Macro 18") },
    { MACRO_19,        wxT("Macro 19") },
    { MACRO_20,        wxT("Macro 20") },
    { MACRO_21,        wxT("Macro 21") },
    { MACRO_22,        wxT("Macro 22") },
    { MACRO_23,        wxT("Macro 23") },
    { MACRO_24,        wxT("Macro 24") },

    // TYPE 6: System Control
    { SYSCTRL_POWERDOWN, wxT("Power Down") },
    { SYSCTRL_SLEEP,   wxT("Sleep") },
    { SYSCTRL_WAKEUP,  wxT("Wakeup") },

    // TYPE 5: Layer Toggle
    { TLAYER_0,        wxT("Toggle Layer 0") },
    { TLAYER_1,        wxT("Toggle Layer 1") },
    { TLAYER_2,        wxT("Toggle Layer 2") },
    { TLAYER_3,        wxT("Toggle Layer 3") },
    { TLAYER_4,        wxT("Toggle Layer 4") },
    { TLAYER_5,        wxT("Toggle Layer 5") },
#if 0 // Jörn defined a maximum of 6 layers
    { TLAYER_6,        wxT("Toggle Layer 6") },
    { TLAYER_7,        wxT("Toggle Layer 7") },
#endif

    //TYPE 4: Momentary Layer Actuation
    { MLAYER_0,        wxT("Momentary Layer 0") },
    { MLAYER_1,        wxT("Momentary Layer 1") },
    { MLAYER_2,        wxT("Momentary Layer 2") },
    { MLAYER_3,        wxT("Momentary Layer 3") },
    { MLAYER_4,        wxT("Momentary Layer 4") },
    { MLAYER_5,        wxT("Momentary Layer 5") },
#if 0 // Jörn defined a maximum of 6 layers
    { MLAYER_6,        wxT("Momentary Layer 6") },
    { MLAYER_7,        wxT("Momentary Layer 7") },
#endif

    //TYPE 3: Reserved for Mouse keys

    //TYPE 2: Media keys
    { MEDIA_PLAY,      wxT("Media Play") },
    { MEDIA_PAUSE,     wxT("Media Pause") },
    { MEDIA_NEXT,      wxT("Media Next") },
    { MEDIA_PREVIOUS,  wxT("Media Prev") },
    { MEDIA_STOP,      wxT("Media Stop") },
    { MEDIA_MUTE,      wxT("Media Mute") },
    { MEDIA_VOLUP,     wxT("Media Vol.Up") },
    { MEDIA_VOLDOWN,   wxT("Media Vol.Down") },
    { MEDIA_EMAIL,     wxT("Media Email") },
    { MEDIA_CALC,      wxT("Media Calc") },
    { MEDIA_BROWSER,   wxT("Media Browser") },
    { MEDIA_SEARCH,    wxT("Media Search") },
    { MEDIA_HOME,      wxT("Media Home") },
    { MEDIA_BACK,      wxT("Media Back") },
    { MEDIA_FORWARD,   wxT("Media Forward") },
    { MEDIA_REFRESH,   wxT("Media Refresh") },

    //TYPE 1: Keyboard modifiers
    { KB_LCTRL,        wxT("Left Control") },
    { KB_LSHFT,        wxT("Left Shift") },
    { KB_LALT,         wxT("Left Alt") },
    { KB_LGUI,         wxT("Left GUI") },
    { KB_RCTRL,        wxT("Right Control") },
    { KB_RSHFT,        wxT("Right Shift") },
    { KB_RALT,         wxT("Right Alt") },
    { KB_RGUI,         wxT("Right GUI") },

    //TYPE 0: Keyboard keycodes
    { KB_UNUSED,       wxT("Unused") },
    { KB_A,            wxT("A") },
    { KB_B,            wxT("B") },
    { KB_C,            wxT("C") },
    { KB_D,            wxT("D") },
    { KB_E,            wxT("E") },
    { KB_F,            wxT("F") },
    { KB_G,            wxT("G") },
    { KB_H,            wxT("H") },
    { KB_I,            wxT("I") },
    { KB_J,            wxT("J") },
    { KB_K,            wxT("K") },
    { KB_L,            wxT("L") },
    { KB_M,            wxT("M") },
    { KB_N,            wxT("N") },
    { KB_O,            wxT("O") },
    { KB_P,            wxT("P") },
    { KB_Q,            wxT("Q") },
    { KB_R,            wxT("R") },
    { KB_S,            wxT("S") },
    { KB_T,            wxT("T") },
    { KB_U,            wxT("U") },
    { KB_V,            wxT("V") },
    { KB_W,            wxT("W") },
    { KB_X,            wxT("X") },
    { KB_Y,            wxT("Y") },
    { KB_Z,            wxT("Z") },
    { KB_1,            wxT("1") },
    { KB_2,            wxT("2") },
    { KB_3,            wxT("3") },
    { KB_4,            wxT("4") },
    { KB_5,            wxT("5") },
    { KB_6,            wxT("6") },
    { KB_7,            wxT("7") },
    { KB_8,            wxT("8") },
    { KB_9,            wxT("9") },
    { KB_0,            wxT("0") },
    { KB_ENTER,        wxT("Enter") },
    { KB_ESC,          wxT("Escape") },
    { KB_BKSPC,        wxT("Backspace") },
    { KB_TAB,          wxT("Tab") },
    { KB_SPACE,        wxT("Space") },
    { KB_MINUS,        wxT("-") },
    { KB_EQUAL,        wxT("=") },
    { KB_LBRCE,        wxT("[") },
    { KB_RBRCE,        wxT("]") },
    { KB_BSLSH,        wxT("\\") },
    { KB_NUMBER,       wxT("# (Non-US)") },
    { KB_SMCLN,        wxT(";") },
    { KB_QUOTE,        wxT("\'") },
    { KB_TILDE,        wxT("~") },
    { KB_COMMA,        wxT(",") },
    { KB_DOT,          wxT(".") },
    { KB_SLASH,        wxT("/") },
    { KB_CAPLK,        wxT("Caps Lock") },
    { KB_F1,           wxT("F1") },
    { KB_F2,           wxT("F2") },
    { KB_F3,           wxT("F3") },
    { KB_F4,           wxT("F4") },
    { KB_F5,           wxT("F5") },
    { KB_F6,           wxT("F6") },
    { KB_F7,           wxT("F7") },
    { KB_F8,           wxT("F8") },
    { KB_F9,           wxT("F9") },
    { KB_F10,          wxT("F10") },
    { KB_F11,          wxT("F11") },
    { KB_F12,          wxT("F12") },
    { KB_PSCRN,        wxT("PrintScreen") },
    { KB_SCRLK,        wxT("Scroll Lock") },
    { KB_PAUSE,        wxT("Pause") },
    { KB_INS,          wxT("Insert") },
    { KB_HOME,         wxT("Home") },
    { KB_PGUP,         wxT("PageUp") },
    { KB_DEL,          wxT("Delete") },
    { KB_END,          wxT("End") },
    { KB_PGDN,         wxT("PageDown") },
    { KB_RIGHT,        wxT("Right") },
    { KB_LEFT,         wxT("Left") },
    { KB_DOWN,         wxT("Down") },
    { KB_UP,           wxT("Up") },
    { KB_NUMLK,        wxT("Num Lock") },
    { KP_SLASH,        wxT("Keypad /") },
    { KP_ASTRX,        wxT("Keypad *") },
    { KP_MINUS,        wxT("Keypad -") },
    { KP_PLUS,         wxT("Keypad +") },
    { KP_ENTER,        wxT("Keypad Enter") },
    { KP_1,            wxT("Keypad 1") },
    { KP_2,            wxT("Keypad 2") },
    { KP_3,            wxT("Keypad 3") },
    { KP_4,            wxT("Keypad 4") },
    { KP_5,            wxT("Keypad 5") },
    { KP_6,            wxT("Keypad 6") },
    { KP_7,            wxT("Keypad 7") },
    { KP_8,            wxT("Keypad 8") },
    { KP_9,            wxT("Keypad 9") },
    { KP_0,            wxT("Keypad 0") },
    { KP_DOT,          wxT("Keypad .") },
    { KB_PIPE,         wxT("| (Non-US)") },
    { KB_APP,          wxT("Application") },
#if 1
// presumably unusable ATM ...
    { KB_POWER,        wxT("Power") },
    { KP_EQUAL,        wxT("Keypad =") },
    { KB_F13,          wxT("F13") },
    { KB_F14,          wxT("F14") },
    { KB_F15,          wxT("F15") },
    { KB_F16,          wxT("F16") },
    { KB_F17,          wxT("F17") },
    { KB_F18,          wxT("F18") },
    { KB_F19,          wxT("F19") },
    { KB_F20,          wxT("F20") },
    { KB_F21,          wxT("F21") },
    { KB_F22,          wxT("F22") },
    { KB_F23,          wxT("F23") },
    { KB_F24,          wxT("F24") },
    { KB_EXECUTE,      wxT("Execute") },
    { KB_HELP,         wxT("Help") },
    { KB_MENU,         wxT("Menu") },
    { KB_SELECT,       wxT("Select") },
    { KB_STOP,         wxT("Stop") },
    { KB_AGAIN,        wxT("Again") },
    { KB_UNDO,         wxT("Undo") },
    { KB_CUT,          wxT("Cut") },
    { KB_COPY,         wxT("Copy") },
    { KB_PASTE,        wxT("Paste") },
    { KB_FIND,         wxT("Find") },
    { KB_MUTE,         wxT("Mute") },
    { KB_VOLUMEUP,     wxT("Volume Up") },
    { KB_VOLUMEDOWN,   wxT("Volume Down") },
    { KB_LCAPLK,       wxT("Locking Caps Lock") },
    { KB_LNUMLK,       wxT("Locking Num Lock") },
    { KB_LSCRLK,       wxT("Locking Scroll Lock") },
    { KP_COMMA,        wxT("Keypad ,") },   // "special key on Brazilian keyboard" (HUT Note 27)
    { KP_EQUALSIGN,    wxT("Keypad Equal Sign") },
    { KB_INTL1,        wxT("International 1") },
    { KB_INTL2,        wxT("International 2") },
    { KB_INTL3,        wxT("International 3") },
    { KB_INTL4,        wxT("International 4") },
    { KB_INTL5,        wxT("International 5") },
    { KB_INTL6,        wxT("International 6") },
    { KB_INTL7,        wxT("International 7") },
    { KB_INTL8,        wxT("International 8") },
    { KB_INTL9,        wxT("International 9") },
    { KB_LANG1,        wxT("Lang 1") },  // Hangul/English
    { KB_LANG2,        wxT("Lang 2") },  // Hanja Conversion
    { KB_LANG3,        wxT("Lang 3") },  // Katakana
    { KB_LANG4,        wxT("Lang 4") },  // Hiragana
    { KB_LANG5,        wxT("Lang 5") },  // Zenkaku/Hankaku
    { KB_LANG6,        wxT("Lang 6") },
    { KB_LANG7,        wxT("Lang 7") },
    { KB_LANG8,        wxT("Lang 8") },
    { KB_LANG9,        wxT("Lang 9") },
    { KB_ALT_ERA,      wxT("Alt Erase") },
    { KB_SYSREQ,       wxT("SysReq/Attention") },
    { KB_CANCEL,       wxT("Cancel") },
    { KB_CLEAR,        wxT("Clear") },
    { KB_PRIOR,        wxT("Prior") },
    { KB_RETURN,       wxT("Return") },
    { KB_SEP,          wxT("Separator") },
    { KB_OUT,          wxT("Out") },
    { KB_OPER,         wxT("Oper") },
    { KB_CLR_AGN,      wxT("Clear/Again") },
    { KB_CRSEL,        wxT("CrSel/Props") },
    { KB_EXSEL,        wxT("ExSel") },
    { KP_00,           wxT("Keypad 00") },
    { KP_000,          wxT("Keypad 000") },
    { KB_THOU_SEP,     wxT("Thousands Separator") },
    { KB_DEC_SEP,      wxT("Decimal Separator") },
    { KB_CUR_UNIT,     wxT("Currency Unit") },
    { KB_CUR_SUBU,     wxT("Currency Sub-unit") },
    { KP_LPAREN,       wxT("Keypad (") },
    { KP_RPAREN,       wxT("Keypad )") },
    { KP_LBRACE,       wxT("Keypad {") },
    { KP_RBRACE,       wxT("Keypad }") },
    { KP_TAB,          wxT("Keypad Tab") },
    { KP_BACKSPACE,    wxT("Keypad Backspace") },
    { KP_A,            wxT("Keypad A") },
    { KP_B,            wxT("Keypad B") },
    { KP_C,            wxT("Keypad C") },
    { KP_D,            wxT("Keypad D") },
    { KP_E,            wxT("Keypad E") },
    { KP_F,            wxT("Keypad F") },
    { KP_XOR,          wxT("Keypad XOR") },
    { KP_POWER,        wxT("Keypad ^") },
    { KP_MOD,          wxT("Keypad %") },
    { KP_LESS,         wxT("Keypad <") },
    { KP_GREATER,      wxT("Keypad >") },
    { KP_BIT_AND,      wxT("Keypad &") },
    { KP_AND,          wxT("Keypad &&") },
    { KP_BIT_OR,       wxT("Keypad |") },
    { KP_OR,           wxT("Keypad ||") },
    { KP_COLON,        wxT("Keypad :") },
    { KP_NUMBER,       wxT("Keypad #") },
    { KP_SPACE,        wxT("Keypad Space") },
    { KP_AT,           wxT("Keypad @") },
    { KP_NOT,          wxT("Keypad !") },
    { KP_MEM_STR,      wxT("Keypad Mem Store") },
    { KP_MEM_RCL,      wxT("Keypad Mem Recall") },
    { KP_MEM_CLR,      wxT("Keypad Mem Clear") },
    { KP_MEM_ADD,      wxT("Keypad Mem Add") },
    { KP_MEM_SUB,      wxT("Keypad Mem Subtract") },
    { KP_MEM_MUL,      wxT("Keypad Mem Multiply") },
    { KP_MEM_DIV,      wxT("Keypad Mem Divide") },
    { KP_PLSMNS,       wxT("Keypad +/-") },
    { KP_CLEAR,        wxT("Keypad Clear") },
    { KP_CLR_ENT,      wxT("Keypad Clear Entry") },
    { KP_BINARY,       wxT("Keypad Binary") },
    { KP_OCTAL,        wxT("Keypad Octal") },
    { KP_DECIMAL,      wxT("Keypad Decimal") },
    { KP_HEXADEC,      wxT("Keypad Hexadecimal") },
#endif

#if 0  // not yet.
    // replaced by Type 2 above ... just not completely.
    { KB_MEDIA_NTRK,   wxT("Next Track") },
    { KB_MEDIA_PTRK,   wxT("Previous Track") },
    { KB_MEDIA_STOP,   wxT("Stop") },
    { KB_MEDIA_PLPS,   wxT("Play/Pause") },
    { KB_MEDIA_PLPS,   wxT("Mute") },
    { KB_MEDIA_PLPS,   wxT("Bass Boost") },
    { KB_MEDIA_LDNS,   wxT("Loudness") },
    { KB_MEDIA_VOLUP,  wxT("Volume+") },
    { KB_MEDIA_VOLDN,  wxT("Volume-") },
    { KB_MEDIA_BASSUP, wxT("Bass+") },
    { KB_MEDIA_BASSDN, wxT("Bass-") },
    { KB_MEDIA_TREBUP, wxT("Treble+") },
    { KB_MEDIA_TREBDN, wxT("Treble-") },
    { KB_MEDIA_MSEL,   wxT("Media Select") },
    { KB_MEDIA_MAIL,   wxT("Mail") },
    { KB_MEDIA_CALC,   wxT("Calculator") },
    { KB_MEDIA_MYCMP,  wxT("My Computer") },
    { KB_MEDIA_WWWSRC, wxT("WWW Search") },
    { KB_MEDIA_WWWHOM, wxT("WWW Home") },
    { KB_MEDIA_WWWBK,  wxT("WWW Back") },
    { KB_MEDIA_WWWFW,  wxT("WWW Forward") },
    { KB_MEDIA_WWWSTP, wxT("WWW Stop") },
    { KB_MEDIA_WWWRFR, wxT("WWW Refresh") },
    { KB_MEDIA_WWWFAV, wxT("WWW Favorites") },
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

void SetupText2HIDMapping()
{
if (text2HID.size())
  return;

for (int i = 0; i < _countof(hidtxts); i++)
  {
  hid2Text[hidtxts[i].hid] = hidtxts[i].txt;
  text2HID[hidtxts[i].txt] = hidtxts[i].hid;
  hidTexts.push_back(hidtxts[i].txt);
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
SetLayout(kbm.GetRows(), kbm.GetCols());
for (int i = 0; i < GetNumberCols(); i++)
  for (int j = 0; j < GetNumberRows(); j++)
    {
    wxUint16 key = kbm.GetKey(i, j);
    int r(i), c(j);
    RC2Internal(r, c);
    SetCellValue(r, c, hid2Text[key]);
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
