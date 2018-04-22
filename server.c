/**
 * Nate Celeste - NTC14
 * Clancy Mitchell - CCM81
 * EECS 338 - Final Project Server Side
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


void *connection(); /* child thread */

// Helper function to conveniently print to stderr AND exit (terminate)
void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    // Check for proper number of commandline arguments
    // Expect program name in argv[0], port # in argv[1]
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // Setup phase
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    struct sockaddr_in serv_addr; // Server address struct
    bzero((char *) &serv_addr, sizeof(serv_addr));
    int portno = atoi(argv[1]); // Port number is commandline argument
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);

    printf("I'm Listening\n");

    while(1) {
        // Service phase
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr); // Address struct length
        int newsockfd = accept(sockfd,
                               (struct sockaddr *) &cli_addr,
                               &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        else {
            pthread_t tid; /* the thread identifiers */
            pthread_create(&tid, NULL, connection, newsockfd);
        }
    }


}

void *connection(void *args) {
    int newsockfd = args;
    char buffer[256];
    int comp;
    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "How many steps does it take for a client to change a light bulb? \n> ");
    int n = write(newsockfd, buffer, sizeof(buffer));
    bzero(buffer, sizeof(buffer));
    n = read(newsockfd, buffer, sizeof(buffer));
    if (n < 0)
        error("ERROR reading from socket");

    comp = atoi(buffer);
    if (comp == 3) {
        sprintf(buffer, "Correct! The 3 steps are: create a socket, connect to it, and send a new bulb.\n");
        printf("Client was correct!\n");
    }

    else {
        sprintf(buffer, "Nope. The 3 steps are: create a socket, connect to it, and send a new bulb.\n");
        printf("Client was wrong!\n");
    }


    n = write(newsockfd, buffer, sizeof(buffer));
    if (n < 0)
        error("ERROR writing to socket");
    close(newsockfd);
    pthread_exit(0);
}
