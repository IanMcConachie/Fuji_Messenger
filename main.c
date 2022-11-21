/*
Fuji Messenger Main

Author: Ian McConachie
Date Created: October 16, 2022
Last Modified: November 4, 2022

Description:
	Fuji Messenger is an instant messaging protocol that only allows messages in the form of haikus.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

// MACRO DEFINITIONS

#define UNUSED __attribute__((unused))
#define MSG_MAX 100


// STRUCT DEFINITIONS

struct packet {
	char message[MSG_MAX];
} packed;


// MODULAR FUNCTIONS

int input_checker(int argc) {
	int ret = 0;

	// Check to see if correct number or args
	if (argc == 4)
		ret = 1;

	return ret;
}


int is_haiku() {
	return 1;
}


// MAIN FUNCTION

int main(int argc, char **argv) {

	// Variable declarations
	int ret = EXIT_SUCCESS;
	struct sockaddr_in dest_addr, my_addr;
	int sockfd;
	char *incoming, *outgoing;
	struct timeval timer;
	fd_set r_set;
	int sel_err;



	// Parsing inputs and seeing if there are any errors
	if (input_checker(argc) == 0) {
		fprintf(stderr, "ERROR: Incorrect number of args\nEXPECTED: ./Fuji [dest. IP address] [dest. port #] [local port #]\n");
		ret = EXIT_FAILURE;
		goto EXIT;
	}

	unsigned long dest_ip = (unsigned long)atoi(argv[1]);
	short dest_port = (short)atoi(argv[2]);
	short src_port = (short)atoi(argv[3]);

	if ((dest_ip == 0)||(dest_port == 0)||(src_port == 0)) {
		fprintf(stderr, "ERROR: Incorrect number of args\nEXPECTED: ./Fuji [dest. IP address] [dest. port #] [local port #]\n");
		ret = EXIT_FAILURE;
		goto EXIT;
	}


	// Creating address structures for source and destination

	dest_addr.sin_family = AF_INET;
	inet_aton("localhost", &dest_addr.sin_addr);
	//dest_addr.sin_addr.s_addr = htonl(dest_ip);
	dest_addr.sin_port = htons(dest_port);

	my_addr.sin_family = AF_INET;
	inet_aton("localhost", &my_addr.sin_addr);
	my_addr.sin_port = htons(src_port);



	// Creating the socket and binding to local port number
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "ERROR: Failed to create socket\n");
		ret = EXIT_FAILURE;
		goto EXIT;
	}


	if ((bind(sockfd, (const struct sockaddr *)&my_addr, sizeof(my_addr))) < 0) {
		fprintf(stderr, "ERROR: Failed to bind to socket\n");
		ret = EXIT_FAILURE;
		goto EXIT;
	}

	timer.tv_sec = 0;
	timer.tv_usec = 5;

	outgoing = (char *)malloc(sizeof(char) * MSG_MAX);
	incoming = (char *)malloc(sizeof(char) * MSG_MAX);

	while(1) {
		fprintf(stdout, "> ");
		fgets(outgoing, MSG_MAX, stdin);

		if (strcmp(outgoing, "r\n") == 0) {
			FD_ZERO(&r_set);
			FD_SET(sockfd, &r_set);
			sel_err = select(sockfd+1, &r_set, NULL, NULL, &timer);
			if (sel_err == -1) {
				printf("ERROR: select function failed\n");
			} else if (sel_err != 0) {
				recvfrom(sockfd, incoming, MSG_MAX, 0, NULL, NULL);
				fprintf(stdout, "%s", incoming);
			}
			
		} else if (strcmp(outgoing, "q\n") == 0) {
			break;

		} else if (strcmp(outgoing, "\n") != 0) {
			sendto(sockfd, (void *)outgoing, MSG_MAX, 0, (const struct sockaddr *)&dest_addr, sizeof(dest_addr));
		}
	}

	//recvfrom(sockfd, (void *)incoming, MSG_MAX, 0, NULL, NULL);	
	//sendto(sockfd, (void *)&join, sizeof(join), 0, (const struct sockaddr *)&dest_addr, sizeof(srv_addr));

	free(outgoing);
	free(incoming);

EXIT:
	return ret;
}