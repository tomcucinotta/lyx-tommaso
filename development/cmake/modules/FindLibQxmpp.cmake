# FindLibQxmpp
# ---------
#
# Try to find libqxmpp library
#
# Once done this will define
#
# ::
#
#  LIBQXMPP_FOUND
#  LIBQXMPP_INCLUDE_DIR
#

find_path(LIBQXMPP_INCLUDEDIR "qxmpp/QXmppSocks.h")
find_library(LIBQXMPP_LIBRARIES NAMES qxmpp)

# use pkg-config to get the directories and then use these values
# in the find_path() and find_library() calls
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_LIBQXMPP QUIET "qxmpp")
endif()

if(PC_LIBQXMPP_VERSION)
  set(LIBQXMPP_VERSION_STRING "${PC_LIBQXMPP_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibQxmpp
  REQUIRED_VARS LIBQXMPP_LIBRARIES
  LIBQXMPP_INCLUDEDIR LIBQXMPP_VERSION_STRING)

mark_as_advanced(LIBQXMPP_LIBRARIES
  LIBQXMPP_INCLUDEDIR)