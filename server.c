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
void *checkAccount(void *args); /* returns the amount of money in a clients account */

sem_t inventoryFlag;

// Helper function to conveniently print to stderr AND exit (terminate)
void error(const char *msg) {
    perror(msg);
    exit(1);
}

char storeInventory[10][50]; /* keep s the storefront's inventory */
int storeStock[10]; /* matches the items above and keeps track of item stock numbers */
int storePrice[10]; /* matches the items above and keeps track of the item prices */
//do we really care?
int storeAccount = 9999; /* Keeps track of the stores money */

struct clientData {
    int clientID; /* keeps track of the clients login ID, this is unique to each client connection */
    char cart[5][50]; /* holds the items added to the clients cart  */
    int clientAccount; /* holds the ammount of money in the clients cart */
};

int main(int argc, char *argv[]) {
    // Check for proper number of commandline arguments
    // Expect program name in argv[0], port # in argv[1]
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    //set up semaphore
    /*if(sem_init(&inventory, 0, 1) < 0) { // 0 = multithreaded
        fprintf(stderr, "ERROR: could not initialize &inventory.\n");
        return -1;
    } */

    //set up inventory to be empty
    int i;
    for(i = 0; i < 10; i++){
        strcpy(storeInventory[i], "empty");
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
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_t tid; /* the thread identifiers */
            pthread_create(&tid, &attr, connection, &newsockfd);
        }
    }
}

void *connection(void *args) {
    int i;
    //setting up client
    struct clientData client;
    client.clientID =  *(int*)args;
    for(i = 0; i < 5; i++){
        strcpy(client.cart[i], "empty");
    }
    printf("Connection Opened - ClientID: %d\n", client.clientID);
    char buffer[256];
    int comp;
    int runFlag = 1;
    while(runFlag) {
        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "What would you like to do?");
        int n = write(client.clientID, buffer, sizeof(buffer));
        bzero(buffer, sizeof(buffer));
        n = read(client.clientID, buffer, sizeof(buffer));
        if (n < 0)
            error("ERROR reading from socket");


        if (strcmp(buffer, "exit\n") == 0) {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "goodbye.\n");
            runFlag = 0;
        } else if (strcmp(buffer, "shop\n") == 0) {
            pthread_t tid; /* the thread identifiers */
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, shop, &client);
            pthread_join(tid, NULL);
        } else if (strcmp(buffer, "sell\n") == 0) {
            pthread_t tid; /* the thread identifiers */
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, sell, &client);
            pthread_join(tid, NULL);
        } else if (strcmp(buffer, "cart\n") == 0) {
            pthread_t tid; /* the thread identifiers */
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, cart, &client);
            pthread_join(tid, NULL);
        } else if (strcmp(buffer, "checkout\n") == 0) {
            pthread_t tid; /* the thread identifiers */
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, checkout, &client);
            pthread_join(tid, NULL);
        } else if (strcmp(buffer, "check account\n") == 0) {
            pthread_t tid; /* the thread identifiers */
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, checkAccount, &client);
            pthread_join(tid, NULL);
        } else {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "please enter a valid command.\n");
        }
        n = write(client.clientID, buffer, sizeof(buffer));
        if (n < 0)
            error("ERROR writing to socket");
    }
    close(client.clientID);
    printf("Connection Closed - ClientID: %d\n", client.clientID);
    pthread_exit(0);
}

void *checkout(void *args) {
    struct clientData *client;
    client = (struct clientData*) args;
    char buffer[256];
    int comp;
    int runFlag = 1;

    printf("Checkout Entered - ClientID: %d\n", client->clientID);

    printf("Checkout Exited - ClientID: %d\n", client->clientID);
    pthread_exit(0);
}

