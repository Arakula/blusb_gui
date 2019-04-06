/*****************************************************************************/
/* KbdWnd.cpp : CKbdWnd implementation                                       */
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

#ifndef wxHAS_IMAGES_IN_RESOURCES
// nothing yet.
//#include "res/Application.xpm"
#endif

/*****************************************************************************/
/* Default colors for key / LED drawing                                      */
/*****************************************************************************/

#define KEYCLR_UNPRESSED     230, 230, 230  // initial unpressed
#define KEYCLR_PRESSED       160, 160, 230  // pressed
#define KEYCLR_RELEASED      160, 230, 160  // released after press
#define KEYCLR_ALERT         230, 160, 160  // error on that key
#define KEYCLR_LEDON           0, 225,   0  // LED lit
#define KEYCLR_LEDON_HI        0, 160,   0  // LED lit highlight

/*===========================================================================*/
/* CKbdWnd class members                                                     */
/*===========================================================================*/

/*****************************************************************************/
/* HookLLKeyboard : (de)installs a low-level keyboard hook                   */
/*****************************************************************************/

static wxVector<CKbdWnd *> regWnds;
static bool bGUIKeyInhibited = false;

#ifdef __WXMSW__
static HHOOK hKeyboardHook = NULL;

static LRESULT CALLBACK LowLevelKeyboardProc
    (
    int nCode,
    WPARAM wParam,
    LPARAM lParam
    )
{
if (nCode != HC_ACTION)  // do not process message 
  return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam); 
 
bool bEatKeystroke = false;
if (bGUIKeyInhibited)
  {
  switch (wParam) 
    {
    case WM_KEYDOWN:  
    case WM_SYSKEYDOWN:  
    case WM_KEYUP:    
    case WM_SYSKEYUP:  
      {
      KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
      if (p->vkCode == VK_LWIN || 
          p->vkCode == VK_RWIN
          || (p->vkCode == VK_ESCAPE && (p->flags & LLKHF_ALTDOWN))
          || (p->vkCode == VK_TAB && (p->flags & LLKHF_ALTDOWN))
          ) 
        bEatKeystroke = true;
      bool bDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
      int scan, key;
      scan = p->scanCode;
      if (p->flags & LLKHF_EXTENDED)
        scan |= 0x100;
      key = KbdGui::GetHIDFromScancode(scan);
#ifdef _DEBUG
      OutputDebugString(wxString::Format("Hook: %c %04x %04x %04x %08x -> %04x -> %04x\n",
                                         (wParam == WM_KEYDOWN ||
                                          wParam == WM_SYSKEYDOWN) ? 'D' : 'U',
                                         p->vkCode, p->scanCode, p->flags, p->time,
                                         scan, key));
#endif

      if (key != KB_UNUSED)
        {
        for (size_t i = 0; i < regWnds.size(); i++)
          {
          if (bDown)
            regWnds[i]->SetKeyState(key, CKbdWnd::ksPressed, true);
          else
            regWnds[i]->SetKeyState(key, CKbdWnd::ksReleased);
          }
        }
      }
      break;
    }
  }
if (bEatKeystroke)
  return 1;
return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}
#endif

void CKbdWnd::HookLLKeyboard(bool bOn)
{
#ifdef __WXMSW__
if (bOn)
  {
  if (hKeyboardHook)
    return;
  hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,
                                   LowLevelKeyboardProc,
                                   GetModuleHandle(NULL),
                                   0);
  }
else
  {
  if (!hKeyboardHook)
    return;
  UnhookWindowsHookEx(hKeyboardHook);
  hKeyboardHook = NULL;
  }
#endif
}

/*****************************************************************************/
/* InhibitGuiKey : inhibit GUI key extravaganzas                             */
/*****************************************************************************/

void CKbdWnd::InhibitGuiKey(bool bOn)
{
bGUIKeyInhibited = bOn;
}

/*****************************************************************************/
/* CKbdWnd Event Table                                                       */
/*****************************************************************************/

