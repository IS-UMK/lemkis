add_library(libpolynomial)
# Add the module file to the library
target_sources(libpolynomial
  PUBLIC
    FILE_SET CXX_MODULES FILES
      polynomial.cxx
)

