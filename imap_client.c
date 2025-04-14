#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 143                // Dovecot port 
#define SERVER "127.0.0.1"
#define BUFFER_SIZE 4096
int tag_counter = 1;

// Send a command from a client to a server over a socket connection
void send_command(int sockfd, const char *cmd_body, int *tag_counter_ptr) {    // sockfd - file descriptor for the socket - active connection to the server
    char full_command[512];
    char tag[10];
    char buffer[BUFFER_SIZE];

    snprintf(tag, sizeof(tag), "TAG%03d", (*tag_counter_ptr)++);                // generate a tag

    snprintf(full_command, sizeof(full_command), "%s %s\r\n", tag, cmd_body);   // build a full command wit a tag
    
    write(sockfd, full_command, strlen(full_command));      // sends the command to the server via the socket
    printf("Client: %s", full_command);                     // what was just sent to the server by client
    
    bzero(buffer, BUFFER_SIZE);                             // clears out the buffer before reading the server's response
    read(sockfd, buffer, BUFFER_SIZE - 1);                  // reads the server's response from the socket into the buffer
    printf("Server: %s", buffer);                           // response from the server
}

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    char username[20], password[20];
    char command[50];
    int running = 1;

    printf("Enter username: ");
    scanf("%20s", username);
    printf("Enter password: ");
    scanf("%20s", password);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket - could not create a socket\n");
        exit(1);
    }

    // Resolve server
    server = gethostbyname(SERVER);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host - no such server\n");
        exit(1);
    }

    // Set up address struct
    bzero((char *) &serv_addr, sizeof(serv_addr));      // clears out the serv_addr structure
    serv_addr.sin_family = AF_INET;                     // IPv4 (make 6 later)
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(PORT);                   // sets the port number for the connection

    // Connect to server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    // If client connects, DOvecot sends a greeting
    bzero(buffer, BUFFER_SIZE);
    read(sockfd, buffer, BUFFER_SIZE - 1);
    printf("Server: %s", buffer);

    // CAPABILITY
    send_command(sockfd, "CAPABILITY", &tag_counter);

    // LOGIN
    char login_command_body[256];
    snprintf(login_command_body, sizeof(login_command_body), "LOGIN %s %s", username, password);
    send_command(sockfd, login_command_body, &tag_counter);

    // LIST mailboxes
    send_command(sockfd, "LIST \"\" \"*\"", &tag_counter);

    while (running) {
        int choice;
        printf("\n-------------- IMAP Client Menu --------------\n");
        printf("1. Select INBOX\n");
        printf("2. Fetch first email header\n");
        printf("3. Logout and exit\n");
        printf("Choose: ");
        scanf("%d", &choice);
        printf("\n");

        switch (choice) {
            case 1:
                send_command(sockfd, "SELECT INBOX", &tag_counter);
                break;
            case 2:
                send_command(sockfd, "FETCH 1 BODY[HEADER]", &tag_counter);
                break;
            case 3:
                send_command(sockfd, "LOGOUT", &tag_counter);
                running = 0;
                break;
            default:
                printf("Invalid choice.\n");
        }
    }

    close(sockfd);
    return 0;
}
