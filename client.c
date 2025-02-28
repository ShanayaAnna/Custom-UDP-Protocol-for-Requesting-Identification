#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "packets.h"

#define SERVER_PORT 12345
#define SERVER_IP "127.0.0.1"
#define CLIENT_ID 1
#define MAX_RETRIES 3
#define TIMEOUT 3 
#define MAX_PACKET_SIZE sizeof(AccessGrantedResponsePacket)

ssize_t send_access_request(int client_socket, struct sockaddr_in *server_addr, socklen_t server_addr_len, unsigned long long subscriber_no, unsigned char technology, unsigned char *response_buffer) {
    AccessRequestPacket request_packet;
    create_access_request_packet(&request_packet, CLIENT_ID, technology, subscriber_no);

    int retries = 0;
    struct timeval timeout = {TIMEOUT, 0};
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (retries < MAX_RETRIES) {
        // Send access request
        if (sendto(client_socket, &request_packet, sizeof(request_packet), 0, (struct sockaddr *)server_addr, server_addr_len) < 0) {
            perror("Send failed");
            return -1;
        }
        printf("Access request sent (Attempt %d) for Subscriber No: %llu with Technology: %x\n", retries + 1, subscriber_no, technology);

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

// Handle the response from the server based on packet type
void handle_server_response(unsigned char *buffer, unsigned long long subscriber_no) {
    if (((AccessGrantedResponsePacket *)buffer)->start_id == START_OF_PACKET_ID) {
        AccessGrantedResponsePacket *response = (AccessGrantedResponsePacket *)buffer;
        if (response->access_ok == ACCESS_OK) {
            printf("Access granted to Subscriber No: %llu\n", subscriber_no);
        }
    } else if (((NotPaidResponsePacket *)buffer)->start_id == START_OF_PACKET_ID) {
        NotPaidResponsePacket *response = (NotPaidResponsePacket *)buffer;
        if (response->not_paid == NOT_PAID) {
            printf("Subscriber No: %llu has not paid.\n", subscriber_no);
        }
    } else if (((NotExistResponsePacket *)buffer)->start_id == START_OF_PACKET_ID) {
        NotExistResponsePacket *response = (NotExistResponsePacket *)buffer;
        if (response->not_exist == NOT_EXIST) {
            printf("Subscriber No: %llu does not exist.\n", subscriber_no);
        }
    } else {
        printf("Unknown response received for Subscriber No: %llu.\n", subscriber_no);
    }
}

// Function to execute the selected test case
void run_test_case(int test_case) {
    int client_socket;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    unsigned char buffer[MAX_PACKET_SIZE];
    unsigned long long subscriber_no;
    unsigned char technology;

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        exit(EXIT_FAILURE);
    }

    switch (test_case) {
        case 1: // Successful Access Granted
            subscriber_no = 4086808821;
            technology = 0x02;
            printf("\n--- Test Case 1: Successful Access Granted ---\n");
            break;
        case 2: // Subscriber Not Paid
            subscriber_no = 4088880012;
            technology = 0x04;
            printf("\n--- Test Case 2: Subscriber Not Paid ---\n");
            break;
        case 3: // Subscriber Does Not Exist (Subscriber Number Not Found)
            subscriber_no = 4090102234;
            technology = 0x04;
            printf("\n--- Test Case 3: Subscriber Does Not Exist (Subscriber Number Not Found) ---\n");
            break;
        case 4: // Subscriber Does Not Exist (Technology Mismatch)
            subscriber_no = 4086808821;  
            technology = 0x01;
            printf("\n--- Test Case 4: Subscriber Does Not Exist (Technology Mismatch) ---\n");
            break;
        default:
            printf("Invalid test case selected.\n");
            return;
    }

    // Create socket for the client
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Send access request and handle response
    ssize_t received = send_access_request(client_socket, &server_addr, server_addr_len, subscriber_no, technology, buffer);
    if (received > 0) {
        handle_server_response(buffer, subscriber_no);
        printf("Received response of size: %zd\n", received);
    } else {
        printf("No valid response received.\n");
    }

    // Close the socket
    close(client_socket);
}

int main() {
    int test_case;

    // Prompt user for input to select a test case
    printf("Select a test case to run:\n");
    printf("1: Successful Access Granted\n");
    printf("2: Subscriber Not Paid\n");
    printf("3: Subscriber Does Not Exist (Subscriber Number Not Found)\n");
    printf("4: Subscriber Does Not Exist (Technology Mismatch)\n");
    printf("Enter your choice (1-5): ");
    scanf("%d", &test_case);

    // Run the selected test case
    run_test_case(test_case);

    return 0;
}
