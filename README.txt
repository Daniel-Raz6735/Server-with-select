Name: Daniel Raz 

to activate program: open linux terminal, navigate to ex4 executeable file location using "cd" command (confirm it using ls command) and type 
valgrind ./chatserver <port> <max_clients>.


list of submitted files:
chatserver.c : Implementation of a simple TCP chat server that all it does is to read text from clients and send them back to them..


private function:
creat_socket(int port) - //this function create and return the main socket, else return -1 if there is any error.

