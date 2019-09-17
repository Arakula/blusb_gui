/*****************************************************************************/
/* blusb_gui.cpp : main program                                              */
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

// THIS still needs to be done for Linux / Mac!
#ifndef wxHAS_IMAGES_IN_RESOURCES
    // include all .xpms needed in wtc32.cpp
#endif

#include "MainFrm.h"
#include "MatrixWnd.h"
#include "blusb_gui.h"

/*****************************************************************************/
/* Default Matrix layer contents                                             */
/*****************************************************************************/

// Default IBM Model M Keyboard Matrix Layout
static MatrixKey defIBMmatrix[] =
  {
  // 0       1         2         3         4          5         6        7          8      9          10     11       12        13        14        15
  KB_LALT,   KB_UP,    KP_DOT,   KP_0,     KB_UNUSED, KB_QUOTE, KB_NONE, KB_F6,     KB_H,  KB_F5,     KB_G,  KB_F4,   KB_PIPE,  KB_ESC,   KB_NONE,  KB_NONE,
  KB_NONE,   KB_INTL6, KP_6,     KP_5,     KP_4,      KB_LBRCE, KB_F7,   KB_RBRCE,  KB_Y,  KB_BKSPC,  KB_T,  KB_F3,   KB_CAPLK, KB_TAB,   KB_LSHFT, KB_NONE,
  KB_NONE,   KB_HOME,  KB_PGUP,  KB_INS,   KB_DEL,    KB_MINUS, KB_F8,   KB_EQUAL,  KB_6,  KB_F9,     KB_5,  KB_F2,   KB_F1,    KB_TILDE, KB_NONE,  KB_LCTRL,
  KB_PSCRN,  KB_END,   KB_PGDN,  KB_F12,   KB_F11,    KB_0,     KB_9,    KB_8,      KB_7,  KB_F10,    KB_4,  KB_3,    KB_2,     KB_1,     KB_NONE,  KB_NONE,
  KB_SCRLK,  KP_PLUS,  KP_9,     KP_8,     KP_7,      KB_P,     KB_O,    KB_I,      KB_U,  KB_INTL4,  KB_R,  KB_E,    KB_W,     KB_Q,     KB_NONE,  KB_NONE,
  KB_UNUSED, KP_ENTER, KP_3,     KP_2,     KP_1,      KB_SMCLN, KB_L,    KB_K,      KB_J,  KB_BSLSH,  KB_F,  KB_D,    KB_S,     KB_A,     KB_NONE,  KB_NONE,
  KB_NONE,   KB_PAUSE, KP_ASTRX, KP_SLASH, KB_NUMLK,  KB_BSLSH, KB_DOT,  KB_COMMA,  KB_M,  KB_ENTER,  KB_V,  KB_C,    KB_X,     KB_Z,     KB_RSHFT, KB_RCTRL,
  KB_RALT,   KB_LEFT,  KP_MINUS, KB_RIGHT, KB_DOWN,   KB_SLASH, KB_NONE, KB_INTL3,  KB_N,  KB_SPACE,  KB_B,  KB_NONE, KB_NONE,  KB_NONE,  KB_NONE,  KB_NONE
  };
// Unused matrix connections on ANSI / European ISO:
// 0/4 - left part of NumPad 0
// 1/1 - lower part of NumPad +     (presumably KB_INTL6 on Brazilian)
// 4/9 - left part of Backspace     (presumably KB_INTL4)
// 5/0 - lower part of NumPad Enter
// 7/7 - left part of Right Shift   (/? on Brazilian - presumably KB_INTL3)

