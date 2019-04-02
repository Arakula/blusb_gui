/*****************************************************************************/
/* usb_ll.cpp : libusb overlay class implementation                          */
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
#include "usb_ll.h"

using namespace std;

#ifndef USE_LIBUSB
#define USE_LIBUSB 1
#endif

#if USE_LIBUSB
// libusb can be used on all target platforms, but is LGPLv2.1

#ifdef _MSC_VER
// Don't tell me about the zero-sized arrays in libusb.h <sigh>
#pragma warning(disable: 4200)
#endif

#include "libusb.h"

#ifdef _MSC_VER
#pragma warning(default: 4200)
#endif

#else // if !(USE_LIBUSB)

// This is a highly OS-dependent area...

#ifdef WIN32

extern "C" {
// Windows - rely on Setup API and DDK
// ATTENTION: Windows DDK 7600.16385 needs at least Platform SDK v7.0 to work!
#include <setupapi.h>
#include <hidsdi.h>
#include <hidclass.h>
}
#endif

#endif

#if USE_LIBUSB

typedef libusb_context UsbLLContext;

#else

/*===========================================================================*/
/* Context class - highly OS-specific, so opaque to the outside              */
/*===========================================================================*/

#ifdef WIN32

class UsbLLContext
{
public:
    UsbLLContext();
    ~UsbLLContext();
    void AddHandle(HANDLE h)
      { hDev.push_back(h); }
    void RemoveHandle(HANDLE h)
      {
      for (size_t i = 0; i < hDev.size(); i++)
        if (h == hDev[i])
          {
          hDev.erase(hDev.begin() + i);
          i--;
          }
      }
    // no need for privacy in this internal class, just keep it all public
    GUID hidGuid;   // HID class GUID
    vector<HANDLE> hDev;    // handle for opened device
    HANDLE hEvRd, hEvWr;  // read/write event handles for overlapped I/O
    CRITICAL_SECTION csRd, csWr;
};

/*****************************************************************************/
/* UsbLLContext : constructor                                                */
/*****************************************************************************/

UsbLLContext::UsbLLContext()
{
// we are only interested in HID GUIDs in here
HidD_GetHidGuid(&hidGuid);
hEvRd = hEvWr = INVALID_HANDLE_VALUE;
InitializeCriticalSection(&csRd);
InitializeCriticalSection(&csWr);
}

/*****************************************************************************/
/* ~UsbLLContext : destructor                                                */
/*****************************************************************************/

UsbLLContext::~UsbLLContext()
{
if (hEvWr != INVALID_HANDLE_VALUE)
  CloseHandle(hEvWr);
if (hEvRd != INVALID_HANDLE_VALUE)
  CloseHandle(hEvRd);
for (size_t i = 0; i < hDev.size(); i++)
  CloseHandle(hDev[i]);
DeleteCriticalSection(&csWr);
DeleteCriticalSection(&csRd);
}

#else
// other OS ...
#endif

#endif


/*===========================================================================*/
/* UsbLL class members                                                       */
/*===========================================================================*/

/*****************************************************************************/
/* Initialize : initialize the object                                        */
/*****************************************************************************/

bool UsbLL::Initialize()
{
#if USE_LIBUSB

if (!IsInitialized())
  return (libusb_init((UsbLLContext **)&ctx) == LIBUSB_SUCCESS);
return true;

#else

if (!IsInitialized())
  {
  ctx = new UsbLLContext;
  return !!ctx;
  }
return true;

#endif
}

/*****************************************************************************/
/* Terminate : uninitialize the object                                       */
/*****************************************************************************/

void UsbLL::Terminate()
{
#if USE_LIBUSB

if (IsInitialized())
  libusb_exit((UsbLLContext *)ctx);

#else

// TODO - OS-specific things

if (IsInitialized())
  {
  delete (UsbLLContext *)ctx;
  ctx = NULL;
  }

#endif
}

/*****************************************************************************/
/* GetDeviceList : fetches list of USB devices                               */
/*****************************************************************************/

