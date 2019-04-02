The source code contains experimental support for direct Windows calls to talk to the USB device.
Doesn't work AT ALL at the moment, so it's commented out.

If you want to play with it, QUITE some things need to be done.
I compiled with the Windows 7 DDK 7600, which still includes XP support.
We don't need anything beyond that.

To activate this by setting
  #define USE_LIBUSB 0
in usb_ll.cpp, the following changes need to be done to the project:

.) add
     $(WINDDK_BASE)\7600.16385.1\Lib\wxp\i386
   to the additional library paths for the linker.
   Or whatever your DDK's path might be.
   WINDDK_BASE is supposed to be a global environment variable containing the base path for the DDK.

.) add the libraries
     hid.lib hidclass.lib
   to the additional dependencies for the linker.

.) add
     $(WINDDK_BASE)\7600.16385.1\inc\api;$(WINDDK_BASE)\7600.16385.1\inc\ddk
   to the include directories for the file usb_ll.cpp.
   That doesn't need to be a global setting; adding it for this single source file is enough.