# Recursive Solutions for Various Problems

This program implements several problems using recursive functions. Each solution is explained in C++ included in project files. This document provides descriptions of problems as well as their mathematical solutions.

# How to run 
To run this program and any other programs in this repository, navigate to the root of the repository and build the project using CMake:
 ```bash
  clear && ./scripts/make_debug.sh && ./scripts/run_debug.sh  && ./scripts/run_tests.sh
 ```
In the tests directory you can see tests of the program in the folder `recursion`

## List of Problems

* `queens`
* `chocolates`
* `is_prime`
* `factorizations`
* `product`
* `sum_decomposition`
* `sum_of_digits`
* `to_binary`
* `iota_sum`
* `tiles`
* `generate_sequences`
* `sequences_from_a_set`
* `non_increasing_decompositions`
* `fibonacci`
* `path_in_maze`
* `subsets`
* `increasing_representations`
* `tower_of_hanoi`
* `more_ones`
* `count_decompositions_as_sum_of_powers`
* `bubble_sort`



# Mathematical Solutions

### 1. N-Queens Problem (`queens`)

#### Description
Place `n` queens on an `n x n` chessboard so that no two queens can attack each other. This means no two queens can be in the same row, column, or diagonal.

#### Mathematical Solution
The solution starts by placing queens one by one in different columns, starting from the leftmost column. For each placement, it checks if the queen can be attacked by any previously placed queens. If a safe position is found, the queen is placed and the algorithm moves to the next row to place the next queen. If no safe position is found, the algorithm backtracks to the previous row and tries the next possible position for the queen. This process continues until all queens are placed on the board or all possibilities are exhausted. 
`EXAMPLE`:
To solve the 4-Queens problem, we start with an empty 4x4 chessboard. We place the first queen in the first row and first column. Next, we move to the second row to place the second queen. The first column is under attack from the first queen, so we place the second queen in the second column instead. Now, we have queens at (0, 0) and (1, 1).
When we try to place the third queen in the third row, we find that columns one, two, and three are all under attack. Therefore, we backtrack and move the second queen to the third column. Now, the board has queens at (0, 0) and (1, 2). Moving to the third row, we place the third queen in the fourth column.
Next, we move to the fourth row. We find that all columns are under attack from previously placed queens. So, we backtrack again. We move the second queen to the fourth column, which leads to a configuration of queens at (0, 0) and (1, 3). Now, we place the third queen in the second column of the third row, and the fourth queen in the fourth row, first column. This configuration still doesn’t work.
Continuing with this approach, we find that a solution emerges when the first queen is placed at (0, 1), the second queen at (1, 3), the third queen at (2, 0), and the fourth queen at (3, 2). This placement ensures that no two queens threaten each other.
`Program`
The N-Queens problem is solved using a recursive backtracking approach in the program. The queens function initiates the process by checking for edge cases where no solutions exist (n = 2 or n = 3), and then sets up the board and solutions container. The solve_n_queens_util function is called to recursively place queens on the board column by column, ensuring no two queens threaten each other. The is_safe function checks for conflicts with previously placed queens. When a valid configuration is found, it's stored in the solutions list. Finally, print_solutions is used to display all valid configurations.

### 2. Chocolates Problem (`chocolates`)

#### Description
Given a certain amount of money, buy as many chocolates as possible. Each chocolate has a price, and you can exchange a specific number of wrappers for additional chocolates.

#### Mathematical Solution
Given the following three values, the task is to find the total number of maximum chocolates you can eat. 
money: Money you have to buy chocolates
price: Price of a chocolate
wrap: Number of wrappers to be returned for getting one extra chocolate.
It may be assumed that all given values are positive integers and greater than 1.
`EXAMPLE`:
Input: money = 16, price = 2, wrap = 2
Output:   15
Price of a chocolate is 2. You can buy 8 chocolates from
amount 16. You can return 8 wrappers back and get 4 more
chocolates. Then you can return 4 wrappers and get 2 more
chocolates. Finally you can return 2 wrappers to get 1
more chocolate.
`Program`
The function `chocolates` calculates how many chocolates you can initially buy with your money. Then, it uses a helper function, `getMaxChocolates`, to keep track of how many extra chocolates you can get by trading wrappers. This process repeats recursively until there aren't enough wrappers to trade for a new chocolate. 

### 3. Prime Number Check (`is_prime`)

#### Description
Determine if a number is prime.

