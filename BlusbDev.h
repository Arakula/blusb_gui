/*****************************************************************************/
/* BlUsbDev.h : declaration of the USB device communication classes          */
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

#ifndef _BlusbDev_h__included_
#define _BlusbDev_h__included_

#include "usb_ll.h"

/*****************************************************************************/
/* Definitions                                                               */
/*****************************************************************************/

#ifdef LIBUSB_H
enum blusb_error {
    BLUSB_SUCCESS = LIBUSB_SUCCESS,
	BLUSB_ERROR_IO = LIBUSB_ERROR_IO,
	BLUSB_ERROR_INVALID_PARAM = LIBUSB_ERROR_INVALID_PARAM,
	BLUSB_ERROR_ACCESS = LIBUSB_ERROR_ACCESS,
	BLUSB_ERROR_NO_DEVICE = LIBUSB_ERROR_NO_DEVICE,
	BLUSB_ERROR_NOT_FOUND = LIBUSB_ERROR_NOT_FOUND,
	BLUSB_ERROR_BUSY = LIBUSB_ERROR_BUSY,
	BLUSB_ERROR_TIMEOUT = LIBUSB_ERROR_TIMEOUT,
	BLUSB_ERROR_OVERFLOW = LIBUSB_ERROR_OVERFLOW,
	BLUSB_ERROR_PIPE = LIBUSB_ERROR_PIPE,
	BLUSB_ERROR_INTERRUPTED = LIBUSB_ERROR_INTERRUPTED,
	BLUSB_ERROR_NO_MEM = LIBUSB_ERROR_NO_MEM,
	BLUSB_ERROR_NOT_SUPPORTED = LIBUSB_ERROR_NOT_SUPPORTED,
	BLUSB_ERROR_OTHER = LIBUSB_ERROR_OTHER,
};
#else
enum blusb_error {
    BLUSB_SUCCESS = 0,
	BLUSB_ERROR_IO = -1,
	BLUSB_ERROR_INVALID_PARAM = -2,
	BLUSB_ERROR_ACCESS = -3,
	BLUSB_ERROR_NO_DEVICE = -4,
	BLUSB_ERROR_NOT_FOUND = -5,
	BLUSB_ERROR_BUSY = -6,
	BLUSB_ERROR_TIMEOUT = -7,
	BLUSB_ERROR_OVERFLOW = -8,
	BLUSB_ERROR_PIPE = -9,
	BLUSB_ERROR_INTERRUPTED = -10,
	BLUSB_ERROR_NO_MEM = -11,
	BLUSB_ERROR_NOT_SUPPORTED = -12,
	BLUSB_ERROR_OTHER = -99,
};
#endif

/*****************************************************************************/
/* BlUsbDev : BlUSB device communication class declaration                   */
/*****************************************************************************/

class BlUsbDev : public UsbLL
{
public:
  BlUsbDev(void);
  ~BlUsbDev(void);

  int Open(wxUint16 vendor = 0x04b3, wxUint16 product = 0x301c,
           wxUint16 Usage = -1, wxUint16 UsagePage = -1);
  bool IsOpen() { return !!handle; }
  void Close() { if (IsOpen()) UsbLL::Close(handle); handle = NULL; }

  int ReadMatrixLayout(int &rows, int &cols);

  int EnableServiceMode();
  int DisableServiceMode();

  int ReadVersion(wxUint8 *buffer, int buflen);

  int ReadPWM(wxUint8 &pwmUSB, wxUint8 &pwmBT);
  int WritePWM(wxUint8 pwmUSB, wxUint8 pwmBT);

  int ReadMatrix(wxUint8 *buffer, int buflen);

  int ReadLayout(wxUint8 *buffer, int buflen);
  int WriteLayout(wxUint8 *buffer, int buflen);

  int ReadDebounce();
  int WriteDebounce(int nDebounce);

  int ReadMacros(wxUint8 *buffer, int buflen);
  int WriteMacros(wxUint8 *buffer, int buflen);

  int GetFwMajorVersion() { return blVer[0]; }
  int GetFwMinorVersion() { return blVer[1]; }
  int GetFwVersion() { return (((int)blVer[0]) << 8) | blVer[1]; }

protected:
  void *handle;
  wxUint8 blVer[2];  // version major/minor

};


#endif // defined(_BlusbDev_h__included_)
