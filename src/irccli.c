#include<string.h>
#include<stdlib.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull
#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6)
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3)
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0)

void stop(char *s)
{
	perror(s);
	exit(1);
}

void initialize_char(char* buf,int size ){
	for(int i=0;i<size;i++){
		buf[i]='\0';
	}
}
//recuperer le temps avec ntpclient
char* ntp_time(){

	int sockf, n;

	int portno = 123;

	char* host_name = "us.pool.ntp.org";

	typedef struct
	{

		uint8_t li_vn_mode;

		uint8_t stratum;
		uint8_t poll;
		uint8_t precision;

		uint32_t rootDelay;
		uint32_t rootDispersion;
		uint32_t refId;

		uint32_t refTm_s;
		uint32_t refTm_f;
		uint32_t origTm_s;
		uint32_t origTm_f;

		uint32_t rxTm_s;
		uint32_t rxTm_f;

		uint32_t txTm_s;
		uint32_t txTm_f;

	} ntp_packet;

	ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	memset( &packet, 0, sizeof( ntp_packet ) );

	*( ( char * ) &packet + 0 ) = 0x1b;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	sockf = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( sockf < 0 )
		stop( "ERROR opening socket" );

	server = gethostbyname( host_name );

	if ( server == NULL )
		stop( "ERROR, no such host" );
	bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );

	serv_addr.sin_family = AF_INET;

	bcopy( ( char* )server->h_addr, ( char* ) &serv_addr.sin_addr.s_addr, server->h_length );

	serv_addr.sin_port = htons( portno );

	if ( connect( sockf, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr) ) < 0 )
		stop( "ERROR connecting" );

	n = write( sockf, ( char* ) &packet, sizeof( ntp_packet ) );

	if ( n < 0 )
		stop( "ERROR writing to socket" );



	n = read( sockf, ( char* ) &packet, sizeof( ntp_packet ) );

	if ( n < 0 )
		stop( "ERROR reading from socket" );


	packet.txTm_s = ntohl( packet.txTm_s );
	packet.txTm_f = ntohl( packet.txTm_f );

	time_t txTm = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA );
	return ctime( ( const time_t* ) &txTm );
}
int main(int argc, char ** argv)
{

	if(argc==3){
		
		fd_set fread;
		fd_set fwrite;
		struct sockaddr_in servaddr;
		int sockfd,valread,i=0,nread,maxfdr,maxfdw,nwrite;
		char buffer_sockfd[300],buffer_stdinfileno[300];
		
		if ( (sockfd=socket(AF_INET, SOCK_STREAM , 0)) == -1)
		{
			stop("socket");
		}

		char file[12];
		char buffer_read[250],time_buffer[325];
		memset((char*)&servaddr,0,sizeof(servaddr));
	
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = inet_addr(argv[1]);
		servaddr.sin_port = htons(strtol(argv[2],NULL,10));




		if (connect(sockfd, (struct sockaddr*)&servaddr,sizeof(servaddr)) < 0) {
			stop("connect");
		}




		if(send(sockfd,"init\n",5,0)<0)
			stop("send");


		write(1,"\033[35mBienvenue client lisez d'abord les consignes dans README.txt!!!!.\n\033[00m",77);
		while(1)
		{


			initialize_char(buffer_sockfd,300);

			initialize_char(buffer_stdinfileno,300);

			initialize_char(time_buffer,325);


			FD_ZERO(&fread);
			FD_ZERO(&fwrite);


			FD_SET(sockfd, &fread);
			FD_SET(STDIN_FILENO, &fread);

			FD_SET(STDOUT_FILENO,&fwrite);
			FD_SET(sockfd,&fwrite);

			if(sockfd>STDIN_FILENO)
				maxfdr=sockfd;
			else
				maxfdr=STDIN_FILENO;

			if(sockfd>STDOUT_FILENO)
				maxfdw=sockfd;
			else
				maxfdw=STDOUT_FILENO;

			nread = select(maxfdr+1, &fread, NULL, NULL, NULL);
			nwrite = select(maxfdw+1,NULL,  &fwrite,NULL,NULL);

			if (FD_ISSET(sockfd, &fread)){
				if (valread=(recv( sockfd , buffer_sockfd,300,0))< 0){
					stop("recv");


				}
				
				strcpy(time_buffer,"\033[33m");
				if(strlen(buffer_sockfd)>0){
					//pour recuperer le temps avec ntpclient on fait: strcat(time_buffer,ntp_time()); ca marche trop bien
					//mais ca reduit les performances de notre programme puisqu'on doit attendre la reponse du serveur ntp
					//donc je vais recuperer le temps directement
					time_t tim;
					time(&tim);
					strcat(time_buffer,ctime(&tim));
					strcat(time_buffer,"\033[00m");
					strcat(time_buffer,buffer_sockfd);
				}

			}





			if (FD_ISSET(STDIN_FILENO, &fread)){

				i=0;
				if((valread=read(STDIN_FILENO,buffer_stdinfileno,300))==-1)
					stop("read");
			
				if(strncmp(buffer_stdinfileno,"/send",5)==0){
					while(buffer_stdinfileno[7+i]!=' '){
						i++;
					}
					initialize_char(file,12);

					strcpy(file,buffer_stdinfileno+i+8);
					file[strlen(file)-1]='\0';
					if(strlen(file)>0){
						initialize_char(buffer_read,250);
						int fdsrc = open(file, O_RDONLY);

						int nchar = read(fdsrc, buffer_read, 250);

						initialize_char(buffer_stdinfileno+i+8,20);
						strcat(buffer_stdinfileno,buffer_read);

					}}



			}




			if (FD_ISSET(STDOUT_FILENO,&fwrite)){
				
					
					if(write(1,time_buffer,strlen(time_buffer))==-1){
						stop("write");
					}
					






			}
			if (FD_ISSET(sockfd, &fwrite)){

				if(send(sockfd,buffer_stdinfileno,strlen(buffer_stdinfileno),0) <0 )
					stop("send");
				


			}

			if(strcmp(buffer_stdinfileno,"/exit\n")==0){
                                        close(sockfd);
                                        exit(0);

                             
			}



		}	


	}
	else {
		printf("entrez l'adresse de votre serveur et le numero de port\n");
		
	}







return 0;
}
