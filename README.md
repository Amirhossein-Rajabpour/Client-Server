# Client-Server
Chat program written in C (multithreaded) 

To compile:
```
gcc -o client client.c -pthread
```
```
gcc -o server server.c -pthread
```
To run:
```
./server [server-port-number]
```
```
./client [server-name] [server-port-number] [client-name]
``` 
It takes input with the following commands:
```
join [group-id]
```
```
leave [group-id]
```
```
send [group-id] [message]
```
```
quit
```
