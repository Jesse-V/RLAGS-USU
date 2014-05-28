To build the binaries a few prerequisites are required.

The binaries link against the ZWO ASI camera and ffmpeg libraries as
well as libv4l, Qt4 and libusb1.  The last three should be fairly easy
to find the development packages for in most distributions.  On Ubuntu,
Mint (and perhaps Debian) you probably need:

  libv4l-dev
  libqt4-dev
  libqt4-dev-bin
  qt4-dev-tools
  qt4-qmake
  libusb-1.0-0-dev

The build requires both gcc and g++.

The binaries link against ffmpeg-1.2.3 and ffmpeg-1.2.4.  Others may work,
but having a separate library just for this project can't do any harm.
The ASI SDK library is available from their website.

There are two scripts, bin/BUILD-PREREQ.sh and bin/INSTALL-PREREQ.sh (both
should be run from the top level directory) that will fetch and build
the ffmpeg binaries and fetch the ASI binaries, then install them both.

yasm is required for the ffmpeg build amongst other things.  Tweaking
may be necessary to get all the prerequisites installed.
