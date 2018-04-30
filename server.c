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
    int clientAccount; /* holds the amount of money in the clients cart */
    int cartStock[5];
    int cartPrice[5];
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
        storeStock[i] = 0;
        storePrice[i] = 0;
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
        client.cartPrice[i] = 0;
        client.cartStock[i] = 0;
    }

    printf("Connection Opened - ClientID: %d\n", client.clientID);
    char buffer[1028];
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
            bzero(buffer, sizeof(buffer));
        } else if (strcmp(buffer, "sell\n") == 0) {
            pthread_t tid; /* the thread identifiers */
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, sell, &client);
            pthread_join(tid, NULL);
            bzero(buffer, sizeof(buffer));
        } else if (strcmp(buffer, "cart\n") == 0) {
            pthread_t tid; /* the thread identifiers */
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, cart, &client);
            pthread_join(tid, NULL);
            bzero(buffer, sizeof(buffer));
        } else if (strcmp(buffer, "checkout\n") == 0) {
            pthread_t tid; /* the thread identifiers */
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, checkout, &client);
            pthread_join(tid, NULL);
            bzero(buffer, sizeof(buffer));
        } else if (strcmp(buffer, "check account\n") == 0) {
            pthread_t tid; /* the thread identifiers */
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, checkAccount, &client);
            pthread_join(tid, NULL);
            bzero(buffer, sizeof(buffer));
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
    char buffer[1028];
    int comp;
    int runFlag = 1;
    int i;

    printf("Checkout Entered - ClientID: %d\n", client->clientID);

    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "Welcome to checkout.");
    int n = write(client->clientID, buffer, sizeof(buffer));
    if (n < 0){
        error("ERROR writing to socket: checkout 1");
    }

    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "Are you ready to checkout?");
    n = write(client->clientID, buffer, sizeof(buffer));
    if (n < 0){
        error("ERROR writing to socket: checkout 2");
    }

    bzero(buffer, sizeof(buffer));
    n = read(client->clientID, buffer, sizeof(buffer));
    if (n < 0){
        error("ERROR reading from socket: checkout 1");
    }

    if (strcmp(buffer, "yes\n") != 0) {
        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "We're sorry.");
        n = write(client->clientID, buffer, sizeof(buffer));
        if (n < 0){
            error("ERROR writing to socket: checkout 3");
        }

        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "please come back when you are ready to checkout.");
        n = write(client->clientID, buffer, sizeof(buffer));
        if (n < 0){
            error("ERROR writing to socket: checkout 4");
        }

        bzero(buffer, sizeof(buffer));
        n = read(client->clientID, buffer, sizeof(buffer));
        if (n < 0){
            error("ERROR reading from socket: checkout 1");
        }
    } else {
        int totalCartPrice;
        for (i = 0; i < 5; i++){
            totalCartPrice = totalCartPrice + (client->cartPrice[i] * client->cartStock[i]);
        }

        if (totalCartPrice > client->clientAccount){
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "You have insufficient to complete this purchase");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: checkout 5");
            }

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "please remove items or sell items");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: checkout 6");
            }

            bzero(buffer, sizeof(buffer));
            n = read(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR reading from socket: checkout 1");
            }
        } else {
            for(i = 0; i < 5; i ++){
                if(strcmp(client->cart[i], "empty") != 0){
                    strcpy(client->cart[i], "empty");
                    client->cartStock[i] = 0;
                    client->cartPrice[i] = 0;
                }
            }
            client->clientAccount = client->clientAccount - totalCartPrice;

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "Purchase successfull");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: checkout 6");
            }

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "Your items will be shipped out shortly");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: checkout 6");
            }

            bzero(buffer, sizeof(buffer));
            n = read(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR reading from socket: checkout 1");
            }
        }
    }

    printf("Checkout Exited - ClientID: %d\n", client->clientID);
    pthread_exit(0);
}

void *cart(void *args) {
    struct clientData *client;
    client = (struct clientData*) args;
    char buffer[1028];
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
            sprintf(buffer, "Item: %s, Quantity: %d\n"
                            "Item: %s, Quantity: %d\n"
                            "Item: %s, Quantity: %d\n"
                            "Item: %s, Quantity: %d\n"
                            "Item: %s, Quantity: %d\n",
                    client->cart[0], client->cartStock[0],
                    client->cart[1], client->cartStock[1],
                    client->cart[2], client->cartStock[2],
                    client->cart[3], client->cartStock[3],
                    client->cart[4], client->cartStock[4]);

            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: cart 3");
            }
        } else if (strcmp(buffer, "remove\n") == 0) {

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "Removing from cart.");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: cart 4");
            }

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "What item would you like to remove?");
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
            int itemKey;
            char tempCart[5][50];

            for(i = 0; i < 5; i++){
                strcat(tempCart[i], client->cart[i]);
                if(strcmp(strcat(tempCart[i], "\n"), buffer) == 0){
                    itemKey = i;
                }
            }


            for(i = 0; i < 10; i++){
                if(strcmp(client->cart[itemKey], storeInventory[i]) == 0){
                    storeStock[i] = storeStock[i] + client->cartStock[itemKey];
                }
            }

            strcpy(client->cart[itemKey], "empty");
            client->cartStock[itemKey] = 0;

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "We've removed the requested Item.");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: cart 4");
            }

        } else {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "please enter a valid command.");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: Shop 3");
            }
        }
    }

    printf("Cart Exited - ClientID: %d\n", client->clientID);
    pthread_exit(0);
}

