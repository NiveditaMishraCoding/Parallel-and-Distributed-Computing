#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main(){

    // Initialize message to be sent
    char message[256] = "hello Sockets!";

    // Create a server socket using TCP (SOCK_STREAM)
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Define the address structure for the server
    struct sockaddr_in server_address;

    // Specify the address family as IPv4
    server_address.sin_family = AF_INET;

    // Set the server port to 9002 and convert it to network byte order
    server_address.sin_port = htons(9002);

    // Set the server IP address
    server_address.sin_addr.s_addr = inet_addr("192.168.77.4");

    // Bind the socket to the defined IP and port
    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    // Listen for client connections with a maximum backlog of 5
    listen(server_socket, 5);

    // Accept client connection
    int client_socket = accept(server_socket, NULL, NULL);

    // Send the predefined message to the client
    send(client_socket, message, sizeof(message), 0);

    // Close the server socket
    close(server_socket);

    // Return 0 for successful execution
    return 0; 
}
