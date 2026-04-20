# Create a library target from header-only files
add_library(PrintersLib INTERFACE)

# Include path so main can access these headers
target_include_directories(PrintersLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})