wxBEGIN_EVENT_TABLE(CKbdWnd, wxScrolledWindow)
    EVT_PAINT(CKbdWnd::OnPaint)
#ifndef __WXMSW__
    // handled through MSWWindowProc() in Windows
    EVT_KEY_DOWN(CKbdWnd::OnKeyDown)
    EVT_CHAR(CKbdWnd::OnChar)
    EVT_KEY_UP(CKbdWnd::OnKeyUp)
#endif
    EVT_LEFT_DOWN(CKbdWnd::OnLButtonDown)
    EVT_TIMER(Kbd_Timer1, CKbdWnd::OnReadLEDTimer)
wxEND_EVENT_TABLE()

/*****************************************************************************/
/* DiffColour : creates a differential colour                                */
/*****************************************************************************/

static wxColour DiffColour
    (
    wxColour &unpressed,
    wxColour &defUnpressed,
    wxColour &defDiff
    )
{
wxColour out;
int r = unpressed.Red(),   rd = defUnpressed.Red()   - defDiff.Red();
int g = unpressed.Green(), gd = defUnpressed.Green() - defDiff.Green();
int b = unpressed.Blue(),  bd = defUnpressed.Blue()  - defDiff.Blue();

if (r >= 128) r -= rd; else r += rd;
if (g >= 128) g -= gd; else g += gd;
if (b >= 128) b -= bd; else b += bd;

return wxColour(max(min(r, 255), 0),
                max(min(g, 255), 0),
                max(min(b, 255), 0));
}

/*****************************************************************************/
/* CKbdWnd : constructor                                                     */
/*****************************************************************************/

CKbdWnd::CKbdWnd
    (
    wxWindow *parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
    )
: wxPanel(), // default constructor, because of SetBackgroundStyle()
  keyFont(wxFontInfo(wxSize(0, 11)).Family(wxFONTFAMILY_SWISS))
{
SetBackgroundStyle(wxBG_STYLE_PAINT);
wxSize szInitial = CalcLayout(size);
Create(parent, id, pos, szInitial, style, name);
SetMinSize(szInitial);

clrBkgnd = wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK);
clrBorder = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
// since at least macOS has dark themes, this needs to be done a bit
// more complicated than just setting a colour <sigh> ...
clrKey[ksUnpressed][0] = clrKey[ksUnpressed][1] =
    wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
// the other colours are derived from EK switch hitter
clrKey[ksPressed][0] = clrKey[ksPressed][1] =
    DiffColour(clrKey[ksUnpressed][0],
               wxColour(KEYCLR_UNPRESSED),
               wxColour(KEYCLR_PRESSED));
clrKey[ksReleased][0] = clrKey[ksReleased][1] =
    DiffColour(clrKey[ksUnpressed][0],
               wxColour(KEYCLR_UNPRESSED),
               wxColour(KEYCLR_RELEASED));
clrKey[ksAlert][0] = clrKey[ksAlert][1] =
    DiffColour(clrKey[ksUnpressed][0],
               wxColour(KEYCLR_UNPRESSED),
               wxColour(KEYCLR_ALERT));
// These are FIXED for now:
clrKey[ksLEDOn][0] = wxColour(KEYCLR_LEDON);
clrKey[ksLEDOn][1] = wxColour(KEYCLR_LEDON_HI);

bAllKeys = false;
getLEDStates = true;
for (int i = 0; i < _countof(bLEDState); i++)
  bLEDState[i] = false;
// set up 2ms timer
t.SetOwner(this, Kbd_Timer1);
t.Start(2);
}

/*****************************************************************************/
/* ~CKbdWnd : destructor                                                     */
/*****************************************************************************/

CKbdWnd::~CKbdWnd(void)
{
CaptureAllKeys(false); // stop capturing in any case
matrix2Key.clear();
hid2Key.clear();
keys.clear();
}

/*****************************************************************************/
/* CaptureAllKeys : switch between local and global key fetching             */
/*****************************************************************************/

