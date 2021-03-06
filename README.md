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

Note

  I ripped off the word list from nytimes with curl and put it in file nytwords.[ch].
  I'll watch it from day to day and see if they change it.

  The idea that words don't end in 's' is quite untrue. For example, nyt uses
  rebus, truss, and floss to name a few.

Note

  If you get fedup with the game, just run getans.sh and it will print
  the answer in "solution" on the string.

  getabs.sh may have to be modified based on where your sqlite3 file is located.
  Just cd, then find to find the location.
  