add_library(libgaussian)
# Add the module file to the library
target_sources(libgaussian
  PUBLIC
    FILE_SET CXX_MODULES FILES
      gaussian_elimination.cxx
      fraction.cxx
)

target_link_libraries(libgaussian libmatrix)