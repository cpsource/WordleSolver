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

Bug Fixes

  Fixed bug with a letter that's both grey and green in one line.

Adding Known Words

  If you find a word that's not in the wos dictionary, just add it to
  wos_words.txt.

What to expect

  Fifty percent of the time, games should end on the third try and
  ninty percent of the time, the game should end on the forth.

Note

  You don't have to use GNU aspell. Just set use_a to zero at main and recompile.
  You will get a longer list that you have to sort through.
  