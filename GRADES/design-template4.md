# CS50 Design review
## Team of 4: add-drop

### Total (46/50)

#### Presentation (12/12 points)

"In 10 minutes you must present an *overview* of your design decisions and how your group will divide the work; the design document provides more detail. Be prepared to answer questions and receive feedback."

* (4/4) Good overview of the design.
* (4/4) Decomposition and data structures.
* (4/4) Plan for division of the work.

#### Document (34/38 points)

"Your design document (written in Markdown) shall describe the major design decisions, plan for testing, and the roles of each member of your group."

**Client (11/14):**

* (2/4) User interface
  - It is not explained how the third parameter to client indicates _spectator_ or _player_.
  - It is not explained that the client sends its moves to the server. THe way this is written, it sounds like the client can play alone.
* (1/2) Inputs and outputs
  - Description of possible gridpoint doesn't incluude `@` for the player or `[A-Z]` for the other players, if any
* (2/2) Functional decomposition into functions/modules
* (2/2) Major data structures
* (2/2) High-level pseudo code (plain English-like language) for logic/algorithmic flow
* (2/2) Testing plan, including unit tests, integration tests, system tests

**Server (23/24):**

* (4/4) User interface
* (3/4) Inputs and outputs
  - It is not explained that the server may receive input from and send output to many clients over the network
* (4/4) Functional decomposition into functions/modules
* (4/4) Major data structures
  - Using a hashtable is too much overhead for tracking no more than 26 players. Should use a simple lookup instead, or even obtain an index directly from the player character (e.g., 'A' = 1, 'B' = 2, ...).
* (4/4) High-level pseudo code (plain English-like language) for logic/algorithmic flow
* (4/4) Testing plan, including unit tests, integration tests, system tests

