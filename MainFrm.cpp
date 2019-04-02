/*****************************************************************************/
/* MainFrm.cpp : implementation of the CMainFrame class                      */
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

#if WITH_SPLASH
#include "Splash.h"
#endif
#include "blusb_gui.h"

#include "MainFrm.h"

#ifndef wxHAS_IMAGES_IN_RESOURCES
  #include "res/Application.xpm"
#endif


/*===========================================================================*/
/* CMatrixPanel class members                                                */
/*===========================================================================*/

/*****************************************************************************/
/* Layout : performa layout operations                                       */
/*****************************************************************************/

bool CMatrixPanel::Layout()
{
wxSize szNew = pMatrix->GetMinSize();
if (szMatrix != szNew)
  {
#if 0
  SetMinSize(szNew);  // propagate matrix's minimum size
  Fit();
  // TODO: replace the above with more elaborate moving/resizing once additional
  //       elements are put on this panel!
  wxWindow *pMain = GetApp()->GetMain();
  if (pMain && szMatrix.x)
    {
    wxSize szMain = pMain->GetSize();
    if (szMain.x > 0 && szMain.y > 0)
      {
      szMain.x += (szNew.x - szMatrix.x);
      szMain.y += (szNew.y - szMatrix.y);
      pMain->SetSize(szMain);
      pMain->Fit();
      }
    }
#endif
  szMatrix = szNew;
  }
return wxPanel::Layout();
}


/*===========================================================================*/
/* CKbdPanel class members                                                   */
/*===========================================================================*/

/*****************************************************************************/
/* CKbdPanel : constructor                                                   */
/*****************************************************************************/

CKbdPanel::CKbdPanel
    (
    wxWindow *parent,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
    )
  : wxPanel(parent, winid, pos, size, style, name)
{
pKbd = new CKbdWnd(this, wxID_ANY, wxPoint(0,0),
                   wxSize(1, 1));  // keyboard sets that up
szKbd = pKbd->GetMinSize();
pKbd->CaptureAllKeys();

// We're not interested in the "released" state at the moment
SetKbdColour(CKbdWnd::ksReleased,
             GetKbdColour(CKbdWnd::ksUnpressed));
SetKbdColour(CKbdWnd::ksReleased | CKbdWnd::ksHighlighted,
             GetKbdColour(CKbdWnd::ksUnpressed | CKbdWnd::ksHighlighted));

SetSize(szKbd); // propagate keyboard's minimum size ATM
SetMinSize(szKbd); // propagate keyboard's minimum size ATM
}

/*****************************************************************************/
/* Layout : performs layout operations                                       */
/*****************************************************************************/

bool CKbdPanel::Layout()
{
wxSize szKbdNew = pKbd->GetMinSize();
if (szKbd != szKbdNew)
  {
  SetMinSize(szKbdNew);  // propagate keyboard's minimum size
  Fit();
  // TODO: replace the above with more elaborate moving/resizing once additional
  //       elements are put on this panel!

  wxWindow *pMain = GetApp()->GetMain();
  if (pMain)
    {
    wxSize szMain = pMain->GetSize();
    if (szMain.x > 0 && szMain.y > 0)
      {
      szMain.x += (szKbdNew.x - szKbd.x);
      szMain.y += (szKbdNew.y - szKbd.y);
      pMain->SetSize(szMain);
      pMain->Fit();
      }
    }
  szKbd = szKbdNew;
  }
return wxPanel::Layout();
}

// TODO: 
// Setting the LEDs has to be done with SendInput() in Windows;
// there seems to be no equivalent in wxWidgets.
// So ... find out substitutions for the other platforms <sigh>


/*===========================================================================*/
/* CMainPanel class members                                                  */
/*===========================================================================*/

/*****************************************************************************/
/* CMainPanel : constructor                                                  */
/*****************************************************************************/

