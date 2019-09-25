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

#define USB_ENABLE_VENDOR_RQ    0x11
#define USB_DISABLE_VENDOR_RQ   0x10
#ifdef OLD_VERSION
#define USB_LED_ON              0x21
#define USB_LED_OFF             0x20
#else
#define USB_READ_BR             0x20
#define USB_WRITE_BR            0x21
#endif

#define USB_READ_MATRIX         0x30
#define USB_READ_LAYOUT         0x40
#ifdef OLD_VERSION
#define USB_WRITE_LAYOUT        0x50
#else
#define USB_WRITE_LAYOUT_OLD    0x50
#define USB_WRITE_LAYOUT        0x41
#define USB_READ_DEBOUNCE       0x50
#define USB_WRITE_DEBOUNCE      0x51
#define USB_READ_MACROS         0x60
#define USB_WRITE_MACROS        0x61
#define USB_READ_VERSION        0x70
#endif

// Definitions for V1.5 and above:

// interface numbers application
#define HID_IFACE_NUMBER_KBD                            0                       
#define HID_IFACE_NUMBER_MEDIA_CTRL_FEAT                1                       

// HID report ids for bootloader
#define HID_REPORT_ID_PAGE_DATA                         0x01                            
#define HID_REPORT_ID_EXIT_BOOTLOADER                   0x02                        

// HID report ids for application

// interface 0 - no ids used
#define HID_REPORT_ID_BOOT                              0x00                            
#define HID_REPORT_ID_KEYBOARD                          0x01                            

// interface 1
#define HID_REPORT_ID_MEDIA                             0x02
#define HID_REPORT_ID_SYSCTRL                           0x03
#define HID_REPORT_ID_FEATURE_READ_WRITE_LAYOUT         0x01        
#define HID_REPORT_ID_FEATURE_READ_WRITE_MACROS         0x02        
#define HID_REPORT_ID_FEATURE_READ_MATRIX               0x03                
#define HID_REPORT_ID_FEATURE_READ_WRITE_BR             0x04
#define HID_REPORT_ID_FEATURE_READ_VERSION              0x05                
#define HID_REPORT_ID_FEATURE_READ_WRITE_DEBOUNCE       0x06        
#define HID_REPORT_ID_FEATURE_ENTER_BOOTLOADER          0x07            

#define SPM_PAGESIZE                                    256

// vendor request ids
#define USB_ENTER_BOOTLOADER                            0xFF
#define USB_EXIT_BOOTLOADER                             0xFE

// defined in layout.h; SHOULD be unnecessary in this low-level code, but isn't
#ifndef NUM_MACROKEYS
#define NUM_MACROKEYS  24
#define LEN_MACRO      8
#endif

/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/

enum blusb_request_recipient
  {
  BLUSB_RECIPIENT_DEVICE       = 0x00,
  BLUSB_RECIPIENT_INTERFACE    = 0x01,
  BLUSB_RECIPIENT_ENDPOINT     = 0x02,
  BLUSB_RECIPIENT_OTHER        = 0x03,
  };
enum blusb_endpoint_direction
  {
  BLUSB_ENDPOINT_IN            = 0x80,
  BLUSB_ENDPOINT_OUT           = 0x00,
  };
enum blusb_request_type
  {
  BLUSB_REQUEST_TYPE_STANDARD  = (0x00 << 5),
  BLUSB_REQUEST_TYPE_CLASS     = (0x01 << 5),
  BLUSB_REQUEST_TYPE_VENDOR    = (0x02 << 5),
  BLUSB_REQUEST_TYPE_RESERVED  = (0x03 << 5),
  };
enum blusb_report_type
  {
  BLUSB_REQUEST_GET_REPORT     = 1,
  BLUSB_REQUEST_SET_REPORT     = 9,
  BLUSB_REQUEST_IN_REPORT      = 1 << 8,
  BLUSB_REQUEST_OUT_REPORT     = 2 << 8,
  BLUSB_REQUEST_FEATURE_REPORT = 3 << 8,
  };

/*===========================================================================*/
/* Structure Definitions                                                     */
/*===========================================================================*/