void CKbdWnd::CaptureAllKeys(bool bOn)
{
if (bOn == bAllKeys)
  return;
bAllKeys = bOn;

#ifdef __WXMSW__
if (bAllKeys)
  {
  if (hKeyboardHook)
    regWnds.push_back(this);
  else
    {
    RAWINPUTDEVICE rid = {0};
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_INPUTSINK; // | RIDEV_NOLEGACY to suppress WM_KEY*,WM_CHAR
    rid.hwndTarget = GetHwnd();
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
    }
  }
else
  {
  for (size_t i = 0; i < regWnds.size(); i++)
    if (this == regWnds[i])
      {
      regWnds.erase(regWnds.begin() + i);
      break;
      }
  RAWINPUTDEVICE rid = {0};
  rid.usUsagePage = 0x01;
  rid.usUsage = 0x06;
  rid.dwFlags = RIDEV_REMOVE;
  rid.hwndTarget = GetHwnd();
  RegisterRawInputDevices(&rid, 1, sizeof(rid));
  }
#endif  // __WXMSW__
}

/*****************************************************************************/
/* SetLayout : put new layout into on-screen keyboard                        */
/*****************************************************************************/

void CKbdWnd::SetLayout(KbdGui const &newlayout)
{
for (int i = 0; i < _countof(bLEDState); i++)
  bLEDState[i] = false;

wxSize szOld = GetMinSize();
layout = newlayout;
wxSize szNew = CalcLayout(szOld);
if (szOld != szNew)
  {
  SetSize(szNew);
  SetMinSize(szNew);
  // propagate size changes (potentially requires overriding parent's Layout())
  if (GetParent())
    GetParent()->Layout();
  }
Refresh();
}

/*****************************************************************************/
/* CalcLayout : calculates the new keyboard layout and size                  */
/*****************************************************************************/

wxSize CKbdWnd::CalcLayout(wxSize sz)
{
if (sz.x < 0 || sz.y < 0)
  return sz;

#if 1
// fixed size
int nMult = 36;  // neatly divisible by 2, 3 and 4

sz.x = 8 + (int)ceil(nMult * layout.GetHorizontalUnits());
sz.y = 8 + (int)ceil(nMult * layout.GetVerticalUnits());
#else
// variable layout based on externally dictated size
int nMult = (int)ceil(sz.x / layout.GetHorizontalUnits());
sz.y = (int)ceil(nMult * layout.GetVerticalUnits());
#endif

keys.clear();
KeyLayout kl;
kl.state = ksUnpressed;
for (size_t i = 0; i < layout.size(); i++)
  {
  kl.def = &layout.GetKey(i);
  if (kl.def->hidcode < 0 &&
      kl.def->matrixrow < 0 &&
      kl.def->matrixcol < 0)
    continue;
  kl.rects = 1;
  kl.rect[0].x = 4 + (int)(kl.def->startx1 * nMult);
  kl.rect[0].width = -2 + (int)(fabs(kl.def->width1) * nMult);
  kl.rect[0].y = 4 + (int)(kl.def->starty1 * nMult);
  if (kl.def->height <= 1.f || kl.def->width1 == kl.def->width2)
    kl.rect[0].height = -2 + (int)(fabs(kl.def->height) * nMult);
  else
    {
    // 2-unit irregular key like ISO Enter
    kl.rects++;
    kl.rect[1].x = 4 + (int)(kl.def->startx2 * nMult);
    kl.rect[1].width = -2 + (int)(fabs(kl.def->width2) * nMult);
    // if 2-part and lower part is smaller (ISO Enter)
    if (kl.def->width2 < kl.def->width1)
      {
      kl.rect[0].height = -2 + nMult;
      kl.rect[1].height = nMult + 1;
      }
    else  // if 2-part and lower part is larger
      {
      kl.rect[0].height = nMult + 1;
      kl.rect[1].height = -2 + nMult;
      }
    kl.rect[1].y = kl.rect[0].y + kl.rect[0].height - 1;
    }
  // if that's a LED, key.rects = 1, but rect2 gives the LED position
  if ((kl.def->hidcode >> 8) == TYPE_LED)
    {
    kl.rect[1].width = 9;
    kl.rect[1].height = 6;
    kl.rect[1].x = kl.rect[0].x + + (kl.rect[0].width - kl.rect[1].width) / 2;
    kl.rect[1].y = kl.rect[0].y + kl.rect[0].height - kl.rect[1].height - 2;
    }

  keys.push_back(kl);
  }

hid2Key.clear();
matrix2Key.clear();
for (size_t i = 0; i < keys.size(); i++)
  {
  KeyLayout &k = keys[i];
  if (k.def->hidcode >= 0)
    hid2Key[k.def->hidcode] = &k;
  wxUint32 pos = (k.def->matrixrow << 24) | k.def->matrixcol;
  if (pos != (wxUint32)-1)
    matrix2Key[pos] = &k;
  }
return sz;
}

