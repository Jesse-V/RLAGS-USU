#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_INIT(GoQat, 1.0.3)
AC_CONFIG_SRCDIR([src/interface.c])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([src/config.h:config.h.in])

# Test for developer mode (i.e. we're compiling in the project directory, not as a software installation).
# Do this test by looking for autogen.sh (defines HAVE___AUTOGEN_SH).
AC_CHECK_FILES([./autogen.sh],[], [])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PATH_PROG(BASH, bash)
AC_PATH_PROG(PERL, perl)

# Checks for libraries.
AH_TEMPLATE([HAVE_LIBSM], [Define to 1 if you have libsm.])
AC_CHECK_LIB([qsiapi], [_ZN10CCCDCamera13get_ConnectedEPb], [], [echo "******** No QSI library - compiling without QSI CCD camera support!"])
AC_CHECK_LIB([parapin], [pin_init_user], [], [echo "******** No parapin library - compiling without parallel port support!"])
AC_CHECK_LIB([xpa], [XPAOpen], [], [echo "******** No xpa library - compiling without DS9 support!"])
AC_CHECK_LIB([grace_np], [GraceOpenVA], [], [echo "******** No grace library - compiling without Grace support!"])

# Checks for modules.  Remember that the PKG_CHECK_MODULES calls initialise the @XXX_CFLAGS@ and @XXX_LIBS@ variables.
# This option for gtk2
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.24)
# This option for gtk3
#PKG_CHECK_MODULES(GTK, gtk+-3.0)
# This option for gtk2
PKG_CHECK_MODULES(LIBGOOCANVAS, goocanvas)
# This option for gtk3
#PKG_CHECK_MODULES(LIBGOOCANVAS, goocanvas-2.0)
PKG_CHECK_MODULES(LIBV4L2, libv4l2,
                         [AC_DEFINE([HAVE_LIBV4L2], [1], [Define to 1 if you have the `libv4l2' library (-lv4l2).])], 
                         [echo "******** No v4l2 library - compiling without V4L support!"])
PKG_CHECK_MODULES(LIBRAW1394, libraw1394 >= 1.1.0, 
                         [AC_DEFINE([HAVE_LIBRAW1394], [1], [Define to 1 if you have the `raw1394' library (-lraw1394).])], 
                         [echo "******** No raw1394 library - compiling without firewire support!"])
PKG_CHECK_MODULES(LIBUSB, libusb-1.0 >= 1.0, 
                         [AC_DEFINE([HAVE_LIBUSB], [1], [Define to 1 if you have the `usb' library (-lusb).])], 
                         [echo "******** No libusb library - compiling without Starlight Xpress camera support!"])
PKG_CHECK_MODULES(LIBUDEV, libudev, 
                         [AC_DEFINE([HAVE_LIBUDEV], [1], [Define to 1 if you have the `udev' library (-ludev).])], 
                         [echo "******** No libudev library - compiling without Starlight Xpress filter wheel support!"])
PKG_CHECK_MODULES(LIBUNICAPGTK, libunicapgtk,
                         [AC_DEFINE([HAVE_LIBUNICAP], [1], [Define to 1 if you have the `unicap' library (-lunicap).])], 
                         [echo "******** No unicapgtk library - compiling without unicap support!"])
PKG_CHECK_MODULES(LIBGTHREAD, gthread-2.0)

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_STRUCT_TM
AC_SYS_LARGEFILE

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset mkdir munmap strcspn])
AC_CHECK_FUNC([_ZN10CCCDCamera16get_ReadoutSpeedERNS_12ReadoutSpeedE], 
                         [AC_DEFINE([HAVE_READOUT_SPEED], [1], [Define to 1 if you have the QSI 'get_ReadoutSpeed' function.])])


AC_CONFIG_FILES([data/DemoScript.pl:data/DemoScript.pl.in])
AC_CONFIG_FILES([data/DemoScript.sh:data/DemoScript.sh.in])
AC_CONFIG_FILES([data/Analyse_HFD.pl:data/Analyse_HFD.pl.in])
AC_CONFIG_FILES([data/GoQat_SExtractor.pl:data/GoQat_SExtractor.pl.in])
AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