// we need to make sure the members of the struct and union are byte-aligned and packed contiguously
// to be able to cast them over the memory buffer
#pragma pack(push, 1)
union hid_page_data_report_t
{
struct
  {
  wxUint8  id;
  wxUint16 page_address;
  wxUint8  page_data[SPM_PAGESIZE];
  };
wxUint8 buffer[SPM_PAGESIZE + 3];
};

union hid_layout_data_report_t
  {
  struct
    {
    wxUint8 id;
    wxUint8 num_pgs;
    wxUint8 pg_cnt;
    wxUint8 page_data[SPM_PAGESIZE];
    };
  wxUint8 buffer[3 + SPM_PAGESIZE];
  };

union hid_macro_data_report_t
  {
  struct
    {
    wxUint8 id;
    wxUint8 macro_data[NUM_MACROKEYS * LEN_MACRO];
    };
  wxUint8 buffer[1 + NUM_MACROKEYS * LEN_MACRO];
  };

union hid_ctrl_report_t
  {
  struct
    {
    wxUint8 id;
    wxUint8 payload[7];
    };
  wxUint8 buffer[8];
  };
#pragma pack(pop)


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

if (GetFwVersion() >= 0x0105)           /* not available in V1.5 and above   */
  return BLUSB_ERROR_NOT_SUPPORTED;     /* so just ignore the call           */

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

if (GetFwVersion() >= 0x0105)           /* not available in V1.5 and above   */
  return BLUSB_ERROR_NOT_SUPPORTED;     /* so just ignore the call           */

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

int rc = 0;
hid_ctrl_report_t ctrl = {0};
// this one is needed to DETERMINE the version ...
// if (GetFwVersion() >= 0x0105)
// so first, try V1.5++ method
ctrl.id = HID_REPORT_ID_FEATURE_READ_VERSION;
rc = ControlTransfer(handle,
                     BLUSB_RECIPIENT_INTERFACE |
                         BLUSB_ENDPOINT_IN |
                         BLUSB_REQUEST_TYPE_CLASS,
                     BLUSB_REQUEST_GET_REPORT,
                     BLUSB_REQUEST_FEATURE_REPORT |
                         HID_REPORT_ID_FEATURE_READ_VERSION,
                     0,
                     ctrl.buffer, sizeof(ctrl.buffer),
                     1000);
// if that did not return a sufficiently large buffer, try old method
if (rc < 2)
  {
  rc = ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_IN |
                           BLUSB_REQUEST_TYPE_VENDOR,
                       USB_READ_VERSION,
                       0, 0,
                       ctrl.buffer, sizeof(ctrl.buffer),
                       1000);
  }
if (rc >= BLUSB_SUCCESS)
  for (int i = 0; i < rc && i < buflen; i++)
    buffer[i] = ctrl.buffer[i];
#ifdef WIN32
else // obviously a pre-v1.5 keyboard with standard HID driver
  {  // so treat it as v1.4 - everything will fail, however.
  buffer[0] = 0x01;
  buffer[1] = 0x04;
  }
#endif
return rc > buflen ? buflen : rc;
}

/*****************************************************************************/
/* ReadPWM : read PWM values for USB / Bluetooth operation                   */
/*****************************************************************************/

int BlUsbDev::ReadPWM(wxUint8 &pwmUSB, wxUint8 &pwmBT)
{
pwmUSB = pwmBT = 0;
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

int rc = 0;
hid_ctrl_report_t ctrl = {0};
if (GetFwVersion() >= 0x0105)
  {
  // try V1.5++ method
  ctrl.id = HID_REPORT_ID_FEATURE_READ_WRITE_BR;
  rc = ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_IN |
                           BLUSB_REQUEST_TYPE_CLASS,
                       BLUSB_REQUEST_GET_REPORT,
                       BLUSB_REQUEST_FEATURE_REPORT |
                           HID_REPORT_ID_FEATURE_READ_WRITE_BR,
                       0,
                       ctrl.buffer, sizeof(ctrl.buffer),
                       1000);
  }
