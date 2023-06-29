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
#include <stdbool.h>
#define PORT 2908 //portul 

typedef struct threadData
{
    int idThread; 
    int desc_client; //desc client
    int idUser; //id user logat
}threadData;

threadData *clienti[100];
int nrClienti=0;

static void *treat(void *);
void raspunde(void *);

int main()
{
    srand(time(NULL));
    struct sockaddr_in server; //server
    struct sockaddr_in clnt; // client
    int sd; //desc socket
    int i=-1;
    //facem socket
    if((sd = socket(AF_INET, SOCK_STREAM ,0)) == -1)
        perror("Eroare socket server. \n");
    
    int on=1; setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&server, sizeof(server));
    bzero(&clnt, sizeof(clnt));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    //atasam socket
    if(bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
        perror("Eroare bind server.\n");
    
    //server asculta
    if(listen(sd,5) == -1)
        perror("Eroare la listen server. \n");

    //folosim threads pt a servi clientii
    pthread_t threads[100];
    while(1)
    {
        int client; 
        threadData *info_client;
        int length=sizeof(clnt);

        fflush(stdout);

        //astept client
        if((client= accept(sd, (struct sockaddr *)&clnt, &length)) < 0)
        {
            perror("Eroare la accept server.\n");
            continue; 
        }

        info_client=(struct threadData *)malloc(sizeof(struct threadData));
        info_client->idThread= ++i;
        info_client->desc_client= client;
        info_client->idUser= -1;
        clienti[i]= info_client;
        nrClienti=i; 

        pthread_create(&threads[i], NULL, &treat, info_client);


    }



    
}

