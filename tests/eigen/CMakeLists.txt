add_executable(eigen_test test_eigen.cxx)
target_link_libraries(eigen_test libeigen libmatrix libexpect)
add_test(NAME "Eigen tests" 
  COMMAND $<TARGET_FILE:eigen_test>)
