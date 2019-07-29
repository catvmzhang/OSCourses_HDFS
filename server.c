#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define MAX_SOCKET_QUEUE_NUM 10000
#define BUFFERSIZE 10000
#define CLIENTNUM 25
int socketfd;
clock_t begin, end;
int order;
pthread_mutex_t mutex;

static void sig_handler(int signalNum){
	close(socketfd);
	double timeSpent = (end - begin) / CLOCKS_PER_SEC;
	printf("total time:%f\n", timeSpent);
	exit(0);
}

void* recvfile(void *args){
	int clientSocketfd = *(int*)args;
	char inputBuffer[BUFFERSIZE];
	char filename[64] = {'\0'};
	static int fileNum = 0;
	sprintf(filename, "/home/ubuntu/output/%d_video.mp4", fileNum);
	fileNum++;
	FILE *fp = fopen(filename, "wb");

	pthread_mutex_lock(&mutex);
	if(begin==0) begin = clock();
	pthread_mutex_unlock(&mutex);
	while(1){
		int size = recv(clientSocketfd, inputBuffer, BUFFERSIZE, 0);
		if(size == 0) break;
		fwrite((void*)inputBuffer, 1, size, fp);
		fflush(fp);
	}
	order++;
	printf("%d ok\n", order);
	
	if(order == CLIENTNUM){
		char *cmd = "/usr/local/hadoop/bin/hadoop fs -copyFromLocal /home/ubuntu/output/ hdfs://master:9000/";
		system(cmd);
		end = clock();
		printf("end\n");
	}

	fclose(fp);
	close(clientSocketfd);
}

int main()
{
	begin = 0; end = 0, order = 0;
	pthread_mutex_init(&mutex, NULL);
	signal(SIGINT, sig_handler);

	int clientSocketfd;
	char *inputBuffer = (char*)malloc(sizeof(char) * BUFFERSIZE);
	memset(inputBuffer, '\0', BUFFERSIZE);

	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd == -1){
		printf("Create socket fail\n!");
		return 0;
	}

	struct sockaddr_in serverInfo, clientInfo;
	int addrlen = sizeof(clientInfo);
	bzero(&serverInfo, sizeof(serverInfo));
	serverInfo.sin_family = PF_INET;
	serverInfo.sin_addr.s_addr = INADDR_ANY;
	serverInfo.sin_port = htons(8877);
	bind(socketfd, (struct sockaddr*)&serverInfo, sizeof(serverInfo));
	listen(socketfd, MAX_SOCKET_QUEUE_NUM);

	while(1){
		clientSocketfd = accept(socketfd, (struct sockaddr*)&clientInfo, &addrlen);
		pthread_t t;
		int *args = (int*)malloc(sizeof(int));
		*args = clientSocketfd;
		pthread_create(&t, NULL, recvfile, (void*)args);
	}

	return 0;

}

