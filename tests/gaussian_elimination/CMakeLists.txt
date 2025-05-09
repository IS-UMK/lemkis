add_executable(gaussian_test gaussian_test.cxx)
target_link_libraries(gaussian_test libgaussian libmatrix libexpect)
add_test(NAME "Gaussian elimination tests" 
  COMMAND $<TARGET_FILE:gaussian_test>)
