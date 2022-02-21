WordleSolver is an assistant that helps you solve Wordle puzzles.

Build Instructions

  Install GNU aspell
  
  It is known to run on Ubuntu Linux

  make

Run Instructions

  Initialize a new game
  	     ./wos -n

  Enter a guess
  	./wos "<letter>:<color initial> , repeated 4 more times"

	letter [a..z]
	color green, yellow. Note that grey is default and not entered

  Example:

	./wos "w:y , a , d , d , y"

	Here, the letter 'w' will be yellow, and the rest will be grey

  After a guess, wos will print out possible words remaining along with
  a letter frequency metric.

  Note also wos produces ans.txt to keep a record of the game.

To play

  First, enter your guess into the web version of Wordle at
  https://www.nytimes.com/games/wordle

  Then, run wos and enter the letters and colors you received from
  the web version.