void *sell(void *args) {
    struct clientData *client;
    client = (struct clientData*) args;
    char buffer[1028];
    int comp;
    int runFlag = 1;
    int i;

    printf("Sell Entered - ClientID: %d\n", client->clientID);

    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "Welcome to the sell page!");
    int n = write(client->clientID, buffer, sizeof(buffer));
    if (n < 0) {
        error("ERROR writing to socket: account 1");
    }

    while (runFlag) {

        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "What do you want to do on the sell page?");
        n = write(client->clientID, buffer, sizeof(buffer));
        if (n < 0) {
            error("ERROR writing to socket: account 1");
        }

        bzero(buffer, sizeof(buffer));
        n = read(client->clientID, buffer, sizeof(buffer));
        if (n < 0) {
            error("ERROR reading from socket: account 1");
        }
        if (strcmp(buffer, "exit sell\n") == 0) {
            bzero(buffer, sizeof(buffer));
            runFlag = 0;
        } else if (strcmp(buffer, "sell\n") == 0) {

            int openSlot = -1;

            for(i = 0; i < 10; i ++){
                if(strcmp(storeInventory[i], "empty") == 0){
                    openSlot = i;
                    break;
                }
            }

            if(openSlot == -1){
                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "The store's inventory is currently full.");
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0){
                    error("ERROR writing to socket: Shop 3");
                }
            } else {
                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "The store's inventory is not currently full.");
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0){
                    error("ERROR writing to socket: Shop 3");
                }

                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "What would you like to sell?");
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0){
                    error("ERROR writing to socket: Shop 3");
                }

                bzero(buffer, sizeof(buffer));
                n = read(client->clientID, buffer, sizeof(buffer));
                if (n < 0) {
                    error("ERROR reading from socket: account 1");
                }


                buffer[strlen(buffer) - 1] = 0;
                strcpy(storeInventory[openSlot], buffer);

                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "Added item to the iventory");
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0){
                    error("ERROR writing to socket: Shop 3");
                }

                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "How many would you like to sell?");
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0){
                    error("ERROR writing to socket: Shop 3");
                }

                bzero(buffer, sizeof(buffer));
                n = read(client->clientID, buffer, sizeof(buffer));
                if (n < 0) {
                    error("ERROR reading from socket: account 1");
                }

                int amountToSell = atoi(buffer);
                storeStock[openSlot] = amountToSell;

                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "Stock updated.");
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0){
                    error("ERROR writing to socket: Shop 3");
                }

                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "How much would you like to sell each item for?");
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0){
                    error("ERROR writing to socket: Shop 3");
                }

                bzero(buffer, sizeof(buffer));
                n = read(client->clientID, buffer, sizeof(buffer));
                if (n < 0) {
                    error("ERROR reading from socket: account 1");
                }

                int priceToSell = atoi(buffer);
                storePrice[openSlot] = priceToSell;

                client->clientAccount = client->clientAccount + (priceToSell * amountToSell);

                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "Total sale amount of %d has been added to your account.", (priceToSell * amountToSell));
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0){
                    error("ERROR writing to socket: Shop 3");
                }
            }

        } else {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "please enter a valid command.");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: Shop 3");
            }
        }
    }

    printf("Sell Exited - ClientID: %d\n", client->clientID);
    pthread_exit(0);
}