// Default 122-key IBM Model M Keyboard Matrix Layout
// \ is at 5/13 (ANSI) and 6/12 (ISO)
static MatrixKey def122matrix[] =
  {
//0         1         2         3        4        5     6     7       8       9       10        11       12        13        14        15        16        17       18        19        
  KB_LGUI,  KB_RALT,  KB_NONE,  KB_NONE, KB_NONE, KB_G, KB_H, KB_F13, KB_F1,  KB_F2,  KB_NONE,  KB_NONE, KB_QUOTE, KB_DOWN,  KB_NONE,  KB_HOME,  KB_NONE,  KB_UP,   KB_LCTRL, KB_APP,   
  KB_NONE,  KB_NONE,  KB_A,     KB_S,    KB_D,    KB_F, KB_J, KB_F14, KB_F15, KB_F3,  KB_K,     KB_L,    KB_SMCLN, KP_4,     KP_6,     KB_RIGHT, KP_5,     KB_DEL,  KP_PLUS,  KB_NONE,  
  KB_NONE,  KB_NONE,  KB_1,     KB_2,    KB_3,    KB_4, KB_7, KB_F16, KB_F4,  KB_F5,  KB_8,     KB_9,    KB_0,     KB_END,   KP_SLASH, KB_NONE,  KB_NUMLK, KB_PGUP, KP_ASTRX, KB_PSCRN, 
  KB_SCRLK, KB_NONE,  KB_TILDE, KB_NONE, KB_NONE, KB_5, KB_6, KB_F17, KB_F18, KB_F6,  KB_EQUAL, KB_NONE, KB_MINUS, KB_BKSPC, KB_NONE,  KB_NONE,  KB_HOME,  KB_INS,  KB_NONE,  KB_ESC,   
  KB_TAB,   KB_NONE,  KB_Q,     KB_W,    KB_E,    KB_R, KB_U, KB_F19, KB_F7,  KB_F8,  KB_I,     KB_O,    KB_P,     KP_7,     KP_9,     KB_NONE,  KP_8,     KB_PGDN, KP_MINUS, KB_PAUSE, 
  KB_NONE,  KB_NONE,  KB_NONE,  KB_NONE, KB_NONE, KB_T, KB_Y, KB_F20, KB_F21, KB_F9,  KB_RBRCE, KB_NONE, KB_LBRCE, KB_BSLSH, KB_NONE,  KB_NONE,  KB_NONE,  KB_END,  KB_NONE,  KB_NONE,  
  KB_CAPLK, KB_RSHFT, KB_Z,     KB_X,    KB_C,    KB_V, KB_M, KB_F22, KB_F10, KB_F11, KB_COMMA, KB_DOT,  KB_BSLSH, KB_ENTER, KP_3,     KB_NONE,  KP_2,     KB_NONE, KB_LEFT,  KP_1,     
  KB_RCTRL, KB_LSHFT, KB_PIPE,  KB_NONE, KB_NONE, KB_B, KB_N, KB_F23, KB_F24, KB_F12, KB_NONE,  KB_NONE, KB_SLASH, KB_NONE,  KP_DOT,   KP_ENTER, KP_0,     KB_NONE, KB_LALT,  KB_SPACE, 
  };                                                                                                                                                                                      
// Not yet determined:
// 5/19 - EX5
// 5/0  - EX6
// 1/19 - EX7
// 1/0  - EX8
// Unused matrix connections on ANSI / European ISO:
// 3/1  - left part of Backspace     (presumably KB_INTL4)
// 6/15 - lower part of NumPad Enter
// 7/13 - left part of NumPad 0
// 7/11 - left part of Right Shift   (/? on Brazilian - presumably KB_INTL3)


/*===========================================================================*/
/* CBlusbGuiApp class members                                                */
/*===========================================================================*/

wxIMPLEMENT_APP(CBlusbGuiApp);
CBlusbGuiApp *GetApp() { return (CBlusbGuiApp *)wxTheApp; }

/*****************************************************************************/
/* CBlusbGuiApp : constructor                                                */
/*****************************************************************************/

CBlusbGuiApp::CBlusbGuiApp()
{
pMain = NULL;
inServiceMode = false;
bCtlLayoutRead = false;
curDefaultLayout = 0;
nDevMatrixRows = nDevMatrixCols = -1;
}

/*****************************************************************************/
/* CBlusbGuiApp event table                                                  */
/*****************************************************************************/

wxBEGIN_EVENT_TABLE(CBlusbGuiApp, wxApp)
  EVT_ACTIVATE_APP(CBlusbGuiApp::OnActivateApp)
wxEND_EVENT_TABLE()

/*****************************************************************************/
/* OnInit : application initialization                                       */
/*****************************************************************************/

