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


#include <string>
#include <map>
#include <vector>

// MACRO DEFINITIONS

#define UNUSED __attribute__((unused))
#define MSG_MAX 100
#define LINE_MAX 500
#define MAX_WORDS 10


using namespace std;

// STRUCT DEFINITIONS

struct packet {
	char message[MSG_MAX];
} packed;

//typedef map<string,int> syllables;
//vector<string> word_list;

// MODULAR FUNCTIONS

int input_checker(int argc) {
	int ret = 0;

	// Check to see if correct number or args
	if (argc == 4)
		ret = 1;

	return ret;
}


void generate_dict(map<string, int> *my_map) {
	int ind;
	string word;
	int syls;
	map<string, int> syllables;

	FILE *f_ptr;

	f_ptr = fopen("syllables.txt", "r");

	if (f_ptr == NULL) {
		fprintf(stderr, "ERROR: Failed to open syllables.txt\n");
	}

	char line[LINE_MAX];

	while(fgets(line, LINE_MAX, f_ptr) != NULL) {
		//printf("%s", line);
		if ((line[0] != '\n')&&(line[0] != '#')) {
			ind = 0;
			word = "";
			while(line[ind] != ' ') {
				word = word + line[ind];
				ind++;
			}
			syls = atoi(line+ind);
			syllables[word] = syls;
		}
	}


	*my_map = syllables;
	fclose(f_ptr);
	
}


int is_haiku(char *str, map<string,int> *my_map) {
	int ret = 1;
	int ind = 0;
	int line_ind = 0;
	int line_cnt = 1;
	int line_syls;
	map<string, int> syllables = *my_map;
	string word;

	printf("\n=====================\n");

	while(str[ind] != '\n') {
		if(str[ind] == '|') {
			line_cnt++;
		}
		ind++;
	}

	if (line_cnt != 3) {
		ret = -2;
		printf("Number of lines: %d\n", line_cnt);
		goto EXIT;
	}

	
	ind = 0;
	while((str[ind] != '\n')) {
		line_syls = 0;
		while((str[ind] != '|')&&(str[ind] != '\0')) {
			word = "";
			while((str[ind] != ' ')&&(str[ind] != '\n')) {
				word = word + str[ind];
				ind++;
			}
			//printf("%d\n", syllables[word]);
			if (syllables.find(word) == syllables.end()) {
				printf("Word not found: %s\n", word.c_str());
				ret = -1;
				goto EXIT;
			} else {
				line_syls = line_syls + syllables[word];
			}
			ind++;
		}
		
		if ((line_ind == 0)&&(line_syls != 5)) {
			ret = 0;
		} else if ((line_ind == 1)&&(line_syls != 7)) {
			ret = 0;
		} else if ((line_ind == 2)&&(line_syls != 5)) {
			ret = 0;
		}
		printf("Line %d syllables: %d\n", line_ind+1, line_syls);
		line_ind++;
		if (line_ind == 3) {
			break;
		}
		ind = ind + 2;
	}

EXIT:
	printf("=====================\n\n");
	return ret;
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

	unsigned long dest_ip;
	short dest_port;
	short src_port;

	map<string,int> syllables;

	// Parsing inputs and seeing if there are any errors
	if (input_checker(argc) == 0) {
		fprintf(stderr, "ERROR: Incorrect number of args\nEXPECTED: ./Fuji [dest. IP address] [dest. port #] [local port #]\n");
		ret = EXIT_FAILURE;
		goto EXIT;
	}

	dest_ip = (unsigned long)atoi(argv[1]);
	dest_port = (short)atoi(argv[2]);
	src_port = (short)atoi(argv[3]);

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

	// Creating syllable dictionary
	generate_dict(&syllables);

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
			int syl_ret = is_haiku(outgoing, &syllables);
			if (syl_ret == 1) {
				printf("Message Sent\n");
				sendto(sockfd, (void *)outgoing, MSG_MAX, 0, (const struct sockaddr *)&dest_addr, sizeof(dest_addr));
			} else if (syl_ret == 0) {  
				printf("ERROR: Not a valid haiku\n");
			} else if (syl_ret == -1) {
				printf("ERROR: Word not recognized\n");
			} else if (syl_ret == -2) {
				printf("ERROR: Incorrect number of lines\n");
			}
		}
	}

	//recvfrom(sockfd, (void *)incoming, MSG_MAX, 0, NULL, NULL);	
	//sendto(sockfd, (void *)&join, sizeof(join), 0, (const struct sockaddr *)&dest_addr, sizeof(srv_addr));

	free(outgoing);
	free(incoming);

EXIT:
	return ret;
}