add_executable(polynomial_test test_polynomial.cxx)
target_link_libraries(polynomial_test libpolynomial libexpect)
add_test(NAME "Polynomial tests" 
  COMMAND $<TARGET_FILE:polynomial_test>)
