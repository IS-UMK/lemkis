# When I start reviewing your code

You must meet the following conditions:

- In short you should be able to compile your library using
  ```bash
  clear && ./scripts/make_debug.sh && ./scripts/run_debug.sh  && ./scripts/run_tests.sh
  ```
  Note that `make_debug.sh` compiles project (the executable is build using `./main.cxx` file), `run_debug.sh` runs the executive and the `run_tests.sh` runs the tests from the `tests` subdirectory. Remember that you should provide tests for each function you provide. These tests should be added to appropriated subdirectory of `./tests/`.
- when your project is already built then it should pass the following check 
  ```bash
  find ./algebra2 -iname '*.cxx' -o -iname '*.ixx' | xargs clang-tidy-18  -p ./build/debug/ --extra-arg=-std=c++26
  ```
  (if you participate in discrete math project change `./algebra` to `./discrete_math` directory).
- your code is formatted according to `.clang-format`, so always run (in the root of this repository or use `--style` flag with appropriate value of `file:<format_file_path>` to set path to `.clang-format` file)
  ```bash
  clang-format -i <file_to_be_formatted>
  ```


# Style requirements of code

Non-exhausting list of requirements (note that some problems are detected by clang-tidy)

- use English only
- avoid magic constants (note that clang-tidy does not catch all magic constants)
- style is consistent - if you use a convention stick to it
- no inline comments. On the other hand documentation of each function is required (at least provide some brief description in a form of a comment). Make sure that documentation is not sloppy and explains any behaviour which might suprise a user.
- not too long functions - if you have complex logic in your function split it using helper functions. Note that clang-tidy is lenient (it allows to use up to about 15 lines of code) - however you should avoid more than +-seven statemets in the body of the function.
- do not use std::cout or c-style prints, take advantage of `std::format` and `std::print` (in particular overload `std::formatter` if you need to print some new structure)
- make sure your code is user-friendly and easy-to-read. In particular use descriptive names for functions, variable, concepts, etc.
- avoid code repetition - use auxiliary functions
- functions/classes/methods should be separated by at least two empty lines
 # Other requirements
You must
 -  provide FULLY implemented api (which is given to you when you are attached to a project). If you do not do so it might result in zero points outcome.
 - provide examples demonstrating basic usage of your api
 - you must provide tests (simple `assert()` tests suffice). In function in your api should have at least one corresponding test. Moreprecisely, each api function should be tested in every possible configuration (here configuration means usually `different set of input parameters`)
 - put your solution in a file  respecting the following pattern `<name_of_classes>/<name_of_project>/<name_of_your_solution.hpp>`
 - use exceptions only in exceptional cases, instead take advantage of `std::expected` or `std::optional` (depending on your needs)
 - use `const` as frequently as possible
 - provide `README.md` file in `<name_of_classes>/<name_of_project>/` subdirectory which explains how to compile you code, how to run examples, some description of project (what it is about), some api summary. Beware, depending on a project It might be required of you to provide additional information in `README.md` e.g. description of an algorithm you use, or a proof of a fact/facts you use etc.

# Some notes

- If your code breaks many of above rules It might result in immediate 0pt result.
- Beware that I might test your program to check if it is correct.
- In case of trouble contact me as soos as possible - let me know that you encountered some problems - otherwise I will NOT accept your explanation of a kind "I could not run this command", "I did not know how to do this and that".
- Clang tidy can sometimes be irritating - in case you cannot cope with some warnings it might be the case that a flag should be turned off. Make sure you inform me if such a case occurs.
