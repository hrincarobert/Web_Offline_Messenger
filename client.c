#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdbool.h>
int port;

void Read(int sd, int len, char mesaj[300])
{
    if (read(sd, &len, sizeof(int)) < 0)
        perror("[Parinte]Err...read");
    if (read(sd, mesaj, len) < 0)
        perror("[Parinte]Err...read");
    // return mesaj;
}
bool Write(int sd, int len, char mesaj[300])
{
    int a;
    if (write(sd, &len, sizeof(int)) < 0)
        perror("[Parinte]Err...write");
    if ((a = write(sd, mesaj, len)) < 0)
        perror("[Parinte]Err...write");
    return a != 0;
}

int check_string(char a[],char b[])
{
    for(int i=0; i<strlen(b); ++i)
    {
        if(a[i]!=b[i])
            return 0;
    }
    return 1;
}


int main(int argc, char *argv[])
{
    int sd;                    // descriptorul de socket
    struct sockaddr_in server; // structura folosita pentru conectare
                               // mesajul trimis
    int nr = 0;
    char buf[10];
    
    /* exista toate argumentele in linia de comanda? */
    if (argc != 3)
    {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi(argv[2]);

    /* cream socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons(port);

    /* ne conectam la server */
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    char comanda_string[100],username[30],password[30],raspuns_server[100], auxiliar[100];

    while(1)
    {
          printf("%s","Bun venit!\n");
          printf("%s", "Introduceti una dintre comenzile: register , login, exit\n");
          bzero(comanda_string,sizeof(comanda_string));
          scanf("%s",comanda_string); 
          Write(sd,strlen(comanda_string),comanda_string); //write1

          if((check_string(comanda_string,"creeare cont"))==1)
          {
                int n=3;
                while(n!=0)
                {
                    printf("Creeare cont:\n");
                    printf("Username: ");
                    scanf("%s",username);
                    printf("\n");
                    printf("password: ");
                    scanf("%s",password);
                    printf("\n");
                    Write(sd, strlen(username), username);
                    Write(sd, strlen(password), password);
                    bzero(raspuns_server,sizeof(raspuns_server));
                    Read(sd,strlen(raspuns_server),raspuns_server);
                    printf("%s",raspuns_server); 

                    bzero(raspuns_server,sizeof(raspuns_server));
                    bzero(username,sizeof(username));
                    bzero(password,sizeof(password));
                    n--;
                }
          }

          if((check_string(comanda_string,"register"))==1)
          {
                printf("Username: ");
                scanf("%s",username);
                printf("\n");
                printf("password: ");
                scanf("%s",password);
                printf("\n");
                Write(sd, strlen(username), username);
                Write(sd, strlen(password), password);

                bzero(raspuns_server,sizeof(raspuns_server));
                Read(sd,strlen(raspuns_server),raspuns_server);
                printf("%s",raspuns_server); 

                bzero(raspuns_server,sizeof(raspuns_server));

                printf("Log in pentru a continua!\n");
                bzero(username,sizeof(username));
                bzero(password,sizeof(password));
                printf("Username: ");
                scanf("%s",username);
                printf("\n");
                printf("password: ");
                scanf("%s",password);
                printf("\n");
                Write(sd, strlen(username), username);
                Write(sd, strlen(password), password);
                Read(sd,strlen(raspuns_server),raspuns_server);
                printf("%s",raspuns_server); 

                /*if(strcmp(raspuns_server,"Logat cu succes.") ==0)
                {*/
                    bzero(raspuns_server,sizeof(raspuns_server));
                    printf("Mesajele offline sunt(daca sunt):\n");
                    Read(sd,strlen(raspuns_server),raspuns_server);
                    printf("\n");
                    printf("%s",raspuns_server); 
                    printf("Intorduceti o comanda : \n 1-Show Online Users \n 2-Show All Users \n 3-Send Message 4-Logout");
                    int raspuns_int,comanda_int;
                    scanf("%d",&comanda_int);
                    write(sd,&comanda_int,sizeof(int));
                    while(1) //intram in login si acceptam comnezi
                    {
                        if(comanda_int==1)
                        {
                            bzero(raspuns_server,sizeof(raspuns_server));
                            printf("Userii online sunt:\n");
                            Read(sd,strlen(raspuns_server),raspuns_server);
                            printf("%s",raspuns_server);

                        }
                        if(comanda_int==2)
                        {
                            bzero(raspuns_server,sizeof(raspuns_server));
                            printf("Toti userii sunt:\n");
                            Read(sd,strlen(raspuns_server),raspuns_server);
                            printf("%s",raspuns_server);

                        }
                        if(comanda_int==3)
                        {
                            bzero(raspuns_server,sizeof(raspuns_server));
                            printf("Cui user doriti sa trimiteti mesaj?\n Specificati username:");
                            scanf("%s",auxiliar);
                            Write(sd,strlen(auxiliar),auxiliar);
                            printf("Mesaj: ");
                            bzero(auxiliar,sizeof(auxiliar));
                            scanf("%s",auxiliar);
                            Write(sd,strlen(auxiliar),auxiliar);
                            Read(sd,strlen(raspuns_server),raspuns_server);
                            printf("%s",raspuns_server);

                        }
                        if(comanda_int==4)
                            break;
                        if(comanda_int==0)
                        {
                            bzero(raspuns_server,sizeof(raspuns_server));
                            printf("\n");
                            Read(sd,strlen(raspuns_server),raspuns_server);
                            printf("%s\n",raspuns_server);
                        }
                        printf("Introduceti alta comanda: \n");
                        scanf("%d",&comanda_int);
                        write(sd,&comanda_int,sizeof(int));
                    }
                //}
                

          }

          if((check_string(comanda_string,"login"))==1)
          {
                bzero(username,sizeof(username));
                bzero(password,sizeof(password));
                printf("Username: ");
                scanf("%s",username);
                printf("\n");
                printf("password: ");
                scanf("%s",password);
                printf("\n");
                Write(sd, strlen(username), username);
                Write(sd, strlen(password), password);
                Read(sd,strlen(raspuns_server),raspuns_server);
                printf("%s",raspuns_server); 

                /*if((strcmp(raspuns_server,"Logat cu succes.")) ==0)
                {*/
                    bzero(raspuns_server,sizeof(raspuns_server));
                    printf("Mesajele offline sunt(daca sunt):\n");
                    Read(sd,strlen(raspuns_server),raspuns_server);
                    printf("\n");
                    printf("%s",raspuns_server); 
                    printf("Intorduceti o comanda : \n 1-Show Online Users \n 2-Show All Users \n 3-Send Message \n 4-Logout");
                    int raspuns_int,comanda_int;
                    scanf("%d",&comanda_int);
                    write(sd,&comanda_int,sizeof(int));
                    while(1) //intram in login si acceptam comnezi
                    {
                        if(comanda_int==1)
                        {
                            bzero(raspuns_server,sizeof(raspuns_server));
                            printf("Userii online sunt:\n");
                            Read(sd,strlen(raspuns_server),raspuns_server);
                            printf("%s",raspuns_server);

                        }
                        if(comanda_int==2)
                        {
                            bzero(raspuns_server,sizeof(raspuns_server));
                            printf("Toti userii sunt:\n");
                            Read(sd,strlen(raspuns_server),raspuns_server);
                            printf("%s",raspuns_server);

                        }
                        if(comanda_int==3)
                        {
                            bzero(raspuns_server,sizeof(raspuns_server));
                            printf("Cui user doriti sa trimiteti mesaj?\n Specificati username:");
                            scanf("%s",auxiliar);
                            Write(sd,strlen(auxiliar),auxiliar);
                            printf("Mesaj: ");
                            bzero(auxiliar,sizeof(auxiliar));
                            scanf("%s",auxiliar);
                            Write(sd,strlen(auxiliar),auxiliar);
                            Read(sd,strlen(raspuns_server),raspuns_server);
                            printf("%s",raspuns_server);

                        }
                        if(comanda_int==4)
                            break;
                        if(comanda_int==0)
                        {
                            bzero(raspuns_server,sizeof(raspuns_server));
                            printf("\n");
                            Read(sd,strlen(raspuns_server),raspuns_server);
                            printf("%s\n",raspuns_server);
                        }
                        printf("Introduceti alta comanda: \n");
                        scanf("%d",&comanda_int);
                        write(sd,&comanda_int,sizeof(int));
                    }
                //}
          }

    }
    close(sd);

}