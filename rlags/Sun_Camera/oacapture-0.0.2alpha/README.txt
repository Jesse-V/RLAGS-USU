This is the second alpha release of oaCapture, an astro-imaging capture
application targeted at Linux.  It should be considered fragile and
likely to fail for unexpected reasons.  No warranty is offered and if
your house burns down, wife leaves you, the goldfish dies or your children
start behaving horribly as a result of using this software then it's
your responsibility.

Binary and source releases are available.  The source release doesn't
have a particularly robust build system in place yet, but anyone
moderately competent should manage to get it built I'm sure.  The source
release also has a long way to go in terms of tidying up the code and
unwinding some of the tangled web created as I worked out how to get
the cameras functioning.

To run the binaries You will need to have some packages already installed.
In particular the Qt4 runtime libraries, libv4l and libusb1.  To build the
binaries you'll need more.  See README-BUILD.txt for more details on this.

In Mint/Ubuntu, the following should be sufficient for the binaries:

  $ sudo apt-get install libqtcore4 libqtgui4 libusb-1.0-0 libv4l-0 libv4lconvert0

For Fedora 19, this was sufficient to get me going:

  $ sudo yum install qt qt-x11

If you're running another distribution then I'm afraid you're on your own
for the moment, but if you let me know what needs installing on different
distributions then I'll add it to this file.  Once I get a proper package
put together then these dependencies will all get sorted anyhow

To install the binaries just run install-binaries.sh as root:

  $ sudo ./install-binaries.sh

The binaries, including a bespoke build of ffmpeg will be installed in
/usr/local/openastro and a symlink to oacapture created in /usr/local/bin

The distribution also includes the ZW Optical SDK shared library as there is
no package for this.

A brief run-through of the knobs and switches is in the instructions.txt file.

In theory, cameras supported by V4L2 on Linux should work with this
application.  That means if, say, "cheese" works, so should oacapture.  In
practice some cameras aren't quite as well supported as they might be.

My modded Lifecam Cinema works, but the exposure time is pretty weird.
I'm really not sure what's going on with that.

I also had a quick test with the Xbox camera.  It has some problems with
the standard UVC driver.  If you're prepared to rebuild your own kernel
module for UVC then you may find these helpful:

  http://www.tanstaafl.co.uk/2012/04/kernel-patch-for-xbox-live-camera-on-linux/
  http://www.tanstaafl.co.uk/2012/04/xbox-live-camera-on-linux-part-3/
  http://www.tanstaafl.co.uk/2012/04/xbox-live-camera-on-linux-part-4/

The timeout part of this patch may also help if the Lifecams give timeout
errors.  The path names in the 3.8 kernel sources have changed slightly
since that patch was made.  I shall provide an update with the details at
some point soon.

I have tested with the SPC900.  On my Mint 15 and Mint 14 installs this
works, somewhat to my surprise.  It looks like anything using a kernel
prior to v3.5 may not support the camera properly, but as the installer
for Ubuntu 12.04 and 11.10 shows me the image from the camera when
creating my user account there's hope there.  I'll look into that in more
detail later.

I have found on my laptop that the SPC900 driver whines that insufficient
bandwidth is available if any other device is plugged into the same USB
bus.  I have no workaround for that other than removing all the other USB
devices.

Also tested are the ZWO ASI120MM and ZWOASI120MC.  Other ZWO cameras
may also work, but I don't have them to test.  The ZWO library produces
some debug output giving USB VIDs and PIDs when it starts and data
transfer sizes when the resolution is changed.  It's safe to ignore that
stuff.

The device nodes created by the kernel for the ASI cameras are accessible
only by root.  In the udev directory there's an additional rules file that
should make them work for anyone.  Where exactly it needs to go may
depend on your distribution.  On Mint 15 it goes in /etc/udev/rules.d and
the udev daemon finds it there automagically.  That's where the install
script will put it.  This isn't perhaps the best solution.  If you know
what you're about it would probably be better to create a new group for
people allowed to connect to the cameras, get udev to create the nodes
in that group and make them group-read/writable only.  On older versions
of Ubuntu (prior to 12.04?) the udev configuration doesn't appear to
work but I haven't looked into why as yet.

My intention is to support as many of the QHY cameras as possible (I
have a QHY5 and QHY5L-II I can test with), the Point Grey cameras (I
only have a Firefly MV there) and the Imaging Source cameras (I have a
DMK21.618, DFK21.618 and DFK21.04).  The Imaging Source cameras appear
to work with the standard UVC drivers given a little tweaking, but I
do still need to do a little work there yet.

My eventual aim is to create a unified API for using any of them such
that I (or anyone else who cares to, for that matter) can write
applications to do whatever interests them without having to worry (too
much) about what it is that's actually on the other end of the cable
providing the images.  I think that's a little way off yet though.  The
API is still something of a mess as I find different cameras with
different features and try to work out how to handle them.

Use the software, create images, report bugs where you find them and
have fun.

I do have many of the usual encumbrances of modern life not to mention a
few less usual ones, so if I'm slow responding to emails please excuse me.
I will read them and reply wherever possible.

Above all,
Clear Skies

james@openastroproject.org
