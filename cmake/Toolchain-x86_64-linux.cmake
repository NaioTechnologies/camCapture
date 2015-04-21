SET( CMAKE_SYSTEM_NAME Linux )

SET( CMAKE_C_COMPILER   "/usr/bin/x86_64-linux-gnu-gcc-4.9" )
SET( CMAKE_CXX_COMPILER "/usr/bin/x86_64-linux-gnu-g++-4.9" )

# Define paths to search for libraries
SET( CMAKE_FIND_ROOT_PATH /usr )
SET( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )

# Don't search in native paths, just the specified root paths
SET( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
SET( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
