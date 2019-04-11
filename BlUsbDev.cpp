/*****************************************************************************/
/* BlUsbDev.cpp : implementation of the USB device communication classes     */
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

#include "BlUsbDev.h"

using namespace std;

// For original firmware,
// #define OLD_VERSION

/*****************************************************************************/
/* Definitions                                                               */
/*****************************************************************************/

#define USB_ENABLE_VENDOR_RQ	0x11
#define USB_DISABLE_VENDOR_RQ	0x10
#ifdef OLD_VERSION
#define USB_LED_ON				0x21
#define USB_LED_OFF				0x20
#else
#define USB_READ_BR				0x20
#define USB_WRITE_BR			0x21
#endif

#define USB_READ_MATRIX			0x30
#define USB_READ_LAYOUT			0x40
#ifdef OLD_VERSION
#define USB_WRITE_LAYOUT		0x50
#else
#define USB_WRITE_LAYOUT_OLD	0x50
#define USB_WRITE_LAYOUT		0x41
#define USB_READ_DEBOUNCE		0x50
#define USB_WRITE_DEBOUNCE		0x51
#define USB_READ_MACROS			0x60
#define USB_WRITE_MACROS		0x61
#define USB_READ_VERSION		0x70
#endif

/*****************************************************************************/
/* Definitions                                                               */
/*****************************************************************************/

enum blusb_request_recipient
  {
  BLUSB_RECIPIENT_DEVICE = 0x00,
  BLUSB_RECIPIENT_INTERFACE = 0x01,
  BLUSB_RECIPIENT_ENDPOINT = 0x02,
  BLUSB_RECIPIENT_OTHER = 0x03,
  };
enum blusb_endpoint_direction
  {
  BLUSB_ENDPOINT_IN = 0x80,
  BLUSB_ENDPOINT_OUT = 0x00,
  };
enum blusb_request_type
  {
  BLUSB_REQUEST_TYPE_STANDARD = (0x00 << 5),
  BLUSB_REQUEST_TYPE_CLASS = (0x01 << 5),
  BLUSB_REQUEST_TYPE_VENDOR = (0x02 << 5),
  BLUSB_REQUEST_TYPE_RESERVED = (0x03 << 5),
  };

/*===========================================================================*/
/* BlUsbDev : USB device communication class members                         */
/*===========================================================================*/

/*****************************************************************************/
/* BlUsbDev : constructor                                                    */
/*****************************************************************************/

BlUsbDev::BlUsbDev(void)
{
handle = NULL;
blVer[0] = blVer[1] = 0x00;
}

/*****************************************************************************/
/* ~BlUsbDev : destructor                                                    */
/*****************************************************************************/

BlUsbDev::~BlUsbDev(void)
{
Close();
}

/*****************************************************************************/
/* Open : opens the first attached Model M                                   */
/*****************************************************************************/

int BlUsbDev::Open
    (
    wxUint16 vendor,
    wxUint16 product,
    wxUint16 Usage,
    wxUint16 UsagePage
    )
{
if (IsOpen())
  return BLUSB_SUCCESS;

vector<void*> devList;
UsbLLDevDesc desc;
handle = NULL;
int rc = BLUSB_ERROR_NOT_FOUND;
int cnt = GetDeviceList(devList);
for (int i = 0; i < cnt; i++)
  {
  if (GetDeviceDescriptor(devList[i], desc) == BLUSB_SUCCESS &&
      vendor == desc.VendorID &&
      product == desc.ProductID &&
      Usage == desc.UsageID &&
      UsagePage == desc.UsagePage)
    {
    rc = UsbLL::Open(devList[i], handle);
    if (rc == BLUSB_SUCCESS)
      ReadVersion(blVer, sizeof(blVer));
    break;
    }
  }
FreeDeviceList(devList);
return rc;
}

/*****************************************************************************/
/* EnableServiceMode : put Model M into service mode                         */
/*****************************************************************************/

int BlUsbDev::EnableServiceMode()
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;
return ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_VENDOR,
                       USB_ENABLE_VENDOR_RQ,
                       0, 0, 0, 0, 1000);
}

/*****************************************************************************/
/* DisableServiceMode : put Model M into normal operation mode               */
/*****************************************************************************/

int BlUsbDev::DisableServiceMode()
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;
return ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_VENDOR,
                       USB_DISABLE_VENDOR_RQ,
                       0, 0, 0, 0, 1000);
}

/*****************************************************************************/
/* ReadVersion : read firmware version                                       */
/*****************************************************************************/

int BlUsbDev::ReadVersion(wxUint8 *buffer, int buflen)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;
wxUint8 ibuffer[8] = { 0 };
int rc = ControlTransfer(handle,
                         BLUSB_RECIPIENT_INTERFACE |
                             BLUSB_ENDPOINT_IN |
                             BLUSB_REQUEST_TYPE_VENDOR,
                         USB_READ_VERSION,
                         0, 0,
                         ibuffer, sizeof(ibuffer),
                         1000);
if (rc >= BLUSB_SUCCESS)
  for (int i = 0; i < rc && i < buflen; i++)
    buffer[i] = ibuffer[i];
return rc > buflen ? buflen : rc;
}

/*****************************************************************************/
/* ReadPWM : read PWM values for USB / Bluetooth operation                   */
/*****************************************************************************/

