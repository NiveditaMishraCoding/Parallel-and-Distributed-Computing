#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(){

    // Create a socket for communication
    int  network_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Specify an address for the socket
    struct sockaddr_in server_address;
    
    // Using IPv4 protocol
    server_address.sin_family = AF_INET;
    
    // Using port 9002
    server_address.sin_port = htons(9002);
    
    // IP address of the server to connect to
    server_address.sin_addr.s_addr = inet_addr("192.168.77.4");

    // Connect the socket to the specified IP and port
    int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    
    // To store the response from the server
    char response[256];
    
    // Receive data from the server
    recv(network_socket, &response, sizeof(response), 0);
    
    // Print the data received from the server
    printf("The server sent: %s \n", response);

    // Close the socket
    close(network_socket);

    return 0;
}
