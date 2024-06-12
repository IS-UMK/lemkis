add_library(libexpect)
# Add the module file to the library
target_sources(libexpect
  PUBLIC
    FILE_SET CXX_MODULES FILES
      expect.cxx
)