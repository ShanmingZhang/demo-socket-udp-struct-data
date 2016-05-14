/*
 ============================================================================
 Name        : demo-udp-server-ex.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*
 demo-udp-03: udp-recv: a simple udp server
 receive udp messages

 usage:  udp-recv

 Paul Krzyzanowski
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/types.h>

#define SERVICE_PORT	21234	/* hard-coded port number */
#define MSGLEN 100

#define	DATA_LEN	497  /* The length of RES_PACKET is 512 = 497 + 1 + 2 + 4 + 8, then the length of UDP packet is 512*/

#pragma pack(1)

struct REQ_MSG {
	struct sockaddr_in s_addr;
	char r_msg[100];
};

struct RES_PACKET {
	__be64 req_id; // 8 byte. req_id is number of request allocated by application/service server while response the request.
	unsigned int data_len;   // 4 byte. the length of data chunk.
	__be16 app_id;    // 2 byte. the identifier of application/service of service provider
	__u8 app_type;  // 1 byte. 'e': energy efficient application/service.
	char data[DATA_LEN];
};

#pragma pack()

void response(int request_id, char * content_name, int sock_id,
		struct sockaddr_in remaddr);

int main(int argc, char **argv) {
	struct sockaddr_in myaddr; /* our address */
	struct sockaddr_in remaddr; /* remote address */
	socklen_t addrlen = sizeof(remaddr); /* length of addresses */
	int recvlen; /* # bytes received */
	int fd; /* our socket */
	int msgcnt = 0; /* count # of messages we received */

	/* create a UDP socket */

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	/* bind the socket to any valid IP address and a specific port */

	memset((char *) &myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(fd, (struct sockaddr *) &myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	/* now loop, receiving data and printing what we received */

	for (;;) {
		printf("waiting on port %d\n", SERVICE_PORT);
		struct REQ_MSG req_msg;
		char* buf_msg = (char*)malloc(sizeof(struct REQ_MSG)+1);
		memset(buf_msg, 0x00, sizeof(struct REQ_MSG)+1);
		recvlen = recvfrom(fd, buf_msg, sizeof(struct REQ_MSG), 0,
				(struct sockaddr *) &remaddr, &addrlen);
		memcpy(&req_msg, buf_msg, sizeof(req_msg));

		printf("%s \n", req_msg.r_msg);
		printf("%s \n", inet_ntoa(req_msg.s_addr.sin_addr));
		break;
//
//		if (recvlen > 0) {
//			req_content_name[recvlen] = 0;
//			printf("received message: \"%s\" (%d bytes)\n", req_content_name, recvlen);
//
//			if (strcmp(req_content_name, "pic0.jpg") == 0) {
//				response(++msgcnt, req_content_name, fd, remaddr);
//			} else {
//				printf(" The requested content dosen't exist in server.");
//			}
//
//		} else {
//			printf("uh oh - something went wrong!\n");
//		}
	}
	/* never exits */
	return 0;
}
//
//void response(int request_id, char * content_name, int sock_id,
//		struct sockaddr_in remaddr) {
//	char buf[BUFSIZE];
//	printf("response the %d request \n", request_id);
//
//	FILE *file_fd = fopen(content_name, "r+");
//	if (file_fd == NULL) {
//		printf("ファイルオープンエラー\n");
//		exit(1);
//	}
//	int index = 0;
//	for (;;) {
//		int readBytes = fread(buf, 1, sizeof(buf)-1, file_fd);
//		if (readBytes == 0) {
//			printf(" file is read over ! \n");
//			//printf("sendLen %d %d \n", readBytes, ++index);
//			sendto(sock_id, buf, readBytes, 0, (struct sockaddr *) &remaddr,
//					sizeof(remaddr));
//			break;
//		} else {
//			//printf("sendLen %d %d ", readBytes, ++index);
//			buf[readBytes] = index;
//			//printf("last char %d \n", buf[readBytes]);
//			sendto(sock_id, buf, readBytes + 1, 0, (struct sockaddr *) &remaddr,
//					sizeof(remaddr));
//			// get client IP address
//			//printf(" the client IP : %s \n ", inet_ntoa(remaddr.sin_addr));
//			//printf(" the client port: %d \n ", ntohs(remaddr.sin_port));
//		}
//		bzero(buf, sizeof(buf));
//	}
//}
