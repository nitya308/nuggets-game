# CS50 Functionally correct and complete
## Team of 4: add-drop

### Total (47/50)

We grade this section out of 50 points.

### Teams of 4 students:

#### Server (30/32)

  * (2/2) server commandline, per spec
  * (8/8) server supports one player, per spec
  * (6/8) server supports multiple players, per spec
	-  some move sequences on `challenge.txt` map result in random movement, typically after a H or L (capital) it gets confused and you must hit another key to make the move.
	
  * (2/2) server supports spectator, per spec
  * (6/6) server supports 'visibility' spec
  * (2/2) server tracks gold, per spec
  * (2/2) server produces Game Over summary, per spec
  * (2/2) new, valid, non-trivial mapfile

#### Client (17/18)
  * (1/2) client commandline, per spec
    - No "**useful** error message" provided for bad or missing arguments.
  * (6/6) client plays as player, per spec
  * (6/6) client plays as spectator, per spec
  * (2/2) client asks for window to grow, per spec
  * (2/2) client prints Game Over summary, per spec
