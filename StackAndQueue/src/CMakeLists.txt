set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(mutex INTERFACE con_stack.hpp con_queue.hpp)

target_link_libraries(mutex INTERFACE)

target_include_directories(mutex INTERFACE
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                            $<INSTALL_INTERFACE:include>
                           )
