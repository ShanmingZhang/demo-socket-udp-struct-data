/*
 ============================================================================
 Name        : demo-udp-server-ex.c
 Author      : Shanming ZHANG
 Version     : version 1.0
 Copyright   : Your copyright notice
 Description : UDP Server for the transmission of self-defined data format
 Environment : Linux
 Date        : 2016.05.14 22:33 JST
 ============================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/types.h>

#define SERVICE_PORT	20123	/* hard-coded server port number */
#define MSGLEN 100
#define	DATA_LEN	497			/* The length of RES_PACKET is 512 = 497 + 1 + 2 + 4 + 8, then the length of UDP packet is 512*/
#define APP_TYPE_E	'e'			/* 'e': energy efficient application/service */
#define APP_ID		101			/* The application identifier */
#define CONT_COUNT	2			/* The number of content provided by application/service */

#pragma pack(1)
struct REQ_MSG {
	struct sockaddr_in src_addr;
	char r_msg[100];
};

struct RES_PACKET {
	__be64 req_id;				/* 8 byte. req_id is number of request allocated by application/service server while response the request. */
	unsigned int data_len;		/* 4 byte. the length of data chunk. */
	__be16 app_id;				/* 2 byte. the identifier of application/service of service provider. */
	__u8 app_type;				/* 1 byte. 'e': energy efficient application/service. */
	char data[DATA_LEN];
};
#pragma pack()

void response(__be64 req_id, char * content_name, __u8 app_type, __be16 app_id,
		unsigned int data_len, int sock_id, struct sockaddr_in remaddr);
int check_request(const char* content_name[],  unsigned int c_count, char req_content[] );

int main(int argc, char **argv) {

	/* content list */
	char * content_list[] = { "pic0.jpg", "pic1.jpg" };

	/* create a udp socket */
	int fd;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	/* server address */
	struct sockaddr_in myaddr;
	memset((char *) &myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(fd, (struct sockaddr *) &myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	struct sockaddr_in remaddr;
	socklen_t addrlen = sizeof(remaddr);

	int recvlen; /* # bytes received */
	__be64 msgcnt = 0; /* request no that is used as request id  */

	for (;;) {
		printf("waiting on port %d\n", SERVICE_PORT);

		struct REQ_MSG req_msg;
		recvlen = recvfrom(fd, (char *)&req_msg, sizeof(struct REQ_MSG), 0,
				(struct sockaddr *) &remaddr, &addrlen);

		printf("%s \n", req_msg.r_msg);
		printf("%s \n", inet_ntoa(req_msg.src_addr.sin_addr));

		if (recvlen > 0) {

			if (check_request(content_list, CONT_COUNT, req_msg.r_msg ) > 0) {
				response(++msgcnt, req_msg.r_msg, APP_TYPE_E, APP_ID, DATA_LEN,
						fd, remaddr);
			} else {
				printf(" The requested content dosen't exist in server.");
			}
		} else {
			printf("uh oh - something went wrong!\n");
		}
	}
	return 0;
}

void response(__be64 req_id, char * content_name, __u8 app_type, __be16 app_id,
		unsigned int data_len, int sock_id, struct sockaddr_in remaddr) {

	printf("response the %lld request \n", req_id);

	FILE *file_fd = fopen(content_name, "r+");
	if (file_fd == NULL) {
		printf(" The content file open error \n");
		exit(1);
	}

	struct RES_PACKET res_packet;
	res_packet.app_type = app_type;
	res_packet.app_id = app_id;
	res_packet.req_id = req_id;

	//int index = 0;
	for (;;) {
		int readBytes = fread(res_packet.data, 1, DATA_LEN, file_fd);
		if (readBytes == 0) {
			printf(" file is read over ! \n");
			//printf(" data size: %d sent in %d time. \n", readBytes, ++index);

			res_packet.data_len = readBytes;
			sendto(sock_id, (char *)&res_packet, sizeof(struct RES_PACKET), 0,
					(struct sockaddr *) &remaddr, sizeof(remaddr));
			break;
		} else {
			//printf(" data size: %d sent in %d time. \n", readBytes, ++index);

			res_packet.data_len = readBytes;
			sendto(sock_id, (char *)&res_packet, sizeof(struct RES_PACKET), 0,
					(struct sockaddr *) &remaddr, sizeof(remaddr));
		}
	}
}

int check_request(const char * content_list[], unsigned int c_count, char req_content[]) {
	int flag = 0;
	int index = 0;
	while ( index < c_count) {
		if (strcmp(content_list[index], req_content) == 0) {
			flag = 1;
			break;
		}
		index++;
	}
	return flag;
}
