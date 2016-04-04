
Project title:

Use sockets and TCP/IP communication to play a distributed game of "hot potato."

Description:

Hot Potato is a children's game in which a "potato" is rapidly tossed around until, at some arbitrary point, the game ends. The child holding the potato is "it." (One does not want to be it.) In this assignment you will create a ring of "player" processes that will pass the potato around; therefore, each player will have a left and a right neighbor. Furthermore, you will create a "ringmaster" process that will start each game, report the results, and shut down the network.
To begin, the ringmaster creates a potato with some number of hops and sends it to a randomly selected player. Each time a player receives the potato, it will decrement the number of hops and append its identity on the potato. If the remaining number of hops is greater than zero, it will randomly select a neighbor and send the potato to that neighbor. The game ends when the hops counter reaches zero. The player holding the potato sends it to the ringmaster indicating the end of this game. The ringmaster prints a trace of the game (from the identities that are appended to the potato) to the screen, and shuts down the network. The trace consists of the path the potato took (that is, the players in the order they held the potato).

Each player process will create three connections: left, right, and ringmaster. The potato can arrive on any of the three connections. Commands and important information may also be received from the ringmaster. The ringmaster will create n connections, one for each player. At the end of a game the ringmaster will receive the potato from the player who is "it."

The assignment is to create one ringmaster and some number of player processes, then play a game and terminate all processes gracefully. You may explicitly create each process from an interactive shell; however, the player processes must exit themselves in response to commands from the ringmaster.

The programs will use Unix sockets for communication.

Establishing communication between all processes will be the most difficult part of the assignment. Therefore, you should start small and work up to the full ring topology.

Your programs must use exactly the format described here. The ringmaster program is invoked as shown below (the words and the angled brackets are figurative, and should be replaced with an appropriate value):

    master <port-number> <number-of-players> <hops>
where number-of-players is greater than one and hops is non-negative.
The player programs are invoked thusly:

    player <master-machine-name> <port-number>

where master-machine-name is the name of the machine on which master was invoked and port-number is the same as was passed to master. Players are numbered beginning with 0. The players are connected in the ring such that the left neighbor of player n is player n-1 and the right neighbor is player n+1. (Player 0 is the right neighbor of last player (number_of_players-1).
Zero (0) is a valid number of hops. In this case, your program must create the ring of processes. After the ring is created, the ringmaster shuts down the game.


The following describes all the output of the master program. 

Initially
    Potato Master on <host-name>
    Players = <number>
    Hops = <number>

Upon connection with a player
    player <number> is on <host-name>

Number players beginning with 0.
When first launching a potato
    All players present, sending potato to player <number>

The player that initially is sent the potato is selected at random.
When it gets the potato back (at end of game)
    Trace of potato:
    <n>,<n>,...

Trace is a comma separated list of player numbers. No spaces or newlines in the list.
The following describes all the output of the player program. Do not have any other output.

After establishing a port with master
    Connected as player <number>

When forwarding potato (to another player)
    Sending potato to <number>

When number of hops reached
     I'm it