int UsbLL::GetDeviceList
    (
    vector<void *>& devList
    )
{
devList.clear();
if (!IsInitialized())
  return 0;

#if USE_LIBUSB

libusb_device **dev_list = NULL;
ssize_t cnt = libusb_get_device_list((libusb_context *)ctx, &dev_list);
for (ssize_t i = 0; i < cnt; i++)
  devList.push_back(dev_list[i]);
// append the list itself as last item
devList.push_back(dev_list);
return cnt;

#else

UsbLLContext *context = (UsbLLContext *)ctx;

#ifdef WIN32

HDEVINFO dis = SetupDiGetClassDevs(&context->hidGuid,
                                  NULL, NULL,
                                  DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
if (dis == INVALID_HANDLE_VALUE)
  return 0;
for (size_t i = 0; ; i++)
  {
  SP_DEVICE_INTERFACE_DATA did = { sizeof(SP_DEVICE_INTERFACE_DATA) };
  if (!SetupDiEnumDeviceInterfaces(dis,
                                   NULL, // No care about specific PDOs
                                   &context->hidGuid,
                                   i,
                                   &did))
    {
    SetupDiDestroyDeviceInfoList(dis);
    break;
    }

  DWORD dwReqSize = 0;
  SetupDiGetInterfaceDeviceDetail(dis, &did, NULL, 0, &dwReqSize, NULL);
  if (!dwReqSize)
    continue;
  SP_DEVICE_INTERFACE_DETAIL_DATA *details =
      (SP_DEVICE_INTERFACE_DETAIL_DATA *)new unsigned char[dwReqSize];
  if (!details)
    continue;
  details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
  ZeroMemory(details->DevicePath, sizeof(details->DevicePath));
  if (!SetupDiGetDeviceInterfaceDetail(dis, &did,
                                       details, dwReqSize,
                                       &dwReqSize, NULL))
    {
    delete[] (unsigned char *)details;
    continue;
    }
  // in Windows, remember the Device Interface Details as "device"
  devList.push_back(details);
  }
SetupDiDestroyDeviceInfoList(dis);
return devList.size();

#else

// TODO
return 0;
#endif

#endif
}

/*****************************************************************************/
/* FreeDeviceList : frees an allocated device list                           */
/*****************************************************************************/

void UsbLL::FreeDeviceList(vector<void *>& devList)
{
#if USE_LIBUSB

if (devList.size())
  {
  libusb_device ** dev_list = (libusb_device **)devList[devList.size() - 1];
  // this blindly assumes that the last entry is the device list itself
  libusb_free_device_list(dev_list, true);
  devList.clear();
  }

#else

#ifdef WIN32

for (size_t i = 0; i < devList.size(); i++)
  delete[] (unsigned char *)devList[i];
devList.clear();

#else
// TODO
#endif

#endif
}

/*****************************************************************************/
/* GetDeviceDescriptor : retrieve a device descriptor for a device           */
/*****************************************************************************/

int UsbLL::GetDeviceDescriptor(void *dev, UsbLLDevDesc &desc)
{
#if USE_LIBUSB

libusb_device_descriptor dev_descr;
int rc = libusb_get_device_descriptor((libusb_device *)dev,
                                      &dev_descr);
if (rc >= 0)
  {
  desc.VendorID = dev_descr.idVendor;
  desc.ProductID = dev_descr.idProduct;
  // VersionNumber, UsageID, Usage?
  // What's the deal with bNumConfigurations?
  desc.VendorName = "";
  desc.ProductName = "";
  desc.SerialNumber = "";
  libusb_device_handle *handle = NULL;
  int rc = libusb_open((libusb_device *)dev, &handle);
  if (rc == LIBUSB_SUCCESS)
    {
    char szBuf[256];
    if (dev_descr.iManufacturer != 0xff &&
        libusb_get_string_descriptor_ascii(handle,
                                           dev_descr.iManufacturer,
                                           (unsigned char *)szBuf,
                                           sizeof(szBuf)) >= LIBUSB_SUCCESS)
      desc.VendorName = szBuf;
    if (dev_descr.iProduct != 0xff &&
        libusb_get_string_descriptor_ascii(handle,
                                           dev_descr.iProduct,
                                           (unsigned char *)szBuf,
                                           sizeof(szBuf)) >= LIBUSB_SUCCESS)
      desc.ProductName = szBuf;
    if (dev_descr.iSerialNumber != 0xff &&
        libusb_get_string_descriptor_ascii(handle,
                                           dev_descr.iSerialNumber,
                                           (unsigned char *)szBuf,
                                           sizeof(szBuf)) >= LIBUSB_SUCCESS)
      desc.ProductName = szBuf;
    libusb_close(handle);
    }
  }
return rc;

#else

#ifdef WIN32

SP_DEVICE_INTERFACE_DETAIL_DATA *details = 
    (SP_DEVICE_INTERFACE_DETAIL_DATA *)dev;

// Open device with just generic query abilities to begin with
HANDLE hTemp = CreateFile(details->DevicePath,
                          GENERIC_READ | GENERIC_WRITE, // but accessible it has to be!
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,    // no SECURITY_ATTRIBUTES structure
                          OPEN_EXISTING, // No special create flags
                          0,       // Open device as non-overlapped so we can get data
                          NULL);   // No template file
if (hTemp == INVALID_HANDLE_VALUE)
  return -1;  // TODO: organize that

int rc = 0;
PHIDP_PREPARSED_DATA PreparsedData = NULL;
if (!HidD_GetPreparsedData (hTemp, &PreparsedData)) 
  rc = -1;  // TODO : organize that

HIDD_ATTRIBUTES attr = { sizeof(HIDD_ATTRIBUTES) };
if (rc == 0 && !HidD_GetAttributes(hTemp, &attr))  // fetch attributes
  rc = -1;
// caps.Usage and caps.UsagePage might be interesting
HIDP_CAPS caps = { 0 };
if (rc == 0 && !HidP_GetCaps(PreparsedData, &caps))  // fetch top-level collection capabilities
  rc = -1;

desc.VendorName = "";
desc.ProductName = "";
desc.SerialNumber = "";
wchar_t buffer[256];
char name[256];
if (rc == 0 &&
    !HidD_GetManufacturerString(hTemp, buffer, sizeof(buffer)))
  rc = -1;
else
  {
  WideCharToMultiByte(CP_ACP, 0,
                      buffer, -1,
                      name, sizeof(name),
                      NULL, NULL);
  desc.VendorName = name;
  }
if (rc == 0 &&
    !HidD_GetProductString(hTemp, buffer, sizeof(buffer)))
  rc = -1;
else
  {
  WideCharToMultiByte(CP_ACP, 0,
                      buffer, -1,
                      name, sizeof(name),
                      NULL, NULL);
  desc.ProductName = name;
  }
if (rc == 0 &&
    !HidD_GetSerialNumberString(hTemp, buffer, sizeof(buffer)))
  rc = -1;
else
  {
  WideCharToMultiByte(CP_ACP, 0,
                      buffer, -1,
                      name, sizeof(name),
                      NULL, NULL);
  desc.SerialNumber = name;
  }

HidD_FreePreparsedData(PreparsedData);

CloseHandle(hTemp);
if (!rc)
  {
  // devDesc->bDeviceClass = ??;
  // devDesc->bDeviceSubClass = ??;
  // devDesc->bDeviceProtocol = ??;
  desc.VendorID = attr.VendorID;
  desc.ProductID = attr.ProductID;
  desc.VersionNumber = attr.VersionNumber;
  desc.UsageID = caps.Usage;
  desc.UsagePage = caps.UsagePage;
  }
return rc;

#else
// TODO
return -1;
#endif

#endif
}

/*****************************************************************************/
/* Open : opens a device                                                     */
/*****************************************************************************/

int UsbLL::Open(void *dev, void *&handle)
{
#if USE_LIBUSB

return libusb_open((libusb_device *)dev,
                   (libusb_device_handle **)&handle);

#else

#ifdef WIN32

UsbLLContext *context = (UsbLLContext *)ctx;

SP_DEVICE_INTERFACE_DETAIL_DATA *details = 
    (SP_DEVICE_INTERFACE_DETAIL_DATA *)dev;

#if 0
// single device
if (context->hDev != INVALID_HANDLE_VALUE)
  CloseHandle(context->hDev);
#endif

HANDLE hDev = CreateFile(details->DevicePath,
                         GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED,
                         NULL);
if (hDev == INVALID_HANDLE_VALUE)
  {
  handle = NULL;
  return -1;
  }
else
  {
  context->AddHandle(hDev);
  handle = hDev;
  // now that the file is open, allocate events for overlapped read/write.
  if (context->hEvRd == INVALID_HANDLE_VALUE)
    context->hEvRd = CreateEvent(NULL, TRUE, TRUE, NULL);
  if (context->hEvWr == INVALID_HANDLE_VALUE)
    context->hEvWr = CreateEvent(NULL, TRUE, TRUE, NULL);
  return 0;
  }

#else
// TODO
return -1;
#endif

#endif
}

/*****************************************************************************/
/* Close : closes a device                                                   */
/*****************************************************************************/

void UsbLL::Close(void *handle)
{
#if USE_LIBUSB

libusb_close((libusb_device_handle *)handle);

#else

#ifdef WIN32

UsbLLContext *context = (UsbLLContext *)ctx;

CloseHandle((HANDLE)handle);
#if 0
// single handle per UsbLL object
if (context->hDev == (HANDLE)handle)
  context->hDev = INVALID_HANDLE_VALUE;
#else
// multiple handles per UsbLL object
context->RemoveHandle((HANDLE)handle);
#endif

#else
// TODO
#endif

#endif
}

/*****************************************************************************/
/* Send : sends a number of bytes to the device                              */
/*****************************************************************************/

int UsbLL::Send(void *dev_handle, void *buf, int len, int timeout)
{
#if USE_LIBUSB
// not used; ControlTransfer does the trick
return -1;

#elif defined(WIN32)

UsbLLContext *context = (UsbLLContext *)ctx;
// Windows places upper limits on the control transfer size
// See: https://msdn.microsoft.com/en-us/library/windows/hardware/ff538112.aspx
unsigned char tmpbuf[4096];

if (dev_handle == INVALID_HANDLE_VALUE ||
    len >= (sizeof(tmpbuf) - 1))
  return -1;

EnterCriticalSection(&context->csWr);
ResetEvent(&context->hEvWr);
OVERLAPPED ov = {0};
ov.hEvent = context->hEvWr;

tmpbuf[0] = 0;
memcpy(tmpbuf + 1, buf, len);
BOOL bOK = WriteFile(dev_handle, tmpbuf, len + 1, NULL, &ov);
if (!bOK)
  {
  if (GetLastError() == ERROR_IO_PENDING)
    {
    DWORD wr = WaitForSingleObject(context->hEvWr, timeout);
    if (wr == WAIT_TIMEOUT)
      CancelIo(dev_handle);
    if (wr == WAIT_OBJECT_0)
      bOK = TRUE;
    }
  }
DWORD n = 0;
if (bOK)
  bOK = GetOverlappedResult(dev_handle, &ov, &n, FALSE);
LeaveCriticalSection(&context->csWr);
if ((int)n > 0)
  return (int)n;
return -1;

#else
// TODO
return -1;
#endif
}

/*****************************************************************************/
/* Receive : receives a buffer from the device                               */
/*****************************************************************************/

int UsbLL::Receive(void *dev_handle, void *buf, int len, int timeout)
{
#if USE_LIBUSB
// not used; ControlTransfer does the trick
return -1;

#elif defined(WIN32)

UsbLLContext *context = (UsbLLContext *)ctx;
// Windows places upper limits on the control transfer size
// See: https://msdn.microsoft.com/en-us/library/windows/hardware/ff538112.aspx
unsigned char tmpbuf[4096];

if (dev_handle == INVALID_HANDLE_VALUE ||
    len >= (sizeof(tmpbuf) - 1))
  return -1;

EnterCriticalSection(&context->csRd);
ResetEvent(&context->hEvRd);
OVERLAPPED ov = {0};
ov.hEvent = context->hEvRd;
BOOL bOK = ReadFile(dev_handle, tmpbuf, len + 1, NULL, &ov);
if (!bOK)
  {
  if (GetLastError() == ERROR_IO_PENDING)
    {
    DWORD wr = WaitForSingleObject(context->hEvRd, timeout);
    if (wr == WAIT_TIMEOUT)
      CancelIo(dev_handle);
    if (wr == WAIT_OBJECT_0)
      bOK = TRUE;
    }
  }
DWORD n = 0;
if (bOK)
  bOK = GetOverlappedResult(dev_handle, &ov, &n, FALSE);
LeaveCriticalSection(&context->csRd);
if ((int)n > len)
  n = len;
if ((int)n > 0)
  {
  memcpy(buf, tmpbuf + 1, n);
  return (int)n;
  }
return -1;

#else
// TODO
return -1;
#endif
}

/*****************************************************************************/
/* ControlTransfer : does a control transfer with a device                   */
/*****************************************************************************/

int UsbLL::ControlTransfer
    (
    void *dev_handle,
    unsigned char request_type,
    unsigned char bRequest,
    unsigned short wValue,
    unsigned short wIndex,
    void *data,
    unsigned short wLength,
    int timeout
    )
{
#if USE_LIBUSB

return libusb_control_transfer((libusb_device_handle *)dev_handle,
                               request_type,
                               bRequest,
                               wValue,
                               wIndex,
                               (unsigned char *)data,
                               wLength,
                               (unsigned int)timeout);

#else

// TODO
return -1;

#endif
}

/*****************************************************************************/
/* ErrorName : retrieve symbolic name for an error code                      */
/*****************************************************************************/

std::string UsbLL::ErrorName(int errcode)
{
#if USE_LIBUSB

return libusb_error_name(errcode);

#else

// TODO ... or not ...
return "";

#endif
}