CMainPanel::CMainPanel(wxWindow *parent)
       : wxPanel(parent, wxID_ANY)
{
wxStaticText *pStatic = new wxStaticText(this, wxID_ANY, wxT("Layers: "));
wxPoint pos = pStatic->GetPosition();
pos.x += 3;
pStatic->Move(pos);
pos.x += pStatic->GetSize().GetX();
pLayers = new wxChoice(this, Blusb_LayerCount, pos);
for (int i = 1; i <= 6; i++)
  pLayers->Append(wxString::Format(wxT("%d"), i));
pLayers->Select(0);
pos = pStatic->GetPosition();
pos.y += (pLayers->GetSize().GetY() - pStatic->GetSize().GetY()) / 2;
pStatic->Move(pos);

#ifdef _DEBUG
if (true)
#else
if (GetApp()->IsDevOpen())
#endif
  {
  pos.x = pLayers->GetPosition().x + pLayers->GetSize().GetWidth() + 20;
  pos.y = pStatic->GetPosition().y;
  wxStaticText *pStDebounce = new wxStaticText(this, wxID_ANY,
                                               wxT("Debounce time (ms): "),
                                               pos);
  pos.x += pStDebounce->GetSize().GetX();
  pos.y = pLayers->GetPosition().y;
  pDebounce = new wxChoice(this, Blusb_Debounce, pos);
  for (int i = 1; i <= 20; i++)
    pDebounce->Append(wxString::Format(wxT("%d"), i));
  int nDebounce = GetApp()->ReadDebounce();
  if (nDebounce < 1 || nDebounce > 20)
    nDebounce = 7;
  pDebounce->Select(nDebounce - 1);
  }
else
  pDebounce = NULL;


wxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
sizerV->AddSpacer(pLayers->GetSize().GetY() + 5);

// needs to be loaded before the matrix window is created
wxString sLayout;
GetApp()->ReadConfig("/Settings/KbdLayout", &sLayout, "ANSI");
KbdGui kbdGui(!sLayout.CmpNoCase("ISO122") ? KbdGui::KbdISO122 :
              !sLayout.CmpNoCase("ANSI121") ? KbdGui::KbdANSI121 :
              !sLayout.CmpNoCase("ISO") ? KbdGui::KbdISO :
              KbdGui::KbdANSI);
bool bLayoutOK = true;
if (sLayout.CmpNoCase("ISO") &&
    sLayout.CmpNoCase("ANSI") &&
    sLayout.CmpNoCase("ISO122") &&
    sLayout.CmpNoCase("ANSI121"))
  bLayoutOK = kbdGui.ReadLayoutFile(sLayout);
if (bLayoutOK)
  {
  GetApp()->SetDefaultLayout(kbdGui);
  GetApp()->SetLayout(GetApp()->GetDefaultLayout());
  kbdGuiLayout = kbdGui;
  }

pMatrixNotebook = new wxNotebook(this, wxID_ANY);
SetLayers(1, true);
// TODO: add keyboard layout page(s?)
sizerV->Add(pMatrixNotebook, wxSizerFlags(1).Expand());

wxSize szMatrix = matrices[0]->GetSize();
szMatrix.y = 10;
pKbd = new CKbdPanel(this, wxID_ANY, wxDefaultPosition, szMatrix);
if (bLayoutOK)
  pKbd->SetKbdLayout(kbdGui);
sizerV->Add(pKbd, wxSizerFlags(0).Expand());

SetSizerAndFit(sizerV);
}

/*****************************************************************************/
/* SetLayers : sets up the number of layers                                  */
/*****************************************************************************/

bool CMainPanel::SetLayers(int nLayers, bool resetExisting)
{
KbdLayout &kbdLayout = GetApp()->GetLayout();
KbdLayout &kbdDefault = GetApp()->GetDefaultLayout();
int i;

for (i = (int)matrices.size(); i > nLayers; i--)
  {
  pMatrixNotebook->DeletePage(i - 1);
  matrices.erase(matrices.begin() + i - 1);
  kbdLayout.RemoveLayer(i - 1);
  }

if (resetExisting)
  {
  for (i = 0; i < (int)matrices.size(); i++)
    {
    kbdLayout[i] = kbdDefault[0];
    CMatrixPanel *pPanel = (CMatrixPanel *)pMatrixNotebook->GetPage(i);
    pPanel->SetKbdMatrix(kbdLayout[i]);
    }
  }

for (i = (int)matrices.size(); i < nLayers; i++)
  {
  CMatrixPanel *pNew = CreateMatrixPage(pMatrixNotebook);
  pMatrixNotebook->InsertPage(i, pNew,
                              wxString::Format(wxT("Matrix Layer %d"), i));
  pNew->SetLayer(i);
  matrices.push_back(pNew);
  if (i >= kbdLayout.GetLayers())
    {
    kbdLayout.AddLayer();
    kbdLayout[i] = kbdDefault[0];
    }
  pNew->SetKbdMatrix(kbdLayout[i]);
  }

pLayers->SetSelection(nLayers - 1);
return true;
}

