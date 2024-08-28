add_library(librecursion)
# Add the module file to the library
target_sources(librecursion
  PUBLIC
    FILE_SET CXX_MODULES FILES
      recursion.cxx
      recursion_helper.cxx
)