void *shop(void *args) {
    struct clientData *client;
    client = (struct clientData*) args;
    char buffer[1028];
    int comp;
    int runFlag = 1;


    //sem_wait(&inventory);
    printf("Shop Entered - ClientID: %d\n", client->clientID);

    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "Welcome to the shop!");
    int n = write(client->clientID, buffer, sizeof(buffer));
    if (n < 0){
        error("ERROR writing to socket: Shop 1");
    }

    while(runFlag) {

        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "What do you want to do in the shop?");
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
        } else if ( strcmp(buffer, "view inventory\n") == 0) {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "Item: %s, Price: $%d, Quantity: %d\n"
                            "Item: %s, Price: $%d, Quantity: %d\n"
                            "Item: %s, Price: $%d, Quantity: %d\n"
                            "Item: %s, Price: $%d, Quantity: %d\n"
                            "Item: %s, Price: $%d, Quantity: %d\n"
                            "Item: %s, Price: $%d, Quantity: %d\n"
                            "Item: %s, Price: $%d, Quantity: %d\n"
                            "Item: %s, Price: $%d, Quantity: %d\n"
                            "Item: %s, Price: $%d, Quantity: %d\n"
                            "Item: %s, Price: $%d, Quantity: %d\n",
                    storeInventory[0], storePrice[0], storeStock[0],
                    storeInventory[1], storePrice[1], storeStock[1],
                    storeInventory[2], storePrice[2], storeStock[2],
                    storeInventory[3], storePrice[3], storeStock[3],
                    storeInventory[4], storePrice[4], storeStock[4],
                    storeInventory[5], storePrice[5], storeStock[5],
                    storeInventory[6], storePrice[6], storeStock[6],
                    storeInventory[7], storePrice[7], storeStock[7],
                    storeInventory[8], storePrice[8], storeStock[8],
                    storeInventory[9], storePrice[9], storeStock[9]);

            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: Shop 2");
            }
        } else if (strcmp(buffer, "add to cart\n") == 0){

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "Entering cart addition sequence");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0) {
                error("ERROR writing to socket: shop 3");
            }

            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "What item do you want to add to cart?");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0) {
                error("ERROR writing to socket: shop 4");
            }

            bzero(buffer, sizeof(buffer));
            n = read(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR reading from socket: Shop 2");
            }

            int i;
            int itemKey = -1;
            char tempInventory[10][50];
            for(i = 0; i < 10; i++){
                strcpy(tempInventory[i], storeInventory[i]);
                if(strcmp(strcat(tempInventory[i], "\n"), buffer) == 0){
                    itemKey = i;
                }
            }


            if(itemKey == -1){
                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "Sorry, that's not a valid item.");
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0) {
                    error("ERROR writing to socket: shop 5");
                }
            } else {
                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "You have selected %s.", storeInventory[itemKey]);
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0) {
                    error("ERROR writing to socket: shop 6");
                }

                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "how many of %s would you like to add to your cart?", storeInventory[itemKey]);
                n = write(client->clientID, buffer, sizeof(buffer));
                if (n < 0) {
                    error("ERROR writing to socket: shop 7");
                }

                int amount;
                bzero(buffer, sizeof(buffer));
                n = read(client->clientID, buffer, sizeof(buffer));
                if (n < 0){
                    error("ERROR reading from socket: Shop 2");
                }

                amount = atoi(buffer);

                if(amount > storeStock[itemKey]){
                    bzero(buffer, sizeof(buffer));
                    sprintf(buffer, "You have entered an amount higher than the inventory");
                    n = write(client->clientID, buffer, sizeof(buffer));
                    if (n < 0) {
                        error("ERROR writing to socket: shop 8");
                    }
                } else {

                    int addedFlag = -1;
                    for(i = 0; i < 5; i++){
                        if(strcmp(client->cart[i], "empty") == 0){
                            strcpy(client->cart[i], storeInventory[itemKey]);
                            client->cartStock[i] = amount;
                            client->cartPrice[i] = storePrice[itemKey];
                            storeStock[itemKey] = storeStock[itemKey] - amount;
                            addedFlag = 1;
                            break;
                        }
                    }

                    if(addedFlag == 1){
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "%d of %s have been added to your cart.", amount, storeInventory[itemKey]);
                        n = write(client->clientID, buffer, sizeof(buffer));
                        if (n < 0) {
                            error("ERROR writing to socket: shop 8");
                        }
                    } else {
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "Your cart is already full.");
                        n = write(client->clientID, buffer, sizeof(buffer));
                        if (n < 0) {
                            error("ERROR writing to socket: shop 8");
                        }
                    }
                }
            }

        } else {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "please enter a valid command.");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: Shop 3");
            }
        }
    }

    //sem_post(&inventory);
    printf("Shop Exited - ClientID: %d\n", client->clientID);
    pthread_exit(0);
}

void *checkAccount(void *args) {
    struct clientData *client;
    client = (struct clientData *) args;
    char buffer[1028];
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
            sprintf(buffer, "Here's your account balance: %d", client->clientAccount);
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0) {
                error("ERROR writing to socket: account 4");
            }
        } else {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "please enter a valid command.");
            n = write(client->clientID, buffer, sizeof(buffer));
            if (n < 0){
                error("ERROR writing to socket: Shop 3");
            }
        }
    }
    printf("Account Exited - ClientID: %d\n", client->clientID);
    pthread_exit(0);
}