bool CBlusbGuiApp::OnInit()
{
if (!wxApp::OnInit() )
  return false;
// command line parameters are set up now

//KbdGui::SetDefault(true);
#if 0
// #ifdef _DEBUG
{
// write the default GUI layout files
KbdGui::SetDefault();
KbdGui guiAnsi;
guiAnsi.WriteLayoutFile(wxT("101KeyANSI.kbl"));
KbdGui::SetDefault(true);
KbdGui guiISO;
guiISO.WriteLayoutFile(wxT("102KeyISO.kbl"));
KbdGui::SetDefault();
}
#endif
#if 0
//#ifdef _DEBUG
{
KbdGui copygui;
wxString err;
if (copygui.ReadLayoutFile(wxT("102KeyISOmod.kbl"), &err))
  copygui.WriteLayoutFile(wxT("102KeyISO2.kbl"));
else
  wxMessageBox(err, wxT("Load 102KeyISOmod.kbl"));
#if 1
if (copygui.ReadLayoutFile(wxT("SamsungGer.kbl"), &err))
  copygui.WriteLayoutFile(wxT("SamsungGer2.kbl"));
else
  wxMessageBox(err, wxT("Load SamsungGer.kbl"));
}
#endif
#endif

SetAppName(wxT("blusb_gui"));
SetVendorName(wxT("Seib"));
SetVendorDisplayName(wxT("Hermann Seib"));
pConfig = wxConfigBase::Get();
wxImage::AddHandler(new wxPNGHandler);

SetupText2HIDMapping();
                                        /* setup default layouts             */
defaultLayout[0].Resize(1, 8, 16, 0, defIBMmatrix);
defaultLayout[1].Resize(1, 8, 20, 0, def122matrix);
layout = defaultLayout[0];              /* and init current to normal M      */

int rc = dev.Open();                    /* try to open the device            */
if (rc == BLUSB_SUCCESS)                /* if done,                          */
  rc = ReadLayout();                    /* fetch current layout from Model M */
if (rc != BLUSB_SUCCESS)
  {
  // dev.Close();
  if (wxMessageBox(wxT("Make sure the Model M is connected to a USB port and switched to USB mode.\n\n")
				   wxT("If it is, and has never been initialized before, just continue. ")
				   wxT("As soon as a layout has been sent to the keyboard, this message will disappear.\n")
#ifdef WIN32
                   wxT("If it is, and has been initialized, and its firmware is V1.04 or below: ")
                   wxT("don't panic! This is a simple driver issue. ")
                   wxT("If you have not already done so, download Zadig from\n")
                   wxT("  http://zadig.akeo.ie")
                   wxT("\nand install the WinUSB driver. ")
                   wxT("If that doesn't work, you can also give the LibUSB-win32 driver a try, whatever is going to work for you.\n")
                   wxT("Once you are done configuring and want to use the Model M as a USB-attached keyboard again, ")
                   wxT("uninstall the Model M USB device in Device Manager including device driver removal ")
                   wxT("and then do a device rescan.\n")
                   wxT("Quirky Windows(c) likes a little tinkering!\n\n")
#endif
                   wxT("Continue without attached Model M?"),
                   wxT("Model M Open Error"),
                   wxYES_NO | wxCENTRE | wxICON_QUESTION) != wxYES)
    return false;
  }

if (GetFwVersion() >= 0x0105)           /* V1.5 and above is always in       */
  inServiceMode = true;                 /* "service mode".                   */

// install hook and disable GUI keys
CKbdWnd::HookLLKeyboard(true);
CKbdWnd::InhibitGuiKey(true);

int screenX = wxSystemSettings::GetMetric(wxSYS_SCREEN_X),
    screenY = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
wxSize szWin;
szWin.x = screenX > 1200 ? 1150 : 1000;
szWin.x = min(szWin.x, screenX);
szWin.y = screenY >= 800 ? 750 : 700;
szWin.y = min(szWin.y, screenY);
pMain = new CMainFrame(wxT("Blusb GUI"),
                       wxDefaultPosition,
                       szWin);
pMain->Show();
pMain->SetKbdLayout(layout);

return true;
}

/*****************************************************************************/
/* OnExit : application termination                                          */
/*****************************************************************************/

int  CBlusbGuiApp::OnExit()
{
RemoveText2HIDMapping();
dev.DisableServiceMode();   // just in case the user didn't.

CKbdWnd::HookLLKeyboard(false);  // make sure the low-level hook is deinstalled

return wxApp::OnExit();
}

/*****************************************************************************/
/* command line handling functionality (called in wxApp::OnInit())           */
/*****************************************************************************/

