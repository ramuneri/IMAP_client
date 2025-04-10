#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 143
#define SERVER "127.0.0.1"
#define BUFFER_SIZE 4096

void send_command(int sockfd, const char *cmd) {
    char buffer[BUFFER_SIZE];
    write(sockfd, cmd, strlen(cmd));
    printf("Client: %s", cmd);
    bzero(buffer, BUFFER_SIZE);
    read(sockfd, buffer, BUFFER_SIZE - 1);
    printf("Server: %s", buffer);
}

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    char username[100], password[100];
    char command[256];
    int running = 1;

    // Ask for credentials
    printf("Enter username: ");
    scanf("%99s", username);
    printf("Enter password: ");
    scanf("%99s", password);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // Resolve server
    server = gethostbyname(SERVER);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(1);
    }

    // Set up address struct
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

    // CAPABILITY
    send_command(sockfd, "A001 CAPABILITY\r\n");

    // LOGIN
    snprintf(command, sizeof(command), "A002 LOGIN %s %s\r\n", username, password);
    send_command(sockfd, command);

    // LIST mailboxes
    send_command(sockfd, "A003 LIST \"\" \"*\"\r\n");

    // Main menu loop
    while (running) {
        int choice;
        printf("\n--- IMAP Client Menu ---\n");
        printf("1. Select INBOX\n");
        printf("2. Fetch first email header\n");
        printf("3. Logout and exit\n");
        printf("Choose: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                send_command(sockfd, "A004 SELECT INBOX\r\n");
                break;
            case 2:
                send_command(sockfd, "A005 FETCH 1 BODY[HEADER]\r\n");
                break;
            case 3:
                send_command(sockfd, "A006 LOGOUT\r\n");
                running = 0;
                break;
            default:
                printf("Invalid choice.\n");
        }
    }

    close(sockfd);
    return 0;
}
