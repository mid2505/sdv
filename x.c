#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define TOTAL_FRAMES 5

typedef struct {
    int seq_num;
    char data[50];
} Packet;

int expected_frame = 0;

void send_ack(int client_sock, int frame_num) {
    Packet ack;
    ack.seq_num = frame_num;
    strcpy(ack.data, "ACK");
    
    send(client_sock, &ack, sizeof(Packet), 0);
    printf("Sent ACK for frame %d\n", frame_num);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
    Packet pkt;
    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    int yes = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    bind(server_sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_sock, 1);
    
    printf("Stop and Wait Server running on port %d\n", PORT);
    printf("Waiting for client...\n");
    
    client_sock = accept(server_sock, (struct sockaddr *)&addr, (socklen_t*)&addr_len);
    printf("Client connected\n\n");
    
    while (expected_frame < TOTAL_FRAMES) {
        int bytes = recv(client_sock, &pkt, sizeof(Packet), 0);
        
        if (bytes <= 0) break;
        
        printf("Received frame %d: %s\n", pkt.seq_num, pkt.data);
        
        // simulate packet loss (20%)
        if (rand() % 10 < 2) {
            printf("Frame %d LOST!\n", pkt.seq_num);
            continue;  // no ACK sent
        }
        
        // check if this is the expected frame
        if (pkt.seq_num == expected_frame) {
            printf("Frame %d accepted and delivered\n", pkt.seq_num);
            expected_frame++;
            send_ack(client_sock, pkt.seq_num);
        } else {
            printf("Frame %d rejected (expected %d)\n", pkt.seq_num, expected_frame);
            // send ACK for last correctly received frame
            if (expected_frame > 0) {
                send_ack(client_sock, expected_frame - 1);
            }
        }
        
        printf("Expected next: %d\n\n", expected_frame);
    }
    
    printf("All frames received successfully!\n");
    close(client_sock);
    close(server_sock);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define TOTAL_FRAMES 5
#define TIMEOUT 3

typedef struct {
    int seq_num;
    char data[50];
} Packet;

char* messages[TOTAL_FRAMES] = {
    "hello",
    "how are you",
    "good day",
    "see you",
    "bye"
};

void send_frame(int sock, int frame_num) {
    Packet pkt;
    pkt.seq_num = frame_num;
    strcpy(pkt.data, messages[frame_num]);
    
    send(sock, &pkt, sizeof(Packet), 0);
    printf("Sent frame %d: %s\n", frame_num, messages[frame_num]);
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    Packet ack_pkt;
    int current_frame = 0;
    time_t send_time;
    
    srand(time(NULL));
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    
    // set timeout for receiving ACKs
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed\n");
        return -1;
    }
    
    printf("Stop and Wait Client connected\n");
    printf("Messages to send: ");
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        printf("%s", messages[i]);
        if (i < TOTAL_FRAMES - 1) printf(", ");
    }
    printf("\n\n");
    
    // Stop and Wait main loop
    while (current_frame < TOTAL_FRAMES) {
        printf("--- Sending frame %d ---\n", current_frame);
        
        // send current frame
        send_frame(sock, current_frame);
        send_time = time(NULL);
        
        // wait for ACK
        int bytes = recv(sock, &ack_pkt, sizeof(Packet), 0);
        
        if (bytes > 0 && strcmp(ack_pkt.data, "ACK") == 0) {
            if (ack_pkt.seq_num == current_frame) {
                printf("Got ACK for frame %d - SUCCESS!\n", current_frame);
                current_frame++;  // move to next frame
                printf("Moving to next frame: %d\n\n", current_frame);
            } else {
                printf("Got wrong ACK %d (expected %d) - RETRANSMIT\n", 
                       ack_pkt.seq_num, current_frame);
            }
        } else {
            printf("TIMEOUT - No ACK received - RETRANSMIT\n");
        }
    }
    
    printf("All frames sent successfully!\n");
    close(sock);
    return 0;
}
