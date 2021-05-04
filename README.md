## ASSIGNMENT - 6
### SNEHAL KUMAR
### 2019101003

### Server
Compile and execute program by- gcc server.c -o server && ./server
Server listens for any client requesting connection, accepts and executes request from client. Sends file if available in directory to client. Receives DONE after download.
If client closes connection, server waits for other client if available to connect.

### Client
Compile and execute program by- gcc cilent.c -o client && ./client
Client connects to server, requests file by command in prompt: 
get \<filename1>\<filename2>..
Client closes connection by command in prompt: exit
note: maximum of 25 files can be requested at a time.