#### Mathematical Solution
To determine if a numbern is prime, start by examining a few special cases. If n is less than or equal to 1, it is not prime. The numbers 2 and 3 are both prime numbers. If 
n is even and greater than 2, it is not a prime number since all even numbers greater than 2 are divisible by 2.
For numbers greater than 3, which are odd, we need to check if they are divisible by any odd number from 3 up to the square root of n. First, compute the square root of n. Then, test divisibility by each odd number starting from 3 and continuing up to the integer part of the square root. If none of these odd numbers divide n evenly, then n is a prime number. If any of these numbers divide n evenly, then n is not a prime number.
`Example`
To determine if 29 is a prime number, we first note that it is greater than 3 and is an odd number, so we need to check its divisibility by odd numbers up to its square root. The square root of 29 is approximately 5.39, so we only need to test divisibility by odd numbers up to 5.
First, we test divisibility by 3. Dividing 29 by 3 yields approximately 9.67, which is not an integer, so 29 is not divisible by 3. Next, we test divisibility by 5. Dividing 29 by 5 gives 5.8, which is also not an integer, so 29 is not divisible by 5.
Since 29 is not divisible by any of these numbers, it does not have any divisors other than 1 and itself. Therefore, 29 is a prime number.
`Program`
The function `is_prime` checks if a number is prime by checking if it can be divided by any number other than 1 and itself. It utilizes a helper function, `is_prime_helper`, which tests divisors starting from 2 up to the square root of the number. If no divisors are found, the number is prime.

### 4. Factorizations (`factorizations`)

#### Description
Find all possible ways to factor a given number into smaller integers.

#### Mathematical Solution
Factorization is the process of decomposing a number into a product of smaller numbers, which, when multiplied together, give the original number.
`Example`
For example we begin with the number 12 and start looking for factors beginning from 2. The smallest factor we find is 2. We then divide 12 by 2, giving us 6, and add 2 to our current list of factors.
Next, we factorize 6 starting again from 2. We find that 2 is also a factor of 6. We divide 6 by 2, resulting in 3, and add another 2 to our list of factors. Now, our list of factors is [2, 2].
We then move on to factorizing 3, starting from 2 again. Since 2 is not a factor of 3, we try the next number, which is 3. We find that 3 is a factor of 3. We divide 3 by 3, resulting in 1, and add 3 to our list of factors. Now, our list of factors is [2, 2, 3].
Since we have reached 1, this completes one factorization of 12 as [2, 2, 3].
We backtrack to explore other possible factorizations. We start again with 12 and find 2 as the first factor, giving us a quotient of 6. This time, instead of choosing 2 again, we check the next possible factor, which is 3. We find that 3 is a factor of 6, and dividing 6 by 3 gives us 2. Adding 3 to our factors, we now have [2, 3].
Next, we factorize 2, which is already a factor. Adding 2 to our factors, we get [2, 3, 2], which we can reorder to [2, 2, 3] (which we already have).
Next, we look at higher factors. We start with 12 and find 3 as a factor, which divides 12 to give us 4. Adding 3 to our factors, we now have [3]. We then factorize 4, finding 2 as a factor of 4, which divides 4 to give us 2. Adding 2 to our factors, we now have [3, 2]. We factorize 2 next and find 2 as a factor, which gives us [3, 2, 2] - the same as [2, 2, 3].
We also find that 6 is a factor of 12, which divides 12 to give us 2. This results in [6, 2], but reordered, it gives [2, 6].
At this point, we have found all the unique factorizations of 12: [2, 2, 3], [2, 6], and [3, 4]. Each list represents a unique way to multiply factors together to obtain the number 12.
`Program`
The function `factorizations` finds all the different combinations of factors that multiply to the given number. The helper function `find_factors` breaks down the number into its factors and continues this process recursively until all factors are identified.

### 5. Product Using Addition (`product`)

#### Description
Calculate the product of two integers using only addition and recursion.

#### Mathematical Solution
The concept of computing a product using addition is based on the idea that multiplication can be understood as repeated addition. To grasp this, consider that multiplying a number a by another number b means adding the number a to itself b times.
`Example`
We begin by calling the function product(3, 4). Since the second parameter b is 4 and not 0, the function computes 3+product(3,3).
Next, the function calls product(3, 3). In this call, b is 3 and still not 0, so it computes 3+product(3,2).
Then, the function proceeds with product(3, 2). Here, b is 2, which is again not 0, so it calculates 3+product(3,1).
Following this, the function calls product(3, 1). With b being 1, the function computes 3+product(3,0) because b is still not 0.
Finally, the function reaches product(3, 0). Since b is 0 in this call, the function returns 0, as per the base case of the recursion.
Now we can trace back through the recursion:
Product(3, 0) returns 0.
Product(3, 1) returns 3+0=3
Product(3, 2) returns 3+3=6
Product(3, 3) returns 3+6=9
Product(3, 4) returns 3+9=12
Thus, product(3, 4) ultimately returns 12, which is the correct result for multiplying 3 by 4 using repeated addition.
`Program`
The function `product` multiplies two numbers by adding one number to itself a specified number of times. It does this recursively by decreasing one of the numbers and adding the other number until the second number reaches zero.

### 6. Sum Decomposition (`sum_decomposition`)

#### Description
Given a number, find all possible ways to write it as a sum of integers.

