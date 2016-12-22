#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];

	if (argc < 3)
	{
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[2]);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd <0)
	{
		error("ERROR opening socket");
	}
	
	server = gethostbyname(argv[1]);
	if(server == NULL)
	{
		fprintf(stderr,"Error, no such host\n");
		exit(0);
	}
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	bcopy((char*)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0)
	{
		error("ERROR connecting");
	}
	/*
	int i;
	
	do
	{
		printf("What do you want to do?  Please select one number from the list \n");
		printf("1. Management of mapping system\n");
		printf("2. Use\n");
		scanf("%d",&i);
	}
	while(!(i == 1 || i == 2));
	int j = 0;
	if (i == 1)
	{
		do
		{
			printf(" You chose Management of mapping system\n");
			printf("What do you want to do?  Please select one number from the list \n");
			printf("1. Add edge\n");
			printf("2. Add vertex \n");
			printf("3. Add edge event \n");
			scanf("%d",&j);
		}
		while(!(j == 1 || j == 2 || j == 3));
		if(j == 1) 
		{
			int src, dest;
			int directional;
			double speed;
			double length;
			printf("Give the source vertex id\n");
			scanf("%d",&src);
			printf("Give the destination vertex id\n");
			scanf("%d",&dest);
			printf("Is it directional? Give 1(yes) or 0(no)\n");
			scanf("%d",&directional);
			printf("Give the speed\n");
			scanf("%f",&speed);
			printf("Give the edge length\n");
			scanf("%d",&length);
		}
		else if(j == 2) 
		{
			int vertex_type;
			char name[10];
			printf("Give the vertex id\n");
			scanf("%d",&vertex_type);
			printf("Give the destination vertex id\n");
			scanf("%s",name);
		}
		else if(j == 3) 
		{
			int src, dest;
			int event;
			printf("Give the source vertex of the edge\n");
			scanf("%d",&src);
			printf("Give the destination vertex id of the edge\n");
			scanf("%d",&dest);
			printf("Is there a event of road closure? Give 1(yes) or 0(no) \n");
			scanf("%d",&event);
		}
	}

	int k = 0;
	if (i == 2)
	{
		do
		{
			printf(" You chose Use\n");
			printf("What do you want to do?  Please select one number from the list \n");
			printf("1. Find trip\n");
			printf("2. Store the graph \n");
			printf("3. Retrieve the graph \n");
			scanf("%d",&k);
		}
		while(!(k == 1 || k == 2 || k == 3));

		if(k == 1) 
		{
			int src, dest;
			printf("Give the source vertex id for the trip \n");
			scanf("%d",&src);
			printf("Give the destination vertex id for the trip \n");
			scanf("%d",&dest);
		}
		else if(k == 2) 
		{
			char name[10];
			printf("Give the file name for storage \n");
			scanf("%s",name);
		}
		else if(k == 3) 
		{
			char name[10];
			printf("Give the file name for storage \n");
			scanf("%s",name);
		}
	}

	char ch1;
	scanf("%c",&ch1);
	*/
	printf("Please enter a message:");
	bzero(buffer,256);
	fgets(buffer,255,stdin);
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0)
	{
		error("ERROR writing to socket");
	}
	bzero(buffer,256);
	n = read(sockfd,buffer,255);
	if (n < 0)
	{
		error("ERROR reading from socket");
	}
	printf("%s\n",buffer);

	return 0;
}


