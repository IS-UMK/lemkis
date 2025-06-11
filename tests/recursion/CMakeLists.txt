add_executable(recursion_test recursion_test.cxx)
target_link_libraries(recursion_test librecursion libexpect)
add_test(NAME "Recursion tests" 
  COMMAND $<TARGET_FILE:recursion_test>)