#### Mathematical Solution
The concept behind the provided code is to find all possible ways to split a number into a specified number of positive integers that sum up to the original number.
`Example`
Suppose we want to decompose the number 7 into exactly 3 positive integers, where each combination of integers should sum up to 7.
We start by considering the smallest possible integer for our decomposition, which is 1. The goal is to find all sets of three numbers that sum up to 7.
Initially, we include the number 1 and then look for two more numbers that together with 1 will total 7. This means we need to find two numbers that sum up to 6 since 7−1=6. We start with 1 and then continue with this approach recursively.
We now need to decompose 6 into 2 positive integers.
The smallest number to try is again 1. Adding another 1, we now need a third number that makes the total sum 6. This means we need to find one number that adds up with 1 and 1 to make 6, which is 4 since 6−1−1=4.
This gives us the combination (1, 1, 5), which sums to 7.
Next, after the first 1, we try the next possible number, which is 2. So, we need to decompose 7−2=5 into 2 numbers.
With 2, we add 1 and need another number to make the total 5. That number is 4 since 5−1=4. This results in the combination (1, 2, 4).
Next, with 2 as our starting point, we look for 2 more numbers that sum up to 5. We try another number, which is 2, so we need one more number to reach 5. This is 3 since 5−2=3, giving us the combination (2, 2, 3).
We have now exhausted all possibilities with starting numbers 1 and 2.
By following this systematic approach, checking each possible number while reducing the problem into smaller subproblems, we eventually gather all valid combinations where three positive integers sum up to 7. For this example, the valid combinations are (1, 1, 5), (1, 2, 4), and (2, 2, 3). Each of these sets of numbers meets the criteria: they are positive integers and their total equals 7.
`Program`
The function `sum_decomposition` generates all possible combinations of numbers that add up to the given number. The helper function `find_combinations` subtracts potential summands from the total and tracks the current combination until all valid combinations are found. 

### 7. Sum of Digits (`sum_of_digits`)

#### Description
Calculate the sum of the digits of a given number.

#### Mathematical Solution
`Example`
For instance you have the number 5678 and want to calculate the sum of its digits using the sum_of_digits function. The function starts by isolating the last digit, which is 8, using the modulus operation (5678 % 10). It then makes a recursive call to handle the remaining number, 567, and adds the isolated digit 8 to whatever sum the recursive call returns.
In the next step, with 567, the function isolates the last digit, 7, and again makes a recursive call with the remaining number, 56. This process repeats, breaking down 56 into 6 and calling the function with 5, and then finally handling the single digit 5.
At this point, when the function is called with 5, it isolates the last digit and makes one final recursive call with 0. Since the number is now zero, the base case is reached, and the function starts returning the sums back up the recursive chain. First, 5 is returned, then 5 + 6, followed by 11 + 7, and finally 18 + 8, resulting in a total sum of 26. This is the sum of the digits 5 + 6 + 7 + 8.
`Program`
The sum_of_digits function calculates the sum of all the digits in a number using recursion. It works by repeatedly taking the last digit of the number, adding it to the sum, and then removing that digit until there are no digits left.
The function first checks if the number is zero. If it is, it returns zero, which stops the recursion. Otherwise, it takes the last digit of the number using the modulus operation (number % 10), adds that digit to the result of calling itself with the rest of the number (number / 10), and continues this process until all digits have been summed.

### 8. Binary Representation (`to_binary`)

#### Description
Convert a number to its binary representation.

#### Mathematical Solution
`Example`
Let say you want to convert the number 13 into its binary representation using the to_binary function. The function starts with 13 and determines that the last binary digit is 1 by calculating 13 % 2. It then calls itself with 13 / 2, which is 6, and continues this process. For 6, the last binary digit is 0 (6 % 2), so the function calls itself with 3 (6 / 2).
When handling 3, the function finds the last binary digit to be 1 (3 % 2) and calls itself with 1 (3 / 2). Finally, for 1, the last binary digit is 1 (1 % 2), and the function calls itself with 0, where it reaches the base case and returns 0.
As the recursion unwinds, the function combines all the binary digits: starting from 1 for 1, then 11 for 3, followed by 110 for 6, and finally 1101 for 13. The result is 1101, which is the binary representation of 13, but returned as a number in base 10.
`Program`
The function `to_binary` converts a number to binary by dividing the number by 2 and recording the remainders. This is done recursively, and the binary number is constructed from the remainders in reverse order.
Now, the function starts by checking if the number is zero. If it is, it returns 0 as the binary representation. If not, it calls itself with half the number (number / 2), which gradually breaks down the number into smaller parts. The function then finds the last binary digit of the current number using number % 2 and combines this digit with the binary result of the smaller number. This process continues until the entire binary number is built.
### 9. Iota Sum (`iota_sum`)

#### Description
Find the sum of the first `n` natural numbers.