// if that did not return a sufficiently large buffer, try old method
if (rc < 2)
  {
  rc = ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_IN |
                           BLUSB_REQUEST_TYPE_VENDOR,
                       USB_READ_BR,
                       0, 0,
                       ctrl.buffer, sizeof(ctrl.buffer),
                       1000);
  }
if (rc >= 1)
  pwmUSB = ctrl.buffer[0];
if (rc >= 2)
  pwmBT = ctrl.buffer[1];
return rc;
}

/*****************************************************************************/
/* WritePWM : write PWM values for USB / Bluetooth operation                 */
/*****************************************************************************/

int BlUsbDev::WritePWM(wxUint8 pwmUSB, wxUint8 pwmBT)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

int rc = BLUSB_ERROR_INVALID_PARAM;
if (GetFwVersion() >= 0x0105)
  {
  // first, try V1.5++ method
  hid_ctrl_report_t ctrl = {0};
  ctrl.id = HID_REPORT_ID_FEATURE_READ_WRITE_BR;
  ctrl.payload[0] = pwmUSB;
  ctrl.payload[1] = pwmBT;

  rc = ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_CLASS,
                       BLUSB_REQUEST_SET_REPORT,
                       BLUSB_REQUEST_FEATURE_REPORT |
                           HID_REPORT_ID_FEATURE_READ_WRITE_BR,
                       0,
                       ctrl.buffer, sizeof(ctrl.buffer),
                       1000);
  }
// if that did not return OK, try old method
if (rc < BLUSB_SUCCESS)
  {
  wxUint8 buffer[8] = { pwmUSB, pwmBT, 0 };
  rc = ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_VENDOR,
                       USB_WRITE_BR,
                       0, 0,
                       buffer, sizeof(buffer),
                       1000);
  }
return rc;
}

/*****************************************************************************/
/* ReadMatrix : read current matrix data (only in service mode!)             */
/*****************************************************************************/

int BlUsbDev::ReadMatrix(wxUint8 *buffer, int buflen)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

int rc = 0;
if (GetFwVersion() >= 0x0105)
  {
  // first, try V1.5++ method
  hid_ctrl_report_t ctrl = {0};
  ctrl.id = HID_REPORT_ID_FEATURE_READ_MATRIX;
  rc = ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_IN |
                           BLUSB_REQUEST_TYPE_CLASS,
                       BLUSB_REQUEST_GET_REPORT,
                       BLUSB_REQUEST_FEATURE_REPORT |
                           HID_REPORT_ID_FEATURE_READ_MATRIX,
                       0,
                       ctrl.buffer, sizeof(ctrl.buffer),
                       1000);
  if (rc >= 2)
    memcpy(buffer, ctrl.buffer, min(buflen, rc));
  }
// if that did not return a sufficiently large buffer, try old method
if (rc < 2)
  {
  rc = ControlTransfer(handle,
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
return rc;
}

/*****************************************************************************/
/* ReadLayout : read current keyboard layout (Model M transfer format)       */
/*****************************************************************************/

int BlUsbDev::ReadLayout(wxUint8 *buffer, int buflen)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

int rc = BLUSB_ERROR_INVALID_PARAM;
if (GetFwVersion() >= 0x0105)
  {
  // first, try V1.5++ method
  hid_layout_data_report_t layout = {0};
  layout.id = HID_REPORT_ID_FEATURE_READ_WRITE_LAYOUT;

  int totlen = 0;
  while (rc >= BLUSB_SUCCESS)
    {
    rc = ControlTransfer(handle,
                         BLUSB_RECIPIENT_INTERFACE |
                             BLUSB_ENDPOINT_IN |
                             BLUSB_REQUEST_TYPE_CLASS,
                         BLUSB_REQUEST_GET_REPORT,
                         BLUSB_REQUEST_FEATURE_REPORT |
                             HID_REPORT_ID_FEATURE_READ_WRITE_LAYOUT,
                         0,
                         layout.buffer, sizeof(layout.buffer),
                         1000);
    if (rc >= sizeof(layout.buffer))
      {
      if (layout.num_pgs < 1)
        break;
      if (layout.pg_cnt < 1)
        return BLUSB_ERROR_OTHER;
      int tgtoff = SPM_PAGESIZE*(layout.pg_cnt - 1);
      if (tgtoff + SPM_PAGESIZE <= buflen)
        {
        memcpy(buffer + tgtoff, layout.page_data, SPM_PAGESIZE);
        if (tgtoff + SPM_PAGESIZE > totlen)
            totlen = tgtoff + SPM_PAGESIZE;
        }
      }
    if (rc < 3 || layout.pg_cnt >= layout.num_pgs)
      break;
    }
  rc = totlen;
  }

// if that did not work, try old method
if (rc < 0)
  {
  rc = ControlTransfer(handle,
                         BLUSB_RECIPIENT_ENDPOINT |
                             BLUSB_ENDPOINT_IN |
                             BLUSB_REQUEST_TYPE_VENDOR,
                         USB_READ_LAYOUT,
                         0, 0,
                         buffer, buflen,
                         1000);
  }
return rc;
}

