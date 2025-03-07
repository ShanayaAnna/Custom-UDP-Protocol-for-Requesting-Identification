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
typedef enum {
    NOT_FOUND,
    TECHNOLOGY_MISMATCH,
    FOUND
} SubscriberStatus;

SubscriberStatus find_subscriber(unsigned long long subscriber_no, unsigned char technology, SubscriberRecord **record) {
    SubscriberRecord *matched_record = NULL;
    for (int i = 0; i < MAX_RECORDS; i++) {
        if (db[i].subscriber_no == subscriber_no) {
            matched_record = &db[i]; // Found subscriber, check technology
            if (db[i].technology == technology) {
                *record = &db[i];
                return FOUND;  // Found subscriber with matching technology
            }
        }
    }
    *record = matched_record;
    return (matched_record) ? TECHNOLOGY_MISMATCH : NOT_FOUND;  // Differentiate cases
}


// send the appropriate response to the client
void send_response(int server_socket, struct sockaddr_in *client_addr, socklen_t client_addr_len, 
                   AccessRequestPacket *request_packet, SubscriberStatus status, SubscriberRecord *record) {
    void *response_packet = NULL;
    size_t response_size = 0;

    if (status == NOT_FOUND) {
        NotExistResponsePacket response;
        create_not_exist_response_packet(&response, request_packet->client_id, request_packet->technology, request_packet->subscriber_no);
        response_packet = &response;
        response_size = sizeof(response);
        printf("Subscriber No: %llu does not exist in the database.\n", request_packet->subscriber_no);
    } 
    else if (status == TECHNOLOGY_MISMATCH) {
        NotExistResponsePacket response;
        create_not_exist_response_packet(&response, request_packet->client_id, request_packet->technology, request_packet->subscriber_no);
        response_packet = &response;
        response_size = sizeof(response);
        printf("Subscriber No: %llu exists but with different technology.\n", request_packet->subscriber_no);
    } 
    else if (record->paid == 0) {
        NotPaidResponsePacket response;
        create_not_paid_response_packet(&response, request_packet->client_id, request_packet->technology, request_packet->subscriber_no);
        response_packet = &response;
        response_size = sizeof(response);
        printf("Subscriber No: %llu has not paid.\n", request_packet->subscriber_no);
    } 
    else {
        AccessGrantedResponsePacket response;
        create_access_granted_response_packet(&response, request_packet->client_id, request_packet->technology, request_packet->subscriber_no);
        response_packet = &response;
        response_size = sizeof(response);
        printf("Access granted to Subscriber No: %llu.\n", request_packet->subscriber_no);
    }

    // Send response to client
    if (sendto(server_socket, response_packet, response_size, 0, (struct sockaddr *)client_addr, client_addr_len) < 0) {
        perror("Send response failed");
    }
    printf("Above Response sent to client for Subscriber No: %llu\n", request_packet->subscriber_no);
}


int main() {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create UDP socket
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", SERVER_PORT);

    // Load the database
    int db_size = load_database();
    if (db_size < 0) {
        close(server_socket);
        exit(1);
    }
    printf("Loading Verification_Database...\n");

    // Server loop to receive requests
    while (1) {
        AccessRequestPacket request_packet;
        ssize_t received = recvfrom(server_socket, &request_packet, sizeof(request_packet), 0, 
                                          (struct sockaddr *)&client_addr, &client_addr_len);
        if (received < 0) {
            perror("Receive failed");
            continue;
        }
    
        SubscriberRecord *record = NULL;
        SubscriberStatus status = find_subscriber(request_packet.subscriber_no, request_packet.technology, &record);
    
        printf("Received request from Subscriber No: %llu\n", request_packet.subscriber_no);
        
        send_response(server_socket, &client_addr, client_addr_len, &request_packet, status, record);
    }

    close(server_socket);
    return 0;
}
