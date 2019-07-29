#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define CLIENT_NUM 25
#define CHUNKSIZE 10000

char serverIP[] = "172.31.83.239";
char filepath[] = "/home/ubuntu/video.mp4";
char* addr;
int filesize;

void* sendfile(void* args){
	int socketfd;

	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd == -1){
		printf("Create socket fail!\n");
		return 0;
	}

	struct sockaddr_in serverInfo;
	bzero(&serverInfo, sizeof(serverInfo));
	serverInfo.sin_family = PF_INET;
	serverInfo.sin_addr.s_addr = inet_addr(serverIP);
	serverInfo.sin_port = htons(8877);

	if(connect(socketfd, (struct sockaddr*)&serverInfo, sizeof(serverInfo)) == -1){
		printf("connection error\n");
		return 0;
	}
	printf("connection establish!\n");

	//send file as a chunk
	int remain=filesize, offset=0;
	int sendSucc=0, thisSucc=0;
	while(remain){
		if(remain<CHUNKSIZE){
			thisSucc = send(socketfd, addr+offset, remain, 0);
			break;
		}
		thisSucc = send(socketfd, addr+offset, CHUNKSIZE, 0);
		offset += CHUNKSIZE;
		remain -= CHUNKSIZE;
		sendSucc += thisSucc;
	}
	printf("transmit successfully (bytes):%d\n", sendSucc);
	close(socketfd);
}

int main()
{
	int fd = open(filepath, O_RDONLY);
	struct stat s;
	fstat(fd, &s);
	filesize = s.st_size;

	addr = (char*)malloc(sizeof(char) * filesize);
	FILE *fp = fopen(filepath, "rb");
	int num = fread(addr, 1, filesize, fp);
	printf("read: %d\n", num);
	fclose(fp);

	pthread_t t[CLIENT_NUM];
	for(int i=0;i<CLIENT_NUM;i++){
		pthread_create(t+i, NULL, sendfile, NULL);
	}

	for(int i=0;i<CLIENT_NUM;i++){
		pthread_join(t[i], NULL);
	}

	return 0;
}