/*****************************************************************************/
/* WriteLayout : write current layout to Model M (Model M transfer format)   */
/*****************************************************************************/

int BlUsbDev::WriteLayout(wxUint8 *buffer, int buflen)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

int rc = BLUSB_ERROR_INVALID_PARAM;
int sent = 0;
if (GetFwVersion() >= 0x0105)
  {
  // first, try V1.5++ method
  hid_layout_data_report_t layout = {0};
  layout.id = HID_REPORT_ID_FEATURE_READ_WRITE_LAYOUT;
  // calculate number of pages necessary
  layout.num_pgs = (buflen + SPM_PAGESIZE - 1) / SPM_PAGESIZE;
  // set current page, starting with 1
  layout.pg_cnt = 1;
  while (1)
    {
    // fill layout buffer and send
    int pgoff = SPM_PAGESIZE*(layout.pg_cnt - 1);
    int pglen = min(SPM_PAGESIZE, buflen - pgoff);
    memcpy(layout.page_data, buffer + pgoff, pglen);
    if (pglen < SPM_PAGESIZE)
      memset(layout.page_data + pglen, 0, SPM_PAGESIZE - pglen);

    rc = ControlTransfer(handle,
                         BLUSB_RECIPIENT_INTERFACE |
                             BLUSB_ENDPOINT_OUT |
                             BLUSB_REQUEST_TYPE_CLASS,
                         BLUSB_REQUEST_SET_REPORT,
                         BLUSB_REQUEST_FEATURE_REPORT |
                             HID_REPORT_ID_FEATURE_READ_WRITE_LAYOUT,
                         0,
                         layout.buffer, sizeof(layout.buffer),
                         1000);
    if (rc < BLUSB_SUCCESS)
      break;
    sent += rc - 3;
    if (layout.pg_cnt == layout.num_pgs)
      {
      rc = sent;
      break;
      }
    layout.pg_cnt++;
    }
  }

if (rc <= 0)
  {
  rc = ControlTransfer(handle,
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
  }
return rc;
}

/*****************************************************************************/
/* ReadDebounce : read current debounce from Model M                         */
/*****************************************************************************/

int BlUsbDev::ReadDebounce()
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

int rc = BLUSB_ERROR_INVALID_PARAM;
hid_ctrl_report_t ctrl = {0};
if (GetFwVersion() >= 0x0105)
  {
  // first, try V1.5++ method
  ctrl.id = HID_REPORT_ID_FEATURE_READ_WRITE_DEBOUNCE;
  rc = ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_IN |
                           BLUSB_REQUEST_TYPE_CLASS,
                       BLUSB_REQUEST_GET_REPORT,
                       BLUSB_REQUEST_FEATURE_REPORT |
                           HID_REPORT_ID_FEATURE_READ_WRITE_DEBOUNCE,
                       0,
                       ctrl.buffer, sizeof(ctrl.buffer),
                       1000);
  }

// if that did not return a sufficiently large buffer, try old method
if (rc < 1)
  {
  rc = ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_IN |
                           BLUSB_REQUEST_TYPE_VENDOR,
                       USB_READ_DEBOUNCE,
                       0, 0,
                       ctrl.buffer, sizeof(ctrl.buffer),
                       1000);
  if (rc >= BLUSB_SUCCESS)
    return ctrl.buffer[GetFwMajorVersion() ? 0 : 4];
  }
