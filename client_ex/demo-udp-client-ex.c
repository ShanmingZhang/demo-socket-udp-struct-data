

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/types.h>

#define SERVICE_PORT	21234	/* hard-coded port number */
#define MSGLEN 100
#define	DATA_LEN	497  /* The length of RES_PACKET is 512 = 497 + 1 + 2 + 4 + 8, then the length of UDP packet is 512*/

#pragma pack(1)
struct REQ_MSG {
	struct sockaddr_in src_addr;
	char r_msg[100];
};
struct RES_PACKET {
	__be64 req_id; // 8 byte. req_id is number of request allocated by application/service server while response the request.
	unsigned int data_len;   // 4 byte. the length of data chunk.
	__be16 app_id; // 2 byte. the identifier of application/service of service provider
	__u8 app_type;  // 1 byte. 'e': energy efficient application/service.
	char data[DATA_LEN];
};
#pragma pack()

int main(void) {

	char server[] = "0.0.0.0";
	printf("Connect to ? : (name or IP address) ");
	scanf("%s", server);

	int fd;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("socket created\n");

	int recv_buf = 1024 * 1024;
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*) &recv_buf, sizeof(int));
	struct sockaddr_in myaddr;
	memset((char *) &myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	if (bind(fd, (struct sockaddr *) &myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	struct sockaddr_in remaddr;
	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(SERVICE_PORT);
	if (inet_aton(server, &remaddr.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	char msg_buf[MSGLEN];
	printf("Please input request content name: ");
	scanf("%s", msg_buf);

	struct REQ_MSG req_msg;
	strncpy(req_msg.r_msg, msg_buf, MSGLEN);
	req_msg.src_addr.sin_addr.s_addr = myaddr.sin_addr.s_addr;

	printf("From %s:%d ", inet_ntoa(myaddr.sin_addr), ntohs(myaddr.sin_port));
	printf("sending request %s to %s port %d\n", req_msg.r_msg, server, SERVICE_PORT);

	sendto(fd, (char *)&req_msg, sizeof(struct REQ_MSG), 0, (struct sockaddr *) &remaddr,
			sizeof(remaddr));

	FILE *file_fd = fopen(msg_buf, "w");
	bzero(msg_buf, sizeof(msg_buf));

	int index = 0;
	int recvlen;

	struct RES_PACKET res_packet;
	for (;;) {

		recvlen = recvfrom(fd, (char *)&res_packet, sizeof(struct RES_PACKET), 0, NULL, NULL);

		if (recvlen > 0) {
			printf("recvlen %d  %d \n", recvlen, ++index);
		} else if (recvlen == 0) {
			printf("one picture recv over!\n");
			break;
		} else {
			printf("recv error!\n");
			exit(1);
		}
		fwrite(&res_packet.data, DATA_LEN, 1, file_fd);
	}

	if (fclose(file_fd) != 0) {
		printf("fclose() error! \n");
	} else {
		printf("fclose ##file_fd## success!\n");
	}

	close(fd);
	return 0;
}
