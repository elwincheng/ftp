#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

#define MAXLINE 4096
// #define MAX_PACKET_SIZE 4096
#define MAX_PACKET_SIZE 1000

int main(int argc, char *argv[]) {

    char *server_ip = argv[1]; // get argument from terminal
    int server_port_number = atoi(argv[2]); 

    int sockfd;
    char buffer[MAXLINE];
    char *message = "ftp";
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { // create a UDP socket 
        perror("socket creation failed"); // AF_INET is for specifying the address family to look for (IPV4)
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr)); // set data to zero

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port_number); // convert 16 bit port number to network byte order (big endian)
    servaddr.sin_addr.s_addr = inet_addr(server_ip);

    int n, len;
		struct timeval start_time, end_time;
    
    char file_name[MAXLINE];
    scanf("%s", file_name);
    scanf("%s", file_name);
		gettimeofday(&start_time, NULL);
    if (access(file_name, F_OK) != -1) { 
        sendto(sockfd, (const char *)message, strlen(message),
            MSG_CONFIRM, (const struct sockaddr *)&servaddr,
            sizeof(servaddr));
    } else {    
        perror("File not found");
        exit(EXIT_FAILURE);
    }

    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                 0, (struct sockaddr *)&servaddr,
                 &len);
		gettimeofday(&end_time, NULL);
    buffer[n] = '\0';

    if (strcmp(buffer, "yes") != 0) {
			exit(42);
    }
		double rtt = (double) (end_time.tv_usec - start_time.tv_usec) / 1000.0;
		printf("Round trip time: %.5f ms\n", rtt);
		printf("A file transfer can start.");

		FILE *file = fopen(file_name, "rb");
		if (file == NULL) {
			perror("File not found");
			exit(EXIT_FAILURE);
		}
		struct timeval timeout;
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		int total_frag = 0;
		fseek(file, 0, SEEK_END);
		long file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		total_frag = (file_size / MAX_PACKET_SIZE) + 1;
    // FILE* file_out = NULL;
    // file_out = fopen("out.png", "wb");

		char file_data[MAX_PACKET_SIZE];
		int retransmit = 0;
		char packet_str[MAXLINE];
		for (int frag_no = 1; frag_no <= total_frag; frag_no++) {
			int packet_size = 0;
			int size = 0;
			int written = 0;
			if (!retransmit) {
				packet_size = fread(file_data, 1, MAX_PACKET_SIZE, file);
				printf("%d: %d\n", frag_no-1, packet_size);
				size = 0;
				written = snprintf(packet_str, sizeof(packet_str), "%d:%d:%d:%s:",
					total_frag, frag_no, packet_size, file_name
				);
				for (int i = 0; i < packet_size; i++) {
					packet_str[i+written] = file_data[i];
				}

			}
        sendto(sockfd, (const char *)packet_str, written + packet_size,
            MSG_CONFIRM, (const struct sockaddr *)&servaddr,
            sizeof(servaddr));
				n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                 0, (struct sockaddr *)&servaddr,
                 &len);
				if (n < 0) {
					printf("ACK not recieved\n");
					retransmit = 1;
					frag_no--;
				} else {
					printf("ACK recieved\n");
					retransmit = 0;
				}

			// printf(packet_str);
		}

		file_data[0] = '\0';
        sendto(sockfd, (const char *)file_data, 1,
            MSG_CONFIRM, (const struct sockaddr *)&servaddr,
            sizeof(servaddr));

    close(sockfd);
    return 0;
}