int BlUsbDev::ReadPWM(wxUint8 &pwmUSB, wxUint8 &pwmBT)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;
wxUint8 ibuffer[8] = { 0 };
int rc = ControlTransfer(handle,
                         BLUSB_RECIPIENT_INTERFACE |
                             BLUSB_ENDPOINT_IN |
                             BLUSB_REQUEST_TYPE_VENDOR,
                         USB_READ_BR,
                         0, 0,
                         ibuffer, sizeof(ibuffer),
                         1000);
pwmUSB = pwmBT = 0;
if (rc >= 1)
  pwmUSB = ibuffer[0];
if (rc >= 2)
  pwmBT = ibuffer[1];
return rc;
}

/*****************************************************************************/
/* WritePWM : write PWM values for USB / Bluetooth operation                 */
/*****************************************************************************/

int BlUsbDev::WritePWM(wxUint8 pwmUSB, wxUint8 pwmBT)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;
wxUint8 buffer[8] = { pwmUSB, pwmBT, 0 };
return ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_VENDOR,
                       USB_WRITE_BR,
                       0, 0,
                       buffer, sizeof(buffer),
                       1000);
}


/*****************************************************************************/
/* ReadMatrix : read current matrix data (only in service mode!)             */
/*****************************************************************************/

int BlUsbDev::ReadMatrix(wxUint8 *buffer, int buflen)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;
return ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_IN |
                           BLUSB_REQUEST_TYPE_VENDOR,
                       USB_READ_MATRIX,
#ifdef OLD_VERSION
                       0, 1,
#else
                       0, 0,
#endif
                       buffer, buflen,
                       1000);
}

/*****************************************************************************/
/* ReadLayout : read current keyboard layout (Model M transfer format)       */
/*****************************************************************************/

int BlUsbDev::ReadLayout(wxUint8 *buffer, int buflen)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;
int rc = ControlTransfer(handle,
                         BLUSB_RECIPIENT_ENDPOINT |
                             BLUSB_ENDPOINT_IN |
                             BLUSB_REQUEST_TYPE_VENDOR,
                         USB_READ_LAYOUT,
                         0, 0,
                         buffer, buflen,
                         1000);
return rc;
}

/*****************************************************************************/
/* WriteLayout : write current layout to Model M (Model M transfer format)   */
/*****************************************************************************/

int BlUsbDev::WriteLayout(wxUint8 *buffer, int buflen)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;
int rc = ControlTransfer(handle,
                         BLUSB_RECIPIENT_ENDPOINT |
                             BLUSB_ENDPOINT_OUT |
                             BLUSB_REQUEST_TYPE_VENDOR,
                         USB_WRITE_LAYOUT,
                         0, 0,
                         buffer, buflen,
                         1000);
#ifdef USB_WRITE_LAYOUT_OLD
if (rc < BLUSB_SUCCESS)
  rc = ControlTransfer(handle,
                         BLUSB_RECIPIENT_ENDPOINT |
                             BLUSB_ENDPOINT_OUT |
                             BLUSB_REQUEST_TYPE_VENDOR,
                         USB_WRITE_LAYOUT_OLD,
                         0, 0,
                         buffer, buflen,
                         1000);  
#endif
return rc;
}

/*****************************************************************************/
/* ReadDebounce : read current debounce from Model M                         */
/*****************************************************************************/

int BlUsbDev::ReadDebounce()
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;
wxUint8 buffer[8] = { 0 };
int rc = ControlTransfer(handle,
                         BLUSB_RECIPIENT_INTERFACE |
                             BLUSB_ENDPOINT_IN |
                             BLUSB_REQUEST_TYPE_VENDOR,
                         USB_READ_DEBOUNCE,
                         0, 0,
                         buffer, sizeof(buffer),
                         1000);
if (rc >= BLUSB_SUCCESS)
  return buffer[GetFwMajorVersion() ? 0 : 4];
return rc;
}

/*****************************************************************************/
/* WriteDebounce : write debounce to Model M                                 */
/*****************************************************************************/

int BlUsbDev::WriteDebounce(int nDebounce)
{
if (nDebounce < 1 || nDebounce > 255)
  return BLUSB_ERROR_INVALID_PARAM;
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

wxUint8 buffer[8] = { 0 };
buffer[GetFwMajorVersion() ? 0 : 4] = (wxUint8)nDebounce;
return ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_VENDOR,
                       USB_WRITE_DEBOUNCE,
                       0, 0,
                       buffer, sizeof(buffer),
                       1000);
}

/*****************************************************************************/
/* ReadMacros : read the macro set                                           */
/*****************************************************************************/

int BlUsbDev::ReadMacros(wxUint8 *buffer, int buflen)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

wxUint8 ibuffer[192] = { 0 };
int rc = ControlTransfer(handle,
                         BLUSB_RECIPIENT_ENDPOINT |
                             BLUSB_ENDPOINT_IN |
                             BLUSB_REQUEST_TYPE_VENDOR,
                         USB_READ_MACROS,
                         0, 0,
                         ibuffer, sizeof(ibuffer),
                         1000);
if (rc >= BLUSB_SUCCESS)
  {
  for (int i = 0; i < rc && i < buflen; i++)
    buffer[i] = ibuffer[i];
  }
return rc > buflen ? buflen : rc;
}

/*****************************************************************************/
/* WriteMacros : write the macro set                                         */
/*****************************************************************************/

int BlUsbDev::WriteMacros(wxUint8 *buffer, int buflen)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

return ControlTransfer(handle,
                       BLUSB_RECIPIENT_ENDPOINT |
                           BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_VENDOR,
                       USB_WRITE_MACROS,
                       0, 0,
                       buffer, buflen,
                       1000);
}