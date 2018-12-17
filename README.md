# Futoshiki

OVERVIEW
--------------------------------------------------
This is an implementation of a Futoshiki game solver for the [Advanced Algorithms] course. The purpose of this project was to implement a futoshiki solver and compare the efficiency between backtracking algorithms.  It was made at the Computer Science course from University of São Paulo (ICMC - USP).

HOW TO COMPILE
--------------------------------------------------

Clone Futoshiki repository:

```bash
  $ git clone https://github.com/wesjrock/grooveshark.git
```

Compiling:

```bash
  $ g++ main.cpp -o executable_name
```

HOW TO USE
--------------------------------------------------

```bash
  There are 3 options which you can run through the command line:
  * Backtracking (without pruning): ./executable_name < futoshiki.dat > output_filename.txt
  * Backtracking with Forward Checking Heuristic: ./executable_name < futoshiki.dat > output_filename.txt
  * Backtracking with Forward Checking and Minimum Remaining Values Heuristic: ./executable_name < futoshiki.dat > output_filename.txt
```

MORE INFO
--------------------------------------------------

You can find more information about task specification and the team in the files: `assignment report.pdf` and `task specification.pdf`

* What is Futoshiki? <https://en.wikipedia.org/wiki/Futoshiki>