/*****************************************************************************/
/* OnPaint : called to repaint the window                                    */
/*****************************************************************************/

void CKbdWnd::OnPaint(wxPaintEvent &ev)
{
wxBufferedPaintDC dc(this);
// wxPaintDC dc(this);

wxBrush brBack(clrBkgnd);
wxPen penBack(clrBkgnd);
wxPen penKeyBound(clrBorder);
wxBrush brKey[ksStates][2] =
  {
    { wxBrush(clrKey[ksUnpressed][0]), wxBrush(clrKey[ksUnpressed][1]) },
    { wxBrush(clrKey[ksPressed][0]),   wxBrush(clrKey[ksPressed][1]) },
    { wxBrush(clrKey[ksReleased][0]),  wxBrush(clrKey[ksReleased][1]) },
    { wxBrush(clrKey[ksAlert][0]),     wxBrush(clrKey[ksAlert][1]) },
    { wxBrush(clrKey[ksLEDOn][0]),     wxBrush(clrKey[ksLEDOn][1]) }
  };
wxPen penKey[ksStates][2] =
  {
    { wxPen(clrKey[ksUnpressed][0]), wxPen(clrKey[ksUnpressed][1]) },
    { wxPen(clrKey[ksPressed][0]),   wxPen(clrKey[ksPressed][1]) },
    { wxPen(clrKey[ksReleased][0]),  wxPen(clrKey[ksReleased][1]) },
    { wxPen(clrKey[ksAlert][0]),     wxPen(clrKey[ksAlert][1]) },
    { wxPen(clrKey[ksLEDOn][0]),     wxPen(clrKey[ksLEDOn][1]) }
  };
KeyClrs kclr[ksStates][2] =
  {
    {
      { brKey[ksUnpressed][0], brKey[ksUnpressed][0], penKeyBound, penKey[ksUnpressed][0] },
      { brKey[ksUnpressed][1], brKey[ksUnpressed][1], penKeyBound, penKey[ksUnpressed][1] }
    },
    {
      { brKey[ksPressed][0], brKey[ksUnpressed][0], penKeyBound, penKey[ksPressed][0] },
      { brKey[ksPressed][1], brKey[ksUnpressed][1], penKeyBound, penKey[ksPressed][1] }
    },
    {
      { brKey[ksReleased][0], brKey[ksUnpressed][0], penKeyBound, penKey[ksReleased][0] },
      { brKey[ksReleased][1], brKey[ksUnpressed][1], penKeyBound, penKey[ksReleased][1] }
    },
    {
      { brKey[ksAlert][0], brKey[ksUnpressed][0], penKeyBound, penKey[ksAlert][0] },
      { brKey[ksAlert][1], brKey[ksUnpressed][1], penKeyBound, penKey[ksAlert][1] }
    },
    {
      { brKey[ksLEDOn][0], brKey[ksUnpressed][0], penKeyBound, penKey[ksLEDOn][0] },
      { brKey[ksLEDOn][1], brKey[ksUnpressed][1], penKeyBound, penKey[ksLEDOn][1] }
    },
  };

dc.SetBrush(brBack);
dc.SetPen(penBack);
dc.SetFont(keyFont);
// Find Out where the window is scrolled to
//wxPoint vb = GetViewStart();     // Top left corner of client
wxRegionIterator upd(GetUpdateRegion()); // get the update rect list
while (upd)
  {
  wxRect rect(upd.GetRect());
  dc.DrawRectangle(rect);
  // Repaint this rectangle        ...some code...
  bool bKeysPainted = false;
  for (size_t i = 0; i < keys.size(); i++)
    {
    int clridx = (keys[i].state & ~ksHighlighted);
    int high = !!(keys[i].state & ksHighlighted);
    if (rect.Intersects(keys[i].rect[0]) ||
        (keys[i].rects > 1 && rect.Intersects(keys[i].rect[1])))
      {
      DrawKey(dc, keys[i], kclr[clridx][high]);
      bKeysPainted = true;
      }
    }
  if (bKeysPainted)
    {
    dc.SetBrush(brBack);
    dc.SetPen(penBack);
    }
  upd++;
  }
}

