# Create a library target from header-only files
add_library(BasketballLib INTERFACE)

# Include path so main can access these headers
target_include_directories(BasketballLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})