/*****************************************************************************/
/* CreateMatrixPage : create a matrix page                                   */
/*****************************************************************************/

CMatrixPanel *CMainPanel::CreateMatrixPage
    (
    wxWindow *parent,
    int numRows,
    int numCols
    )
{
wxSizerFlags flagsBorder = wxSizerFlags().Border().Centre();

CMatrixPanel *page = new CMatrixPanel(parent, numRows, numCols);
wxSizer *sizerPage = new wxBoxSizer(wxHORIZONTAL);
sizerPage->Add(page->GetMatrix(), wxSizerFlags(1).Expand());
page->SetSizerAndFit(sizerPage);

return page;
}

/*****************************************************************************/
/* SetKbdLayout : loads a keyboard layout into the layers                    */
/*****************************************************************************/

void CMainPanel::SetKbdLayout(KbdLayout &layout)
{
SetLayers(layout.GetLayers());
for (int i = 0; i < layout.GetLayers(); i++)
  {
  CMatrixPanel *pPanel = matrices[i];
  pPanel->SetKbdMatrix(layout[i]);
  }
}

/*****************************************************************************/
/* SelectMatrix : select a matrix position in the current layer              */
/*****************************************************************************/

void CMainPanel::SelectMatrix(int row, int col)
{
CMatrixPanel *pPanel = (CMatrixPanel *)pMatrixNotebook->GetCurrentPage();
pPanel->SelectMatrix(row, col);
}


/*===========================================================================*/
/* CMainFrame class members                                                  */
/*===========================================================================*/

/*****************************************************************************/
/* CMainFrame Event Table                                                    */
/*****************************************************************************/

wxBEGIN_EVENT_TABLE(CMainFrame, wxFrame)
    EVT_MENU(Blusb_Quit,  CMainFrame::OnExit)
    EVT_MENU(Blusb_About, CMainFrame::OnAbout)

    EVT_CLOSE(CMainFrame::OnClose)

    EVT_TIMER(Blusb_Timer1, OnReadMatrixTimer)
    EVT_CHOICE(Blusb_LayerCount, CMainFrame::OnLayerCount)
    EVT_CHOICE(Blusb_Debounce, CMainFrame::OnDebounce)

    EVT_MENU(Blusb_ResetLayout, CMainFrame::OnReset)
    EVT_MENU(Blusb_ReadLayout, CMainFrame::OnReadLayout)
    EVT_UPDATE_UI(Blusb_ReadLayout, CMainFrame::OnUpdateReadLayout)
    EVT_MENU(Blusb_WriteLayout, CMainFrame::OnWriteLayout)
    EVT_UPDATE_UI(Blusb_WriteLayout, CMainFrame::OnUpdateWriteLayout)
    EVT_MENU(Blusb_ServiceMode, CMainFrame::OnServiceMode)
    EVT_UPDATE_UI(Blusb_ServiceMode, CMainFrame::OnUpdateServiceMode)
    EVT_MENU(Blusb_ReadFile, CMainFrame::OnReadFile)
    EVT_MENU(Blusb_WriteFile, CMainFrame::OnWriteFile)
    EVT_MENU(Blusb_Kbd_ANSI, CMainFrame::OnKbdANSI)
    EVT_UPDATE_UI(Blusb_Kbd_ANSI, CMainFrame::OnUpdateKbdANSI)
    EVT_MENU(Blusb_Kbd_ISO, CMainFrame::OnKbdISO)
    EVT_UPDATE_UI(Blusb_Kbd_ISO, CMainFrame::OnUpdateKbdISO)
    EVT_MENU(Blusb_Kbd_ANSI121, CMainFrame::OnKbdANSI121)
    EVT_UPDATE_UI(Blusb_Kbd_ANSI121, CMainFrame::OnUpdateKbdANSI121)
    EVT_MENU(Blusb_Kbd_ISO122, CMainFrame::OnKbdISO122)
    EVT_UPDATE_UI(Blusb_Kbd_ISO122, CMainFrame::OnUpdateKbdISO122)
    EVT_MENU(Blusb_Kbd_Reset, CMainFrame::OnKbdReset)
    EVT_MENU(Blusb_Kbd_Load, CMainFrame::OnKbdLoad)
    EVT_MENU(Blusb_Kbd_Save, CMainFrame::OnKbdSave)