void *cart(void *args) {
    struct clientData *client;
    client = (struct clientData*) args;
    char buffer[256];
    int comp;
    int runFlag = 1;

    printf("Cart Entered - ClientID: %d\n", client->clientID);

    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "Welcome to your cart.");
    int n = write(client->clientID, buffer, sizeof(buffer));
    if (n < 0){
        error("ERROR writing to socket: cart 1");
    }

    while(runFlag) {

        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "What do you want to do with your cart?");
        n = write(client->clientID, buffer, sizeof(buffer));
        if (n < 0){
            error("ERROR writing to socket: cart 2");
        }

        bzero(buffer, sizeof(buffer));
        n = read(client->clientID, buffer, sizeof(buffer));
        if (n < 0){
            error("ERROR reading from socket: cart 1");
        }
        if (strcmp(buffer, "exit cart\n") == 0) {
            bzero(buffer, sizeof(buffer));
            runFlag = 0;
        } else if (strcmp(buffer, "check cart\n") == 0) {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "%s\n"
                            "%s\n"
                            "%s\n"
                            "%s\n"
                            "%s\n",
                    client->cart[0],
                    client->cart[1],
                    client->cart[2],
                    client->cart[3],
                    client->cart[4]);

            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: cart 3");
            }
        } else if (strcmp(buffer, "remove\n") == 0) {

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "Removing from cart.\n");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: cart 4");
            }

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "What you like to remove?\n");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: cart 5");
            }

            bzero(buffer, sizeof(buffer));
            n = read(client->clientID, buffer, sizeof(buffer));
            if (n < 0) {
                error("ERROR reading from socket: cart 2");
            }

            int i;
            for(i = 0; i < 10; i++){
                if(strcmp(strcat(client->cart[i], "\n"), buffer) == 0){
                    strcpy(client->cart[i], "empty");
                }
            }

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "We've removed the requested Item.\n");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: cart 4");
            }

        }
    }

    printf("Cart Exited - ClientID: %d\n", client->clientID);
    pthread_exit(0);
}

void *sell(void *args) {
    struct clientData *client;
    client = (struct clientData*) args;
    char buffer[256];
    int comp;
    int runFlag = 1;

    printf("Sell Entered - ClientID: %d\n", client->clientID);

    printf("Sell Exited - ClientID: %d\n", client->clientID);
    pthread_exit(0);
}

void *shop(void *args) {
    struct clientData *client;
    client = (struct clientData*) args;
    char buffer[256];
    int comp;
    int runFlag = 1;


    //sem_wait(&inventory);

    printf("Shop Entered - ClientID: %d\n", client->clientID);
    while(runFlag) {
        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "%s\n"
                "%s\n"
                "%s\n"
                "%s\n"
                "%s\n"
                "%s\n"
                "%s\n"
                "%s\n"
                "%s\n"
                "%s\n",
                storeInventory[0],
                storeInventory[1],
                storeInventory[2],
                storeInventory[3],
                storeInventory[4],
                storeInventory[5],
                storeInventory[6],
                storeInventory[7],
                storeInventory[8],
                storeInventory[9]);
        int n = write(client->clientID, buffer, sizeof(buffer));
        if (n < 0){
            error("ERROR writing to socket: Shop 1");
        }

        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "What do you want to buy?");
        n = write(client->clientID, buffer, sizeof(buffer));
        if (n < 0){
            error("ERROR writing to socket: Shop 2");
        }

        bzero(buffer, sizeof(buffer));
        n = read(client->clientID, buffer, sizeof(buffer));
        if (n < 0){
            error("ERROR reading from socket: Shop 1");
        }
        if (strcmp(buffer, "exit shop\n") == 0) {
            bzero(buffer, sizeof(buffer));
            runFlag = 0;
        }

    }

    //sem_post(&inventory);
    printf("Shop Exited - ClientID: %d\n", client->clientID);
    pthread_exit(0);
}


//Should the only way to add money be to sell stuff? I think that makes sense, and simplifies life...
void *addMoney(void *args) {
    struct clientData *client;
    client = (struct clientData*) args;
    char buffer[256];
    sprintf(buffer, "MONEY TEST");
}

void *checkAccount(void *args) {
    struct clientData *client;
    client = (struct clientData *) args;
    char buffer[256];
    int comp;
    int runFlag = 1;
    printf("Account Entered - ClientID: %d\n", client->clientID);

    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "Welcome to your account!");
    int n = write(client->clientID, buffer, sizeof(buffer));
    if (n < 0) {
        error("ERROR writing to socket: account 1");
    }

    while (runFlag) {

        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "What do you want to check?");
        n = write(client->clientID, buffer, sizeof(buffer));
        if (n < 0) {
            error("ERROR writing to socket: account 1");
        }

        bzero(buffer, sizeof(buffer));
        n = read(client->clientID, buffer, sizeof(buffer));
        if (n < 0) {
            error("ERROR reading from socket: account 1");
        }
        if (strcmp(buffer, "exit account\n") == 0) {
            bzero(buffer, sizeof(buffer));
            runFlag = 0;
        } else if (strcmp(buffer, "id\n") == 0) {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "Here's your ID: %d\n", client->clientID);
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0) {
                error("ERROR writing to socket: account 3");
            }
        } else if (strcmp(buffer, "balance\n") == 0) {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "Here's your account balance: %d\n", client->clientAccount);
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0) {
                error("ERROR writing to socket: account 4");
            }
        }
    }
    printf("Account Exited - ClientID: %d\n", client->clientID);
    pthread_exit(0);
}