void CBlusbGuiApp::OnInitCmdLine(wxCmdLineParser& parser)
{
// enable standard wxWidgets command line parameters:
// /h, --help, --verbose
wxApp::OnInitCmdLine(parser);

#if 0 // not yet!
// then add our own parameters
static wxCmdLineEntryDesc cmdParms[] =
  {
  // kind, shortname, longname, description, type, flags
  { wxCMD_LINE_SWITCH,
        wxT("l"), wxT("log"), wxT("enable logging"),
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_SWITCH_NEGATABLE },
  { wxCMD_LINE_SWITCH,
        wxT("o"), wxT("nologo"), wxT("disable splash screen"),
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_SWITCH_NEGATABLE },
  { wxCMD_LINE_NONE }
  };
parser.SetDesc(cmdParms);
#endif
}

bool CBlusbGuiApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
#if 0  // not yet!
// fetch parsed command line parameters
#endif
return wxApp::OnCmdLineParsed(parser);
}


/*****************************************************************************/
/* OnActivateApp : called when the app is (de)activated                      */
/*****************************************************************************/

void CBlusbGuiApp::OnActivateApp(wxActivateEvent& event)
{
CKbdWnd::InhibitGuiKey(event.GetActive());
event.Skip();
}

/*****************************************************************************/
/* SetDefaultLayout : set best matching default layout for passed GUI layout */
/*****************************************************************************/

void CBlusbGuiApp::SetDefaultLayout(KbdGui const &gui)
{
int rows, cols;
gui.GetMatrixLayout(rows, cols);
// use the smallest default layout >= new matrix
int usedef = 0;                     /* default to default default :-)    */
for (int i = GetDefaultLayouts() - 1; i >= 0; i--)
  {
  KbdLayout const &dl = GetDefaultLayout(i);
  int dlrows = dl.GetRows(), dlcols = dl.GetCols();
  if ((dlrows > rows && dlcols >= cols) ||
      (dlrows == rows && dlcols >= cols))
    usedef = i;
  }
SetDefaultLayout(usedef);
}

/*****************************************************************************/
/* ReadMatrixLayout : read matrix layout from keyboard                       */
/*****************************************************************************/