wxEND_EVENT_TABLE()


/*****************************************************************************/
/* CMainFrame : constructor                                                  */
/*****************************************************************************/

CMainFrame::CMainFrame
    (
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size
    )
  : wxFrame(NULL, wxID_ANY, title, pos, size)
{
SetIcon(wxICON(Application));
wxMenu *menuFile = new wxMenu;
menuFile->Append(wxID_EXIT);

wxMenu *menuLayout = new wxMenu;
menuLayout->Append(Blusb_ResetLayout, wxT("Reset Layout"),
                 wxT("Reset layout to default values"));
menuLayout->Append(Blusb_ReadFile, wxT("Load from File..."),
                 wxT("Read layout from file"));
menuLayout->Append(Blusb_WriteFile, wxT("Save to File..."),
                 wxT("Write layout to file"));
//if (GetApp()->IsDevOpen())
  {
  menuLayout->AppendSeparator();
  menuLayout->Append(Blusb_ReadLayout, wxT("Read Layout"),
                   wxT("Read layout from attached Model M keyboard"));
  menuLayout->Append(Blusb_WriteLayout, wxT("Write Layout"),
                   wxT("Write layout to attached Model M keyboard"));
  menuLayout->AppendCheckItem(Blusb_ServiceMode, wxT("Service Mode"),
                   wxT("Put attached Model M keyboard into Service Mode"));
  }
menuLayout->AppendSeparator();
menuLayout->Append(Blusb_Kbd_ANSI, wxT("ANSI Keyboard Layout"),
                 wxT("Use ANSI Keyboard Layout"),
                 true);
menuLayout->Append(Blusb_Kbd_ISO, wxT("ISO Keyboard Layout"),
                 wxT("Use ISO Keyboard Layout"),
                 true);
menuLayout->Append(Blusb_Kbd_ANSI121, wxT("ANSI 121 Keyboard Layout"),
                 wxT("Use ANSI 121 Keyboard Layout"),
                 true);
menuLayout->Append(Blusb_Kbd_ISO122, wxT("ISO 122 Keyboard Layout"),
                 wxT("Use ISO 122 Keyboard Layout"),
                 true);
menuLayout->Append(Blusb_Kbd_Load, wxT("Load Keyboard Layout..."),
                 wxT("Load Keyboard Layout from file"));
menuLayout->Append(Blusb_Kbd_Save, wxT("Save Keyboard Layout..."),
                 wxT("Save Keyboard Layout to file"));
#if 0
// not necessary here, since blusb_gui doesn't discriminate between
// "unpressed" (i.e., never pressed) and "released" keys
menuLayout->Append(Blusb_Kbd_Reset, wxT("Reset Keyboard"),
                 wxT("Reset all keys on keyboard to unpressed"));
#endif

wxMenu *menuHelp = new wxMenu;
menuHelp->Append(wxID_ABOUT);
wxMenuBar *menuBar = new wxMenuBar;

menuBar->Append( menuFile, wxT("&File"));
menuBar->Append( menuLayout, wxT("&Layout"));
menuBar->Append( menuHelp, wxT("&Help"));
SetMenuBar( menuBar );

// TODO: create a wxNotebook object to hold 2 child windows:
// one for the matrix (which is a wxGrid object plus a layer selector)
// and one for the keyboard display
// below that, a control area that shows the USB connection
// to the keyboard
m_panel = new CMainPanel(this);

// set up 10ms timer
t.SetOwner(this, Blusb_Timer1);
t.Start(10);

CreateStatusBar();
SetStatusText(wxT("Ready"));
}

