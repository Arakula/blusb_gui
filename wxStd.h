/*****************************************************************************/
/* wxStd.h : main header file for wxWidgets configuration etc.               */
/*****************************************************************************/
// needs to be done for each application, and needs to be included everywhere
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

#ifndef _WXSTD_H__INCLUDED_
#define _WXSTD_H__INCLUDED_

// wxWidgets standard headers ...
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
#ifndef WX_PRECOMP
    #include "wx/wx.h"
    #include "wx/mdi.h"
#endif

#include "wx/docview.h"
#include "wx/apptrait.h"
#include "wx/cmdline.h"
#include "wx/config.h"
#include "wx/fileconf.h"
#include "wx/dynarray.h"
#include "wx/dir.h"
#include "wx/toolbar.h"
#include "wx/image.h"
#include "wx/splash.h"
#include "wx/mediactrl.h"
#include "wx/stdpaths.h"
#include "wx/filename.h"
#include "wx/textfile.h"
#include "wx/utils.h"
#include "wx/wfstream.h"
#include "wx/mstream.h"
#include "wx/docmdi.h"
#include "wx/grid.h"
#include "wx/notebook.h"
#include "wx/utils.h"
#include "wx/msgdlg.h"
#include "wx/dcbuffer.h"

// Macros that would normally be defined in Windows
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

#endif // !defined(_WXSTD_H__INCLUDED_)
