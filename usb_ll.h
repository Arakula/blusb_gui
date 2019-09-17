/*****************************************************************************/
/* usb_ll.h : USB Low-Level communication class                              */
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

#ifndef _usb_ll_h__defined_
#define _usb_ll_h__defined_

#include <vector>
#include <string>

/*****************************************************************************/
/* UsbLLDevDesc : our internal device descriptor containing what we need     */
/*****************************************************************************/

struct UsbLLDevDesc
  {
  unsigned short VendorID;
  unsigned short ProductID;
  unsigned short VersionNumber;
  unsigned short UsageID;
  unsigned short UsagePage;
  std::string VendorName;
  std::string ProductName;
  std::string SerialNumber;
  // purely informational items:
  unsigned char Bus;
  unsigned char DeviceAddress;
  UsbLLDevDesc()
    {
    VendorID = ProductID = (unsigned short )-1;
    UsageID = UsagePage = (unsigned short )-1;
    VersionNumber = 0;
    Bus = DeviceAddress = 0;
    }
  };

/*****************************************************************************/
/* UsbLL : basic low-level USB communication class                           */
/*****************************************************************************/

class UsbLL
{
public:
  UsbLL() { ctx = NULL; Initialize(); }
  virtual ~UsbLL() { Terminate(); }

  bool Initialize();
  void Terminate();
  bool IsInitialized() { return !!ctx; }

  int GetDeviceList(std::vector<void *>& devList);
  void FreeDeviceList(std::vector<void *>& devList);

  int GetDeviceDescriptor(void *dev, UsbLLDevDesc &desc);

  int Open(void *dev, void *&handle);
  void Close(void *handle);

  int Send(void *dev_handle, void *buf, int len, int timeout = 1000);
  int Receive(void *dev_handle, void *buf, int len, int timeout = 1000);
  int ControlTransfer(void *dev_handle,
	                  unsigned char request_type,
                      unsigned char bRequest,
                      unsigned short wValue,
                      unsigned short wIndex,
                      void *data,
                      unsigned short wLength,
                      int timeout = 1000);

  std::string ErrorName(int errcode);

protected:
  void *ctx;
};

#endif // defined(_usb_ll_h__defined_)