/*****************************************************************************/
/* OnExit : called when wxID_EXIT comes in                                   */
/*****************************************************************************/

void CMainFrame::OnExit(wxCommandEvent& event)
{
t.Stop();
Close(true);
}

/*****************************************************************************/
/* OnClose called when the window is about to be closed                      */
/*****************************************************************************/

void CMainFrame::OnClose(wxCloseEvent &event)
{
CNoServiceMode nosm;                    /* no service mode in here!          */

if (event.CanVeto() && GetApp()->IsLayoutModified())
  {
  if (wxMessageBox("The current layout has not been saved; "
                     "do you really want to quit?",
                   "Please confirm",
                   wxICON_QUESTION | wxYES_NO) != wxYES)
    {
    event.Veto();
    return;
    }
  }
event.Skip();
}

/*****************************************************************************/
/* OnAbout : called when wxID_ABOUT comes in                                 */
/*****************************************************************************/

void CMainFrame::OnAbout(wxCommandEvent& event)
{
CNoServiceMode nosm;                    /* no service mode in here!          */

#if WITH_SPLASH
CSplashWnd::ShowSplashScreen(this, 0);
#else
wxMessageBox(wxT("Graphical BlUSB Layout Configuration Tool\n\n")
             wxT("Copyright (c) H. Seib, 2017-19"),
             wxT("About BlUSB_Gui"),
             wxOK | wxICON_INFORMATION );
#endif
}

/*****************************************************************************/
/* OnReadMatrixTimer : 10ms timer to read out the keyboard matrix            */
/*****************************************************************************/

void CMainFrame::OnReadMatrixTimer(wxTimerEvent& event)
{
wxUint8 buffer[sizeof(bufferLast)];
if (GetApp()->IsDevOpen() && GetApp()->InServiceMode())
  {
  if (GetApp()->ReadMatrixPos(buffer, sizeof(buffer)) >= BLUSB_SUCCESS)
    {
    bool bChanged = false;
    if (buffer[7])
      {
      for (int i = 0; i < 2; i++)
        if (buffer[i] ^ bufferLast[i])
          {
          bChanged = true;
          break;
          }
      if (bChanged)
        {
        // this is not exactly true, but we only get the last press,
        // so assure that only one key is shown pressed
        if (bufferLast[0] != 0xff)
          SetKeyState(bufferLast[0], bufferLast[1], CKbdWnd::ksReleased);
        memcpy(bufferLast, buffer, sizeof(bufferLast));
        SelectMatrix(buffer[0], buffer[1]);
        SetKeyState(buffer[0], buffer[1], CKbdWnd::ksPressed);
        }
      }
    else if (buffer[7] != bufferLast[7])
      {
      SetKeyState(bufferLast[0], bufferLast[1], CKbdWnd::ksReleased);
      // memcpy(bufferLast, buffer, sizeof(bufferLast));
      memset(bufferLast, 0xff, sizeof(bufferLast));
      }
    }
  }
}

/*****************************************************************************/
/* OnLayerCount : layer count changes                                        */
/*****************************************************************************/

void CMainFrame::OnLayerCount(wxCommandEvent& event)
{
m_panel->SetLayers(m_panel->GetLayerChoice());
}

/*****************************************************************************/
/* OnDebounce : debounce changes                                             */
/*****************************************************************************/

void CMainFrame::OnDebounce(wxCommandEvent& event)
{
if (GetApp()->WriteDebounce(m_panel->GetDebounce()) < BLUSB_SUCCESS)
  {
  CNoServiceMode nosm;                  /* no service mode in here!          */
  wxMessageBox(wxT("Error writing new debounce value to keyboard"),
               wxT("Model M Error"),
               wxCANCEL | wxCENTRE);
  return;
  }
}

/*****************************************************************************/
/* OnReadLayout : read layout from attached keyboard                         */
/*****************************************************************************/

