# FindMySQLConnectorCPP.cmake

# Try to find MySQL Connector/C++

# This module defines
# MySQLConnectorCPP_FOUND, if false, do not try to use MySQL Connector
# MySQLConnectorCPP_INCLUDE_DIR, where to find mysql_connection.h, etc.
# MySQLConnectorCPP_LIBRARIES, the libraries to link against to use MySQL Connector

# Check for Homebrew and set hints accordingly
find_program(HOMEBREW_EXECUTABLE brew)

if (HOMEBREW_EXECUTABLE)
  message(STATUS "Homebrew found at ${HOMEBREW_EXECUTABLE} - using it to find MySQL")
  execute_process(
          COMMAND ${HOMEBREW_EXECUTABLE} --prefix
          OUTPUT_VARIABLE HOMEBREW_PREFIX
          OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  message(STATUS "Homebrew prefix is ${HOMEBREW_PREFIX}")
  # Homebrew MySQL include and lib directories
  set(HOMEBREW_MYSQL_INCLUDE_DIR "${HOMEBREW_PREFIX}/include/mysqlx")
  set(HOMEBREW_MYSQL_LIB_DIR     "${HOMEBREW_PREFIX}/lib")
  set(HOMEBREW_MYSQL_BIN_DIR     "${HOMEBREW_PREFIX}/bin")
else ()
  set(HOMEBREW_MYSQL_INCLUDE_DIR "")
  set(HOMEBREW_MYSQL_LIB_DIR "")
endif ()

find_library(MySQLConnectorCPP_LIBRARY
        NAMES mysqlcppconn8.2.8.3.0
        PATHS /usr/local/lib /usr/lib
        DOC "MySQL Connector/C++ library"
)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set the MySQLConnectorCPP_FOUND variable
find_package_handle_standard_args(MySQLConnectorCPP DEFAULT_MSG MySQLConnectorCPP_LIBRARY)

if(MySQLConnectorCPP_FOUND)
  set(MySQLConnectorCPP_LIBRARIES ${MySQLConnectorCPP_LIBRARY})
else()
  set(MySQLConnectorCPP_LIBRARIES)
endif()

set(MySQLConnectorCPP_INCLUDE_DIRS "${HOMEBREW_PREFIX}/include/mysqlx")
mark_as_advanced(MySQLConnectorCPP_INCLUDE_DIR MySQLConnectorCPP_LIBRARY)