#### Mathematical Solution
`Example`
If you call iota_sum with the value 4, the function first computes the sum of numbers up to 4. It starts by checking if 4 is 0, which it is not, so it adds 4 to the result of calling iota_sum with 3. The function then does the same for 3, adding it to the result of iota_sum with 2. This pattern continues: iota_sum with 2 adds it to the result of iota_sum with 1, and iota_sum with 1 adds it to the result of iota_sum with 0. When iota_sum reaches 0, it returns 0. As the recursion unwinds, the sums are added together: 1 plus 0 is 1, 2 plus 1 is 3, 3 plus 3 is 6, and finally, 4 plus 6 is 10. Thus, the function returns 10, which is the sum of all numbers from 1 to 4.
`Program`
The function `iota_sum` adds up the first `n` natural numbers by recursively adding each number to the sum of the numbers before it. The sum of the first `n` natural numbers can also be calculated using the formula `n * (n + 1) / 2`. The iota_sum function calculates the sum of all integers from 1 up to a given number up_to. It works by calling itself recursively: if up_to is 0, it returns 0 because there are no numbers to add. For any positive value of up_to, the function returns the value of up_to plus the result of calling itself with up_to decreased by 1. This process continues until it reaches the base case of 0, effectively adding all integers from 1 to up_to.

### 10. Tiles (`tiles`)

#### Description
Calculate the number of the largest power-of-2 tiles needed to completely cover a grid of given size.

#### Mathematical Solution
`Example`
For instance you have a rectangular area that is 7 units wide and 5 units tall, and you want to cover it with the fewest square tiles possible, where the side lengths of the tiles must be powers of 2 (e.g., 1x1, 2x2, 4x4).
First, the `find_largest_tile` function finds the largest tile that can fit within both the width and height. For 7 and 5, the largest possible square tile is 4x4 because 4 is the largest power of 2 that fits within both dimensions.
You place a 4x4 tile in the top-left corner of the area. This covers most of the area, but it leaves three smaller rectangles: one that is 3 units wide and 5 units tall below the tile, one that is 4 units wide and 1 unit tall to the right of the tile, and one that is 3x1 in the bottom-right corner.
The function then recursively applies the same process to each of these smaller rectangles. For the 3x5 area, the largest square tile that fits is 2x2. You place a 2x2 tile, and then recursively cover the remaining 1x5, 2x3, and 1x3 areas. This continues until the entire area is covered.
By always choosing the largest tile that fits, the function ensures that the number of tiles used is minimized. In this example, you might end up using a combination of 4x4, 2x2, and 1x1 tiles to completely cover the 7x5 area.
`Program`
The code is designed to solve the problem of covering a rectangular area using the minimum number of square tiles, where each tile's side length is a power of 2. The `find_largest_tile` function determines the largest square tile that can fit within a given dimension by repeatedly doubling the tile size until it exceeds the given size. The main function, `tiles`, then uses this information to cover the rectangle. It begins by placing the largest possible tile in the top-left corner of the area, and then recursively tackles the remaining uncovered regions, which are the areas to the right, below, and the bottom-right corner of the placed tile. This process continues until the entire area is covered, with the function summing up the number of tiles needed at each step. By always selecting the largest possible tile that fits, the function attempts to minimize the total number of tiles required to cover the entire area.

### 12. Sequences from a Set (`sequences_from_a_set`)

#### Description
Create all possible sequences of letters from a given set with a length less than or equal to a maximum length.

#### Mathematical Solution
`Concept`
The code is designed to generate all possible sequences of a given set of letters, where the sequences can be of any length from 1 up to a specified maximum length. The main idea is to use a recursive approach to explore all possible combinations of the letters. Starting with an empty sequence, the code builds longer sequences by adding one letter at a time from the set, and then recursively repeating this process until the sequences reach the desired maximum length. Each valid sequence is stored in a result list. This method ensures that every possible combination of the letters, including repeated letters, is generated and captured.
`Example`
Imagine you have a set of letters {a, b} and you want to generate all possible sequences of these letters with lengths up to 2. The code starts with an empty sequence and begins adding letters to it. First, it creates the single-letter sequences "a" and "b". Then, it takes these sequences and adds each letter again to create the two-letter sequences: "aa", "ab", "ba", and "bb". By the end, the code produces all possible sequences of 1 or 2 letters using the given set: "a", "b", "aa", "ab", "ba", and "bb".
`Program`
The function `sequences_from_a_set` generates all possible sequences by utilizing the helper function`generate_sequences` 
The `generate_sequences` function recursively builds sequences by adding each letter to the current sequence. It continues this process until the maximum length is reached or all letters are used up. The resulting sequences are stored in a result vector.

### 13. Non-Increasing Decompositions (`non_increasing_decompositions`)

#### Description
Generate all possible non-increasing decompositions of a given number.

