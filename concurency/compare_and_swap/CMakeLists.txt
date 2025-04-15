set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# include_directories(./include/node)

add_library(cas INTERFACE cas.hpp)
target_link_libraries(cas INTERFACE)
target_include_directories(cas INTERFACE
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                            $<INSTALL_INTERFACE:include>
                           )