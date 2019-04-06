# blusb_gui
GUI Configuration Program for the Blusb Universal Model M Controller

## Dependencies

blusb_gui currently relies on two open-source libraries which should
allow a multiplatform build; currently, only a Windows build using
Microsoft Visual Studio 2008 has been realized.

### wxWidgets

This program uses the wxWidgets library; currently, this is expected in a directory
  `..\..\wxWidgets-3.1.2`
relative to the program directory for the Visual Studio 2008 build.
To change that to your environment's layout, simply do a global search-and-replace
in blusb_gui.vcproj for the above.

The wxWidgets library can be found [here](https://wxwidgets.org/).

### libusb

This program uses the libusb library for USB communication;
currently, this is expected in a directory
  `..\..\libusb`
relative to the program directory for the Visual Studio 2008 build.
To change that to your environment's layout, simply do a global search-and-replace
in blusb_gui.vcproj for the above.

The libusb library can be found [here](https://libusb.info/).
If you are using Linux, chances are your distribution already includes libusb,
so you don't need to create your own build.
