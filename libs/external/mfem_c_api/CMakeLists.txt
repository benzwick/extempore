cmake_minimum_required(VERSION 3.12)
project(mfem_c_api)

# Find MFEM installation
find_package(MFEM REQUIRED)

# Create shared library with C API wrapper
add_library(mfem_c_api SHARED
  mfem_c_api.cpp)

target_include_directories(mfem_c_api
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

# Link against MFEM's exported target
target_link_libraries(mfem_c_api
  PRIVATE
    mfem)

# Set output name to match what Extempore expects
set_target_properties(mfem_c_api PROPERTIES
  OUTPUT_NAME "mfem_c_api"
  VERSION 1.0.0
  SOVERSION 1)

# Install rules
install(TARGETS mfem_c_api
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib)

install(FILES mfem_c_api.h
  DESTINATION include)
