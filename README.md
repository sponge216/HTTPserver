HTTP server written in C, using the WinAPI's WinSock2 and processthreadsapi libraries.
The server handles GET requests from clients using a multi-threaded approach. Whenever a connection is made, a thread is created that handles all matters pertaining to said request

Side note, this project was created as a fun way to understand the behind-the-scenes of how servers operate and how they handle requests.
