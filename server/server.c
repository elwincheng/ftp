#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 4096
#define MAX_PACKET_SIZE 1000

int main(int argc, char *argv[]) {


    int sockfd;           // get a socket file descriptor
    char buffer[MAXLINE]; // used to store recieved data
    char *yes = "yes";    // used to send yes back to client.
    struct sockaddr_in servaddr, cliaddr; 

    int port = atoi(argv[1]);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { // creates a socket and returns an integer that is the socket file descriptor
        perror("socket creation failed"); // sock_dgram -> udp. sock_stream -> tcp
        exit(EXIT_FAILURE); // AF_INET is an address family that designates the type of addresses that your socket can communicate with (IPV4)
    }

    memset(&servaddr, 0, sizeof(servaddr)); // initialize struct data to zero to get rid of garbage values
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; // specifies socket to bind to all available interfaces
    servaddr.sin_port = htons(port); // htons converts port to network byte order (big endian) translates to language the network will understand

	// associate a specific ip address and port number to a socket. 
    // allows the server to listen for incoming UDP packets in that ip address and port number
    //passively listening to incoming connections
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int len, n;
    len = sizeof(cliaddr);
			
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0';

    if (strcmp(buffer, "ftp") == 0) 
    {
        
        sendto(sockfd, (const char *)yes, strlen(yes),
            MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
    }
    FILE* file = NULL;
    // file = fopen("out.png", "wb");
		// return 0;

		int count = 0;
		while (1) {
			// printf("HI\n");
			int total_frag, frag_no, packet_size;
			char file_name[MAXLINE];
			char file_data[MAXLINE];
			char* ack = "ack";
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
		if (n == 1) break;
			int size = sscanf(buffer, "%d:%d:%d:%[^:]:", &total_frag, &frag_no, &packet_size, file_name);
			if (frag_no == 1) {
				file = fopen(file_name, "wb");

				if (file == NULL) {
						perror("Error opening file");
						exit(42);
				}
			}
			printf("%d: %d\n", count++, size);
			printf("%d:%d:%d:%s:", frag_no, total_frag, packet_size, file_name);
			size = n - packet_size;
			fwrite(buffer + size, 1, packet_size, file);
			sendto(sockfd, (char * ) ack, sizeof(ack), MSG_CONFIRM, (struct sockaddr *)&cliaddr, sizeof(cliaddr));

		}
        fclose(file);
    // It will block and wait until data recieved from the socket and store into buffer. 

    // printf("Client : %s\n", buffer);

    return 0;
}