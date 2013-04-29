###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################
ACLOCAL_AMFLAGS                                             =   -I m4

# The list of header files that belong to the library (to be installed later)
HEADER_FILES                                                =   ../Includes/libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Manager.h \
                                                                ../Includes/libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Error.h

# The sources to add to the library and to add to the source distribution
SOURCE_FILES                                                =   $(HEADER_FILES)                                 \
                                                                ../Sources/ARNETWORK_OSSPECIFIC_Manager.c       \
                                                                ../Sources/ARNETWORK_OSSPECIFIC_Error.c

# The library names to build (note we are building static and shared libs)
if DEBUG_MODE
lib_LTLIBRARIES                                             =   libarnetwork_osspecific_dbg.la
libarnetwork_osspecific_dbg_ladir                           =   $(includedir)/libARNetwork_OSSpecific
libarnetwork_osspecific_dbg_la_HEADERS                      =   $(HEADER_FILES)
libarnetwork_osspecific_dbg_la_SOURCES                      =   $(SOURCE_FILES)
else
lib_LTLIBRARIES                                             =   libarnetwork_osspecific.la
libarnetwork_osspecific_ladir                               =   $(includedir)/libARNetwork_OSSpecific
libarnetwork_osspecific_la_HEADERS                          =   $(HEADER_FILES)
libarnetwork_osspecific_la_SOURCES                          =   $(SOURCE_FILES)
endif

check_PROGRAMS                                              =   ../TestBench/autoTest/autoTest                      \
                                                                ../TestBench/bidirectionalTest/bidirectionalTest    \
                                                                ../TestBench/defaultTest/defaultTest

___TestBench_autoTest_autoTest_SOURCES                      =   ../TestBench/autoTest/autoTest.c
___TestBench_bidirectionalTest_bidirectionalTest_SOURCES    =   ../TestBench/bidirectionalTest/bidirectionalTest.c
___TestBench_defaultTest_defaultTest_SOURCES                =   ../TestBench/defaultTest/defaultTest.c
if DEBUG_MODE
___TestBench_autoTest_autoTest_LDADD                        =   -larsal_dbg                     \
                                                                libarnetwork_osspecific_dbg.la
___TestBench_bidirectionalTest_bidirectionalTest_LDADD      =   -larsal_dbg                     \
                                                                libarnetwork_osspecific_dbg.la
___TestBench_defaultTest_defaultTest_LDADD                  =   -larsal_dbg                     \
                                                                libarnetwork_osspecific_dbg.la
else
___TestBench_autoTest_autoTest_LDADD                        =   -larsal                     \
                                                                libarnetwork_osspecific.la
___TestBench_bidirectionalTest_bidirectionalTest_LDADD      =   -larsal                     \
                                                                libarnetwork_osspecific.la
___TestBench_defaultTest_defaultTest_LDADD                  =   -larsal                     \
                                                                libarnetwork_osspecific.la
endif

CLEAN_FILES                                                 =   libarnetwork_osspecific.la       \
                                                                libarnetwork_osspecific_dbg.la

EXTRA_DIST                                                  =   bootstrap

# Ensure the distribution of the doxygen configuration file
# Doxygen support
if HAVE_DOXYGEN
include doxygen.am
EXTRA_DIST                                                  +=  doxygen.ac  \
                                                                doxygen.am  \
                                                                $(DX_CONFIG)
endif
