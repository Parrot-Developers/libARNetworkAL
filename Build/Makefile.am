###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################
ACLOCAL_AMFLAGS                                     =   -I m4

# The list of header files that belong to the library (to be installed later)
HEADER_FILES                                        =   ../Includes/libARNetworkAL/ARNETWORKAL_Manager.h    \
                                                        ../Includes/libARNetworkAL/ARNETWORKAL_Frame.h      \
                                                        ../Includes/libARNetworkAL/ARNETWORKAL_Error.h      \
                                                        ../Includes/libARNetworkAL/ARNetworkAL.h

# The sources to add to the library and to add to the source distribution
SOURCE_FILES                                        =   $(HEADER_FILES)                                     \
                                                        ../Sources/Wifi/ARNETWORKAL_WifiNetwork.c           \
                                                        ../Sources/ARNETWORKAL_Manager.c                    \
                                                        ../gen/Sources/ARNETWORKAL_Error.c

if HAVE_COREBLUETOOTH
SOURCE_FILES                                        +=  ../Sources/BLE/ARNETWORKAL_BLENetwork.m             
endif

# The library names to build (note we are building static and shared libs)
if DEBUG_MODE
lib_LTLIBRARIES                                     =   libarnetworkal_dbg.la
libarnetworkal_dbg_ladir                            =   $(includedir)/libARNetworkAL
libarnetworkal_dbg_la_HEADERS                       =   $(HEADER_FILES)
libarnetworkal_dbg_la_SOURCES                       =   $(SOURCE_FILES)
else
lib_LTLIBRARIES                                     =   libarnetworkal.la
libarnetworkal_ladir                                =   $(includedir)/libARNetworkAL
libarnetworkal_la_HEADERS                           =   $(HEADER_FILES)
libarnetworkal_la_SOURCES                           =   $(SOURCE_FILES)
endif

CLEAN_FILES                                         =   libarnetworkal.la       \
                                                        libarnetworkal_dbg.la

EXTRA_DIST                                          =   bootstrap

# Ensure the distribution of the doxygen configuration file
# Doxygen support
if HAVE_DOXYGEN
include doxygen.am
EXTRA_DIST                                          +=  doxygen.ac  \
                                                        doxygen.am  \
                                                        $(DX_CONFIG)
endif
