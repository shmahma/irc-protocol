#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD SET, FD ISSET, FD ZERO macros
#include <time.h>
#define TRUE 1
#define FALSE 0
#define PORT 8000
#define ADDR "127.0.0.1"
#define Max_clients 30

typedef struct {

	char *name;
	int indice_cli[Max_clients];
	int indice_owner;

} channel;

void stop(char* msg){
	perror("bind failed");
	exit(EXIT_FAILURE);

}

int compare_pseudo(char* buffer,char pseudo[Max_clients][25],int * pseudo_is_set,int k){

	for(int i=0;i<Max_clients;i++){

		if(i!=k &&pseudo_is_set[i]==1 && strlen(buffer)<=25 && strcmp(buffer,pseudo[i])==0){

			return i;
		}

	}

	return -1;
}


void initialize_char(char* buf,int size ){
	for(int i=0;i<size;i++){
		buf[i]='\0';
	}
}



int read_commande(char *buffer,char* command, char arg[50][50]){

	char* ligne_command=buffer;
	int i=0,k=0,j=0,size_argi=50;

	while(ligne_command[i]==' '){
		i++;
	}
	for( i=i;ligne_command[i]!=' '&&ligne_command[i]!='\n'&&ligne_command[i]!='\0';i++){
		command[j]=ligne_command[i];
		j++;
	}

	strcpy(arg[k],command);

	k++;
	while(ligne_command[i]!='\0'){

		if(ligne_command[i]!=' '){

			char *argi=calloc(1,size_argi);

			j=0;
			while(ligne_command[i]!=' ' && ligne_command[i]!='\0'){
				argi[j]=ligne_command[i];
				j++;
				i++;
			}
			strcpy(arg[k],argi);

			k++;
		}
		if(ligne_command[i]!='\0')
			i++;

	}
	initialize_char(arg[k],50);

	return k;
}


