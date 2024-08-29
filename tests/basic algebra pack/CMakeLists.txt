add_executable(basic_test test_basic_algebra_pack.cxx)
target_link_libraries(basic_test libmatrix libexpect libgaussian libbasicpack)
add_test(NAME "Basicalg tests" 
  COMMAND $<TARGET_FILE:basic_test>)