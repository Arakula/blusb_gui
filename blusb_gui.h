/*****************************************************************************/
/* blusb_gui.h : main program definitions                                    */
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

#ifndef _blusb_gui_h__included_
#define _blusb_gui_h__included_

#include "BlUsbDev.h"
#include "MainFrm.h"
#include "KbdGuiLayout.h"

/*****************************************************************************/
/* CBlusbGuiApp : main program class                                         */
/*****************************************************************************/

class CBlusbGuiApp : public wxApp
{
public:
	CBlusbGuiApp();

    virtual bool OnInit() wxOVERRIDE;
    virtual int  OnExit() wxOVERRIDE;
    virtual void OnInitCmdLine(wxCmdLineParser& parser) wxOVERRIDE;
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser) wxOVERRIDE;

    CMainFrame *GetMain() { return pMain; }

    bool IsDevOpen() { return dev.IsOpen(); }
    int EnableServiceMode()
      {
      int rc = dev.EnableServiceMode();
      inServiceMode = (rc >= BLUSB_SUCCESS);
      return rc;
      }
    int DisableServiceMode()
      {
      inServiceMode = false;
      return dev.DisableServiceMode();
      }
    bool InServiceMode() { return inServiceMode; }
    int ReadVersion(wxUint8 *buffer, int buflen)
      { return dev.ReadVersion(buffer, buflen); }
    int GetFwMajorVersion()
      { return dev.GetFwMajorVersion(); }
    int GetFwMinorVersion()
      { return dev.GetFwMinorVersion(); }
    int GetFwVersion()
      { return dev.GetFwVersion(); }
    int ReadMatrixLayout(int &rows, int &cols)
      { return dev.ReadMatrixLayout(rows, cols); }
    int ReadMatrixPos(wxUint8 *buffer, int buflen)
      { return dev.ReadMatrix(buffer, buflen); }
    int ReadPWM(wxUint8 &pwmUSB, wxUint8 &pwmBT)
      { return dev.ReadPWM(pwmUSB, pwmBT); }
    int WritePWM(wxUint8 pwmUSB, wxUint8 pwmBT)
      { return dev.WritePWM(pwmUSB, pwmBT); }
    KbdLayout &GetLayout() { return layout; }
    void SetLayout(KbdLayout const &org) { layout = org; }
    KbdLayout &GetDefaultLayout(int nNum = -1) { return defaultLayout[nNum < 0 ? curDefaultLayout : nNum]; }
    void SetDefaultLayout(int nNum = 0) { curDefaultLayout = nNum; }
    void SetDefaultLayout(KbdGui const &gui);
    int GetDefaultLayouts() { return _countof(defaultLayout); }
    int ReadLayout(KbdLayout *p = NULL);
    int ReadLayout(wxString const &filename, KbdLayout *p = NULL);
    int WriteLayout(KbdLayout *p = NULL);
    int WriteLayout(wxString const &filename, bool bNative = true, KbdLayout *p = NULL);
    bool IsLayoutModified() { return layout.IsModified(); }
    void SetLayoutModified(bool bOn = true) { layout.SetModified(bOn); }
    int ReadDebounce() { return dev.ReadDebounce(); }
    int WriteDebounce(int nDebounce) { return dev.WriteDebounce(nDebounce); }

public:
    bool ReadConfig(wxString const &key, wxString *str, wxString const &defval = "")
      { return pConfig->Read(key, str, defval); }
    bool ReadConfig(wxString const &key, long *l, long const defval = 0)
      { return pConfig->Read(key, l, defval); }
    bool WriteConfig(wxString const &key, wxString const &str)
      { return pConfig->Write(key, str); }
    bool WriteConfig(wxString const &key, long const l)
      { return pConfig->Write(key, l); }

private:
    wxDECLARE_EVENT_TABLE();
private:
    void OnActivateApp(wxActivateEvent& event);

protected:
    BlUsbDev dev;
    bool inServiceMode;
    KbdLayout layout, defaultLayout[2];
    int curDefaultLayout;
    CMainFrame *pMain;
    wxConfigBase *pConfig;

};
wxDECLARE_APP(CBlusbGuiApp);

CBlusbGuiApp *GetApp();

/*****************************************************************************/
/* CNoServiceMode : little helper class to inhibit service mode              */
/*****************************************************************************/

class CNoServiceMode
{
public:
  CNoServiceMode()
    {
    bInServiceMode = GetApp()->InServiceMode();
    if (bInServiceMode)
      GetApp()->DisableServiceMode();
    }
  ~CNoServiceMode()
    {
    if (bInServiceMode)
      GetApp()->EnableServiceMode();
    }
protected:
  bool bInServiceMode;
};

#endif // !defined(_blusb_gui_h__included_)
