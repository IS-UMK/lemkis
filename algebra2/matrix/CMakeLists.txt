add_library(libmatrix)
# Add the module file to the library
target_sources(libmatrix
  PUBLIC
    FILE_SET CXX_MODULES FILES
      matrix.cxx
)