Author: Vanessa Amfo-Ayeh

---Project 2 Design Notes---
Word Frequency List: implemented a **singly linked list** to store word frequencies for each file. To optimize the Jensen-Shannon Distance (JSD) calculation, maintain this list in alphabetical order during the insertion phase. This allows us to calculate the JSD in O(n+m) time using a two-pointer approach
Dynamic Allocation: Since the project prohibits assuming a maximum word length or file count, use `realloc` for our file-tracking array and dynamic `malloc` for every individual word string based on its actual length.
Tokenizing: uses a 1024-byte buffer with `read()`. It identifies "word characters" (alphanumeric and hyphens). If a word is split across two `read()` calls, buffer the prefix and concatenate it with the suffix from the next read to ensure no words are lost.
JSD Calculation: handle words that exist in only one of the two files by assigning them a frequency of 0 in the opposite file when calculating the Mean Distribution (M).
Sorting: After calculating JSD for all pairs, store the results in a struct array and use `qsort` to sort them by the combined word count in descending order before printing.

—Testing Plans—
1. Functional Testing (Word Frequency)
Case Sensitivity: Create a file with "Apple", "apple", and "APPLE". Verifying the output treats them as three instances of "apple".
Special Characters: Test files with punctuation (e.g., "can't", "hello!"). Verify "can't" is split into "can" and "t", and "hello!" becomes "hello".
Hyphens: Test "well-known". Verify it is treated as a single token.

2. Directory Traversal
Nested Folders: Create a directory structure three levels deep. Verify the program recursively finds all `.txt` files.
Hidden Files: Include a `.hidden.txt` file and verify the program skips it.
Empty Directories: Ensure the program does not crash when encountering an empty folder.

3. JSD Calculation Accuracy
Identity Test: Compare a file to itself. The JSD result should be $0.00000$.
Disjoint Sets Compare a file containing "abc" to a file containing "xyz". Verify the result against a manual calculation.
Empty Files: Test files with no valid tokens. The program should handle these without division-by-zero errors.

4. Robustness & Memory
Large Files: Process a file larger than the 1024-byte buffer to ensure the word-stitching logic works across `read()` calls.
Valgrind/ASAN: Run the program with `-fsanitize=address` to ensure no memory leaks or buffer overflows occur during long directory traversals.
Invalid Paths: Provide a non-existent directory as an argument and verify the program prints a helpful error message to `stderr`.
