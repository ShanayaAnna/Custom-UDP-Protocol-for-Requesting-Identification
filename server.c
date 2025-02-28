#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "packets.h"

#define SERVER_PORT 12345
#define DATABASE_FILE "Verification_Database.txt"

// Database structure
typedef struct {
    unsigned long long subscriber_no;
    unsigned char technology;
    int paid;
} SubscriberRecord;

void handle_error(const char *msg) {
    perror(msg);
    exit(1);
}

// load the database
int load_database(SubscriberRecord *db[]) {
    FILE *file = fopen(DATABASE_FILE, "r");
    if (!file) {
        handle_error("Failed to open database");
    }

    size_t size = 0;
    while (fscanf(file, "%llu %hhu %d", &db[size]->subscriber_no, &db[size]->technology, &db[size]->paid) != EOF) {
        db[size] = malloc(sizeof(SubscriberRecord));  // Allocate memory for each record
        size++;
    }
    fclose(file);
    return size;
}

// find a subscriber in the database
SubscriberRecord *find_subscriber(SubscriberRecord *db[], int db_size, unsigned long long subscriber_no, unsigned char technology) {
    for (int i = 0; i < db_size; i++) {
        if (db[i]->subscriber_no == subscriber_no && db[i]->technology == technology) {
            return db[i];
        }
    }
    return NULL;
}

// send the appropriate response to the client
void send_response(int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len, AccessRequestPacket *request_packet, SubscriberRecord *record) {
    if (!record) {
        // Subscriber does not exist
        NotExistResponsePacket response;
        create_not_exist_response_packet(&response, request_packet->client_id, request_packet->technology, request_packet->subscriber_no);
        sendto(sockfd, &response, sizeof(response), 0, (struct sockaddr *)client_addr, addr_len);
    } else if (record->paid == 0) {
        // Subscriber has not paid
        NotPaidResponsePacket response;
        create_not_paid_response_packet(&response, request_packet->client_id, request_packet->technology, request_packet->subscriber_no);
        sendto(sockfd, &response, sizeof(response), 0, (struct sockaddr *)client_addr, addr_len);
    } else {
        // Subscriber permitted to access
        AccessGrantedResponsePacket response;
        create_access_granted_response_packet(&response, request_packet->client_id, request_packet->technology, request_packet->subscriber_no);
        sendto(sockfd, &response, sizeof(response), 0, (struct sockaddr *)client_addr, addr_len);
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) handle_error("Socket creation failed");

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        handle_error("Bind failed");
    }

    // Load the database
    SubscriberRecord *db[10];
    int db_size = load_database(db);
    
    // Server loop to receive requests
    while (1) {
        AccessRequestPacket request_packet;
        ssize_t received_size = recvfrom(sockfd, &request_packet, sizeof(request_packet), 0, 
                                          (struct sockaddr *)&client_addr, &addr_len);
        if (received_size < 0) {
            perror("Receive failed");
            continue;
        }

        // Find subscriber in the database
        SubscriberRecord *record = find_subscriber(db, db_size, request_packet.subscriber_no, request_packet.technology);
        
        // Send response to client
        send_response(sockfd, &client_addr, addr_len, &request_packet, record);
    }

    close(sockfd);
    return 0;
}