int main(int argc , char *argv[])
{
	int opt = TRUE;
	int socketserv , addrlen , new_socket , client_socket[10],activity, i , valread , sd;
	int max_sd,j;
	struct sockaddr_in servaddress;

	char buffer[300],pseudo_temporaire[Max_clients][25],registered_pseudo[Max_clients][25];
	int pseudo_temporaire_is_set[Max_clients];
	int registered_pseudo_is_set[Max_clients];
	char pseudo_buffer[300];
	char command[50];
	char arg[50][50];
	char bufff[300];
	char motdepass[Max_clients][25];
	int init[Max_clients];
	char buffer_second[300];
	int client_is_in_channel[Max_clients];
	channel* tab_chaine[Max_clients];

	for (i=0;i<Max_clients;i++){
		init[i]=0;
	}

	for (i=0;i<Max_clients;i++){
		pseudo_temporaire_is_set[i]=0;
	}

	for (i=0;i<Max_clients;i++){
		registered_pseudo_is_set[i]=0;
	}

	for (i=0;i<Max_clients;i++){
		client_is_in_channel[i]=0;
	}
	for (i=0;i<Max_clients;i++){
		tab_chaine[i]=NULL;
	}


	fd_set readfds;


	for (i = 0; i < Max_clients; i++)
	{
		client_socket[i] = 0;
	}

	if( (socketserv = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		stop("socket failed");
	}


	if( setsockopt(socketserv, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		stop("setspckopt");
	}


	servaddress.sin_family = AF_INET;
	servaddress.sin_addr.s_addr=inet_addr(ADDR);
	servaddress.sin_port = htons( PORT );


	if (bind(socketserv, (struct sockaddr *)&servaddress, sizeof(servaddress))<0)
	{
		stop("bind failed");
	}
	printf("\033[35;1;4mListener on port\033[00m %d \n", PORT);


	if (listen(socketserv, Max_clients) < 0)
	{
		stop("listen");
	}
	addrlen = sizeof(servaddress);
	puts("\033[35;1;4mWaiting for connections ...\033[00m");

	while(TRUE)
	{

		FD_ZERO(&readfds);

		FD_SET(socketserv, &readfds);
		max_sd = socketserv;

		for ( i = 0 ; i < Max_clients ; i++){

			sd = client_socket[i]; 
			if(sd > 0)
				FD_SET( sd , &readfds);


			if(sd > max_sd)
				max_sd = sd;
		}
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

		if ((activity < 0) && (errno!=EINTR))
		{
			printf("select error");
		}
		if (FD_ISSET(socketserv, &readfds)){
			if ((new_socket = accept(socketserv, (struct sockaddr *)&servaddress, (socklen_t*)&addrlen))<0)
			{
				stop("accept");

			}


			printf("\033[35;1;4mNew connection,\033[00m socket fd is %d , ip is : %s , port : %d \n" , new_socket, inet_ntoa(servaddress.sin_addr) , ntohs(servaddress.sin_port));


			for (i = 0; i < Max_clients; i++)
			{

				if( client_socket[i] == 0 )
				{
					client_socket[i] = new_socket;
					printf("\033[35;1;4mAdding to list of sockets as\033[00m %d\n" , i);

					break;
				}
			}



		}

		for (i = 0; i < Max_clients; i++)
		{
			sd = client_socket[i];
			if (FD_ISSET( sd , &readfds))
			{

				initialize_char(buffer,300);
				initialize_char(command,50);
				int k=2;
				while(strlen(arg[k])>0){
					initialize_char(arg[k],50);
					k++;}
				initialize_char(bufff,300);
				initialize_char(buffer_second,300);
				initialize_char(pseudo_buffer,300);

				if ((valread = read( sd , buffer, 300)) == 0)
				{


					getpeername(sd , (struct sockaddr*)&servaddress , (socklen_t*)&addrlen);
					printf("\033[35;1;4mHost disconnected,\033[00mip %s , port %d\n" , inet_ntoa(servaddress.sin_addr) , ntohs(servaddress.sin_port));


					close( sd );
					client_socket[i] = 0;
					init[i]=0;
					pseudo_temporaire_is_set[i]=0;
					registered_pseudo_is_set[i]=0;
					initialize_char(pseudo_temporaire[i],25);
					initialize_char(registered_pseudo[i],25);
					initialize_char(motdepass[i],25);
				}




				else
				{



					buffer[valread-1] = '\0';
					if(pseudo_temporaire_is_set[i]==1){
						printf("\033[32;1;4mserv recev:\033[00m\033[35;1;4m%s:\033[00m %s\n",pseudo_temporaire[i],buffer);}
					else{
						printf("\033[32;1;4mserv recev:\033[00m %s\n",buffer);
					}



					if(init[i]==0){
						if(pseudo_temporaire_is_set[i]==0){


							if( send(client_socket[i], "\033[32;1;4menvoyez votre pseudo:\033[00m", strlen("\033[32;1;4menvoyez votre pseudo:\033[00m"), 0)==-1 )

							{

								perror("send");
							}


						}
						init[i]=1;	
					}


					else if(init[i]==1){

						if(pseudo_temporaire_is_set[i]==0 && strlen(buffer)>0){


							int cmp=compare_pseudo(buffer,pseudo_temporaire,pseudo_temporaire_is_set,i);
							if(cmp!=-1){
								if(write(1,"pseudo invalide\n",16)==-1)
									stop("write");

								strcpy(bufff, "\033[32;1;4mce pseudo deja existe envoyez votre pseudo:\033[00m");



							}
							if (cmp==-1){
								if(write(1,"pseudo valide\n",14)==-1)
									stop("write");
								initialize_char(pseudo_temporaire[i],25);
								strncpy(pseudo_temporaire[i],buffer,strlen(buffer));
								pseudo_temporaire_is_set[i]=1;
								initialize_char(bufff,300);

							}


						}


						else if(pseudo_temporaire_is_set[i]==1 || registered_pseudo_is_set[i]==1){


							read_commande(buffer,command,arg);

							if((strncmp(command,"/nickname",9)==0)&& strlen(arg[2])==0){
								int cmp=compare_pseudo(arg[1],pseudo_temporaire,pseudo_temporaire_is_set,i);
								if(cmp!=-1){
									if(write(1,"pseudo invalide\n",16)==-1)
										stop("write");

									strcpy(bufff, "\033[32;1;4mce pseudo deja existe\033[00m ");

								}


								else{	
									initialize_char(pseudo_temporaire[i],25);
									strncpy(pseudo_temporaire[i],arg[1],strlen(arg[1]));
									strcpy(bufff,"\033[32;1;4mpseudo modifier\033[00m ");
									pseudo_temporaire_is_set[i]=1;
								}
							}
							else if(( strncmp(command,"/register",9)==0)&&strlen(arg[2])>0){
								if(compare_pseudo(arg[1],registered_pseudo,registered_pseudo_is_set,i)==-1){

									initialize_char(motdepass[i],25);
									strcpy(motdepass[i],arg[2]);
									initialize_char(registered_pseudo[i],25);
									strncpy(registered_pseudo[i],arg[1],strlen(arg[1]));
									strcpy(bufff,"\033[32;1;4mpseudo enregistre\033[00m ");
									if(strncmp(pseudo_temporaire[i],arg[1],strlen(arg[1]))==0){
										pseudo_temporaire_is_set[i]=0;
										initialize_char(pseudo_temporaire[i],25);
										strcat(bufff,"\033[32;1;4menvoyez votre nouveau pseudo: \033[00m");
									}
									registered_pseudo_is_set[i]=1;


								}
								else{
									strcpy(bufff,"\033[32;1;4m");
									strcat(bufff,"pseudo deja enregistrer par un autre utilisateur ");
									strcat(bufff,"\033[00m");


								}}
							else if((strncmp(command,"/unregister",11)==0)&&strlen(arg[2])>0&&strcmp(arg[1],registered_pseudo[i])==0){
								if(strcmp(motdepass[i],arg[2])==0&&registered_pseudo_is_set[i]==1){
									registered_pseudo_is_set[i]=0;

									initialize_char(registered_pseudo[i],25);

									strcpy(bufff,"\033[32;1;4mpseudo supprimer\033[00m ");
									initialize_char(motdepass[i],25);





								}
								else{
									strcpy(bufff,"\033[32;1;4m");
									strcat(bufff,"pseudo pas enregistrer/motdpass incorrecte");
									strcat(bufff,"\033[00m ");



								}


							}
							else if((strncmp(command,"/nickname",9)==0)&&strlen(arg[2])>0){
								if(strcmp(motdepass[i],arg[2])==0&&registered_pseudo_is_set[i]==1&&strcmp(arg[1],registered_pseudo[i])==0){

									initialize_char(pseudo_temporaire[i],25);
									strncpy(pseudo_temporaire[i],registered_pseudo[i],strlen(registered_pseudo[i]));
									if((j=compare_pseudo(registered_pseudo[i],pseudo_temporaire,pseudo_temporaire_is_set,i))!=-1){

										initialize_char(pseudo_temporaire[j],25);
										pseudo_temporaire_is_set[j]=0;
										if( send(client_socket[j], "\033[32;1;4mun autre utilisateur utilise votre pseudo envoyez un autre pseudo: \033[00m",strlen("\033[32;1;4mun autre utilisateur utilise votre pseudo envoyez un autre pseudo: \033[00m") , 0)!=-1 )
										{

											perror("send");
										}





									}
									strcpy(bufff,"\033[32;1;4m");
									strcat(bufff,"vous pouvez utilise ce pseudo ");
									strcat(bufff,"\033[00m");


								}
								else{
									strcpy(bufff,"\033[32;1;4m");
									strcat(bufff,"pseudo pas enregistrer/motdpass incorrecte ");
									strcat(bufff,"\033[00m");



								}



							}
							else if(strncmp(command,"/date",5)==0){

								time_t tim;


								time(&tim);
								strcpy(bufff,"\033[32;1;4m");
								strcat(bufff,"Server time is:");
								strcat(bufff,ctime(&tim));
								strcat(bufff,"\033[00m");


							}
							else if(strncmp(command+1,"mp",2)==0){
								if(((j=compare_pseudo(arg[1],pseudo_temporaire,pseudo_temporaire_is_set,-1))!=-1)){
									if(j!=i){
										int k=2;
										strcpy(buffer_second,"\033[35;1;4m");
										strcat(buffer_second,pseudo_temporaire[j]);
										strcat(buffer_second,":");
										strcat(buffer_second,"\033[00m");

										strcat(buffer_second,"\033[36m");
										strcat(buffer_second,"private ");
										strcat(buffer_second,"\033[00m");

										while(strlen(arg[k])>0){
											strcat(buffer_second,arg[k]);
											strcat(buffer_second," ");
											k++;
										}


										strcat(buffer_second,"\033[36m");
										strcat(buffer_second," from ");
										strcat(buffer_second,": ");
										strcat(buffer_second,"\033[00m");
										strcat(buffer_second,"\033[35;1;4m");
										strcat(buffer_second,pseudo_temporaire[i]);
										strcat(buffer_second,"\033[00m");

										strcat(buffer_second,"\n");
										strcat(buffer_second,"\033[32;1;4m");
										strcat(buffer_second,"je suis le serveur envoyez votre commande:");
										strcat(buffer_second,"\033[00m\n");

										if( send(client_socket[j],buffer_second, strlen(buffer_second),0 )==-1)
										{

											perror("send");
										}}
									else{
										strcpy(bufff,"\033[32;1;4m");
										strcat(bufff,"attention vous essayez d'envoyer quelque chose a vous meme ");
										strcat(bufff,"\033[00m");

									}



								}
								else{

									strcpy(bufff,"\033[32;1;4m");
									strcat(bufff,"pseudo n existe pas ");
									strcat(bufff,"\033[00m");


								}

							}
							else if((strncmp(command,"/alerte",7)==0)&&compare_pseudo(arg[1],pseudo_temporaire,pseudo_temporaire_is_set,-1)==-1){
								char* str=calloc(1,sizeof(buffer_second));
								strcpy(buffer_second,"\033[31;1m");
								int k=1;
								while(strlen(arg[k])>0){
									strcat(buffer_second,arg[k]);
									strcat(buffer_second," ");
									k++;
								}
								strcat(buffer_second,"\033[00m");
								strcat(buffer_second,"\n");
								strcat(buffer_second,"\033[32;1;4m");
								strcat(buffer_second,"je suis le serveur envoyez votre commande:\n");
								strcat(buffer_second,"\033[00m");
								for(int j=0;j<Max_clients;j++){

									if(client_socket[j]!=0&&client_socket[j]!=client_socket[i]){
										strcpy(str,"\033[35;1;4m");
										strcat(str,pseudo_temporaire[j]);
										strcat(str,":");
										strcat(str,"\033[00m");
										strcat(str,buffer_second);
										if( send(client_socket[j],str, strlen(str),0 )==-1)
										{

											//perror("send");
										}
										initialize_char(str,sizeof(buffer_second));
									}

								}

							}

							else if((strncmp(command,"/alerte",7)==0)&&strlen(arg[2])>0){
								if((j=compare_pseudo(arg[1],pseudo_temporaire,pseudo_temporaire_is_set,-1))!=-1){

									if(j!=i){
										int k=2;
										strcpy(buffer_second,"\033[35;1;4m");
										strcat(buffer_second,pseudo_temporaire[j]);
										strcat(buffer_second,":");
										strcat(buffer_second,"\033[00m");
										strcat(buffer_second,"\033[31;1m");
										while(strlen(arg[k])>0){
											strcat(buffer_second,arg[k]);
											strcat(buffer_second," ");
											k++;
										}
										strcat(buffer_second,"\033[00m");
										strcat(buffer_second,"\n");
										strcat(buffer_second,"\033[32;1;4m");
										strcat(buffer_second,"je suis le serveur envoyez votre commande:\n");
										strcat(buffer_second,"\033[00m");


										if( send(client_socket[j],buffer_second, strlen(buffer_second),0 )==-1)
										{

											perror("send");
										}}
									else{
										strcpy(bufff,"\033[32;1;4m");
										strcat(bufff,"attention vous essayez d'envoyer quelque chose a vous meme ");
										strcat(bufff,"\033[00m");

									}


								}

								else{

									strcpy(bufff,"\033[32m;1;4m");
									strcat(bufff,"pseudo n existe pas ");
									strcat(bufff,"\033[00m");
								}

							}
							else if((strncmp(command,"/send",5)==0)&&strlen(arg[2])!=0){
								if((j=compare_pseudo(arg[1],pseudo_temporaire,pseudo_temporaire_is_set,-1))!=-1){

									if(j!=i){
										int k=3;
										strcpy(buffer_second,"\033[35;1;4m");
										strcat(buffer_second,pseudo_temporaire[j]);
										strcat(buffer_second,":");
										strcat(buffer_second,"\033[00m");
										strcat(buffer_second,arg[2]);
										strcat(buffer_second," ");

										while(strlen(arg[k])>0){
											strcat(buffer_second,arg[k]);
											strcat(buffer_second," ");
											k++;
										}
										strcat(buffer_second,"\n");
										strcat(buffer_second,"\033[32;1;4m");
										strcat(buffer_second,"je suis le serveur envoyez votre commande:");
										strcat(buffer_second,"\033[00m\n"); 

										if( send(client_socket[j],buffer_second, strlen(buffer_second),0 )==-1)
										{

											perror("send");
										}}
									else{
										strcpy(bufff,"\033[32m;1;4m");
										strcat(bufff,"attention vous essayez d'envoyer quelque chose a vous meme ");
										strcat(bufff,"\033[00m");

									}}
								else{

									strcpy(bufff,"\033[32;1;4m");
									strcat(bufff,"pseudo n existe pas ");
									strcat(bufff,"\033[00m");


								}


							}
							else if((strncmp(command,"/join",5)==0)){
								int n=0,k=0;
								for( n=0;n<Max_clients;n++){

									if(tab_chaine[n]!=NULL&&strncmp(tab_chaine[n]->name,arg[1],strlen(arg[1]))==0){
										for( k=0;k<Max_clients;k++){
											if(tab_chaine[n]->indice_cli[k]==-1){
												tab_chaine[n]->indice_cli[k]=i;
												client_is_in_channel[i]=1;
												break;
											}
										}
										if(k!=Max_clients){
											break;}
									}

								}
								if(n==Max_clients){

									tab_chaine[i]=calloc(1,sizeof(channel));
									tab_chaine[i]->name=calloc(1,25);
									strncpy(tab_chaine[i]->name,arg[1],strlen(arg[1]));
									for( n=0;n<Max_clients;n++){
										tab_chaine[i]->indice_cli[n]=-1;}
									tab_chaine[i]->indice_cli[0]=i;
									tab_chaine[i]->indice_owner=i;
									client_is_in_channel[i]=1;
									strcpy(bufff,"\033[32;1;4m");
									strcat(bufff,"You created channel ");
									strcat(bufff,arg[1]);
									strcat(bufff," ");
									strcat(bufff,"\033[00m");



								}
								else{
									strcpy(bufff,"\033[32;1;4m");
									strcat(bufff,"You joined channel ");
									strcat(bufff,arg[1]);
									strcat(bufff," ");
									strcat(bufff,"\033[00m");
								}
							}
							else if(strncmp(command,"/kick",5)==0&&client_is_in_channel[i]==1){
								if(tab_chaine[i]!=NULL){


									int n=-1;
									if( (n=compare_pseudo(arg[1],pseudo_temporaire,pseudo_temporaire_is_set,-1))!=-1&&client_is_in_channel[n]==1){

										if(n!=i){
											for(int k=0;k<Max_clients;k++){

												if(tab_chaine[i]->indice_cli[k]==n){
													tab_chaine[i]->indice_cli[k]=-1;
													client_is_in_channel[n]=0;
													break;

												}
											}

											if(k!=Max_clients){
												strcpy(buffer_second,"\033[35;1;4m");
												strcat(buffer_second,pseudo_temporaire[n]);
												strcat(buffer_second,":");
												strcat(buffer_second,"\033[00m");
												strcat(buffer_second,"\033[32;1;4m");
												strcat(buffer_second,pseudo_temporaire[i]);
												strcat(buffer_second," vous a expulser de ");
												strcat(buffer_second,tab_chaine[i]->name);
												strcat(buffer_second," ");
												strcat(buffer_second,"je suis le serveur envoyez votre commande:");
												strcat(buffer_second,"\033[00m\n");
												if( send(client_socket[n],buffer_second, strlen(buffer_second),0 )==-1)
												{

													perror("send");
												}}
										}
										else{


											for(int k=0;k<Max_clients;k++){
												if(tab_chaine[i]->indice_cli[k]!=-1){
													strcpy(buffer_second,"\033[35;1;4m");
													strcat(buffer_second,pseudo_temporaire[tab_chaine[i]->indice_cli[k]]);
													strcat(buffer_second,":");
													strcat(buffer_second,"\033[00m");
													strcat(buffer_second,"\033[32;1;4m");
													strcat(buffer_second,pseudo_temporaire[i]);
													strcat(buffer_second," vous a expulser de ");
													strcat(buffer_second,tab_chaine[i]->name);
													strcat(buffer_second," ");
													strcat(buffer_second,"je suis le serveur envoyez votre commande:");
													strcat(buffer_second,"\033[00m\n");
													if( send(client_socket[tab_chaine[i]->indice_cli[k]],buffer_second, strlen(buffer_second),0 )==-1)
													{

														//perror("send");
													}

													client_is_in_channel[tab_chaine[i]->indice_cli[k]]=0;
													tab_chaine[i]->indice_cli[k]=-1;
													initialize_char(buffer_second,sizeof(buffer_second));


												}


											}
											tab_chaine[i]=NULL;	

										}

									}
									else{

										strcpy(bufff,"\033[32;1;4m");
										strcat(bufff,"utilisateur non trouve/utilisateur n pas dans la channel ");
										strcat(bufff,"\033[00m");


									}
								}
								else{

									strcpy(bufff,"\033[32;1;4m");
									strcat(bufff,"vous n'etes pas le modérateur de cette channel ");
									strcat(bufff,"\033[00m");


								}




							}

							else if(client_is_in_channel[i]==0){

								char* str=calloc(1,sizeof(buffer_second));
								strcpy(buffer_second,arg[0]);
								strcat(buffer_second," ");
								int k=1;
								while(strlen(arg[k])>0){
									strcat(buffer_second,arg[k]);
									strcat(buffer_second," ");
									k++;
								}

								strcat(buffer_second,"\033[35;1;4m");
								strcat(buffer_second,"from:");
								strcat(buffer_second,pseudo_temporaire[i]);
								strcat(buffer_second,"\033[00m\n");
								strcat(buffer_second,"\033[32;1;4m");
								strcat(buffer_second,"je suis le serveur envoyez votre commande:");
								strcat(buffer_second,"\033[00m\n");


								for(int j=0;j<Max_clients;j++){

									if(client_socket[j]!=0&&client_is_in_channel[j]==0&&client_socket[j]!=client_socket[i]){
										strcpy(str,"\033[35;1;4m");
										strcat(str,pseudo_temporaire[j]);
										strcat(str,":");
										strcat(str,"\033[00m");
										strcat(str,buffer_second);
										if( send(client_socket[j],str, strlen(str),0 )==-1)
										{

											//perror("send");
										}
										initialize_char(str,sizeof(buffer_second));									}




								}





							}
							else{

								char* str=calloc(1,sizeof(buffer_second));
								strcpy(buffer_second,arg[0]);
								strcat(buffer_second," ");
								int k=1;
								while(strlen(arg[k])>0){
									strcat(buffer_second,arg[k]);
									strcat(buffer_second," ");
									k++;
								}

								strcat(buffer_second,"\033[35;1;4m");
								strcat(buffer_second,"from:");
								strcat(buffer_second,pseudo_temporaire[i]);
								strcat(buffer_second,"\033[00m\n");
								strcat(buffer_second,"\033[32;1;4m");
								strcat(buffer_second,"je suis le serveur envoyez votre commande:");
								strcat(buffer_second,"\033[00m\n");

								int n=0;

								for( n=0;n<Max_clients;n++){

									if(tab_chaine[n]!=NULL){
										for( k=0;k<Max_clients;k++){
											if(tab_chaine[n]->indice_cli[k]==i){
												break;}        
										}

										if(k!=Max_clients){
											break;
										}       
									}
								}


								for(j=0;j<Max_clients;j++){
									if(tab_chaine[n]->indice_cli[j]!=-1&&tab_chaine[n]->indice_cli[j]!=i&&client_socket[tab_chaine[n]->indice_cli[j]]!=0){
										strcpy(str,"\033[35;1;4m");
										strcat(str,pseudo_temporaire[tab_chaine[n]->indice_cli[j]]);
										strcat(str,":");
										strcat(str,"\033[00m");
										strcat(str,buffer_second);

										if( send(client_socket[tab_chaine[n]->indice_cli[j]],str, strlen(str),0 )==-1)
										{

											//perror("send");
										}
										initialize_char(str,sizeof(buffer_second));
									}

								}

							}
						}

						strcpy(pseudo_buffer,"\033[35;1;4m");


						if(pseudo_temporaire_is_set[i]==1){
							strcat(pseudo_buffer,pseudo_temporaire[i]);
							strcat(pseudo_buffer,":");

						}
						strcat(pseudo_buffer,"\033[00m");
						strcat(pseudo_buffer,bufff);
						if(pseudo_temporaire_is_set[i]==1){
							strcat(pseudo_buffer,"\033[32;1;4m");
							strcat(pseudo_buffer,"je suis le serveur envoyez votre commande:");
							strcat(pseudo_buffer,"\033[00m\n");
						}
						if(send(client_socket[i],pseudo_buffer,strlen(pseudo_buffer),0)==-1)
							stop("send");



					}




				}}		
		}		}

		return 0;


	}