else
  return ctrl.buffer[0];
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

int rc = BLUSB_ERROR_INVALID_PARAM;
hid_ctrl_report_t ctrl = {0};
if (GetFwVersion() >= 0x0105)
  {
  // first, try V1.5++ method
  ctrl.id = HID_REPORT_ID_FEATURE_READ_WRITE_DEBOUNCE;
  ctrl.payload[0] = (wxUint8)nDebounce;

  rc = ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_CLASS,
                       BLUSB_REQUEST_SET_REPORT,
                       BLUSB_REQUEST_FEATURE_REPORT |
                           HID_REPORT_ID_FEATURE_READ_WRITE_DEBOUNCE,
                       0,
                       ctrl.buffer, sizeof(ctrl.buffer),
                       1000);
  }
// if that did not return OK, try old method
if (rc < BLUSB_SUCCESS)
  {
  ctrl.buffer[GetFwMajorVersion() ? 0 : 4] = (wxUint8)nDebounce;
  return ControlTransfer(handle,
                         BLUSB_RECIPIENT_INTERFACE |
                             BLUSB_ENDPOINT_OUT |
                             BLUSB_REQUEST_TYPE_VENDOR,
                         USB_WRITE_DEBOUNCE,
                         0, 0,
                         ctrl.buffer, sizeof(ctrl.buffer),
                         1000);
  }
return rc;
}

/*****************************************************************************/
/* ReadMacros : read the macro set                                           */
/*****************************************************************************/

