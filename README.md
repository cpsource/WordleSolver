WordleSolver is an assistant that helps you solve Wordle puzzles.

Note: This code doesn't run yet, but will soon.

Build Instructions

  Install GNU aspell
  
  It is known to run on Ubuntu Linux

  ./make

Run Instructions

  Initialize a new game
  	     ./wos -n

  Enter a guess
  	./wos "<letter>,<color> ... repeated 4 more times

	letter [a..z]
	color green, gray, yellow

  After a guess, wos will print out possible words remaining

Why stop at six tries?

  I modified the game to give you ten tries insted of six.

Advanced strategies

  Soon, wos will sort possible words in their letter probability of
  occurring in the English language.

  Aditionally, if you are guessing, why try a word with an unknown
  double letter?
