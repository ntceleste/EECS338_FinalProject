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
#include <semaphore.h>

void *connection(void *args); /* client connection manager */
void *checkout(void *args); /* client checkout */
void *sell(void *args); /* client sells */
void *shop(void *args); /* client shops stores inventory */
void *cart(void *args); /* client checks their cart */
void *addMoney(void *args); /* for admin use only, adds money to user accounts */
void *checkAccount(void *args); /* returns the ammount of money in a clients account */

sem_t inventory;

// Helper function to conveniently print to stderr AND exit (terminate)
void error(const char *msg) {
    perror(msg);
    exit(1);
}

char storeInventory[50][50]; /* keep s the storefront's inventory */
int storeStock[50]; /* matches the items above and keeps track of item stock numbers */
int storePrice[50]; /* matches the items above and keeps track of the item prices */ s
int storeAccount = 9999; /* Keeps track of the stores money */

struct clientData {
  int clientID;
  char cart[10][50];
  int clientAccount;
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
    struct clientData client;
    client.clientID = args;
    char buffer[256];
    int comp;
    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "How many steps does it take for a client to change a light bulb? \n> ");
    int n = write(client.clientID, buffer, sizeof(buffer));
    bzero(buffer, sizeof(buffer));
    n = read(client.clientID, buffer, sizeof(buffer));
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


    n = write(client.clientID, buffer, sizeof(buffer));
    if (n < 0)
        error("ERROR writing to socket");
    close(newsockfd);
    pthread_exit(0);
}

void *checkout(void *args) {
  int newsockfd = args;
  char buffer[256];
  sprintf(buffer, "CHECKOUT TEST");
}

void *cart(void *args) {
  int newsockfd = args;
  char buffer[256];
  sprintf(buffer, "CART TEST");
}

void *sell(void *args) {
  int newsockfd = args;
  char buffer[256];
  sprintf(buffer, "SELL TEST");
}

void *shop(void *args) {
  int newsockfd = args;
  char buffer[256];
  sprintf(buffer, "SHOP TEST");
}

void *addMoney(void *args) {
  int newsockfd = args;
  char buffer[256];
  sprintf(buffer, "MONEY TEST");
}

void *checkAccount(void *args) {
  int newsockfd = args;
  char buffer[256];
  sprintf(buffer, "ACCOUNT AMMOUNT TEST");
}