int BlUsbDev::ReadMacros(wxUint8 *buffer, int buflen)
{
if (!IsOpen())
  return BLUSB_ERROR_NO_DEVICE;

int rc = BLUSB_ERROR_INVALID_PARAM;
if (GetFwVersion() >= 0x0105)
  {
  // first, try V1.5++ method
  hid_macro_data_report_t macros = {0};
  macros.id = HID_REPORT_ID_FEATURE_READ_WRITE_MACROS;
  rc = ControlTransfer(handle,
                       BLUSB_RECIPIENT_INTERFACE |
                           BLUSB_ENDPOINT_IN |
                           BLUSB_REQUEST_TYPE_CLASS,
                       BLUSB_REQUEST_GET_REPORT,
                       BLUSB_REQUEST_FEATURE_REPORT |
                           HID_REPORT_ID_FEATURE_READ_WRITE_MACROS,
                       0,
                       macros.buffer, sizeof(macros.buffer),
                       1000);
  if (rc >= 1)
    for (int i = 0; i < rc && i < buflen; i++)
      buffer[i] = macros.buffer[i];
  }
// if that did not return a sufficiently large buffer, try old method
if (rc < 1)
  {
  wxUint8 ibuffer[192] = { 0 };
  rc = ControlTransfer(handle,
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

int rc = BLUSB_ERROR_INVALID_PARAM;
if (GetFwVersion() >= 0x0105)
  {
  // first, try V1.5++ method
  hid_macro_data_report_t macros = {0};
  macros.id = HID_REPORT_ID_FEATURE_READ_WRITE_MACROS;
  memcpy(macros.macro_data, buffer, min(buflen, NUM_MACROKEYS*LEN_MACRO));
  rc = ControlTransfer(handle,
                       BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_CLASS |
                           BLUSB_RECIPIENT_INTERFACE,
                       BLUSB_REQUEST_SET_REPORT,
                       BLUSB_REQUEST_FEATURE_REPORT |
                           HID_REPORT_ID_FEATURE_READ_WRITE_MACROS,
                       0,
                       macros.buffer, sizeof(macros.buffer),
                       1000);
  }
// if that did not return OK, try old method
if (rc < BLUSB_SUCCESS)
  return ControlTransfer(handle,
                         BLUSB_RECIPIENT_ENDPOINT |
                             BLUSB_ENDPOINT_OUT |
                             BLUSB_REQUEST_TYPE_VENDOR,
                         USB_WRITE_MACROS,
                         0, 0,
                         buffer, buflen,
                         1000);
return rc;
}

/*****************************************************************************/
/* EnterBootloader : puts device into boot loader mode                       */
/*****************************************************************************/

int BlUsbDev::EnterBootloader()
{
if (GetFwVersion() >= 0x0105)
  {
  // only works in V1.5++
  hid_ctrl_report_t ctrl = { 0 };
  ctrl.id = HID_REPORT_ID_FEATURE_ENTER_BOOTLOADER;
  return ControlTransfer(handle,
                         BLUSB_ENDPOINT_OUT |
                             BLUSB_REQUEST_TYPE_CLASS |
                             BLUSB_RECIPIENT_INTERFACE,
                         BLUSB_REQUEST_SET_REPORT,
                         BLUSB_REQUEST_FEATURE_REPORT |
                             HID_REPORT_ID_FEATURE_ENTER_BOOTLOADER,
                         0,
                         ctrl.buffer, sizeof(ctrl.buffer),
                         1000);
  }
return BLUSB_ERROR_INVALID_PARAM;
}

/*****************************************************************************/
/* ExitBootloader : puts device back into keyboard mode                      */
/*****************************************************************************/

int BlUsbDev::ExitBootloader()
{
// only works in V1.5++
hid_ctrl_report_t ctrl = { 0 };
ctrl.id = HID_REPORT_ID_EXIT_BOOTLOADER;
return ControlTransfer(handle,
                       BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_CLASS |
                           BLUSB_RECIPIENT_INTERFACE,
                       BLUSB_REQUEST_SET_REPORT,
                       BLUSB_REQUEST_FEATURE_REPORT |
                           HID_REPORT_ID_EXIT_BOOTLOADER,
                       0,
                       ctrl.buffer, sizeof(ctrl.buffer),
                       1000);
}

/*****************************************************************************/
/* UpdateFirmware : writes new firmware out to the device                    */
/*****************************************************************************/

int BlUsbDev::UpdateFirmware
    (
    wxUint16 startAddr,                 /* start address to write to         */
    wxUint16 endAddr,                   /* end address to write to           */
    wxUint8 *buffer                     /* buffer (start = start address)    */
    )
{
// only works in V1.5++
hid_page_data_report_t hid_data = { 0 };
hid_data.id = HID_REPORT_ID_PAGE_DATA;
int rc = BLUSB_SUCCESS;

// Attention: the following assumes that SPM_PAGESIZE is 2^something.
// If this should ever change, the block calculations need to be redone!
const wxUint16 mask = SPM_PAGESIZE - 1;
wxUint16 pgSend = startAddr & mask;
wxUint16 pgEnd = (startAddr + mask) & ~mask;
if (pgSend >= pgEnd)
  return BLUSB_ERROR_INVALID_PARAM;
while (rc >= BLUSB_SUCCESS &&
       pgSend < pgEnd)
  {
  hid_data.page_address = pgSend;
  wxUint16 bufOff = pgSend - startAddr;
  if (pgSend < startAddr)
    {
    memset(hid_data.page_data, 0xff, startAddr - pgSend);
    memcpy(hid_data.page_data + startAddr - pgSend,
           buffer,
           SPM_PAGESIZE - (startAddr - pgSend));
    }
  else if (pgSend + SPM_PAGESIZE > endAddr)
    {
    memcpy(hid_data.page_data,
           buffer + bufOff,
           endAddr + 1 - pgSend);
    memset(hid_data.page_data + endAddr + 1 - pgSend,
           0xff,
           SPM_PAGESIZE - (endAddr + 1 - pgSend));
    }
  else
    memcpy(hid_data.page_data,
           buffer + bufOff,
           SPM_PAGESIZE);
  rc = ControlTransfer(handle,
                       BLUSB_ENDPOINT_OUT |
                           BLUSB_REQUEST_TYPE_CLASS |
                           BLUSB_RECIPIENT_INTERFACE,
                       BLUSB_REQUEST_SET_REPORT,
                       BLUSB_REQUEST_FEATURE_REPORT |
                           HID_REPORT_ID_PAGE_DATA,
                       0,
                       hid_data.buffer, sizeof(hid_data.buffer),
                       1000);
  pgSend += SPM_PAGESIZE;
  }

return rc;
}
