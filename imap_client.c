#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 143
#define SERVER "imap.example.com"  // Replace with real server
#define BUFFER_SIZE 4096

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // Get server by name
    server = gethostbyname(SERVER);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(1);
    }

    // Setup server address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    // Read greeting
    bzero(buffer, BUFFER_SIZE);
    read(sockfd, buffer, BUFFER_SIZE - 1);
    printf("Server: %s", buffer);

    // Send CAPABILITY command
    char *command = "A001 CAPABILITY\r\n";
    write(sockfd, command, strlen(command));
    printf("Client: %s", command);

    // Read response
    bzero(buffer, BUFFER_SIZE);
    read(sockfd, buffer, BUFFER_SIZE - 1);
    printf("Server: %s", buffer);

    // Close socket
    close(sockfd);
    return 0;
}
