#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "packets.h"

#define SERVER_PORT 12345
#define SERVER_IP "127.0.0.1"
#define CLIENT_ID 1
#define TECHNOLOGY 0x04 // 4G
#define MAX_RETRIES 3
#define TIMEOUT 3 
#define MAX_PACKET_SIZE sizeof(AccessGrantedResponsePacket)

// send access request
ssize_t send_access_request(int client_socket, struct sockaddr_in *server_addr, socklen_t server_addr_len, unsigned long long subscriber_no, unsigned char *response_buffer) {
    AccessRequestPacket request_packet;
    create_access_request_packet(&request_packet, CLIENT_ID, TECHNOLOGY, subscriber_no);

    int retries = 0;
    struct timeval timeout = {TIMEOUT, 0};
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (retries < MAX_RETRIES) {
        // Send access request
        if (sendto(client_socket, &request_packet, sizeof(request_packet), 0, (struct sockaddr *)server_addr, server_addr_len) < 0) {
            perror("Send failed");
            return -1;
        }
        printf("Access request sent (Attempt %d) for Subscriber No: %llu\n", retries + 1, subscriber_no);

        // Try to receive response
        ssize_t received = recvfrom(client_socket, response_buffer, MAX_PACKET_SIZE, 0, NULL, NULL);
        
        if (received > 0) {
            printf("Response received for Subscriber No: %llu\n", subscriber_no);
            return received;  // Return the received response size
        }
        printf("No response received. Retrying...\n");
        retries++;
    }

    printf("Max retries reached. No response from server.\n");
    return -1;
}


// handle the response from the server
void handle_server_response(unsigned char *buffer, unsigned long long subscriber_no) {
    if (((AccessGrantedResponsePacket *)buffer)->start_id == START_OF_PACKET_ID) {
        AccessGrantedResponsePacket *response = (AccessGrantedResponsePacket *)buffer;
        if (response->access_ok == ACCESS_OK) {
            printf("Access granted to Subscriber No: %llu\n", subscriber_no);
        }
    } else if (((NotPaidResponsePacket *)buffer)->start_id == START_OF_PACKET_ID) {
        NotPaidResponsePacket *response = (NotPaidResponsePacket *)buffer;
        if (response->not_paid == NOT_PAID) {
            printf("Subscriber has not paid.\n");
        }
    } else if (((NotExistResponsePacket *)buffer)->start_id == START_OF_PACKET_ID) {
        NotExistResponsePacket *response = (NotExistResponsePacket *)buffer;
        if (response->not_exist == NOT_EXIST) {
            printf("Subscriber does not exist.\n");
        }
    } else {
        printf("Unknown response received.\n");
    }
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    unsigned long long subscriber_no = 4086808821;

    // Create UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        exit(EXIT_FAILURE);
    }

    // Buffer to store response
    unsigned char buffer[MAX_PACKET_SIZE];

    // Send access request and receive response in the same function
    ssize_t received = send_access_request(client_socket, &server_addr, server_addr_len, subscriber_no, buffer);
    if (received > 0) {
        handle_server_response(buffer, subscriber_no);
        printf("Received response of size: %zd\n", received);
    } else {
        printf("No valid response received.\n");
    }

    // Close the socket
    close(client_socket);
    return 0;
}
