add_executable(matrix_test test_matrix.cxx)
target_link_libraries(matrix_test libmatrix Catch2)
add_test(NAME "Matrix tests" 
  COMMAND $<TARGET_FILE:matrix_test>)