#### Mathematical Solution
`Concept`
The function `non_increasing_decompositions` generates all sequences of non-increasing integers that sum to the given number. 
`Example`
For example the number to be decomposed is 4.
When you call the function non_increasing_decompositions(4), it aims to find all possible ways to break down the number 4 into sequences of numbers that are non-increasing, meaning each number in the sequence is less than or equal to the one before it. The function starts by initializing an empty list to store the results and another list to keep track of the current sequence being formed.
The function then begins by considering the largest possible number that can start a sequence, which is 4 itself. It adds 4 to the current sequence and notices that the remainder is now 0, so [4] is a valid sequence that sums to 4. This sequence is added to the result list.
Next, the function backtracks to explore other possibilities. It then considers starting the sequence with 3, which leaves 1 to be decomposed. The function adds 3 to the current sequence and then adds 1, forming the sequence [3, 1], which is another valid decomposition. This sequence is also added to the result list.
Continuing, the function considers sequences that start with 2. It first pairs 2 with another 2, forming [2, 2], and adds this sequence to the result. It then explores sequences where 2 is followed by two 1s, leading to [2, 1, 1], which is another valid decomposition that gets added to the results.
Finally, the function considers the smallest possible number, 1, and creates the sequence [1, 1, 1, 1], which sums to 4. This sequence is added to the list of results.
In the end, the function returns a list containing all these sequences: [4], [3, 1], [2, 2], [2, 1, 1], and [1, 1, 1, 1]. These represent all possible non-increasing ways to break down the number 4.
`Program`
The helper function `generate_decompositions` performs the recursive decomposition by:
Checking if the remaining number is zero. If it is, the current sequence is added to the results. It also iterates through possible values for the next element in the sequence, starting from the minimum of `number` and `max_value`, down to 1. For each value: Adding the value to the current sequence, recursively calling `generate_decompositions` with the updated number (`number - i`) and the new maximum value (`i`). Removes the value from the current sequence after the recursive call completes (backtracking). The decompositions are stored in a result vector and are printed using the `print_decompositions` function.

### 14. Fibonacci (`fibonacci`)

#### Description
Finds the nth Fibonacci number.

#### Mathematical Solution
`Concept`
The Fibonacci sequence starts with 0 and 1. Each subsequent number in the sequence is the sum of the two earlier ones.
`Example`
Let's say you want to find the 5th Fibonacci number using the fibonacci function. When you call fibonacci(5), the function doesn't immediately know the answer, so it breaks the problem down into smaller parts.
First, it checks if 5 is 0 or 1. Since it's neither, the function calls itself twice: once with fibonacci(4) and once with fibonacci(3). Now, to figure out fibonacci(4), the function again breaks it down into fibonacci(3) and fibonacci(2), and so on. This continues until the function reaches the base cases where it knows the answer: fibonacci(1) returns 1, and fibonacci(0) returns 0.
As the function begins to return from these recursive calls, it adds the results together. For instance, fibonacci(2) returns 1 (since fibonacci(1) + fibonacci(0) = 1 + 0), and fibonacci(3) returns 2 (since fibonacci(2) + fibonacci(1) = 1 + 1). Eventually, all these results combine to give fibonacci(5) as 5, which is the 5th Fibonacci number. This method of breaking the problem down into smaller, easier-to-solve parts and then combining the results is how the recursive function calculates the Fibonacci number.
`Program`
A function called fibonacci calculates the nth Fibonacci number using a recursive method. When you call this function with a specific value for nth, it first checks whether nth is 0 or 1. If nth is 0, the function returns 0 because the Fibonacci sequence starts with 0 at the 0th position. If nth is 1, it returns 1 because the Fibonacci sequence continues with 1 at the 1st position.

For any value of nth greater than 1, the function calculates the Fibonacci number by adding the results of two recursive calls: one with nth - 1 and another with nth - 2. This process continues, breaking down the problem into smaller subproblems, until it reaches the base cases of 0 or 1. The sum of these smaller Fibonacci numbers gives the nth Fibonacci number. Thus, the function effectively builds up the Fibonacci sequence through a series of recursive calculations.
### 15. Path in Maze (`path_in_maze`)

#### Description
Given a maze (0 stands for a path, 1 for a wall) of shape `rows x columns`, checks if there is a path from point `(startx, starty)` to `(endx, endy)`. You can move in basic 4 directions (up, down, left, right).

