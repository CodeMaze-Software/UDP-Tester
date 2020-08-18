//
//  main.c
//  Emulator - tester
//
//  Created by Dariusz Adamczyk on 11/08/2020.
//  Copyright © 2020 Dariusz Adamczyk. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 1024

#define SIGNAL_IDX          0
#define SEQUENCE_IDX        1
#define SETTINGS_NUM_IDX    2
#define IDX_OFFSET          2
#define BITMAP_IDX          2
#define RELAY_TO_SET_IDX    3



enum signal_codes
{
    ACK             = 0x11, // ACK signal
    RELAY_SIG       = 0x21, // Relay signal
    GET_STATE_SIG   = 0x31, // Get state signal
    REL_STATE_SIG   = 0x41, // Relay state signal
    QUIT_SIG        = 0x51, // Quit signal
};
  
int main() {
    int sockfd;
    struct sockaddr_in     servaddr;
    
    unsigned int output_data[32];
    unsigned int input_data[32];

    unsigned int curr_tx_seq_num = 1;
    unsigned int last_tx_seq_num = 1;
 
    
    char console_cmd;
  
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
  
    memset(&servaddr, 0, sizeof(servaddr));
    
    char port[6];
    
    printf("Type destination port...\r\n");
    scanf("%5s", port);
    printf("Typed port is: %s\r\n", port);
    
    fflush(stdin);
    
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
      
    socklen_t len;
    
    printf("Tester help:\r\n"
           "[1] - Get status\r\n"
           "[2] - Relays ON\r\n"
           "[3] - Relays OFF\r\n"
           "[4] - Quit\r\n"
           "[5] - Unknown signal[random value]\r\n"
           "[6] - Sequence Error[random value]\r\n"
           "[7] - Range Error[random value]\r\n"
           );
    
    while(1)
    {
        memset(input_data, 0, sizeof(input_data));
        
        printf("\r\nPress key [1-7] and than push Enter...\r\n");
        console_cmd = getchar();
        
        fflush(stdin);
        
        if(console_cmd == '1')
        {
            memset(output_data, 0, sizeof(output_data));
                
            output_data[SIGNAL_IDX] = GET_STATE_SIG;
            output_data[SEQUENCE_IDX] = last_tx_seq_num++;
            
            sendto(sockfd, output_data, sizeof(output_data), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            recvfrom(sockfd, input_data, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
            
            
            if(input_data[SIGNAL_IDX] == ACK)
                curr_tx_seq_num = input_data[SEQUENCE_IDX];
                
            if(curr_tx_seq_num == last_tx_seq_num)
            {
                last_tx_seq_num = curr_tx_seq_num;
                
                printf("ACK\r\n");
                recvfrom(sockfd, input_data, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
            
                if(input_data[SIGNAL_IDX] == REL_STATE_SIG)
                {
                    printf("[GET REL STATE]\r\n[RELAY 0]:%d\r\n[RELAY 1]:%d\r\n", (input_data[BITMAP_IDX] >> 0) & 1U, (input_data[BITMAP_IDX] >> 1) & 1U);
                    
                    output_data[SIGNAL_IDX] = ACK;
                    output_data[SEQUENCE_IDX] = ++ input_data[SEQUENCE_IDX];
                    
                    sendto(sockfd, output_data, sizeof(output_data), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
                }
            }
            else
            {
                recvfrom(sockfd, input_data, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
                memset(output_data, 0, sizeof(output_data));
            }
        }
        
        if(console_cmd == '2')
        {
            memset(output_data, 0, sizeof(output_data));
                
            output_data[SIGNAL_IDX] = RELAY_SIG;
            output_data[SEQUENCE_IDX] = last_tx_seq_num++;
            output_data[SETTINGS_NUM_IDX] = 0x02;
            output_data[3] = 0x00;
            output_data[4] = 0x01;
            output_data[5] = 0x01;
            output_data[6] = 0x01;
            
            sendto(sockfd, output_data, sizeof(output_data), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            recvfrom(sockfd, input_data, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
            
            if(input_data[SIGNAL_IDX] == ACK)
                curr_tx_seq_num = input_data[SEQUENCE_IDX];
                
            if(curr_tx_seq_num == last_tx_seq_num)
            {
                last_tx_seq_num = curr_tx_seq_num;
                
                printf("ACK\r\n");
            }
            else
            {
                printf("SEQ Error\r\n");
            }
        }
        
        if(console_cmd == '3')
        {
            memset(output_data, 0, sizeof(output_data));
                
            output_data[SIGNAL_IDX] = RELAY_SIG;
            output_data[SEQUENCE_IDX] = last_tx_seq_num++;
            output_data[SETTINGS_NUM_IDX] = 0x02;
            output_data[3] = 0x00;
            output_data[4] = 0x00;
            output_data[5] = 0x01;
            output_data[6] = 0x00;
            
            sendto(sockfd, output_data, sizeof(output_data), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            recvfrom(sockfd, input_data, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
            
            if(input_data[SIGNAL_IDX] == ACK)
                curr_tx_seq_num = input_data[SEQUENCE_IDX];
                
            if(curr_tx_seq_num == last_tx_seq_num)
            {
                last_tx_seq_num = curr_tx_seq_num;
                
                printf("ACK\r\n");
            }
            else
            {
                printf("SEQ Error\r\n");
            }
        }
        
        if(console_cmd == '4')
        {
            memset(output_data, 0, sizeof(output_data));
                
            output_data[SIGNAL_IDX] = QUIT_SIG;
            output_data[SEQUENCE_IDX] = last_tx_seq_num++;
            
            sendto(sockfd, output_data, sizeof(output_data), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            recvfrom(sockfd, input_data, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
            
            if(input_data[SIGNAL_IDX] == ACK)
                curr_tx_seq_num = input_data[SEQUENCE_IDX];
            
            if(curr_tx_seq_num == last_tx_seq_num)
            {
                last_tx_seq_num = curr_tx_seq_num;
                
                printf("ACK\r\n");
                
                break;
            }
            else
            {
                printf("SEQ Error\r\n");
            }
        }
        
        if(console_cmd == '5')
        {
            memset(output_data, 0, sizeof(output_data));
                
            output_data[SIGNAL_IDX] = rand() % 100;
            output_data[SEQUENCE_IDX] = last_tx_seq_num++;
            
            sendto(sockfd, output_data, sizeof(output_data), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            recvfrom(sockfd, input_data, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
            
            if(input_data[SIGNAL_IDX] == ACK)
                curr_tx_seq_num = input_data[SEQUENCE_IDX];
            
            if(curr_tx_seq_num == last_tx_seq_num)
            {
                last_tx_seq_num = curr_tx_seq_num;
                
                printf("ACK\r\n");
            }
            else
            {
                printf("SEQ Error\r\n");
            }
        }
        
        if(console_cmd == '6')
        {
            memset(output_data, 0, sizeof(output_data));
                
            output_data[SIGNAL_IDX] = RELAY_SIG;
            output_data[SEQUENCE_IDX] = rand() % 100;
            
            sendto(sockfd, output_data, sizeof(output_data), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            recvfrom(sockfd, input_data, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
            
            if(input_data[SIGNAL_IDX] == ACK)
                curr_tx_seq_num = input_data[SEQUENCE_IDX];
            
            if(curr_tx_seq_num == last_tx_seq_num)
            {
                last_tx_seq_num = curr_tx_seq_num;
                
                printf("ACK\r\n");
            }
            else
            {
                printf("SEQ Error\r\n");
            }
        }
        
        if(console_cmd == '7')
        {
            memset(output_data, 0, sizeof(output_data));
                
            output_data[SIGNAL_IDX] = RELAY_SIG;
            output_data[SEQUENCE_IDX] = last_tx_seq_num++;
            output_data[SETTINGS_NUM_IDX] = rand() % 100;
            output_data[3] = 0x00;
            output_data[4] = 0x00;
            output_data[5] = 0x01;
            output_data[6] = 0x00;
            
            sendto(sockfd, output_data, sizeof(output_data), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            recvfrom(sockfd, input_data, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
            
            if(input_data[SIGNAL_IDX] == ACK)
                curr_tx_seq_num = input_data[SEQUENCE_IDX];
                
           if(curr_tx_seq_num == last_tx_seq_num)
            {
                last_tx_seq_num = curr_tx_seq_num;
                
                printf("ACK\r\n");
            }
            else
            {
                printf("SEQ Error\r\n");
            }
        }
        
        
    }
      
    close(sockfd);
    return 0;
}