void CMainFrame::OnReadLayout(wxCommandEvent& event)
{
CNoServiceMode nosm;                    /* no service mode in here!          */

if (GetApp()->IsLayoutModified() &&
    wxMessageBox("The current layout has not been saved; "
                     "do you really want to overwrite it?",
                   "Please confirm",
                   wxICON_QUESTION | wxYES_NO) != wxYES)
  return;

wxBusyCursor wait;
if (GetApp()->ReadLayout() < BLUSB_SUCCESS)
  {
  CNoServiceMode nosm;                  /* no service mode in here!          */
  wxMessageBox(wxT("Error reading layout from keyboard"),
               wxT("Model M Error"),
               wxCANCEL | wxCENTRE);
  return;
  }
SetKbdLayout(GetApp()->GetLayout());
}

/*****************************************************************************/
/* OnUpdateReadLayout : update the visual appearance                         */
/*****************************************************************************/

void CMainFrame::OnUpdateReadLayout(wxUpdateUIEvent& event)
{
event.Enable(GetApp()->IsDevOpen());
}

/*****************************************************************************/
/* OnWriteLayout : write layout to attached keyboard                         */
/*****************************************************************************/

void CMainFrame::OnWriteLayout(wxCommandEvent& event)
{
wxBusyCursor wait;
if (GetApp()->WriteLayout() < BLUSB_SUCCESS)
  {
  CNoServiceMode nosm;                  /* no service mode in here!          */
  wxMessageBox(wxT("Error writing layout to keyboard"),
               wxT("Model M Error"),
               wxOK | wxCENTRE);
  }
#if 1
// saving to keyboard counts as a switch to "unmodified"
else
  GetApp()->SetLayoutModified(false);
#endif
}

/*****************************************************************************/
/* OnUpdateWriteLayout : update the visual appearance                        */
/*****************************************************************************/

void CMainFrame::OnUpdateWriteLayout(wxUpdateUIEvent& event)
{
event.Enable(GetApp()->IsDevOpen());
}

/*****************************************************************************/
/* OnReset : reset keyboard layout to default                                */
/*****************************************************************************/

void CMainFrame::OnReset(wxCommandEvent& event)
{
CNoServiceMode nosm;                    /* no service mode in here!          */

if (GetApp()->IsLayoutModified() &&
    wxMessageBox("The current layout has not been saved; "
                   "do you really want to reset it?",
                 "Please confirm",
                 wxICON_QUESTION | wxYES_NO) != wxYES)
  return;

GetApp()->GetLayout() = GetApp()->GetDefaultLayout();
SetKbdLayout(GetApp()->GetLayout());
}

/*****************************************************************************/
/* OnServiceMode : puts Model M into Service or Normal mode                  */
/*****************************************************************************/

void CMainFrame::OnServiceMode(wxCommandEvent& event)
{
if (event.IsChecked())
  {
  // memset(bufferLast, 0, sizeof(bufferLast));
  memset(bufferLast, 0xff, sizeof(bufferLast));
  GetApp()->EnableServiceMode();
  }
else
  GetApp()->DisableServiceMode();
}

/*****************************************************************************/
/* OnUpdateServiceMode : update the visual appearance                        */
/*****************************************************************************/

void CMainFrame::OnUpdateServiceMode(wxUpdateUIEvent& event)
{
event.Enable(GetApp()->IsDevOpen());
}

/*****************************************************************************/
/* OnReadFile : called to read a layout file                                 */
/*****************************************************************************/

void CMainFrame::OnReadFile(wxCommandEvent& event)
{
CNoServiceMode nosm;                    /* no service mode in here!          */

if (GetApp()->IsLayoutModified() &&
    wxMessageBox("The current layout has not been saved; "
                     "do you really want to load another?",
                   "Please confirm",
                   wxICON_QUESTION | wxYES_NO) != wxYES)
  return;

wxFileDialog of(this, wxT("Open BlUSB File"), wxT("."), wxEmptyString,
                wxT("BlUSB Files (*.blu)|*.blu|All Files (*)|*.*"),
                wxFD_OPEN | wxFD_FILE_MUST_EXIST);
int rc = of.ShowModal();
if (rc != wxID_OK)
  return;
if (GetApp()->ReadLayout(of.GetPath()) < BLUSB_SUCCESS)
  {
  wxMessageBox(wxT("Error reading layout from ") + of.GetPath(),
               wxT("Model M Error"),
               wxCANCEL | wxCENTRE);
  return;
  }
SetKbdLayout(GetApp()->GetLayout());
}

