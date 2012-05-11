#include "kpacket.h"
#include <sys/types.h> /* open() */
#include <sys/stat.h>  /* open() */
#include <fcntl.h>     /* open() */
#include <unistd.h>    /* read(), write() */
#include <stdio.h>     /* fprintf() */
#include <stdlib.h>    /* EXIT_FAILURE, EXIT_SUCCESS */

#define RETRY_MAX 3

/* Absolute maximum length for kermit packet we will use */
/* 4 bytes header + max data (94) + 1 byte checksum + \r\n + NUL = 102 */
#define KPACKET_MAXLEN 102

char kpacket[KPACKET_MAXLEN];
int kfd, kseq = 0;

/* kermit_send
 *
 * description
 *   fills kermit packet and sends it
 * input
 *   type: packet type
 *   data: data
 *   len: data length in bytes (not including packet stuff), 0-94
 * return value
 *   return of write()
 * notes
 *   data field is not encoded by this function
 */
int kermit_send(int type, char *data, int len) {
	int i, s = 0;
	kpacket[0] = 01;
	s += kpacket[1] = tochar(len + 3);
	s += kpacket[2] = tochar(kseq % 64);
	s += kpacket[3] = type;
	for (i = 0; i < len; ++i) {
		s += data[i];
		kpacket[i+4] = data[i];
	}
	i += 4;
	kpacket[i++] = tochar((s + ((s & 192) / 64)) & 63);
	kpacket[i++] = '\r';
	kpacket[i++] = '\n';
	return write(kfd, kpacket, i);
}

/* kermit_recv
 *
 * description
 *   waits for server data and receives exactly one packet
 * return value
 *   -1: read error
 *   1: success
 * notes
 *   data field is not decoded by this function
 */
int kermit_recv(void) {
	int len = 1;
	while (1) {
		if (read(kfd, kpacket, 1) <= 0)
			return -1;
		if (kpacket[0] == 01)
			break;
	}
	while (1) {
		len += read(kfd, kpacket + len, KPACKET_MAXLEN - len);
		if (len >= 2 && len >= unchar(kpacket[1]))
			break;
	}
	kpacket[len] = 0;
	printf("len=%d seq=%d t='%c'\n", (int) unchar(kpacket[1]), (int) unchar(kpacket[2]), (int) kpacket[3]);
	kseq = unchar(kpacket[2]);
	if (kpacket[3] == 'X' || kpacket[3] == 'D') {
		printf("%s\n", kpacket + 4);
	}
	kermit_send(KPACKET_TYPE_ACK, NULL, 0);
}


int kermit_get(void) {
	
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "error: too few arguments\n");
		return EXIT_FAILURE;
	}
	if ((kfd = open(argv[1], O_RDWR)) < 0) {
		fprintf(stderr, "error: failed to open port %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	kermit_send(KPACKET_TYPE_CMD, "CLEAR", 5);
	while (1) kermit_recv();
	return EXIT_SUCCESS;
}