static void *treat(void * arg)
{		
		struct threadData tdL; 
		tdL= *((struct threadData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct threadData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};

void Read(int sd, int len, char *mesaj, threadData td)
{
    int lenBytes = 0, mesajBytes = 0;
    if ((lenBytes = read(sd, &len, sizeof(int))) < 0)
        perror("[Parinte]Err...read");
    if ((mesajBytes = read(sd, mesaj, len)) < 0)
        perror("[Parinte]Err...read");
    if (lenBytes == 0 && mesajBytes == 0)
    {
        close(sd);
        close(clienti[td.idThread]->desc_client);
        clienti[td.idThread]->desc_client = -1;
    }
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

void Register(int sd, threadData td)
{
    //alocam stringurile
    char *username= (char*)malloc(50*sizeof(char));
    char *password= (char* )malloc(50*sizeof(char));
    char *r=(char* )malloc(50*sizeof(char));

    bzero(username, sizeof(username));
    bzero(password, sizeof(password));
    bzero(r, sizeof(r));

    //citim din client
    Read(sd, strlen(username),username,td);
    Read(sd, strlen(password),password,td);

    if(username==NULL || password==NULL)
        {
            strcpy(r, "Completati username si parola\n");
            write(sd, strlen(r),r);
        }

    //incercam sa ne contectam la baza de date
    sqlite3 *db;
    int rc = sqlite3_open("YAS.db", &db);

    if (rc != SQLITE_OK) {
        
        printf("Eroare deschidere BD din register\n");
        sqlite3_close(db);
        
    }
    
    //string pt insert
    char insert[200]="INSERT INTO USERS(id,username,password) VALUES(";
    int x= rand() % 100000;
    char st[100];
    sprintf(st,"%d",x);
    strcat(insert,st);
    strcat(insert,",'");
    strcat(insert,username);
    strcat(insert,"','");
    strcat(insert,password);
    strcat(insert,"');");

    char *err_msg = "Eroare insert register\n";
    rc = sqlite3_exec(db, insert, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        
        
    } 
    else
    {
        strcpy(r,"Te-ai inregistrat cu succes.\n");
        Write(sd, strlen(r),r);
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        
    }
    

}

static int callback_login(void *max_id, int count, char **data, char **columns)
{   
    int *max_id_int = max_id;
    const char *max_as_text = data[0];
    *max_id_int=atoi(max_as_text);
    return 0;
}

int Login(int sd,threadData td)
{
    char *username= (char*)malloc(50*sizeof(char));
    char *password= (char* )malloc(50*sizeof(char));
    char *r=(char* )malloc(50*sizeof(char));

    bzero(username, sizeof(username));
    bzero(password, sizeof(password));
    bzero(r, sizeof(r));

    //citim din client
    Read(sd, strlen(username),username,td);
    Read(sd, strlen(password),password,td);

    sqlite3 *db;
    int rc = sqlite3_open("YAS.db", &db);

    if (rc != SQLITE_OK) {
        
        printf("Eroare deschidere BD din Login\n");
        sqlite3_close(db);
        
    }

    char select[200]="SELECT id from USERS WHERE username='";
    strcat(select,username);
    strcat(select,"' and password='");
    strcat(select,password);
    strcat(select,"';");

    char *err_msg = "Eroare select callback login\n";
    int id_user;
    rc = sqlite3_exec(db, select, callback_login, &id_user, &err_msg);


    if (rc != SQLITE_OK ) {
        printf("Eroare select data la baza de date in Login\n");
        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return 1;
    } 
    if(id_user<0)
    {
        strcpy(r,"Username sau parola gresita.\n");
        write(sd,strlen(r),r);
        sqlite3_close(db);
    }
    else 
    {
        strcpy(r,"Logat cu succes.\n");
        Write(sd,strlen(r),r);
        clienti[td.idThread]->idUser=id_user;
        sqlite3_close(db);
        return id_user;
    }
    
}

void InsertMesajBD(int sender, int receiver, char mesaj[], int isOnOFF,sqlite3 *db)
{

    /*sqlite3 *db;
    int rc = sqlite3_open("YAS.db", &db);

    if (rc != SQLITE_OK) {
        
        printf("Eroare deschidere BD din InsertMesajDB\n");
        sqlite3_close(db);
        
    }*/
    int rc;

    char insert[200]="INSERT INTO MESAJE1(id,sender,receiver,text,isOFF,isRead) VALUES(";
    char st[100]; int id,upper=200000,lower=100001;
    id = (rand() % (upper - lower + 1)) + lower ;
    sprintf(st, "%d", id);
    strcat(insert,st);
    bzero(st,sizeof(st));
    sprintf(st,"%d",sender);
    strcat(insert,",");
    strcat(insert,st);
    strcat(insert,",");
    bzero(st,sizeof(st));
    sprintf(st,"%d",receiver);
    strcat(insert, st);
    strcat(insert, ",'");
    strcat(insert, mesaj);
    strcat(insert,"',");
    bzero(st,sizeof(st));
    sprintf(st,"%d",isOnOFF);
    strcat(insert, st);
    strcat(insert,",");
    int x=0;
    bzero(st,sizeof(st));
    sprintf(st,"%d",x);
    strcat(insert,st);
    strcat(insert,");");

    char *err_msg = "Eroare la insert mesaj in BD\n";
    rc = sqlite3_exec(db, insert, 0, 0, &err_msg);
    

}

static int callback_select_mesaj_on(void *max_id, int count, char **data, char **columns)
{   
    char *max_id_int = max_id;
    const char *max_as_text=data[0];
    strcpy(max_id_int,max_as_text);
    strcat(max_id_int, "\n");
    return 0;

}

void SendMesaj(char msj[] ,int sender, int receiver)
{
    sqlite3 *db;
    int rc = sqlite3_open("YAS.db", &db);
    int rc1,rc2;
    if (rc != SQLITE_OK) {
        
        printf("Eroare deschidere BD din SendMesaj\n");
        sqlite3_close(db);
        
    }
    char *err_msg1 = "Eroare select callback mesaj\n";
    char *err_msg2 = "Eroare update callback mesaj\n";
    char output_query[100];
   
    char select[200]="SELECT text from MESAJE1 WHERE receiver=;";
    char st[100];
    sprintf(st,"%d",sender);
    strcat(select,st);
    strcat(select, "  AND receiver=");
    bzero(st,sizeof(st));
    sprintf(st,"%d",receiver);
    strcat(select,";");
    bzero(st,sizeof(st));


    char update[200]="UPDATE MESAJE1 set isRead=1 WHERE sender=";
    sprintf(st,"%d",sender);
    strcat(select,st);
    bzero(st,sizeof(st));
    strcat(select, " AND receiver=");
    sprintf(st,"%d",receiver);
    strcat(select,st);
    bzero(st,sizeof(st));
    strcat(select," AND isOFF=0 AND isRead=0;");
    bzero(st,sizeof(st));

    int ok=0;
    for(int i=0; i<=nrClienti; ++i)
    {
        if(clienti[i]->idUser==receiver) 
        {
            ok=1; //am gasit userul
            if(clienti[i]->desc_client >=0) // receiver este online 
            {
                InsertMesajBD(sender,receiver,msj,0,db); //bagam mesaj in BD

                //luam mesajul din BD

                rc1 = sqlite3_exec(db, select, callback_select_mesaj_on, output_query, &err_msg1);
                if (rc1 != SQLITE_OK ) {
                    printf("Eroare select data la baza de date din SendMesaj\n");
                    sqlite3_free(err_msg1);
                    sqlite3_close(db);
                    return 1;
                }

                //trimitem mesajul la receiver
                Write(clienti[i]->desc_client,strlen(output_query),output_query);
                
                //update isRead=1 
                rc2=sqlite3_exec(db,update,0,0,&err_msg2);
                if (rc2 != SQLITE_OK ) {
                    printf("Eroare update data la baza de date din SendMesaj\n");
                    sqlite3_free(err_msg2);
                    sqlite3_close(db);
                    
                    return 1;
                } 
    
            }
        }
        if(ok==1)
            break;
    }

    //user este offline
    if(ok==0)
    {
        //inseram mesaj offline in BD
        InsertMesajBD(sender,receiver,msj,1,db);
    }
    sqlite3_close(db);
}

static int callback_select_measj_off(void *max_id, int count, char **data, char **columns)
{
    char *max_id_int = max_id;
    const char *max_as_text;
    for(int i=0; i<count; ++i)
    {
        max_as_text=data[i];
        strcat(max_id_int,max_as_text);
        strcat(max_id_int,"\n");
    }
    //const char *max_as_text=data[0];
    //strcpy(max_id_int,max_as_text);
   
    return 0;
}

void sendOfflineMessages(int receiver)
{
    sqlite3 *db; int rc1,rc2;
    int rc = sqlite3_open("YAS.db", &db);
    if (rc != SQLITE_OK) {
        
        printf("Eroare deschidere BD din sendOFflineMEssages\n");
        sqlite3_close(db);
        
    }

    char *err_msg1 = "Eroare select callback OfflineMesaj\n";
    char *err_msg2 = "Eroare update callback OfflineMesaj\n";
    char st[100];

    char select[200]="SELECT text from MESAJE1 WHERE receiver=";
    sprintf(st,"%d",receiver);
    strcat(select,st);
    bzero(st,sizeof(st));
    strcat(select," AND isOFF=1 AND isRead=0;");

    char update[200]="UPDATE MESAJE1 set isRead=1 WHERE receiver=";
    sprintf(st,"%d",receiver);
    strcat(update,st);
    bzero(st,sizeof(st));
    strcat(update, " AND isOFF=1 AND isRead=0;");

    char output_query[200];

    int ok=0; int poz; char raspuns[200];
    for(int i=0; i<=nrClienti; ++i)
    {
        if(clienti[i]->idUser== receiver)
        {
            ok=1; //am gasit user
            rc1 = sqlite3_exec(db, select, callback_select_measj_off, output_query, &err_msg1);
            if (rc1 != SQLITE_OK ) {
                    printf("Eroare select data la baza de date din SendOFfMEsaj\n");
                    sqlite3_free(err_msg1);
                    sqlite3_close(db);
                    
                    return 1;
                } 
            
            Write(clienti[i]->desc_client,strlen(output_query),output_query);

            rc2=sqlite3_exec(db,update,0,0,&err_msg2);
                if (rc2 != SQLITE_OK ) {
                    printf("Eroare update data la baza de date din SendMesajOff\n");
                    sqlite3_free(err_msg2);
                    sqlite3_close(db);
                
                    return 1;}
        }
        if(ok==1)
            break;
    }
    sqlite3_close(db);

}

static int callback_select_allusers(void *max_id, int count, char **data, char **columns)
{
    char *max_id_int = max_id;
    const char *max_as_text;
    for(int i=0; i<count; ++i)
    {
        max_as_text=data[i];
        strcat(max_id_int,max_as_text);
        strcat(max_id_int,"\n");
    }
    //const char *max_as_text=data[0];
    //strcpy(max_id_int,max_as_text);
   
    return 0;
}

void ShowAllUsers(int cine_cere)
{
    sqlite3 *db; int rc1;
    int rc = sqlite3_open("YAS.db", &db);
    if (rc != SQLITE_OK) {
        
        printf("Eroare deschidere BD din ShowAllUsers\n");
        sqlite3_close(db);
        
    }
    int ok=0; char *err_msg1 = "Eroare select callback ShowAllUsers";
    char output_query[100];
    char select[100]="SELECT username from USERS;";
    for(int i=0; i<=nrClienti; ++i)
    {
        if(clienti[i]->idUser == cine_cere)
        {
            ok=1;

            rc1 = sqlite3_exec(db, select, callback_select_allusers, output_query, &err_msg1);
            if (rc1 != SQLITE_OK ) {
                    printf("Eroare select data la baza de date din ShowALlUsers\n");
                    sqlite3_free(err_msg1);
                    sqlite3_close(db);
                    
                    return 1;
                } 

            Write(clienti[i]->desc_client,strlen(output_query),output_query);

        }
        if(ok==1)
            break;
    }
    sqlite3_close(db);
}

static int callback_select_ShowOnlineUsers(void *max_id, int count, char **data, char **columns)
{   
    char *max_id_int = max_id;
    const char *max_as_text=data[0];
    strcpy(max_id_int,max_as_text);
    strcat(max_id_int, "\n");
    return 0;
}

void ShowOnlineUsers(int cine_vrea)
{
    sqlite3 *db; int rc1; char raspuns[200]; int poz;
    int rc = sqlite3_open("YAS.db", &db);
    if (rc != SQLITE_OK) {
        
        printf("Eroare deschidere BD din ShowOnlineUsers\n");
        sqlite3_close(db);
        
    }
    bzero(raspuns,sizeof(raspuns));
    char *err_msg1 = "Eroare select callback ShowAOnlineUsers\n";
    char select[100]; char output_query[50];
    int id_check; char st[100]; bzero(select,sizeof(select));
    int ok=0;
    for(int j=0; j<=nrClienti; ++j)
    {
        if(clienti[j]->idUser == cine_vrea)
        {
            ok=1; poz=j;
            for(int i=0; i<=nrClienti; ++i)
            {
                id_check=clienti[i]->idUser;
                if(clienti[i]->desc_client >=0) //user este online deci afisam
                {
                    strcat(select,"SELECT username FROM USERS WHERE id=");
                    sprintf(st,"%d",id_check);
                    strcat(select,st);
                    strcat(select, ";");

                    rc1 = sqlite3_exec(db, select, callback_select_ShowOnlineUsers, output_query, &err_msg1);
                    if (rc1 != SQLITE_OK ) {
                            printf("Eroare select data la baza de date din ShowOnlineUsers\n");
                            sqlite3_free(err_msg1);
                            sqlite3_close(db);
                            
                            return 1;
                        } 
                    strcat(raspuns,output_query);
                

                }
                bzero(select,sizeof(select));
                bzero(st,sizeof(st));
                bzero(output_query,sizeof(output_query));
                
            }

        }
        if(ok==1)
            break;
    }

    Write(clienti[poz]->desc_client,strlen(raspuns),raspuns);
    sqlite3_close(db);
}
static int callback_GetUserID(void *max_id, int count, char **data, char **columns)
{   
    int *max_id_int = max_id;
    const char *max_as_text = data[0];
    *max_id_int=atoi(max_as_text);
    return 0;
}


int GetUserID(char user[])
{
    sqlite3 *db; int rc1;
    int rc = sqlite3_open("YAS.db", &db);
    if (rc != SQLITE_OK) {
        
        printf("Eroare deschidere BD din GetUserID\n");
        sqlite3_close(db);
        
    }
    char *err_msg1 = "Eroare select callback GetUserId\n";
    char select[100]="SELECT id FROM USERS WHERE username='";
    strcat(select,user);
    strcat(select, "';");
    int id_user;
    rc1 = sqlite3_exec(db, select, callback_GetUserID, &id_user, &err_msg1);
    if (rc1 != SQLITE_OK ) {
        printf("Eroare select data la baza de date in GetUserID\n");
        sqlite3_free(err_msg1);
        sqlite3_close(db);
        
        return 1;
    } 
    return id_user;
    sqlite3_close(db);
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

void raspunde(void *arg)
{
    int i=0,len,nr; char reg_atat[30]="creeare cont"; char reg[30]="register"; char log[30]="login"; char ext[30]="exit";
    struct threadData tdL; char nume_user[30]; bzero(nume_user,sizeof(nume_user));
    tdL = *((struct threadData *)arg); int id_userx; int q; int contor=0;
    char mesaj_dinclient[200],comanda_client[100];
    while(1)
    {
        bzero(comanda_client,sizeof(comanda_client));
        Read(tdL.desc_client,strlen(comanda_client),comanda_client,tdL); //read 1
        if(strlen(comanda_client)==0)
            break;
        printf("Am primit coamanda din client\n");
        if((check_string(comanda_client,reg_atat)) ==1 )
        {
            int n=3;
            while(n!=0)
            {
                Register(tdL.desc_client,tdL);
                n--;
            }
        }
        if((check_string(comanda_client,reg)) ==1 )
        {
            Register(tdL.desc_client,tdL);
            q=Login(tdL.desc_client,tdL);
            tdL.idUser=q;
            clienti[nrClienti]->idUser=q; 
            for(int a=0; a<nrClienti; ++a)
            {
                if(tdL.desc_client==clienti[a]->desc_client){
                    clienti[a]->idUser=Login(tdL.desc_client,tdL);
                    q=clienti[a]->idUser; }
                    
            }
            tdL.idUser=q;
            sendOfflineMessages(tdL.idUser);
            int nr_comanda;
            //ShowOnlineUsers= 1 
            //ShowAllUsers=2
            //SendMesaj=3
            read(tdL.desc_client, &nr_comanda,sizeof(int));
            while(1)
            {
                if(nr_comanda==1)
                 {
                    ShowOnlineUsers(tdL.idUser);
                 }
                if(nr_comanda==2)
                {
                    ShowAllUsers(tdL.idUser);
                }
                if(nr_comanda==3)
                {
                    Read(tdL.desc_client, sizeof(nume_user), nume_user,tdL);
                    id_userx=GetUserID(nume_user);
                    Read(tdL.desc_client,sizeof(mesaj_dinclient),mesaj_dinclient,tdL);
                    SendMesaj(mesaj_dinclient,tdL.idUser,id_userx);
                    bzero(mesaj_dinclient,sizeof(mesaj_dinclient));

                }
                if(nr_comanda==4)
                    close(tdL.desc_client);
                read(tdL.desc_client, &nr_comanda,sizeof(int));
            }
        }
        if((check_string(comanda_client,log)) ==1)
        {
            q=Login(tdL.desc_client,tdL);
            tdL.idUser=q;
            clienti[nrClienti]->idUser=q; 
            sendOfflineMessages(tdL.idUser);
            int nr_comanda;
            //ShowOnlineUsers= 1 
            //ShowAllUsers=2
            //SendMesaj=3
            read(tdL.desc_client, &nr_comanda,sizeof(int));
            while(1)
            {
                if(nr_comanda==1)
                 {
                    ShowOnlineUsers(tdL.idUser);
                 }
                if(nr_comanda==2)
                {
                    ShowAllUsers(tdL.idUser);
                }
                if(nr_comanda==3)
                {
                    Read(tdL.desc_client, sizeof(nume_user), nume_user,tdL);
                    id_userx=GetUserID(nume_user);
                    Read(tdL.desc_client,sizeof(mesaj_dinclient),mesaj_dinclient,tdL);
                    SendMesaj(mesaj_dinclient,tdL.idUser,id_userx);
                    bzero(mesaj_dinclient,sizeof(mesaj_dinclient));

                }
                read(tdL.desc_client, &nr_comanda,sizeof(int));
            }
        }
        if((check_string(comanda_client,ext))==1)
            exit;
        
            
    }


}