/*****************************************************************************/
/* OnWriteFile : called to write a configuration file                        */
/*****************************************************************************/

void CMainFrame::OnWriteFile(wxCommandEvent& event)
{
CNoServiceMode nosm;                    /* no service mode in here!          */

wxFileDialog of(this, wxT("Save to BlUSB File"), wxT("."), wxEmptyString,
                wxT("BlUSB Files (*.blu)|*.blu|All Files (*)|*.*"),
                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
int rc = of.ShowModal();
if (rc != wxID_OK)
  return;

if (GetApp()->WriteLayout(of.GetPath(), !of.GetFilterIndex()) < BLUSB_SUCCESS)
  wxMessageBox(wxT("Error writing layout to ") + of.GetPath(),
               wxT("Model M Error"),
               wxOK | wxCENTRE);
}

/*****************************************************************************/
/* SetKbdGuiLayout : setup new Keyboard GUI layout                           */
/*****************************************************************************/

bool CMainFrame::SetKbdGuiLayout(KbdGui &layout)
{
if (!m_panel)
  return false;

int currows, curcols;
m_panel->GetKbdGuiLayout().GetMatrixLayout(currows, curcols);
int kbdrows = 8, kbdcols = curcols;
GetApp()->ReadMatrixLayout(kbdrows, kbdcols);
int newrows, newcols;
layout.GetMatrixLayout(newrows, newcols);

m_panel->SetKbdGuiLayout(layout);

// TODO:
// We got the following matrix definitions now:
// 1) current matrix on screen
// 2) matrix layout expected by the keyboard
// 3) matrix layout mandated by the GUI layout
// What if these don't match? Hmmm ...
// Ignore the attached keyboard's matrix layout for now.

if (newrows > 1 && newcols > 1 &&  // <= 1 indicates "no matrix definitions"
    (newrows != currows || newcols != curcols))
  {
  wxString s = wxString::Format(wxT("The new layout redefines the matrix size ")
                                wxT("from %dx%d to %dx%d!\n")
                                wxT("Do you want to reset the matrix ")
                                wxT("definitions to the default for the new ")
                                wxT("keyboard layout?"),
                                currows, curcols, newrows, newcols);
  if (wxMessageBox(s, wxT("Matrix Redefinition"),
                   wxICON_QUESTION | wxYES_NO | wxCENTRE) == wxYES)
    {
    GetApp()->SetDefaultLayout(layout);
    m_panel->SetLayers(1, true);
    }
  }

return true;
}

/*****************************************************************************/
/* OnKbdAnsi : set keyboard layout to ANSI default                           */
/*****************************************************************************/

void CMainFrame::OnKbdANSI(wxCommandEvent& event)
{
KbdGui newGui(KbdGui::KbdANSI);
if (SetKbdGuiLayout(newGui))
  GetApp()->WriteConfig("/Settings/KbdLayout", "ANSI");
}

/*****************************************************************************/
/* OnUpdateKbdANSI : update the visual appearance                            */
/*****************************************************************************/

void CMainFrame::OnUpdateKbdANSI(wxUpdateUIEvent& event)
{
wxString s;
// This should be made more performant
GetApp()->ReadConfig("/Settings/KbdLayout", &s, "ANSI");
event.Check(!s.CmpNoCase(wxT("ANSI")));
}

/*****************************************************************************/
/* OnKbdISO : set keyboard layout to ISO default                             */
/*****************************************************************************/

void CMainFrame::OnKbdISO(wxCommandEvent& event)
{
KbdGui newGui(KbdGui::KbdISO);
if (SetKbdGuiLayout(newGui))
  GetApp()->WriteConfig("/Settings/KbdLayout", "ISO");
}

/*****************************************************************************/
/* OnUpdateKbdISO : update the visual appearance                             */
/*****************************************************************************/

void CMainFrame::OnUpdateKbdISO(wxUpdateUIEvent& event)
{
wxString s;
// This should be made more performant
GetApp()->ReadConfig("/Settings/KbdLayout", &s, "ANSI");
event.Check(!s.CmpNoCase(wxT("ISO")));
}

/*****************************************************************************/
/* OnKbdAnsi121 : set keyboard layout to ANSI 121 default                    */
/*****************************************************************************/

void CMainFrame::OnKbdANSI121(wxCommandEvent& event)
{
KbdGui newGui(KbdGui::KbdANSI121);
if (SetKbdGuiLayout(newGui))
  GetApp()->WriteConfig("/Settings/KbdLayout", "ANSI121");
}

/*****************************************************************************/
/* OnUpdateKbdANSI121 : update the visual appearance                         */
/*****************************************************************************/

void CMainFrame::OnUpdateKbdANSI121(wxUpdateUIEvent& event)
{
wxString s;
// This should be made more performant
GetApp()->ReadConfig("/Settings/KbdLayout", &s, "ANSI");
event.Check(!s.CmpNoCase(wxT("ANSI121")));
}

/*****************************************************************************/
/* OnKbdISO122 : set keyboard layout to ISO 122 default                      */
/*****************************************************************************/

void CMainFrame::OnKbdISO122(wxCommandEvent& event)
{
KbdGui newGui(KbdGui::KbdISO122);
if (SetKbdGuiLayout(newGui))
  GetApp()->WriteConfig("/Settings/KbdLayout", "ISO122");
}

/*****************************************************************************/
/* OnUpdateKbdISO122 : update the visual appearance                          */
/*****************************************************************************/

void CMainFrame::OnUpdateKbdISO122(wxUpdateUIEvent& event)
{
wxString s;
// This should be made more performant
GetApp()->ReadConfig("/Settings/KbdLayout", &s, "ANSI");
event.Check(!s.CmpNoCase(wxT("ISO122")));
}

/*****************************************************************************/
/* OnKbdReset reset the keyboard state                                       */
/*****************************************************************************/

void CMainFrame::OnKbdReset(wxCommandEvent& event)
{
ResetKeyState();
}

/*****************************************************************************/
/* OnKbdLoad : load a keyboard layout file                                   */
/*****************************************************************************/

void CMainFrame::OnKbdLoad(wxCommandEvent& event)
{
CNoServiceMode nosm;                    /* no service mode in here!          */

wxFileDialog of(this, wxT("Open BlUSB Keyboard Layout File"), wxT("."), wxEmptyString,
                wxT("BlUSB Keyboard Layout Files (*.kbl)|*.kbl|All Files (*)|*.*"),
                wxFD_OPEN | wxFD_FILE_MUST_EXIST);
int rc = of.ShowModal();
if (rc != wxID_OK)
  return;
KbdGui kg;
wxString err;
if (kg.ReadLayoutFile(of.GetPath(), &err))
  {
  if (SetKbdGuiLayout(kg))
    GetApp()->WriteConfig("/Settings/KbdLayout", of.GetPath());
  }
else
  wxMessageBox(err, wxT("Load ") + of.GetPath());
}

/*****************************************************************************/
/* OnKbdSave : saves a keyboard layout file                                  */
/*****************************************************************************/

void CMainFrame::OnKbdSave(wxCommandEvent& event)
{
CNoServiceMode nosm;                    /* no service mode in here!          */

if (!m_panel)
  return;

wxFileDialog of(this, wxT("Save to BlUSB Keyboard Layout File"), wxT("."), wxEmptyString,
                wxT("BlUSB Keyboard Layout Files (*.kbl)|*.kbl|All Files (*)|*.*"),
                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
int rc = of.ShowModal();
if (rc != wxID_OK)
  return;

if (m_panel->GetKbdGuiLayout().WriteLayoutFile(of.GetPath()))
  GetApp()->WriteConfig("/Settings/KbdLayout", of.GetPath());
else
  {
  wxString err(wxT("Error writing current keyboard layout to "));
  wxMessageBox(err + of.GetPath(), wxT("Save Keyboard Layout"));
  }
}
