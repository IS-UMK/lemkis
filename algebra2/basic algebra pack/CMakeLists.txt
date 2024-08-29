add_library(libbasicpack)
# Add the module file to the library
target_sources(libbasicpack
  PUBLIC
    FILE_SET CXX_MODULES FILES
      basic_algebra_2_pack.cxx
)

target_link_libraries(libbasicpack libmatrix)