/*****************************************************************************/
/* MainFrm.h : interface of the CMainFrame class                             */
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

#ifndef _MainFrm_h__included_
#define _MainFrm_h__included_

#include "KbdGuiLayout.h"

#include "MatrixWnd.h"
#include "KbdWnd.h"

/*===========================================================================*/
/* Constants                                                                 */
/*===========================================================================*/

// menu commands and controls ids
enum
  {
  // file menu
  Blusb_Quit = wxID_EXIT,
  Blusb_About = wxID_ABOUT,

  // Timers
  Blusb_Timer1 = 1,

  // navigation menu
  Blusb_TabForward = 200,
  Blusb_TabBackward,

  Blusb_LayerCount,
  Blusb_Debounce,

  Blusb_ResetLayout,
  Blusb_ReadLayout,
  Blusb_WriteLayout,
  Blusb_ReadFile,
  Blusb_WriteFile,
  Blusb_ServiceMode,

  Blusb_Kbd_ANSI,
  Blusb_Kbd_ISO,
  Blusb_Kbd_ANSI121,
  Blusb_Kbd_ISO122,
  Blusb_Kbd_Reset,
  Blusb_Kbd_Load,
  Blusb_Kbd_Save,

  Blusb_Max
  };


/*****************************************************************************/
/* CMatrixPanel : a matrix panel and its associated matrix window            */
/*****************************************************************************/

class CMatrixPanel : public wxPanel
{
public:
    CMatrixPanel(wxWindow *parent,
            int numRows = NUMROWS, int numCols = NUMCOLS,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const wxString& name = wxPanelNameStr)
            : wxPanel(parent, winid, pos, size, style, name)
      {
      pMatrix = new CMatrixWnd(this, numRows, numCols);
      szMatrix = pMatrix->GetMinSize();
      }

    CMatrixWnd *GetMatrix() { return pMatrix; }
    void SetLayer(int layer) { pMatrix->SetLayer(layer); }
    int GetLayer() { return pMatrix->GetLayer(); }
    void SetKbdMatrix(KbdMatrix const &kbm)
      { pMatrix->SetKbdMatrix(kbm); }
    void SelectMatrix(int row, int col)
      { pMatrix->SelectMatrix(row, col); }

    bool Layout();

protected:
    CMatrixWnd *pMatrix;
    wxSize szMatrix;
};

/*****************************************************************************/
/* CKbdPanel : panel containing the on-screen keyboard and associated buttons*/
/*****************************************************************************/

class CKbdPanel : public wxPanel
{
public:
    CKbdPanel(wxWindow *parent,
              wxWindowID winid = wxID_ANY,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              long style = wxTAB_TRAVERSAL | wxNO_BORDER,
              const wxString& name = wxPanelNameStr);

    void SetKbdLayout(KbdGui const &newlayout)
      { pKbd->SetLayout(newlayout); }

    void SetKeyState(int hidcode, int newstate)
      { pKbd->SetKeyState(hidcode, newstate); }
    void SetKeyState(int matrixrow, int matrixcol, int newstate)
      { pKbd->SetKeyState(matrixrow, matrixcol, newstate); }
    void ResetKeyState()
      { pKbd->ResetKeyState(); }

    wxColour const &GetKbdBkgndColour()
      { return pKbd->GetBkgndColour(); }
    void SetKbdBkgndColour(wxColour const &newClr)
      { pKbd->SetBkgndColour(newClr); }
    wxColour const &GetKbdColour(int ks)
      { return pKbd->GetColour(ks); }
    void SetKbdColour(int ks, wxColour const &newClr)
      { pKbd->SetColour(ks, newClr); }

    bool Layout();

protected:
    CKbdWnd *pKbd;
    wxSize szKbd;
};

/*****************************************************************************/
/* CMainPanel : panel on main window                                         */
/*****************************************************************************/

class CMainPanel : public wxPanel
{
public:
    CMainPanel(wxWindow *parent);

