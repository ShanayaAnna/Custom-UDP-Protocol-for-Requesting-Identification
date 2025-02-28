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


// send access request
void send_access_request(int sockfd, struct sockaddr_in *server_addr, unsigned long long subscriber_no) {
    AccessRequestPacket request_packet;
    create_access_request_packet(&request_packet, CLIENT_ID, TECHNOLOGY, subscriber_no);

    ssize_t sent = sendto(sockfd, &request_packet, sizeof(request_packet), 0, 
                               (struct sockaddr *) server_addr, sizeof(*server_addr));
    if (sent < 0){
        perror("Send Failed");
        exit(EXIT_FAILURE);
    }
    printf("Access request sent for Subscriber No: %llu\n", subscriber_no);
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

    // Send an access request to server
    send_access_request(client_socket, &server_addr, subscriber_no);

    // Wait for response (blocking call)
    unsigned char buffer[sizeof(AccessGrantedResponsePacket)];
    ssize_t received = recvfrom(client_socket, buffer, sizeof(buffer), 0, NULL, NULL);
    if (received < 0){
        perror("Receive failed");
        exit(EXIT_FAILURE);
    }

    // Handle the response from the server
    handle_server_response(buffer, subscriber_no);
    printf("Received response of size: %zd\n", received);


    // Close the socket
    close(client_socket);
    return 0;
}