#### Mathematical Solution
`Example`
Imagine you have a 3x3 maze where 0 represents a path and 1 represents a wall. The maze looks like this:
0 1 0
0 0 0
1 0 1
You want to find out if there's a path from the top-left corner (0, 0) to the bottom-right corner (2, 2). When you call the path_in_maze function with these start and end points, the function first checks if the start and end points are the same. Since they're not, it moves on to check if the current position (0, 0) is valid. The position is valid, so the function marks it as visited.
Next, the function attempts to move in each of the four possible directions: down, up, right, and left. It first tries moving down to (1, 0), where it again checks if this position is valid and not visited. Since (1, 0) is a valid path, the function continues recursively from this new position.
From (1, 0), it tries moving down again to (2, 0), but this position is blocked by a wall (1), so the function backtracks. It then tries moving right to (1, 1), which is valid. From (1, 1), the function can either move down, right, or up. Moving right to (1, 2) is valid, so the function proceeds.
Finally, from (1, 2), the function moves down to (2, 2), which is the target position. Since the start and end points now match, the function returns true, indicating that a path exists from the top-left to the bottom-right corner of the maze.
`Program`
A function named path_in_maze determines whether there is a path from a starting point to an ending point in a maze. The maze is represented as a vector of booleans, where 0 (or false) indicates a path and 1 (or true) represents a wall. The function checks if a path exists from the starting coordinates (startx, starty) to the ending coordinates (endx, endy).
When the function is called, it first checks if the starting point is the same as the ending point. If they match, it returns true because the path has been found. If they don't match, the function checks whether the current position (startx, starty) is valid for movement using the is_valid_move function (not defined in the snippet). If the move is invalid (e.g., the current position is a wall or out of bounds), the function returns false, indicating that no path exists from that direction.
If the move is valid, the function marks the current position as visited by setting it to true in the maze. It then recursively attempts to find a path by moving in each of the four possible directions: down, up, right, and left. The function explores these directions one by one. If any of these recursive calls return true, it indicates that a path has been found, and the function returns true.
If no path is found in any direction, the function backtracks by marking the current position as unvisited (resetting it to false in the maze) and returns false. This process ensures that the function explores all possible paths and backtracks correctly when necessary.
In summary, this function uses recursion to explore possible paths in the maze, marking positions as visited and backtracking when needed, until it either finds a path to the endpoint or determines that no such path exists.

### 16. Subsets (`subsets`)

#### Description
Return all possible non-empty proper subsets of numbers.

