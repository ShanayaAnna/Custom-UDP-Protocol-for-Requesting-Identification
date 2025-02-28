#ifndef PACKETS_H
#define PACKETS_H

#include <stdint.h>

#define MAX_PAYLOAD_SIZE 255


#define START_OF_PACKET_ID 0xFFFF
#define END_OF_PACKET_ID 0xFFFF

// Access Permission Types
#define ACC_PER_REQUEST 0xFFF8
#define NOT_PAID 0xFFF9
#define NOT_EXIST 0xFFFA
#define ACCESS_OK 0xFFFB

// Access Request Packet (Client to Server)
typedef struct {
    unsigned int start_id;       // 0xFFFF (Start of Packet ID)
    unsigned char client_id;     // Max 0xFF (Client ID)
    unsigned int acc_per;        // 0xFFF8 (Access Permission)
    unsigned char segment_no;    // Segment Number (1 byte)
    unsigned char length;        // Max 0xFF (Length)
    unsigned char technology;    // (02, 03, 04, 05)
    unsigned long long subscriber_no; // Source Subscriber No (Max 0xFFFFFFFFFF)
    unsigned int end_id;         // 0xFFFF (End of Packet ID)
} AccessRequestPacket;

typedef struct {
    unsigned int start_id;       
    unsigned char client_id;     
    unsigned int not_paid;       // 0xFFF9 (Not Paid)
    unsigned char segment_no;    
    unsigned char length;        
    unsigned char technology;    
    unsigned long long subscriber_no; 
    unsigned int end_id;         
} NotPaidResponsePacket;

typedef struct {
    unsigned int start_id;       
    unsigned char client_id;     
    unsigned int not_exist;      
    unsigned char segment_no;    
    unsigned char length;        
    unsigned char technology;    
    unsigned long long subscriber_no; 
    unsigned int end_id;         
} NotExistResponsePacket;

typedef struct {
    unsigned int start_id;       
    unsigned char client_id;     
    unsigned int access_ok;      // 0xFFFB (Access OK)
    unsigned char segment_no;    
    unsigned char length;        
    unsigned char technology;    
    unsigned long long subscriber_no; 
    unsigned int end_id;         // 0xFFFF (End of Packet ID)
} AccessGrantedResponsePacket;

// Function prototypes
void create_access_request_packet(AccessRequestPacket *packet, unsigned char client_id, unsigned char technology, unsigned long long subscriber_no);
void create_not_paid_response_packet(NotPaidResponsePacket *packet, unsigned char client_id, unsigned char technology, unsigned long long subscriber_no);
void create_not_exist_response_packet(NotExistResponsePacket *packet, unsigned char client_id, unsigned char technology, unsigned long long subscriber_no);
void create_access_granted_response_packet(AccessGrantedResponsePacket *packet, unsigned char client_id, unsigned char technology, unsigned long long subscriber_no);
void print_access_request_packet(const AccessRequestPacket *packet);
void print_not_paid_response_packet(const NotPaidResponsePacket *packet);
void print_not_exist_response_packet(const NotExistResponsePacket *packet);
void print_access_granted_response_packet(const AccessGrantedResponsePacket *packet);

#endif // PACKETS_H
