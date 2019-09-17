/*****************************************************************************/
/* KbdGuiLayout.h - GUI layout definition for the keyboard                   */
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

#ifndef _KbdGuiLayout_h___included_
#define _KbdGuiLayout_h___included_

#include "layout.h"

/*****************************************************************************/
/* MatrixKey : in BlUSB, one key in the matrix is an uint16_t (HID+type)     */
/*****************************************************************************/

typedef wxUint16 MatrixKey;

/*****************************************************************************/
/* MacroKey : in BlUSB, one key in the macro is an uint8_t (HID?)            */
/*****************************************************************************/

typedef wxUint8 MacroKey;

/*****************************************************************************/
/* KbdMatrix : class definition for a keyboard matrix                        */
/*****************************************************************************/

class KbdMatrix
  {
  public:
    KbdMatrix(int rows = 0, int cols = 0, MatrixKey const *values = NULL)
      : rows(0), cols(0)
      { Resize(rows, cols, values); }
    KbdMatrix(KbdMatrix const &org)
      : rows(0), cols(0)
      { DoCopy(org); }
    virtual ~KbdMatrix() {}
    KbdMatrix &operator=(KbdMatrix const &org)
      { return DoCopy(org); }

    void Resize(int newrows = 0, int newcols = 0, MatrixKey const *values = NULL)
      {
      // Theoretically, we could copy the old contents to the new position here
      // wxVector<MatrixKey> oldkeys(keys);
      // int oldrows = rows, oldcols = cols;
      newrows = max(min(newrows, MAXROWS), 0);
      newcols = max(min(newcols, MAXCOLS), 0);
      if (newrows * newcols &&
          newrows * newcols > rows * cols)
        keys.resize(newrows * newcols);
      rows = newrows;
      cols = newcols;
      SetKeys(values);
      }
    int GetRows() const { return rows; }
    int GetCols() const { return cols; }
    MatrixKey &GetKeys() { return keys[0]; }
    int GetKey(int row, int col) const
      { return (row < rows && col < cols) ? keys[row * cols + col] : KB_UNUSED; }
    void SetKey(int row, int col, MatrixKey value)
      { if (row < rows && col < cols) keys[row * cols + col] = value; }
    void SetKeys(MatrixKey const *values = NULL)
      {
      if (values)
        {
        for (int i = 0; i < rows * cols; i++)
          keys[i] = values[i];
        }
      else
        {
        for (int i = 0; i < rows * cols; i++)
          keys[i] = KB_UNUSED;
        }
      }

  protected:
    int rows, cols;
    wxVector<MatrixKey> keys;
    KbdMatrix &DoCopy(KbdMatrix const &org)
      {
      rows = org.rows;
      cols = org.cols;
      keys.assign(org.keys.begin(), org.keys.end());
      return *this;
      }
  };

/*****************************************************************************/
/* KbdMacro : class definition for a keyboard macro                          */
/*****************************************************************************/

// TODO: QUITE a LOT still missing here. Including understanding macros 8-)
class KbdMacro
  {
  public:
    KbdMacro(int mods = 0, int keys = 0, MacroKey const *values = NULL)
      : mods(mods)
      { Resize(keys, values); }

    void Resize(int newkeys = 0, MacroKey const *values = NULL)
      {
      // TODO: get that right!
      newkeys = max(min(newkeys, 6), 0);
      keys.resize(newkeys);
      SetKeys(values);
      }

    void SetKeys(MacroKey const *values = NULL)
      {
      if (values)
        {
        for (size_t i = 0; i < keys.size(); i++)
          keys[i] = values[i];
        }
      else
        {
        for (size_t i = 0; i < keys.size(); i++)
          keys[i] = KB_UNUSED;
        }
      }

  protected:
    wxUint8 mods;
    wxVector<MacroKey> keys;
    KbdMacro &DoCopy(KbdMacro const &org)
      {
      mods = org.mods;
      keys.assign(org.keys.begin(), org.keys.end());
      return *this;
      }

  };

/*****************************************************************************/
/* KbdLayout : class definition for a keyboard layout (layers of matrices)   */
/*****************************************************************************/

