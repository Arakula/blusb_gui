/*****************************************************************************/
/* KbdWnd.h : CKbdWnd declaration                                            */
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

#ifndef _KbdWnd_h__included_
#define _KbdWnd_h__included_

#include "KbdGuiLayout.h"

#define KEYCLR_BOARD         216, 216, 220
#define KEYCLR_TEXT           76,  82,  84
#define KEYCLR_BORDER         76,  82,  84
#define KEYCLR_UNPRESSED     230, 230, 230
#define KEYCLR_UNPRESSED_HI  230, 230, 230
#define KEYCLR_PRESSED       160, 160, 230
#define KEYCLR_PRESSED_HI    160, 160, 230
#define KEYCLR_RELEASED      160, 230, 160
#define KEYCLR_RELEASED_HI   160, 230, 160
#define KEYCLR_ALERT         230, 160, 160
#define KEYCLR_ALERT_HI      230, 160, 160
#define KEYCLR_LEDON           0, 225,   0
#define KEYCLR_LEDON_HI        0, 160,   0

/*****************************************************************************/
/* CKbdWnd class declaration                                                 */
/*****************************************************************************/

class CKbdWnd : public wxPanel
{
public:
    enum KeyState
      {
      ksUnpressed,
      ksPressed,
      ksReleased,
      ksAlert,
      ksLEDOn,

      ksStates,
      ksHighlighted = (1 << 3)
      };
    enum
      {
      // Timers
      Kbd_Timer1 = 1,
      };
    struct KeyLayout
      {
      int rects;
      wxRect rect[2];
      KeyState state;
      GuiKey const *def;
      };
    struct KeyClrs
      {
      // brushes for up to 2 rectangles
      wxBrush &br1, &br2;  // 1: key bkgnd  2: led bkgnd
      wxPen &pen1, &pen2;  // 1: boundary 2: rect overlap
      };

public:
    CKbdWnd(wxWindow *parent,
            wxWindowID id = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = 0,
            const wxString& name = wxT("Keyboard"));
    virtual ~CKbdWnd(void);

    void SetLayout(KbdGui const &newlayout);

    void SetKeyState(KeyLayout *key, int newstate, bool passOn = false);
    void SetKeyState(int hidcode, int newstate, bool passOn = false)
      { SetKeyState(FindHID(hidcode), newstate, passOn); }
    void SetKeyState(int matrixrow, int matrixcol, int newstate, bool passOn = false)
      { SetKeyState(FindMatrix(matrixrow, matrixcol), newstate, passOn); }
    void ResetKeyState()
      {
      for (size_t i = 0; i < keys.size(); i++)
        SetKeyState(&keys[i], ksUnpressed);
      }
    wxColour const &GetBkgndColour() { return clrBkgnd; }
    void SetBkgndColour(wxColour const &newClr)
      {
      clrBkgnd = newClr;
      Refresh();
      }
    wxColour const &GetColour(int ks)
      { return clrKey[ks & ~ksHighlighted][!!(ks & ksHighlighted)]; }
    void SetColour(int ks, wxColour const &newClr)
      {
      clrKey[ks & ~ksHighlighted][!!(ks & ksHighlighted)] = newClr;
      Refresh();
      }
    bool GetLEDStates() { return getLEDStates; }
    void GetLEDStates(bool bOn) { getLEDStates = bOn; }

    bool CapturingAllKeys() { return bAllKeys; }
    void CaptureAllKeys(bool bOn = true);

    KeyLayout *FindHID(int hidcode)
      { return hid2Key[hidcode]; }
    KeyLayout *FindMatrix(int row, int col)
      { return matrix2Key[(row << 24) | col]; }

    // Functionality to prohinit GUI key propagation
    static void HookLLKeyboard(bool bOn = true); // needs a better name sometime
    static void InhibitGuiKey(bool bOn = true);

private:
    wxDECLARE_EVENT_TABLE();

    void OnPaint(wxPaintEvent &ev);
    void OnKeyDown(wxKeyEvent &);
    void OnChar(wxKeyEvent &);
    void OnKeyUp(wxKeyEvent &);
    void OnLButtonDown(wxMouseEvent& event);
    void OnReadLEDTimer(wxTimerEvent& event);

protected:
#ifdef __WXMSW__
    virtual WXLRESULT
    MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif // __WXMSW__

    void DrawKey(wxDC &dc, KeyLayout const &key, KeyClrs const &clrs);
    void RefreshKey(KeyLayout *key)
      {
      RefreshRect(key->rect[0]);
      RefreshRect(key->rect[1]);
      // Update();
      }

protected:
    KbdGui layout;
    wxVector<KeyLayout> keys;
    WX_DECLARE_HASH_MAP(wxUint32, KeyLayout *, wxIntegerHash, wxIntegerEqual, KbdIndex2Key);
    KbdIndex2Key hid2Key;
    KbdIndex2Key matrix2Key;
    wxColour clrBkgnd, clrBorder;
    wxColour clrKey[ksStates][2];
    wxFont keyFont;
    wxTimer t;
    bool bLEDState[3], getLEDStates;
    bool bAllKeys;
protected:
    wxSize CalcLayout(wxSize sz);

};


#endif // defined(_KbdWnd_h__included_)