int CBlusbGuiApp::ReadMatrixLayout(int &rows, int &cols)
{
if (!dev.IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

if (nDevMatrixRows > 0)
  {
  rows = nDevMatrixRows;
  cols = nDevMatrixCols;
  return BLUSB_SUCCESS;
  }

// read current layout from controller to determine the matrix layout

wxMemoryBuffer mb;                      /* fetch current layout from Model M */
mb.SetBufSize(4096);
mb.SetDataLen(4096);                    /* wxWidgets memory buffer needs both*/
wxUint8 *lbuf = (wxUint8 *)mb.GetData();
memset(lbuf, 0, 4096);
int rc = dev.ReadLayout(lbuf, mb.GetDataLen());
if (rc < BLUSB_SUCCESS)
  return rc;

int fwVer = dev.GetFwVersion();
int numlayers_max = (fwVer < 0x0105) ? NUMLAYERS_MAX_OLD : NUMLAYERS_MAX;
int numlayers_bytes = (fwVer < 0x0105) ? 2 : 1;
int numcols = -1;                       /* calculate # columns in buffer     */
if (lbuf[0] > 0 &&                      /* did we receive meaningful data?   */
    lbuf[0] <= numlayers_max &&
    rc > numlayers_bytes)
  {
  if (fwVer >= 0x0105)  // this is DEFINITELY 20.
    {
    rows = nDevMatrixRows = NUMROWS;
    cols = nDevMatrixCols = NUMCOLS;
    return BLUSB_SUCCESS;
    }
  else
    {
    // if so, look whether it's a multiple of 20 columns
    int layerbytes = (rc - numlayers_bytes) / lbuf[0];
    if (layerbytes * lbuf[0] == (rc - numlayers_bytes))
      {
      int layercols = layerbytes / (2 * NUMROWS);
      if (layercols * (2 * NUMROWS) == layerbytes)
        numcols = layercols;

      rows = nDevMatrixRows = NUMROWS;
      cols = nDevMatrixCols = numcols;
      return BLUSB_SUCCESS;
      }
    }
  }

return BLUSB_ERROR_NO_DEVICE;
}

/*****************************************************************************/
/* ReadLayout : read layout from Model M or file                             */
/*****************************************************************************/

int CBlusbGuiApp::ReadLayout(KbdLayout *p)
{
int numrows = -1, numcols = -1;         /* get rows / columns in buffer      */
int rc = ReadMatrixLayout(numrows, numcols);
if (rc < BLUSB_SUCCESS)
  return rc;

int fwVer = dev.GetFwVersion();
if (!p)
  p = &layout;
wxMemoryBuffer mb;                      /* fetch current layout from Model M */
mb.SetBufSize(4096);
mb.SetDataLen(4096);                    /* wxWidgets memory buffer needs both*/
wxUint8 *lbuf = (wxUint8 *)mb.GetData();
memset(lbuf, 0, 4096);
rc = dev.ReadLayout(lbuf, mb.GetDataLen());
if (rc < BLUSB_SUCCESS)
  return rc;
if (rc == 0)                            /* uninitialized controller ?        */
  {
#if 0
  // this could be used for automatic controller initialization
  lbuf[0] = 1;                          /* write out an empty layer          */
  if (dev.WriteLayout(lbuf, mb.GetDataLen()) >= BLUSB_SUCCESS)
    {
    lbuf[0] = 0;                        /* then read again                   */
    rc = dev.ReadLayout(lbuf, mb.GetDataLen());
    }
#endif
  }

wxUint8 *macbuf = NULL;
int macsize = 0;
wxMemoryBuffer macmb;
if (fwVer >= 0x0104)                    /* known first macro container       */
  {
  macmb.SetBufSize(1024);
  macmb.SetDataLen(1024);               /* wxWidgets memory buffer needs both*/
  macbuf = (wxUint8 *)macmb.GetData();
  memset(macbuf, 0xff, macmb.GetDataLen());
  if ((macsize = dev.ReadMacros(macbuf, macmb.GetDataLen())) <= 0)
    macsize = 0;
  }

if (!p->Import(lbuf, mb.GetBufSize(), numrows, numcols,
	macbuf, macsize, (fwVer >= 0x0105) ? 1 : 0))
  return -103;
bCtlLayoutRead = true;
return BLUSB_SUCCESS;
}

int CBlusbGuiApp::ReadLayout(wxString const &filename, KbdLayout *p)
{
if (!p)
  p = &layout;
bool bOK = p->ReadFile(filename);
if (bOK)
  bCtlLayoutRead = false;
return bOK ? BLUSB_SUCCESS : -100;
}

/*****************************************************************************/
/* WriteLayout : write layout to Model M or file                             */
/*****************************************************************************/

int CBlusbGuiApp::WriteLayout(KbdLayout *p)
{
wxMemoryBuffer mb;                      /* write current layout to Model M   */
mb.SetBufSize(4096);
mb.SetDataLen(4096);                    /* wxWidgets memory buffer needs both*/
wxUint8 *lbuf = (wxUint8 *)mb.GetData();
int lbufsz = mb.GetDataLen();

int fwVer = dev.GetFwVersion();

int numrows = -1, numcols = -1;         /* get rows / columns in buffer      */
int rc = ReadMatrixLayout(numrows, numcols);
if (rc < BLUSB_SUCCESS)                 /* matrix not determined?            */
  {
  if (dev.IsOpen())                     /* ... mhm. Empty controller?        */
    {
    // try automatic controller initialization
    memset(lbuf, 0, lbufsz);
    lbuf[0] = 1;                        /* write out an empty layer          */
    if (dev.WriteLayout(lbuf, lbufsz) >= BLUSB_SUCCESS)
      rc = ReadMatrixLayout(numrows, numcols);
    }
  if (rc < BLUSB_SUCCESS)
    return rc;
  }

if (!p)
  p = &layout;
if (!p->Export(lbuf, lbufsz, numrows, numcols, (fwVer >= 0x0105) ? 1 : 0))
  return BLUSB_ERROR_OVERFLOW;
rc = dev.WriteLayout(lbuf, lbufsz);
if (rc >= BLUSB_SUCCESS)
  {
  // Macros
  }
return rc;
}

int CBlusbGuiApp::WriteLayout
    (
    wxString const &filename,
    bool bNative,
    KbdLayout *p
    )
{
if (!p)
  p = &layout;

return p->WriteFile(filename, bNative) ? BLUSB_SUCCESS : -100;
}