class KbdLayout
  {
  public:
    // TODO: devise macro initialization functionality
    KbdLayout(int layers = 0, int rows = 0, int cols = 0, int macros = 0, MatrixKey *values = NULL)
      : layers(0), rows(0), cols(0), macros(0)
      {
      Resize(layers, rows, cols, macros, values);
      SetModified(false);
      }
    KbdLayout(KbdLayout const &org)
      { DoCopy(org); }
    virtual ~KbdLayout() {}
    KbdLayout &operator=(KbdLayout const &org)
      { return DoCopy(org); }
    KbdMatrix &operator[](int n) { return layer[n]; }

    bool InsertLayer(int pos, int count = 1)
      {
      if (count <= 0 || count + layers > NUMLAYERS_MAX)
        return false;
      for (int i = 0; i < count; i++)
        layer.insert(layer.begin() + pos + i, KbdMatrix(rows, cols));
      layers += count;
      SetModified();
      return true;
      }
    bool RemoveLayer(int pos, int count = 1)
      {
      if (pos < 0 || count <= 0 || pos >= layers)
        return false;
      if (pos + count > layers)
        count = layers - pos;
      layer.erase(layer.begin() + pos, layer.begin() + pos + count - 1);
      layers -= count;
      SetModified();
      return true;
      }
    bool AddLayer(int count = 1)
      { return InsertLayer(layers, count); }
    // TODO: devise macro initialization functionality
    void Resize(int layers = 0, int rows = 0, int cols = 0, int macros = 0, MatrixKey *values = NULL)
      {
      layers = max(min(layers, NUMLAYERS_MAX), 0);
      rows = max(min(rows, MAXROWS), 0);
      cols = max(min(cols, MAXCOLS), 0);
      macros = max(min(macros, NUM_MACROKEYS), 0);
      if (this->layers != layers ||
          this->rows != rows ||
          this->cols != cols)
        SetModified();
      this->layers = layers;
      this->rows = rows;
      this->cols = cols;
      if (layers > 0 && rows > 0 && cols > 0)
        {
        for (int i = 0; i < layers; i++)
          {
          if (i >= (int)layer.size())
            layer.push_back(KbdMatrix(rows, cols, values));
          else
            layer[i].Resize(rows, cols, values);
          if (values)
            values += (rows * cols);
          }
        }
      }
    int GetLayers() const { return layers; }
    int GetMaxLayers() const { return (int)layer.size(); }
    int GetRows() const { return rows; }
    int GetCols() const { return cols; }
    int GetMacros() const { return macros; }
    MatrixKey &GetKeys(int layernum = 0) { return layer[layernum].GetKeys(); }
    int GetKey(int layernum, int row, int col)
      { return layer[layernum].GetKey(row, col); }
    void SetKey(int layernum, int row, int col, MatrixKey value)
      {
      if (layer[layernum].GetKey(row, col) != value)
        SetModified();
      return layer[layernum].SetKey(row, col, value);
      }
    KbdMacro &GetMacro(int macronum = 0) { return macro[macronum]; }
    // TODO: some sort of macro getter/setter

    // import layout in Model M USB transfer format
    bool Import(wxUint8 *layout, int bufsize /* in bytes! */ = -1,
                int tgtrows = NUMROWS, int tgtcols = NUMCOLS,
                wxUint8 *macrobuf = NULL, int macrosize = 0,
				int transformat = 0)  // 0=V<1.5, 1=V>=1.5
      {
      int newLayers = layout[0];  // sanitize
      newLayers = max(min(newLayers, NUMLAYERS_MAX), 0);
      if (bufsize >= 0 &&
          bufsize < (int)sizeof(wxUint16) * (1 + newLayers * tgtrows * tgtcols))
        return false;
      // sanitize incoming macros
      int macros = macrobuf ? macrosize / LEN_MACRO : 0;
      for (int mac = 0; mac < macros; mac++)
        {
        wxUint8 *pmac = macrobuf + mac * LEN_MACRO;
        /* layout:
           pmac[0] = mods
           pmac[1] = reserved
           pmac[2] .. pmac[7] = macro keys
        */
        // sanity check: must not consist of FF only
        int zeros = 0, ffs = 0;
        for (int i = 0; i < LEN_MACRO; i++)
          if (pmac[i] == 0xff)
            ffs++;
        // reformat uninitialized macro areas
        if (ffs == LEN_MACRO)
          memset(pmac, KB_UNUSED, LEN_MACRO);
        }
      Resize(newLayers, tgtrows, tgtcols, macros);
	  if (transformat == 1)
        layout += sizeof(wxUint8);
	  else
        layout += sizeof(wxUint16);
      for (int l = 0; l < newLayers; l++)
        for (int r = 0; r < tgtrows; r++)
          for (int c = 0; c < tgtcols; c++)
            {
            // Controller sends LSB first, whatever our internal format might be
            wxUint16 k = *layout++;
            k += (*layout++) << 8;
            SetKey(l, r, c, k);
            }
      // TODO: import the macros!
      SetModified(false);
      return true;
      }
    // export layout to Model M USB transfer format
    bool Export(wxUint8 *buf, int &bufsize /* in bytes!*/,
                int tgtrows = NUMROWS, int tgtcols = NUMCOLS,
				int transformat = 1)  // 0=V<1.5, 1=V>=1.5
      {
      if (bufsize < 1 + (int)sizeof(wxUint16) * (layers * tgtrows * tgtcols))
        return false;
	  // restrict to maximum layers for the device!
	  int ilayers = layers;
	  if (transformat == 0 && ilayers > NUMLAYERS_MAX_OLD)
		  ilayers = NUMLAYERS_MAX_OLD;
      bufsize = 1 + (int)sizeof(wxUint16) * (ilayers * tgtrows * tgtcols);
      *buf++ = ilayers;
      for (int l = 0; l < ilayers; l++)
        for (int r = 0; r < tgtrows; r++)
          for (int c = 0; c < tgtcols; c++)
            {
            wxUint16 k = GetKey(l, r, c);
            // LSB first, whatever our internal format might be
            *buf++ = k & 0xff;
            *buf++ = k >> 8;
            }
      // TODO: export the macros!
      return true;
      }
    bool ReadFile(wxString const &filename);
    bool WriteFile(wxString const &filename, bool bNative = true,
                   int tgtrows = NUMROWS, int tgtcols = NUMCOLS);

    bool IsModified() { return bModified; }
    void SetModified(bool bOn = true) { bModified = bOn; }

  protected:
    bool bModified;
    int layers, rows, cols, macros;
    wxVector<KbdMatrix> layer;
    wxVector<KbdMacro> macro;
    KbdLayout &DoCopy(KbdLayout const &org)
      {
      layers = org.layers;
      rows = org.rows;
      cols = org.cols;
      macros = org.macros;
      layer.assign(org.layer.begin(), org.layer.end());
      macro.assign(org.macro.begin(), org.macro.end());
      SetModified(false);
      return *this;
      }
  };

