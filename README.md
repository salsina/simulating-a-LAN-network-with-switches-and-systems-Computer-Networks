# simulating-a-LAN-network-with-switches-and-systems--Computer-Networks
An API to communicate to the switches and systems in a LAN network

This projects consists of three main files: switch.cpp, system.cpp, manage.cpp

All the API can be seen using the help command

The API consists of creating a switch, system, connecting the switches together and connecting systems to switches and sending files from a system to another system.

In this project a spanning tree algorithm has been used to find the MST(minimum spanning tree) before sending files to other systems. By doing this we can prevent the network from cycles which lead to infinit loops.

The manage.cpp file tracks the performance of network and sends commands to switches and systems. All communications are done using named pipes.
