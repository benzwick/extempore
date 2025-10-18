# CMake script to patch MFEM's CMakeLists.txt to include C API wrapper
# This is run during the PATCH_COMMAND step of ExternalProject_Add

# Copy C API files
file(COPY
  "${MFEM_CAPI_SOURCE_DIR}/mfem_c_api.h"
  "${MFEM_CAPI_SOURCE_DIR}/mfem_c_api.cpp"
  DESTINATION "${MFEM_SOURCE_DIR}")

# Read MFEM's CMakeLists.txt
file(READ "${MFEM_SOURCE_DIR}/CMakeLists.txt" MFEM_CMAKE_CONTENT)

# Check if already patched (to make this idempotent)
string(FIND "${MFEM_CMAKE_CONTENT}" "mfem_c_api.cpp" ALREADY_PATCHED)

if(ALREADY_PATCHED EQUAL -1)
  # Find the line that adds mfem library sources
  # MFEM's CMakeLists.txt has: mfem_add_library(mfem ${SOURCES} ${HEADERS} ${MASTER_HEADERS})
  # We want to add our C API source to SOURCES before that line

  string(REPLACE
    "mfem_add_library(mfem \${SOURCES} \${HEADERS} \${MASTER_HEADERS})"
    "list(APPEND SOURCES mfem_c_api.cpp)\nlist(APPEND HEADERS mfem_c_api.h)\nmfem_add_library(mfem \${SOURCES} \${HEADERS} \${MASTER_HEADERS})"
    MFEM_CMAKE_CONTENT
    "${MFEM_CMAKE_CONTENT}")

  # Write the patched CMakeLists.txt back
  file(WRITE "${MFEM_SOURCE_DIR}/CMakeLists.txt" "${MFEM_CMAKE_CONTENT}")

  message(STATUS "Patched MFEM CMakeLists.txt to include C API wrapper")
else()
  message(STATUS "MFEM CMakeLists.txt already patched for C API")
endif()
