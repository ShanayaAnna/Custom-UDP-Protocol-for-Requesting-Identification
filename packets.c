#include "packets.h"
#include<stdio.h>
#include <string.h>

// Function to create the Access Request Packet
void create_access_request_packet(AccessRequestPacket* packet, uint8_t client_id, uint8_t technology, uint64_t subscriber_no) {
    packet->start_id = 0xFFFF;
    packet->client_id = client_id;
    packet->acc_per = 0xFFF8;
    packet->segment_no = 1;  // Or whatever is appropriate
    packet->length = sizeof(AccessRequestPacket);  // Fill length with size of the packet
    packet->technology = technology;  // E.g., 0x02 for 2G, 0x03 for 3G, etc.
    packet->subscriber_no = subscriber_no;
    packet->end_id = 0xFFFF;
}

// Function to create a Not Paid Response Packet
void create_not_paid_response_packet(NotPaidResponsePacket* packet, uint8_t client_id, uint8_t technology, uint64_t subscriber_no) {
    packet->start_id = 0xFFFF;
    packet->client_id = client_id;
    packet->not_paid = 0xFFF9;
    packet->segment_no = 1;  // Or whatever is appropriate
    packet->length = sizeof(NotPaidResponsePacket);
    packet->technology = technology;
    packet->subscriber_no = subscriber_no;
    packet->end_id = 0xFFFF;
}

// Function to create a Not Exist Response Packet
void create_not_exist_response_packet(NotExistResponsePacket* packet, uint8_t client_id, uint8_t technology, uint64_t subscriber_no) {
    packet->start_id = 0xFFFF;
    packet->client_id = client_id;
    packet->not_exist = 0xFFFA;
    packet->segment_no = 1;  // Or whatever is appropriate
    packet->length = sizeof(NotExistResponsePacket);
    packet->technology = technology;
    packet->subscriber_no = subscriber_no;
    packet->end_id = 0xFFFF;
}

// Function to create an Access Granted Response Packet
void create_access_granted_response_packet(AccessGrantedResponsePacket* packet, uint8_t client_id, uint8_t technology, uint64_t subscriber_no) {
    packet->start_id = 0xFFFF;
    packet->client_id = client_id;
    packet->access_ok = 0xFFFB;
    packet->segment_no = 1;  // Or whatever is appropriate
    packet->length = sizeof(AccessGrantedResponsePacket);
    packet->technology = technology;
    packet->subscriber_no = subscriber_no;
    packet->end_id = 0xFFFF;
}

// Function to print AccessRequestPacket
void print_access_request_packet(const AccessRequestPacket *packet) {
    printf("AccessRequestPacket:\n");
    printf("Start ID: 0x%X\n", packet->start_id);
    printf("Client ID: %u\n", packet->client_id);
    printf("Access Permission: 0x%X\n", packet->acc_per);
    printf("Segment No: %u\n", packet->segment_no);
    printf("Length: %u\n", packet->length);
    printf("Technology: %u\n", packet->technology);
    printf("Subscriber No: %llu\n", packet->subscriber_no);
    printf("End ID: 0x%X\n", packet->end_id);
}

// Function to print NotPaidResponsePacket
void print_not_paid_response_packet(const NotPaidResponsePacket *packet) {
    printf("NotPaidResponsePacket:\n");
    printf("Start ID: 0x%X\n", packet->start_id);
    printf("Client ID: %u\n", packet->client_id);
    printf("Not Paid: 0x%X\n", packet->not_paid);
    printf("Segment No: %u\n", packet->segment_no);
    printf("Length: %u\n", packet->length);
    printf("Technology: %u\n", packet->technology);
    printf("Subscriber No: %llu\n", packet->subscriber_no);
    printf("End ID: 0x%X\n", packet->end_id);
}

// Function to print NotExistResponsePacket
void print_not_exist_response_packet(const NotExistResponsePacket *packet) {
    printf("NotExistResponsePacket:\n");
    printf("Start ID: 0x%X\n", packet->start_id);
    printf("Client ID: %u\n", packet->client_id);
    printf("Not Exist: 0x%X\n", packet->not_exist);
    printf("Segment No: %u\n", packet->segment_no);
    printf("Length: %u\n", packet->length);
    printf("Technology: %u\n", packet->technology);
    printf("Subscriber No: %llu\n", packet->subscriber_no);
    printf("End ID: 0x%X\n", packet->end_id);
}

// Function to print AccessGrantedResponsePacket
void print_access_granted_response_packet(const AccessGrantedResponsePacket *packet) {
    printf("AccessGrantedResponsePacket:\n");
    printf("Start ID: 0x%X\n", packet->start_id);
    printf("Client ID: %u\n", packet->client_id);
    printf("Access OK: 0x%X\n", packet->access_ok);
    printf("Segment No: %u\n", packet->segment_no);
    printf("Length: %u\n", packet->length);
    printf("Technology: %u\n", packet->technology);
    printf("Subscriber No: %llu\n", packet->subscriber_no);
    printf("End ID: 0x%X\n", packet->end_id);
}