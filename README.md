# When I start reviewing your code

You must meet the following conditions:

- you code compiles using ```clang++-18 -std=gnu++26 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror <your_cpp_file> -o <your_output_file>```
- it is formatted according to `.clang-format`, so always run ```clang-format -i <file_to_be_formatted>```
- should generate no errors as it comes to clang-tidy requirements, so always run ``` clang-tidy-18 ./polynomial.hpp  -- -std=gnu++26 -stdlib=libc++```
  


# Style requirements of code

Non-exhausting list of requirements (note that some problems are detected by clang-tidy)

- use English only
- style is consistent - if you use a convention stick to it
- no inline comments. On the other hand documentation of each function is required (at least provide some brief description in a form of a comment). Make sure that documentation is not sloppy and explains any behaviour which might suprise a user.
- not too long functions - if you have complex logic in your function split it using helper functions.
- do not use std::cout or c-style prints, take advantage of `std::format` and `std::print` (in particular overload `std::formatter` if you need to print some new structure)
- make sure your code is user-friendly and easy-to-read. In particular use descriptive names for functions, variable, concepts, etc.
- avoid code repetition - use auxiliary functions
- functions/classes/methods should be separated by at least two empty lines
 # Other requirements
You must
 -  provide FULLY implemented api (which is given to you when you are attached to a project)
 - provide examples demonstrating basic usage of your api
 - you must provide tests (simple `assert()` tests suffice). In function in your api should have at least one corresponding test. Moreprecisely, each api function should be tested in every possible configuration (here configuration means usually `different set of input parameters`)
 - put your solution in a file  respecting the following pattern `<name_of_classes>/<name_of_project>/<name_of_your_solution.hpp>`
 - use exceptions only in exceptional cases, instead take advantage of `std::expected` or `std::optional` (depending on your needs)
 - use `const` as frequently as possible


# Some notes

- If your code breaks many of above rules It might result in immediate 0pt result.
- Beware that I might test your program to check if it is correct.