/*****************************************************************************/
/* GuiKey : definition for one key                                           */
/*****************************************************************************/

struct GuiKey
  {
  int row;                              /* start row for the key             */
  float height;                         /* key height                        */
  int hidcode;                          /* HID code for that key             */
  int matrixrow, matrixcol;             /* matrix position for that key      */
                                        /* -1/-1 indicates empty space       */
  float width1, width2;                 /* top / bottom with for the key     */
                                        /* width2 is mainly for ISO Enter,   */
                                        /*   which has 2 different sizes     */
  wxString label[2];                    /* 2 labels                          */
                                        /* 0 : key label text                */
                                        /* 1 : for logging etc.              */
  float startx1, starty1;               /* top X/Y position for the key      */
  float startx2, starty2;               /* bottom X/Y position for the key   */
  // startx2/y2 can be deduced from row/height, but if the height becomes
  // non-integral at a later point, having them separately is a good thing.
  };

/*****************************************************************************/
/* KbdGui : definition for the whole keyboard GUI                            */
/*****************************************************************************/

class KbdGui
  {
  public:
    enum
      {
      KbdDefault = -1,
      KbdANSI,
      KbdISO,
      KbdANSI121,
      KbdISO122,
      };
    KbdGui(int kbdStyle = KbdDefault);
    virtual ~KbdGui();
    KbdGui(KbdGui const &org) { DoCopy(org); }
    KbdGui &operator=(KbdGui const &org) { return DoCopy(org); }

    GuiKey &operator[](int index) { return keys[index]; }

    static void SetDefault(bool bISO = false, bool b122 = false);

    // get the display name
    wxString const &GetName() { return layoutName; }
    size_t size() const { return keys.size(); }
    GuiKey const &GetKey(int index) const { return keys[index]; }
    float GetHorizontalUnits() { return unitsH; }
    float GetVerticalUnits() { return unitsV; }
    void GetMatrixLayout(int &nRows, int &nCols) const
      { nRows = nMaxRow + 1; nCols = nMaxCol + 1; }
    int GetMaxRow() { return nMaxRow; }
    int GetMaxCol() { return nMaxCol; }

    bool ReadLayoutFile(wxString const &filename, wxString *error = NULL);
    bool WriteLayoutFile(wxString const &filename);

    // convert OS Virtual Key -> HID
    static int GetHID(int /* wxKeyCode */ vkey);
    // convert scan code -> HID
    static int GetHIDFromScancode(int scancode);

  protected:
    KbdGui &DoCopy(KbdGui const &org);
    void CalcLayout();

  protected:
    static GuiKey *pDefault;
    static int nDefault;
    static wxString sDefault;
    wxString layoutName;
    wxVector<GuiKey> keys;
    float unitsV, unitsH;
    int nMaxRow, nMaxCol;
  };

#endif // !defined(_KbdGuiLayout_h___included_)