/*****************************************************************************/
/* DrawKey : draws a single key                                              */
/*****************************************************************************/

void CKbdWnd::DrawKey
    (
    wxDC &dc,
    KeyLayout const &key,
    KeyClrs const &clrs
    )
{
GuiKey const &def = *key.def;
bool bIsLed = ((def.hidcode >> 8) == TYPE_LED);
dc.SetBrush(bIsLed ? clrs.br2 : clrs.br1);
dc.SetPen(clrs.pen1);
wxRect rc(key.rect[0]);
dc.DrawRectangle(rc);
if (key.rects > 1)
  {
  wxRect rc2(key.rect[1]);
  dc.DrawRectangle(rc2);
  wxRect rc3(rc);
  rc3 = rc3.Intersect(rc2);
  dc.SetPen(clrs.pen2);
  dc.DrawLine(rc3.x + 1, rc3.y, rc3.x + rc3.width - 1, rc3.y);
  }

// if that's a LED, key.rects = 1, but rect2 gives the LED position
if (bIsLed)
  {
  dc.SetBrush(clrs.br1);
  dc.DrawRectangle(key.rect[1]);
  rc.height -= key.rect[1].height + 2;
  }

rc.Deflate(1, 1);
dc.DrawLabel(def.label[0],
             wxNullBitmap,
             rc,
             wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
}

/*****************************************************************************/
/* SetKeyState : sets a key to a new state                                   */
/*****************************************************************************/

void CKbdWnd::SetKeyState(KeyLayout *key, int newstate, bool passOn)
{
if (key &&
    (key->def->hidcode >= 0 ||
     (key->def->matrixcol >= 0 && key->def->matrixrow >= 0)) &&
    key->state != (KeyState)newstate)
  {
  key->state = (KeyState)newstate;
  RefreshKey(key);
  if (passOn &&
      key->def->matrixcol >= 0 &&
      key->def->matrixrow >= 0)
    GetApp()->GetMain()->SelectMatrix(key->def->matrixrow,
                                      key->def->matrixcol);
  }
}

// this is going to be a long, interesting journey
// into the abyss of OS dependencies ...

/*****************************************************************************/
/* MSWWindowProc : specialized window procedure for MS Windows variant       */
/*****************************************************************************/

#ifdef __WXMSW__
WXLRESULT CKbdWnd::MSWWindowProc
    (
    WXUINT nMsg,
    WXWPARAM wParam,
    WXLPARAM lParam
    )
{
switch (nMsg)
  {
  case WM_INPUT :
    {
    if (RIM_INPUT == GET_RAWINPUT_CODE_WPARAM(wParam))
      {
      UINT uiSize;
      // fetch necessary buffer size
      GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
                      NULL, &uiSize,
                      sizeof(RAWINPUTHEADER));
      BYTE *lpb = new BYTE[uiSize];
      if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
                          lpb, &uiSize,
                          sizeof(RAWINPUTHEADER)) == uiSize)
        {
        RAWINPUT *raw = (RAWINPUT *)lpb;
        if (raw->header.dwType == RIM_TYPEKEYBOARD)
          {
          wxString s(wxString::Format(wxT(" Kbd: MakeCode=%04x Flags=%04x VKey=%04x\n"),
                                      raw->data.keyboard.MakeCode,
                                      raw->data.keyboard.Flags,
                                      raw->data.keyboard.VKey));
          OutputDebugString(s);

          int scancode = raw->data.keyboard.MakeCode;
#if 0
          // use specific scan code
          if (raw->data.keyboard.Flags & RI_KEY_E0)
            scancode |= 0xe000;
          else if (raw->data.keyboard.Flags & RI_KEY_E1)
            scancode |= 0xe100;
#else
          // use general "extended" flag
          if (raw->data.keyboard.Flags & RI_KEY_E0)
            {
            // Ctrl+Break is sent as 
            // Scan  VKey      Flags
            // -------------------------
            // 0x46  VK_CANCEL RI_KEY_E0
            scancode |= 0x100;
            }
          else if (raw->data.keyboard.Flags & RI_KEY_E1)
            {
            // Only used in weird sequences, so ignore it.
            // PAUSE is sent as a 4-message sequence:
            // Scan  VKey      Flags
            // -------------------------
            // 0x1D  VK_PAUSE  RI_KEY_E1
            // 0x45  0xff
            // 0x1D  VK_PAUSE  RI_KEY_E1 | RI_KEY_BREAK
            // 0x45  0xff      RI_KEY_BREAK
            scancode |= 0x200;
            }

          // Alt+Print is sent as
          // Scan  VKey      Flags
          // -------------------------
          // 0x54  VK_SNAPSHOT  -

          // NumLock is sent as
          // Scan  VKey      Flags
          // -------------------------
          // 0x45  VK_NUMLOCK  -


          if (scancode == 0x45 && 
              raw->data.keyboard.VKey == VK_NUMLOCK)
            scancode = 0x145;
#endif
          int hid = KbdGui::GetHIDFromScancode(scancode);
          if (hid != KB_NONE)
            {
            if (raw->data.keyboard.Flags & RI_KEY_BREAK)
              SetKeyState(hid, ksReleased);
            else // it's RI_KEY_MAKE
              SetKeyState(hid, ksPressed, true);
            }
          }
        }
      delete[] lpb;
      }
    // else it's RIM_INPUTSINK while we're in the background. Not interested.
    }
    break;
  }
