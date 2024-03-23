# Socket Demonstration in C

Demonstrates communication between a server and a client over a network using low level sockets on Linux systems.
## How to run the programs
* Copy the program folder to your desired location.
* Navigate to the folder and launch a terminal from the folder location. You can also you the cd command from the command line.
* Compile client.c and server.c using the make command as bellow
```bash
make
```
This should output two files: server and client.
* Make startServer.sh and startClient.sh executable with the following command:
```bash
chmod u+x *.sh
```
* Start the server, passing it a port number. Any unused port number will do.
```bash
./startServer.sh 1234
```
* Start the client, passing it host name and a port number.
```bash
./startClient.sh localhost 1234
```
On start, the client requests for a username to connect to the server with.
```bash
Enter username: 
```
Username must not contain space character(s).

Once a connection has been established. You can interact with the server with a few commands.

* `READ` 
fetches the next message for the user if one exits.
* `COMPOSE <username>` Tells the server that the user wants to send a message to the user `<username>`.
* The compose command can be followed by a message(s) to the user.
```bash
>>> COMPOSE francis
>>> Hi Francis. How are you?
```
* `EXIT` Closes both server and client instance.

The server sends replies according to the commands it recieives.

**NOTE:** An unknown command will cause the server to close the connection.

