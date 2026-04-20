add_library(libeigen)
# Add the module file to the library
target_sources(libeigen
  PUBLIC
    FILE_SET CXX_MODULES FILES
      eigen.cxx
)

target_link_libraries(libeigen libmatrix)