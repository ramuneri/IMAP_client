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

int tag_counter = 1;

int send_command(int sockfd, const char *cmd_body, int *tag_counter_ptr) {
    char full_command[512];
    char tag[10];
    char buffer[BUFFER_SIZE];

    snprintf(tag, sizeof(tag), "TAG%03d", (*tag_counter_ptr)++);
    snprintf(full_command, sizeof(full_command), "%s %s\r\n", tag, cmd_body);

    write(sockfd, full_command, strlen(full_command));
    printf("Client: %s", full_command);

    bzero(buffer, BUFFER_SIZE);
    read(sockfd, buffer, BUFFER_SIZE - 1);
    printf("Server: %s", buffer);

    return strstr(buffer, "OK") != NULL;
}

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    char username[20], password[20];
    char command[256];
    int running = 1;
    int inbox_selected = 0;

    printf("Enter username: ");
    scanf("%20s", username);
    printf("Enter password: ");
    scanf("%20s", password);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(SERVER);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    bzero(buffer, BUFFER_SIZE);
    read(sockfd, buffer, BUFFER_SIZE - 1);
    printf("Server: %s", buffer);

    send_command(sockfd, "CAPABILITY", &tag_counter);

    snprintf(command, sizeof(command), "LOGIN %s %s", username, password);
    if (!send_command(sockfd, command, &tag_counter)) {
        fprintf(stderr, "Authentication failed. Exiting.\n");
        close(sockfd);
        return 1;
    }

    while (running) {
        int choice;
        printf("\n\n\n-------------- IMAP Client Menu --------------\n");
        printf("1. Show folder structure (LIST)\n");
        printf("2. Create new folder\n");
        printf("3. Delete a folder\n");
        printf("4. Rename a folder\n");
        printf("------------------\n");
        printf("5. Select INBOX\n");
        printf("6. List all message numbers in INBOX\n");
        printf("7. Fetch specific email header\n");
        printf("8. Fetch specific email body\n");
        printf("9. Mark email as seen\n");
        printf("10. Delete email\n");
        printf("------------------\n");
        printf("11. Send raw IMAP command\n");
        printf("12. Logout and exit\n");
        printf("Choose: ");
        scanf("%d", &choice);
        getchar();

        printf("\n\n");

        if (choice >= 6 && choice <= 10 && !inbox_selected) {
            printf("Please select INBOX first (Option 5).\n");
            continue;
        }

        switch (choice) {
            case 1:
                send_command(sockfd, "LIST \"\" \"*\"", &tag_counter);
                break;
            case 2:
                printf("Enter new folder name: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = 0;
                snprintf(buffer, sizeof(buffer), "CREATE %s", command);
                send_command(sockfd, buffer, &tag_counter);
                break;
            case 3:
                printf("Enter folder name to delete: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = 0;
                snprintf(buffer, sizeof(buffer), "DELETE %s", command);
                send_command(sockfd, buffer, &tag_counter);
                break;
            case 4: {
                char new_name[256];
                printf("Enter folder to rename: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = 0;
                printf("Enter new folder name: ");
                fgets(new_name, sizeof(new_name), stdin);
                new_name[strcspn(new_name, "\n")] = 0;
                snprintf(buffer, sizeof(buffer), "RENAME %s %s", command, new_name);
                send_command(sockfd, buffer, &tag_counter);
                break;
            }
            case 5:
                printf("Fetching available mailboxes...\n");
                send_command(sockfd, "LIST \"\" \"*\"", &tag_counter);
                printf("\nEnter mailbox to select (ex.: INBOX/Work): ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = 0;
                snprintf(buffer, sizeof(buffer), "SELECT %s", command);
                if (send_command(sockfd, buffer, &tag_counter)) {
                    inbox_selected = 1;
                    snprintf(buffer, sizeof(buffer), "FETCH 1:* (FLAGS)");
                    send_command(sockfd, buffer, &tag_counter);
                } else {
                    printf("Failed to select mailbox '%s'.\n", command);
                }
                break;
            case 6:
                send_command(sockfd, "FETCH 1:* (FLAGS)", &tag_counter);
                break;
            case 7: {
                int msg_id;
                printf("Enter message number: ");
                scanf("%d", &msg_id);
                getchar();
                snprintf(command, sizeof(command), "FETCH %d BODY[HEADER]", msg_id);
                send_command(sockfd, command, &tag_counter);
                break;
            }
            case 8: {
                int msg_id;
                printf("Enter message number: ");
                scanf("%d", &msg_id);
                getchar();
                snprintf(command, sizeof(command), "FETCH %d BODY[TEXT]", msg_id);
                send_command(sockfd, command, &tag_counter);
                break;
            }
            case 9: {
                int msg_id;
                printf("Enter message number to mark as seen: ");
                scanf("%d", &msg_id);
                getchar();
                snprintf(command, sizeof(command), "STORE %d +FLAGS (\\Seen)", msg_id);
                send_command(sockfd, command, &tag_counter);
                break;
            }
            case 10: {
                int msg_id;
                printf("Enter message number to delete: ");
                scanf("%d", &msg_id);
                getchar();
                snprintf(command, sizeof(command), "STORE %d +FLAGS (\\Deleted)", msg_id);
                send_command(sockfd, command, &tag_counter);
                send_command(sockfd, "EXPUNGE", &tag_counter);
                break;
            }
            case 11:
                printf("Enter raw IMAP command: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = 0;
                send_command(sockfd, command, &tag_counter);
                break;
            case 12:
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
