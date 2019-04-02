/*****************************************************************************/
/* MatrixWnd.h : CMatrixWnd declaration                                      */
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

#ifndef _MatrixWnd_h__included_
#define _MatrixWnd_h__included_

#include "KbdGuiLayout.h"

/*****************************************************************************/
/* CMatrixWnd class declaration                                              */
/*****************************************************************************/

class CMatrixWnd : public wxGrid
{
public:
    CMatrixWnd(wxWindow *parent, int numRows = NUMROWS, int numCols = NUMCOLS, bool switched = true);
    virtual ~CMatrixWnd(void);

    // Attention: these must be done before SetKbdMatrix()!
    void SetOrientation(bool switched = true);
    void SetLayout(int numRows = NUMROWS, int numCols = NUMCOLS);

    void SetLayer(int layer) { layernum = layer; }
    int GetLayer() { return layernum; }
    void SetKbdMatrix(KbdMatrix const &kbm);
    void SelectMatrix(int row, int col);

    wxSize GetMatrixSize()
      { return wxSize(GetColRight(GetNumberCols() - 1),
                      GetRowBottom(GetNumberRows() - 1)); }

protected:
    void RC2Internal(int &row, int &col)
      {
      if (bColsRowsSwitched)
        {
        int i = col;
        col = row;
        row = i;
        }
      }
    // may be overridden if necessary
    virtual void OnMatrixChanged(int row, int col, wxUint16 key);

private:
    wxDECLARE_EVENT_TABLE();

    void OnKeyDown(wxKeyEvent &);
    void OnChar(wxKeyEvent &);
    void OnKeyUp(wxKeyEvent &);
    void OnCellValueChanged( wxGridEvent& );

protected:
    int layernum;
    bool bColsRowsSwitched;

};

void SetupText2HIDMapping();
void RemoveText2HIDMapping();


#endif // defined(_MatrixWnd_h__included_)