    int GetLayerChoice() { return pLayers->GetSelection() + 1; }
    int GetLayers() { return (int)matrices.size(); }
    bool SetLayers(int nLayers, bool resetExisting = false);

    int GetDebounce() { return pDebounce->GetSelection() + 1; }

    void SetKbdLayout(KbdLayout &layout);
    void SetKbdGuiLayout(KbdGui &layout)
      {
      pKbd->SetKbdLayout(layout);
      kbdGuiLayout = layout;
      }
    KbdGui &GetKbdGuiLayout() { return kbdGuiLayout; }

    void SelectMatrix(int row, int col);

    void SetKeyState(int hidcode, int newstate)
      { if (pKbd) pKbd->SetKeyState(hidcode, newstate); }
    void SetKeyState(int matrixrow, int matrixcol, int newstate)
      { if (pKbd) pKbd->SetKeyState(matrixrow, matrixcol, newstate); }
    void ResetKeyState()
      { if (pKbd) pKbd->ResetKeyState(); }

protected:
    CMatrixPanel *CreateMatrixPage(wxWindow *parent,
                                   int numRows = NUMROWS, int numCols = NUMCOLS);

    wxChoice *pLayers;
    wxChoice *pDebounce;
    wxNotebook *pMatrixNotebook;
    wxVector<CMatrixPanel *> matrices;
    CKbdPanel *pKbd;
    KbdGui kbdGuiLayout;
};

/*****************************************************************************/
/* CMainFrame : main frame window class                                      */
/*****************************************************************************/

class CMainFrame : public wxFrame
{
public:
    CMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
private:
    wxDECLARE_EVENT_TABLE();
private:
    // void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent &event);

    void OnReadMatrixTimer(wxTimerEvent& event);

    void OnLayerCount(wxCommandEvent& event);
    void OnDebounce(wxCommandEvent& event);
    void OnReset(wxCommandEvent& event);
    void OnReadLayout(wxCommandEvent& event);
    void OnUpdateReadLayout(wxUpdateUIEvent& event);
    void OnWriteLayout(wxCommandEvent& event);
    void OnUpdateWriteLayout(wxUpdateUIEvent& event);
    void OnServiceMode(wxCommandEvent& event);
    void OnUpdateServiceMode(wxUpdateUIEvent& event);
    void OnReadFile(wxCommandEvent& event);
    void OnWriteFile(wxCommandEvent& event);
    void OnKbdANSI(wxCommandEvent& event);
    void OnUpdateKbdANSI(wxUpdateUIEvent& event);
    void OnKbdISO(wxCommandEvent& event);
    void OnUpdateKbdISO(wxUpdateUIEvent& event);
    void OnKbdANSI121(wxCommandEvent& event);
    void OnUpdateKbdANSI121(wxUpdateUIEvent& event);
    void OnKbdISO122(wxCommandEvent& event);
    void OnUpdateKbdISO122(wxUpdateUIEvent& event);
    void OnKbdReset(wxCommandEvent& event);
    void OnKbdLoad(wxCommandEvent& event);
    void OnKbdSave(wxCommandEvent& event);

public:
    void SetKbdLayout(KbdLayout &layout)
      { if (m_panel) m_panel->SetKbdLayout(layout); }
    bool SetKbdGuiLayout(KbdGui &layout);
    void SelectMatrix(int row, int col)
      { if (m_panel) m_panel->SelectMatrix(row, col); }
    void SetKeyState(int hidcode, int newstate)
      { if (m_panel) m_panel->SetKeyState(hidcode, newstate); }
    void SetKeyState(int matrixrow, int matrixcol, int newstate)
      { if (m_panel) m_panel->SetKeyState(matrixrow, matrixcol, newstate); }
    void ResetKeyState()
      { if (m_panel) m_panel->ResetKeyState(); }

private:
    CMainPanel *m_panel;
    wxTimer t;
    wxUint8 bufferLast[8];

};


#endif // defined(_MainFrm_h__included_)