return wxPanel::MSWWindowProc(nMsg, wParam, lParam);
}
#endif // __WXMSW__

/*****************************************************************************/
/* OnKeyDown : called when a key is pressed on the window                    */
/*****************************************************************************/

void CKbdWnd::OnKeyDown(wxKeyEvent &ev)
{
//#if 1
#ifdef _DEBUG
// not for release builds until it really works!

int key = KB_UNUSED;

#if defined(wxHAS_RAW_KEY_CODES) && defined(__WXMSW__)
if (!bAllKeys)
  {
  // get the raw key flags
  wxUint32 rawFlags = ev.GetRawKeyFlags();
  // Bits 16 .. 23: scancode
  // Bit 24: flag whether extended key
  // Bit 30: previous key state (1 = was down, i.e., autorepeat)
  int scancode = (rawFlags >> 16) & 0x1ff;
  if (rawFlags & (1 << 30))
    key = KB_UNUSED;
  else
    key = KbdGui::GetHIDFromScancode(scancode);
  }

#else
int kc = ev.GetKeyCode();               /* get key code from event           */
int key = KbdGui::GetHID(kc);           /* retrieve HID for that key         */
#endif

if (key != KB_UNUSED)
  {
  SetKeyState(key, ksPressed, true);
  ev.Skip(false);
  }

#endif
// default: do nothing. wxGrid gets it anyway.
}

