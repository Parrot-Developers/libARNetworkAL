#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_INIT([libARNetworkAL], [0.1.0], [ardrone@ardrone.org])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(m4)

# automake initialisation and check for minimal automake API version 1.9
AM_INIT_AUTOMAKE

# use the C compiler for the following checks
AC_LANG([C])

##########################################################################
# Debug compilation support
##########################################################################
AC_MSG_CHECKING([whether to build with debug information])
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],
    [enable debug data generation (def=no)])],
    [debugit="$enableval"],
    [debugit=no])
AC_MSG_RESULT([$debugit])

if test x"$debugit" = x"yes"; then
    AC_DEFINE([DEBUG],[],[Debug Mode])
    CFLAGS+=" -g -Wall -Werror -Wno-uninitialized -O0"
else
    AC_DEFINE([NDEBUG],[],[No-debug Mode])
    CFLAGS+=" -O3"
fi
AM_CONDITIONAL([DEBUG_MODE], [test "$debugit" = "yes"])

##########################################################################
# Non versionned .so compilation support (for Android)
##########################################################################
AC_MSG_CHECKING([whether to put version number in .so files])
AC_ARG_ENABLE([so-version],
    [AS_HELP_STRING([--disable-so-version],
    [disable version names in .so files [default=enabled]])],
    [sover="$enableval"],
    [sover=yes])
AC_MSG_RESULT([$sover])

##########################################################################
# Define libraries directories
##########################################################################

# get the directories of libARSAL
AC_ARG_WITH([libARSALInstallDir],
            AS_HELP_STRING([--with-libARSALInstallDir=DIR],[directory of the libARSAL installation @<:@default=PREFIX@:>@]),
            [if test x$withval = xno; then
              libARSALInstallDir=""
            else
              libARSALInstallDir="$withval"
            fi], 
            [if test x$prefix = xNONE; then
              libARSALInstallDir=""
            else
              libARSALInstallDir="$prefix"
            fi])

AC_ARG_WITH([libARSALIncludeDir],
            AS_HELP_STRING([--with-libARSALIncludeDir=DIR],[directory of the libARSAL headers @<:@default=ARSALInstallDir/include@:>@]),
            [if test x$withval = xno; then
              [if test ! -z $libARSALInstallDir; then
                 libARSALIncludeDir="$libARSALInstallDir/include"
              else
                 libARSALIncludeDir=""
              fi]
            else
              libARSALIncludeDir="$withval"
            fi],
              [if test ! -z $libARSALInstallDir; then
                 libARSALIncludeDir="$libARSALInstallDir/include"
              else
                 libARSALIncludeDir=""
              fi])

AC_ARG_WITH([libARSALLibDir],
            AS_HELP_STRING([--with-libARSALLibDir=DIR],[directory of the libARSAL libraries @<:@default=ARSALInstallDir/lib@:>@]),
            [if test x$withval = xno; then
              [if test ! -z $libARSALInstallDir; then
                libARSALLibDir="$libARSALInstallDir/lib"
              else
                libARSALLibDir=""
              fi]
            else
              libARSALLibDir="$withval"
            fi],
              [if test ! -z $libARSALInstallDir; then
                libARSALLibDir="$libARSALInstallDir/lib"
              else
                libARSALLibDir=""
              fi])

##########################################################################
# Define includes directories
##########################################################################
# include local headers first
CFLAGS+=" -I$srcdir/../Includes -I$srcdir/../Sources"
# add the headers directory of libSal
if test ! -z $libARSALIncludeDir; then
  CFLAGS+=" -I$libARSALIncludeDir"
fi

CPPFLAGS="$CFLAGS"
##########################################################################
# Define binaries directories
##########################################################################

# add the binaries directory of libSal
if test ! -z $libARSALLibDir; then
  LDFLAGS+=" -L$libARSALLibDir"
fi

##########################################################################
# Check headers
##########################################################################

# Checks for libSAL.h
AC_CHECK_HEADERS([libARSAL/ARSAL_Print.h libARSAL/ARSAL_Thread.h libARSAL/ARSAL_Mutex.h \
libARSAL/ARSAL_Socket.h libARSAL/ARSAL_Time.h],,
	AC_MSG_ERROR(The libARSAL headers are required in order to build the library !
- Use --with-libARSALInstallDir of --with-libARSALIncludeDir to indicate a specific include path.))

AC_LANG_PUSH([Objective C])
OBJCFLAGS+=" -std=gnu99 $CFLAGS"

# Checks for CoreBluetooth/CoreBluetooth.h
AC_CHECK_HEADERS([CoreBluetooth/CoreBluetooth.h], , 
	AC_MSG_WARN(The CoreBluetooth headers are required to use bluetooth features ! ))
AM_CONDITIONAL([HAVE_COREBLUETOOTH], [test x"$ac_cv_header_CoreBluetooth_CoreBluetooth_h" = x"yes"])
AC_LANG_POP([Objective C])

##########################################################################
# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

AC_CHECK_PROGS([DOXYGEN], [doxygen])
m4_include(doxygen.ac)
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([libARNetwork], [doxygen.cfg], [$srcdir/../Doc])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Generate libtool version info number
if test x"$sover" = x"yes"; then
	LDFLAGS+=" -release $PACKAGE_VERSION"
else
    LDFLAGS+=" -avoid-version"
fi

# distribute additional compiler and linker flags
# --> set these variables instead of CFLAGS or LDFLAGS
AC_SUBST([CFLAGS])
AC_SUBST([LDFLAGS])
AC_SUBST([OBCJFLAGS])

# Checks for library functions.


# Generates Makefile
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo "
  $PACKAGE_NAME version $PACKAGE_VERSION
  Prefix.........: $prefix
  Debug Build....: $debugit
  Core bluetooth.: $ac_cv_header_CoreBluetooth_CoreBluetooth_h
  C Compiler.....: $CC $CFLAGS
  ObjC Compiler..: $CC $OBJCFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
  Doxygen........: ${DOXYGEN:-NONE}
"
