#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "packets.h"

#define SERVER_PORT 12345
#define DATABASE_FILE "Verification_Database.txt"
#define MAX_RECORDS 10  // Fixed size for the database

typedef struct {
    unsigned long long subscriber_no;
    unsigned char technology;
    int paid;
} SubscriberRecord;
SubscriberRecord db[MAX_RECORDS];

void handle_error(const char *msg) {
    perror(msg);
    exit(1);
}

// load the database
int load_database() {
    FILE *file = fopen(DATABASE_FILE, "r");
    if (!file) {
        perror("Failed to open database");
        return -1;
    }

    size_t size = 0;
    while (size < MAX_RECORDS && !feof(file)) {
        SubscriberRecord record;
        fscanf(file, "%llu %hhu %d", &record.subscriber_no, &record.technology, &record.paid);

        db[size] = record;
        size++;
    }
    fclose(file);
    return size;
}

// find a subscriber in the database
SubscriberRecord *find_subscriber(unsigned long long subscriber_no, unsigned char technology) {
    for (int i = 0; i < MAX_RECORDS; i++) {
        if (db[i].subscriber_no == subscriber_no && db[i].technology == technology) {
            return &db[i];
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
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // Load the database
    int db_size = load_database();
    if (db_size < 0) {
        close(sockfd);
        exit(1);
    }

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
        SubscriberRecord *record = find_subscriber(request_packet.subscriber_no, request_packet.technology);
        
        // Send response to client
        send_response(sockfd, &client_addr, addr_len, &request_packet, record);
    }

    close(sockfd);
    return 0;
}