#### Mathematical Solution
`Example`
Let`s give an example where we use the subsets function to find all non-empty subsets of the set {1, 2, 3}.
When you call the subsets function with this set, it first converts the set into an array of integers, resulting in {1, 2, 3}. The generate_subsets function is then invoked to recursively generate all subsets.
Initially, the generate_subsets function starts with an empty subset and an index of 0. It checks if the current subset is non-empty and adds it to the result if it is. Since the subset is empty at the beginning, nothing is added yet.
The function then starts a loop from index 0. It adds the element 1 from the array to the current subset, making the subset {1}. It then calls itself recursively to explore further subsets that include 1. Within this recursive call, the function adds 2 to the subset, resulting in {1, 2}, and continues to explore subsets including 2. Next, it adds 3 to form the subset {1, 2, 3} and makes further recursive calls.
The recursion explores all possible subsets including and excluding each element. After exploring all paths with the inclusion of 1, the function backtracks, removing 1 from the subset and starts exploring subsets that do not include 1 but include 2 and 3. This process continues recursively for all elements and their combinations.
In the end, all non-empty subsets of {1, 2, 3} are generated. The final result includes the subsets: {1}, {1, 2}, {1, 2, 3}, {1, 3}, {2}, {2, 3}, and {3}. Each subset is added to the result vector during the recursion, and the function eventually returns this vector, containing all possible non-empty subsets of the original set.
`Program`
The code provides two functions designed to find and return all possible non-empty subsets of a given set of integers.
The subsets function is the main function that starts the process. It takes a std::set<std::int32_t> of integers and initializes the necessary variables for generating subsets. It converts the set into an array of integers using dynamic memory allocation, copying the set’s elements into this array. Then it calls the generate_subsets helper function to populate the result vector with all possible subsets.
The generate_subsets function is responsible for the recursive generation of subsets. It starts by checking if the current subset (stored in the subset vector) is non-empty. If it is, this subset is added to the subsets vector. The function then iterates through the remaining elements of the array, adding each element to the current subset and recursively calling itself to explore further subsets that include this element. After the recursive call, it removes the last element added (backtracking) to explore other possible subsets that exclude this element.
Overall, the code builds up all possible subsets of the given set by recursively including and excluding each element, while ensuring that only non-empty subsets are added to the final result.

### 17. Increasing Representations (`increasing_representations`)

#### Description
Given the number of digits `number_of_digits` in a number, returns all `n`-digit numbers whose digits are strictly increasing from left to right. Leading zeros are allowed so for example, a two-digit `01` stands for number `1`.

#### Mathematical Solution
`Example`
Imagine you want to generate all 2-digit numbers where the digits strictly increase from left to right. You call the `increasing_representations` function with `number_of_digits` set to 2.
The function starts by initializing an empty vector results to store the generated numbers. It then calls the generate_numbers function, beginning with 0 as the current number, 2 as the total number of digits, and -1 as the last digit used (this ensures the first digit can start from 0).
As generate_numbers runs, it begins by adding a digit greater than -1 (starting from 0) to the current number. For the first digit, it tries 0, then 1, then 2, and so on up to 9. For each of these, it tries to add a second digit that is greater than the first. For example, if the first digit is 0, it tries adding 1, 2, ..., up to 9. This results in numbers like 01, 02, 03, ..., 09.
If the first digit is 1, the function generates numbers like 12, 13, 14, ..., 19. Similarly, if the first digit is 2, it generates numbers like 23, 24, and so on.
After exploring all possible combinations of strictly increasing digits for the specified number of digits, the function collects all these numbers in the results vector and returns them. So, for 2 digits, the final output would include numbers like 01, 02, 03, 12, 13, and so on, up to 89.
`Program`
The code defines a function called `increasing_representations` that generates all numbers with a specified number of digits (`number_of_digits`), where the digits strictly increase from left to right. The function allows leading zeros, meaning that a sequence like "01" would be valid and represent the number 1.
In this function, the results vector is initialized to store the resulting numbers. The generate_numbers function is then called to start the recursive process of building these numbers. The initial call to generate_numbers starts with 0 as the current number, the total number of digits specified by `number_of_digits`, -1 as the last digit (to ensure that the first digit can start from 0), and the results vector to store the valid numbers generated during the recursion.
This recursive process will generate all possible sequences of digits that strictly increase from left to right, respecting the condition that each digit must be greater than the previous one. The final result is a vector of all such numbers.

### 18. Tower of Hanoi (`tower_of_hanoi`)

#### Description
Solve the Tower of Hanoi problem by presenting the steps involved in moving a tower of disks from one rod to another.

#### Mathematical Solution
`Example`
For example we use the `tower_of_hanoi` function to solve the Tower of Hanoi problem with 3 discs.
When you call tower_of_hanoi(3), the function initializes an empty vector called moves to store each step of the solution. It then calls the `hanoi` function, starting with all 3 discs on the source peg 'A', aiming to move them to the destination peg 'C', using peg 'B' as an auxiliary.
The `hanoi` function starts by considering the problem of moving 3 discs. It first needs to move the top 2 discs from 'A' to 'B'. To do this, it recursively calls itself to move 1 disc (the top one) from 'A' to 'C' using 'B' as an auxiliary. The first move recorded is "Move disk 1 from A to C".
Next, the function moves the second disc from 'A' to 'B' and records "Move disk 2 from A to B". It then moves the previously moved disc from 'C' to 'B', recording "Move disk 1 from C to B". Now, the top 2 discs are on peg 'B'.
The next step is to move the third disc (the largest one) directly from 'A' to 'C', which is recorded as "Move disk 3 from A to C".
Finally, the function needs to move the 2 discs on 'B' to 'C' to complete the process. It first moves the top disc from 'B' to 'A', recording "Move disk 1 from B to A". Then it moves the second disc from 'B' to 'C', recording "Move disk 2 from B to C". Finally, it moves the smallest disc from 'A' to 'C', recording "Move disk 1 from A to C".
After all these steps, the function returns the moves vector, which now contains the complete sequence of moves:
"Move disk 1 from A to C"
"Move disk 2 from A to B"
"Move disk 1 from C to B"
"Move disk 3 from A to C"
"Move disk 1 from B to A"
"Move disk 2 from B to C"
"Move disk 1 from A to C"
This sequence provides a step-by-step solution to transferring 3 discs from peg 'A' to peg 'C', following the rules of the Tower of Hanoi.
`Program`
The code provided solves the classical Tower of Hanoi problem, where the objective is to move a stack of discs from one peg to another, using a third peg as an auxiliary, while adhering to specific rules. The rules are: only one disc can be moved at a time, a disc can only be placed on top of a larger disc or an empty peg, and all discs must be moved from the source peg to the destination peg.
The `tower_of_hanoi` function is the main function that initiates the process. It takes the number of discs as input and creates a vector called moves to store each move made during the process. The function then calls the helper function `hanoi`, passing the number of discs, the source peg ('A'), the destination peg ('C'), the auxiliary peg ('B'), and the moves vector.
The `hanoi` function is where the recursion happens. If there's only one disc (n == 1), the function simply records the move to transfer that disc from the source peg to the destination peg. For more than one disc, the function first recursively moves n-1 discs from the source peg to the auxiliary peg, then moves the nth disc from the source peg to the destination peg, and finally, it moves the n-1 discs from the auxiliary peg to the destination peg.
This recursive process breaks down the problem into smaller and smaller subproblems until it reaches the simplest case of moving a single disc. The sequence of all these moves is stored in the moves vector, which the `tower_of_hanoi` function returns at the end. This vector contains a detailed step-by-step solution to the Tower of Hanoi problem for the given number of discs.

### 19. Generate Bit Representations (`more_ones`)

#### Description
Generate all possible bit representations of length `number_of_bits` with the condition that each prefix of the number contains more ones than zeros.

#### Mathematical Solution
`Example`
For example you want to generate all 3-bit binary strings where each prefix has more ones than zeros. You call the more_ones function with number_of_bits set to 3.
The function starts with an empty string and zero counts for both ones and zeros. The generate_bit_representations function begins by adding a '1' to the string since a prefix with more ones than zeros must start with a '1'. The string now becomes "1".
From here, the function can add either another '1' (because ones are still equal to zeros) or a '0' (since the current number of ones is greater than zeros). Suppose it adds a '1', making the string "11". Now, it can only add a '0' to maintain the condition of more ones than zeros in any prefix. This results in the string "110", which is added to the result list.
Alternatively, after forming the string "1", the function could add a '0', making the string "10". However, this string is invalid because adding a '0' here would result in an equal number of ones and zeros, which violates the condition. Therefore, the function doesn't explore this path further.
Following this recursive process, the valid 3-bit strings generated by the function would be "111" and "110". These are the only 3-bit representations where every prefix has more ones than zeros.
`Program`  
The `more_ones` function generates possible bit representations recursively. It takes into account the number of ones and zeros added so far and the remaining bits to be added. The conditions for generating the bit representations are:
- If the number of remaining bits is zero, the current representation is added to the result vector.
- If adding a one does not violate the condition (onesCount + 1 >= zerosCount), a one is added to the current representation, and the function is called recursively.
- If adding a zero does not violate the condition (onesCount > zerosCount), a zero is added to the current representation, and the function is called recursively.
The resulting bit representations are stored in a vector and returned.

### 20. Count Decompositions as Sum of Powers (`count_decompositions_as_sum_of_powers`)

#### Description
Count all unique decompositions of a given number as the sum of powers of consecutive integers. 

#### Mathematical Solution
`Example`
Imagine you want to decompose the number 10 into sums of squares, where each term in the sum is a perfect square (e.g.,y^2). You call the function `count_decompositions_as_sum_of_powers(10, 2)`.
The function starts by attempting to break down 10 using the smallest possible square, which is 1^2=1. Subtracting 1 from 10 leaves you with 9. The function then recursively tries to decompose the remaining 9. For 9, it tries 2^2=4, which leaves 5 to decompose. It continues this process, checking each square and subtracting it from the remaining value.
For example, the function will find that 10=1^2+3^2 and 10=3^2+1^2, but since the order doesn't matter, these are considered the same decomposition. Other valid decompositions might include 10=1^2+1^2+2^2+2^2, and so forth.
The function keeps track of all unique ways to achieve this, ensuring that it doesn't count the same combination in different orders. When all possibilities are explored, the function returns the total number of unique decompositions.
`Program`
The `count_decompositions_as_sum_of_powers` function calculates the number of unique decompositions of a given number into the sum of powers of integers.
The function `count_decompositions` performs the recursive decomposition by iterating through potential bases starting from `currentBase`. It adds the current base to the decomposition and recursively calls itself with the updated parameters. After exploring all possibilities for the current base, it removes the last base from the decomposition and continues the search using new base.

### 21. Bubble Sort (`bubble_sort`)

#### Description
Sorts a given vector using the bubble sort algorithm and returns the sorted vector.

#### Mathematical Solution
`Example`
Starting with the vector [5, 2, 9, 1, 5], the `bubble_sort` function calls `bubble_pass` to begin sorting.
In the first pass of the `bubble_pass` function, it compares adjacent elements and swaps them if necessary. Initially, it compares the first two elements, 5 and 2, and swaps them because 5 is greater than 2. The vector then becomes [2, 5, 9, 1, 5]. Next, it compares 5 and 9, leaving them as is since 5 is not greater than 9. It then compares 9 and 1, swapping them because 9 is greater than 1, resulting in [2, 5, 1, 9, 5]. Finally, it compares 9 and 5, swapping them to get [2, 5, 1, 5, 9]. This completes the first pass, with the largest element, 9, correctly positioned at the end of the vector.
The `bubble_pass` function is called again with one less element to sort (n becomes 4), and the process repeats. The function now sorts the vector [2, 5, 1, 5] in the same manner. After several passes, each time reducing the number of elements to consider, the vector gradually becomes more ordered. The process continues until no further swaps are needed, and the vector is fully sorted.
In the end, the sorted vector returned by `bubble_sort` is [1, 2, 5, 5, 9].
`Program`
The code implements the bubble sort algorithm to sort a vector of integers. The sorting process is split into two functions: `bubble_pass` and `bubble_sort`.
The `bubble_sort` function is the entry point, taking a vector of integers and initiating the sorting process. It first determines the size of the vector and then calls the `bubble_pass` function to perform the actual sorting. After sorting, it returns the sorted vector.
The `bubble_pass` function handles the core sorting logic. It works by repeatedly comparing and swapping adjacent elements in the vector if they are out of order, effectively "bubbling" the largest unsorted element to the end of the vector with each pass. This process is recursive, with each call to `bubble_pass` reducing the number of elements to consider by one (since the last element of each pass is already in its correct position). The recursion continues until the entire vector is sorted.


#### Authors
* Andrii Brilliant
* Radosław Gawryszewski
* Hubert Krakowiak
* Paweł Kulesza