/*****************************************************************************/
/* OnChar : called for character events                                      */
/*****************************************************************************/

void CKbdWnd::OnChar(wxKeyEvent &ev)
{
//#if 1
#ifdef _DEBUG
// not for release builds until it really works!

int key = KB_UNUSED;

#if defined(wxHAS_RAW_KEY_CODES) && defined(__WXMSW__)
if (!bAllKeys)
  {
  // get the raw key flags
  wxUint32 rawFlags = ev.GetRawKeyFlags();
  // Bits 16 .. 23: scancode
  // Bit 24: flag whether extended key
  int scancode = (rawFlags >> 16) & 0x1ff;
  if (rawFlags & (1 << 30))
    key = KB_UNUSED;
  else
    key = KbdGui::GetHIDFromScancode(scancode);
  }

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
// default: do nothing. wxGrid gets it anyway.
}

/*****************************************************************************/
/* OnKeyUp : called when a key is released on the window                     */
/*****************************************************************************/

void CKbdWnd::OnKeyUp(wxKeyEvent &ev)
{
//#if 1
#ifdef _DEBUG
// not for release builds until it really works!

int key = KB_UNUSED;

#if defined(wxHAS_RAW_KEY_CODES) && defined(__WXMSW__)
if (!bAllKeys)
  {
  // get the raw key flags
  wxUint32 rawFlags = ev.GetRawKeyFlags();
  // Bits 16 .. 23: scancode
  // Bit 24: flag whether extended key
  int scancode = (rawFlags >> 16) & 0x1ff;
  key = KbdGui::GetHIDFromScancode(scancode);
  }
#else
// not for release builds until it really works!
int kc = ev.GetKeyCode();
int key = KbdGui::GetHID(kc);           /* retrieve HID for that key         */
#endif

if (key != KB_UNUSED)
  {
  SetKeyState(key, ksReleased,
#ifdef __WXMSW__
  // without installing a keyboard hook, Print Screen is only released
              (key == KB_PSCRN)
#else
              false
#endif
              );

  ev.Skip(false);
  }

#endif
// default: do nothing. wxGrid gets it anyway.
}

/*****************************************************************************/
/* OnLButtonDown : called when the left mouse button is clicked on window    */
/*****************************************************************************/

void CKbdWnd::OnLButtonDown(wxMouseEvent& event)
{
#if 0
// if necessary, do this in a subclass
if (!this->HasFocus())
  {
  SetFocus();
  Refresh();
  Update();
  }
#endif
  
// look whether it's on a key
for (size_t i = 0; i < keys.size(); i++)
  if (keys[i].rect[0].Contains(event.GetPosition()) ||
      (keys[i].def->width1 != keys[i].def->width2 &&
       keys[i].rect[1].Contains(event.GetPosition())))
    {
    if (keys[i].def->matrixrow >= 0 &&
        keys[i].def->matrixcol >= 0)
      GetApp()->GetMain()->SelectMatrix(keys[i].def->matrixrow,
                                        keys[i].def->matrixcol);
    break;
    }
}

/*****************************************************************************/
/* OnReadLEDTimer : called to set the current LED state                      */
/*****************************************************************************/

void CKbdWnd::OnReadLEDTimer(wxTimerEvent& event)
{
if (!getLEDStates)                      /* do nothing if configured out      */
  return;
// Read current LED states
bool bLEDs[_countof(bLEDState)] =
  {
  (wxGetKeyState(WXK_NUMLOCK) != 0),
  (wxGetKeyState(WXK_CAPITAL) != 0),
  (wxGetKeyState(WXK_SCROLL) != 0)
  };
for (int i = 0; i < _countof(bLEDState); i++)
  {
  if (bLEDState[i] != bLEDs[i])
    {
    SetKeyState(0xfe00 + i,
                bLEDs[i] ?
                    CKbdWnd::ksLEDOn :
                    CKbdWnd::ksUnpressed);
    bLEDState[i] = bLEDs[i];
    }
  }
}
