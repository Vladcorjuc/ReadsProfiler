#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <mysql.h>
#include <pthread.h>
#include <map>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <fcntl.h> 
#include <time.h>
#include <sys/wait.h>
#include <stdbool.h> 

using namespace std;

#define SOCKET_TCP_CHECK(descriptor)      					if((descriptor = socket(AF_INET, SOCK_STREAM,0)) == -1){ perror("[SERVER]: Socket error "); return errno;}
#define BIND_CHECK(descriptor,structure)					if(bind(descriptor,(struct sockaddr *)&structure, sizeof(struct sockaddr))==-1){  perror("[SERVER] Bind error "); return errno; }
#define LISTEN_CHECK(descriptor)                   			if(listen(descriptor,1)==-1){ perror("[SERVER]: Listen error "); return errno;}
#define FORK_CHECK(PARENT)                          		PARENT = fork(); if (PARENT== -1){perror("[SERVER]: Fork error "); return errno;}
#define ACCEPT_CHECK(client,descriptor,structure,length)    client= accept(descriptor,(struct sockaddr *)&structure,&length); if(client<0){perror("[SERVER] Fork error "); continue; }
#define READ_CHECK(from,in,size)                   			if(read(from,in,size)<0){perror("[SERVER]: Read error "); return errno;}
#define WRITE_CHECK(from,in,size)                   		if(write(from,in,size)<0){perror("[SERVER]: Write error "); return errno;}
#define WRITE_INSIDE_FUNC_CHECK(from,in,size)				if(write(from,in,size)<0){perror("[SERVER]: Write error ");}
#define CONNECT_TO_DATABASE_CHECK(connect)					connect=mysql_init(NULL);\
															if (!connect){ \
																perror("[SERVER]: Creating connection to Database ");return errno;}\
															connect=mysql_real_connect(connect, DATABASE_SERVER, DATABASE_USER, DATABASE_PASSWORD , DATABASE_USED ,0,NULL,0);\
															if (connect){\
																perror("[SERVER]: Connecting to Database ");}\
															else{	\
																perror("[SERVER]: Connecting to Database ");return errno;}

#define READ_CLIENT_CHECK(client,in,size)              		if(read(client,in,size)<0){perror("[SERVER]: Read error from client "); close (client); continue;}
#define WRITE_CLIENT_CHECK(client,in,size)             		if(write(client,in,size)<0){perror("[SERVER]: Write error from client "); close (client); continue;}
#define DATABASE_SERVER "localhost"
#define DATABASE_USER "bookstore"
#define DATABASE_PASSWORD "bookstore"
#define DATABASE_USED "books"
typedef char BYTE;

#define PORT 2222

extern int errno;
class book{
	private:
	struct{
		BYTE isNull=1;
	char * location;
	char* title;
	char* nickname;
	char* author;
	char* genres;
	}informations;
	

	public:
	book(){
	}
	book(const book & copyCatBook){ 
		informations.location=strdup(copyCatBook.getLocation()); 
		informations.title=strdup(copyCatBook.getTitle()); 
		informations.author=strdup(copyCatBook.getAuthor()); 
		informations.genres=strdup(copyCatBook.getGenres()); informations.isNull=0; 
		}
	char * getLocation() const { return this->informations.location;}
	char * getTitle()const {return this->informations.title;}
	char * getAuthor()const {return this->informations.author;}
	char * getGenres()const { return this->informations.genres;}
	bool isNull(){return this->informations.isNull;}
	void setIsNotNull(){
		informations.isNull=0;
	}
	void setLocation(char * bookLocation){
		informations.location=strdup(bookLocation);
	}
	void setTitle(char* bookTitle){
		informations.title=strdup(bookTitle);
	}
	void setAuthor( char* bookAuthor){
		informations.author=strdup(bookAuthor);
	}
	void setNickname( char * bookNickname){
		informations.nickname=strdup(bookNickname);
	}
	void AddGenres( char * bookGenre){
		informations.genres=strdup(bookGenre);
	}


};
int addUserToDB(MYSQL * dbConnect, const char * username){

	MYSQL_RES *res_set;	
	MYSQL_ROW row;
	mysql_query (dbConnect,"select id,username from users;");
	unsigned int i =0;
	res_set = mysql_store_result(dbConnect);
	unsigned int numrows = mysql_num_rows(res_set);
	while (((row= mysql_fetch_row(res_set)) !=NULL )){
 		if(strcmp(row[i+1],username)==0){
			 return atoi(row[i]);
		 }
	}
	char MYSQLcommand[1000];
	sprintf(MYSQLcommand,"insert into users (username) VALUES (\"%s\");",username);
	mysql_query (dbConnect,MYSQLcommand);

	mysql_query (dbConnect,"select id,username from users;");
	i =0;
	res_set = mysql_store_result(dbConnect);
	numrows = mysql_num_rows(res_set);
	while (((row= mysql_fetch_row(res_set)) !=NULL )){
 		if(strcmp(row[i+1],username)==0){
			 return atoi(row[i]);
		 }
	}

	
	return 0;
}
void updateUserPreferences(MYSQL* dbConnect,int userID){
	map<string,int> genreFrecv;
	map<string,int> authorFrecv;

	MYSQL_RES *res_set;	
	MYSQL_ROW row;
	char MYSQLcommand[1000];
	sprintf(MYSQLcommand,"select author_name,genre from book_owned where id=%d;",userID);
	mysql_query (dbConnect,MYSQLcommand);
	unsigned int i =0;
	res_set = mysql_store_result(dbConnect);
	unsigned int numrows = mysql_num_rows(res_set);
	while (((row= mysql_fetch_row(res_set)) !=NULL )){
		if(authorFrecv.find(row[i])==authorFrecv.end())
			authorFrecv.insert(pair<string,int> (row[i],1));
		else
			authorFrecv[row[i]]++;
		if(genreFrecv.find(row[i+1])==genreFrecv.end())
			genreFrecv.insert(pair<string,int> (row[i+1],1));
		else
			genreFrecv[row[i+1]]++;
		
	}
	struct{
		int value=0;
		string str;
	}max1,max2;
	for( auto const& x: authorFrecv){

		if(x.second>max1.value){
			max2.value=max1.value;
			max2.str=max1.str;
			max1.value=x.second;
			max1.str=x.first;
		}
		else if(x.second==max1.value){
			max2.value=x.second;
			max2.str=x.first;
		}
	}
	printf("%s %s %d\n",max1.str.c_str(),max2.str.c_str(),userID);
	sprintf(MYSQLcommand,"update users set fst_fav_author=\'%s\' where id=%d;",max1.str.c_str(),userID);
	mysql_query (dbConnect,MYSQLcommand);
	if(!max2.str.empty()){
		sprintf(MYSQLcommand,"update users set scd_fav_author=\'%s\' where id=%d;",max2.str.c_str(),userID);
		mysql_query (dbConnect,MYSQLcommand);
	}
	max1.value=max2.value=0;
	max1.str.clear();
	max2.str.clear();
	for( auto const& x: genreFrecv){

		if(x.second>max1.value){
			max2.value=max1.value;
			max2.str=max1.str;
			max1.value=x.second;
			max1.str=x.first;
		}
		else if(x.second==max1.value){
			max2.value=x.second;
			max2.str=x.first;
		}
	}
	printf("%s %s %d\n",max1.str.c_str(),max2.str.c_str(),userID);
	sprintf(MYSQLcommand,"update users set fst_fav_genre=\'%s\' where id=%d;",max1.str.c_str(),userID);
	mysql_query (dbConnect,MYSQLcommand);
	if(!max2.str.empty()){
		sprintf(MYSQLcommand,"update users set scd_fav_genre=\'%s\' where id=%d;",max2.str.c_str(),userID);
		mysql_query (dbConnect,MYSQLcommand);
	}

}
void addBookToUser(MYSQL * dbConnect,int userID,book& bookIN){
	char MYSQLcommand[1000];
	sprintf(MYSQLcommand,"insert into book_owned (id,book_name,author_name,genre) VALUES (%d,\"%s\",\"%s\",\"%s\");",
	userID,bookIN.getTitle(),bookIN.getAuthor(),bookIN.getGenres());
	int error=mysql_query (dbConnect,MYSQLcommand);
	if(error==0){
		updateUserPreferences(dbConnect,userID);
		printf("[SERVER]: User with id=%d updated \n",userID);
	}
}
void parseBookInfos(char* location,book &bookIN){

	char * line=NULL;
	size_t len;

	bool CONTENENT=false;
	FILE * bookDescriptor=fopen(location,"r");
	while(getline(&line,&len,bookDescriptor)){
			switch(line[0]){
				case 't':{
					char title[200];
					strcpy(title,line+3);
					title[strlen(title)-1]='\0';
					bookIN.setTitle(title);
					break;
				}
					
				case 'a':{
					char author[100];
					strcpy(author,line+3);
					author[strlen(author)-1]='\0';
					bookIN.setAuthor(author);
					break;
				}
					
				case 'n':{
					char nickname[100];
					strcpy(nickname,line+3);
					nickname[strlen(nickname)-1]='\0';
					bookIN.setNickname(nickname);
					break;
				}
				case 'g':{
					char genre[500];
					strcpy(genre,line+3);
					genre[strlen(genre)-1]='\0';
					bookIN.AddGenres(genre);
					break;
				}
				case 'B':{
					CONTENENT=true;
					break;
				}
				default: break;
					
			}
			if(CONTENENT==true)break;
	}
	free(line);
}
void TransferBookContenent(char* location,book & bookIN, int clientDescriptor){
	FILE * bookDescriptor= fopen(location,"r");
	bool CONTENENT=false;
	char * line=NULL;
	size_t size;
	while(1){
		getline(&line,&size,bookDescriptor);
		if(line[0]=='B'){
			break;
		}
	}
	char ch;
	while(fread(&ch,1,1,bookDescriptor)){
		WRITE_INSIDE_FUNC_CHECK(clientDescriptor,&ch,sizeof(char));
	}
	ch='\0';
	WRITE_INSIDE_FUNC_CHECK(clientDescriptor,&ch,sizeof(char));
	fclose(bookDescriptor);
	free(line);
}
book  searchBookByTitle(const char * path ,const char * titleToSearch){
	book bookSearch;
	int titleLen=strlen(titleToSearch);
	struct dirent *de;
    DIR* dr=opendir(path);
	while ((de = readdir(dr)) != NULL){
		struct stat s;
		char possiblePath[200];
		strcpy(possiblePath,path);
		strcat(possiblePath,"/");
		strcat(possiblePath,de->d_name);
		if(strcmp(de->d_name,"..")!=0&&strcmp(de->d_name,".")!=0){
			parseBookInfos(possiblePath,bookSearch);
			int differences=0;
			for(int i=0;i<titleLen-1;i++){
				if((bookSearch.getTitle())[i]!=titleToSearch[i])
					differences++;
			}
			if(differences<=((titleLen*2)/10)){
				bookSearch.setIsNotNull();
				bookSearch.setLocation(possiblePath);
				return bookSearch;
			}
		}
	}
	closedir(dr);
	return bookSearch;

}
void suggestionDBquerys(MYSQL * dbConnect, int client, const char * query){
	MYSQL_RES *res_set;	
	MYSQL_ROW row;
	int error=mysql_query (dbConnect,query);
	if(error==0){
		unsigned int i =0;
		res_set = mysql_store_result(dbConnect);
		unsigned int numrows = mysql_num_rows(res_set);
		WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
		while (((row= mysql_fetch_row(res_set)) !=NULL )){
			int firstRowSize=strlen(row[i]);
			WRITE_INSIDE_FUNC_CHECK(client,&firstRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i],firstRowSize);

			int secondRowSize=strlen(row[i+1]);
			WRITE_INSIDE_FUNC_CHECK(client,&secondRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i+1],secondRowSize);
		}
	}
	else{
		unsigned int numrows=0;
		WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
	}
	
	
}
void browseAll(MYSQL* dbConnect,int client){
	MYSQL_RES *res_set;	
	MYSQL_ROW row;
	mysql_query (dbConnect,"select title,author from all_books;");
	unsigned int i =0;
	res_set = mysql_store_result(dbConnect);
	unsigned int numrows = mysql_num_rows(res_set);
	WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
	while (((row= mysql_fetch_row(res_set)) !=NULL )){
		int firstRowSize=strlen(row[i]);
		WRITE_INSIDE_FUNC_CHECK(client,&firstRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[i],firstRowSize);

		int secondRowSize=strlen(row[i+1]);
		WRITE_INSIDE_FUNC_CHECK(client,&secondRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[i+1],secondRowSize);
	}
	

}

void browseByAuthor(MYSQL* dbConnect, int client,const char * author){
	MYSQL_RES *res_set;	
	MYSQL_ROW row;

	char MYSQLcommand[1000];
	sprintf(MYSQLcommand,"select title,author from all_books where author=\"%s\";",author);
	mysql_query (dbConnect,MYSQLcommand);
	unsigned int i =0;
	res_set = mysql_store_result(dbConnect);
	unsigned int numrows = mysql_num_rows(res_set);
	WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
	while (((row= mysql_fetch_row(res_set)) !=NULL )){
		int firstRowSize=strlen(row[i]);
		WRITE_INSIDE_FUNC_CHECK(client,&firstRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[i],firstRowSize);

		int secondRowSize=strlen(row[i+1]);
		WRITE_INSIDE_FUNC_CHECK(client,&secondRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[i+1],secondRowSize);
	}
}

void browseByGenre(MYSQL* dbConnect, int client,const char * genre){
	MYSQL_RES *res_set;	
	MYSQL_ROW row;

	char MYSQLcommand[1000];
	sprintf(MYSQLcommand,"select title,author from %s left join all_books on %s.id=all_books.id;",genre,genre);
	int error=mysql_query (dbConnect,MYSQLcommand);
	if(error==0){
	unsigned int i =0;
	res_set = mysql_store_result(dbConnect);
	unsigned int numrows = mysql_num_rows(res_set);

	WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
	while (((row= mysql_fetch_row(res_set)) !=NULL )){
		int firstRowSize=strlen(row[i]);
		WRITE_INSIDE_FUNC_CHECK(client,&firstRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[i],firstRowSize);

		int secondRowSize=strlen(row[i+1]);
		WRITE_INSIDE_FUNC_CHECK(client,&secondRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[i+1],secondRowSize);
	}

	}
	else{
		int numrows=0;
		WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
	}
}


int main ()
{	
	MYSQL* dbConnect;
	CONNECT_TO_DATABASE_CHECK(dbConnect);

    struct sockaddr_in server;
    struct sockaddr_in from;
	int serverDescriptor;

	/*create server socket*/
	SOCKET_TCP_CHECK(serverDescriptor);
	perror("[SERVER]: CREATE SOCKET");

    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);
	BIND_CHECK(serverDescriptor,server);
	perror("[SERVER]: BIND");

	LISTEN_CHECK(serverDescriptor);
	perror("[SERVER]: LISTEN");
    while (1)
    {
    	int client;
    	socklen_t length = sizeof (from);

		ACCEPT_CHECK(client,serverDescriptor,from,length);
		perror("[SERVER]: ACCEPT");
		

		int usernameSize,userID;
		READ_CLIENT_CHECK(client,&usernameSize,sizeof(int));
		char * username=(char*)malloc(usernameSize*sizeof(char));
		READ_CLIENT_CHECK(client,username,usernameSize);
		userID=addUserToDB(dbConnect,username);
		
		free(username);

    	int pid;

		FORK_CHECK(pid);
    	if (pid > 0) {
    		close(client);
    		while(waitpid(-1,NULL,WNOHANG));
    		continue;
    	} 
		else if (pid == 0) {
			bool EXIT=false;
    		close(serverDescriptor);
			while(!EXIT){
				BYTE typeOfCommand;
    			READ_CLIENT_CHECK(client,&typeOfCommand,sizeof(BYTE));
				switch(typeOfCommand){
					case 0: {/*EXIT*/
						printf("[SERVER]: CLIENT EXIT\n");
						EXIT=true;
						break;
					}
					case 1: {/*Search and Download by title*/
						
						int titleSize;
						READ_CLIENT_CHECK(client,&titleSize,sizeof(int));
						char * titleToSearch=(char*)malloc(titleSize*sizeof(char));
						READ_CLIENT_CHECK(client,titleToSearch,titleSize);
						titleToSearch[titleSize]='\0';

						book clientBook=searchBookByTitle("BOOKS",titleToSearch);
						if(clientBook.isNull()){
							BYTE foundFlag=0;
							WRITE_CLIENT_CHECK(client,&foundFlag,sizeof(BYTE));

						}
						else{
							BYTE foundFlag=1;
							WRITE_CLIENT_CHECK(client,&foundFlag,sizeof(BYTE));

							char infoMsg[1000];
							

							sprintf(infoMsg,"Title: %s\nAuthor: %s\nGenres: %s\n\nExcelent Choice!\n",clientBook.getTitle(),
									clientBook.getAuthor(),clientBook.getGenres());

							int msgSize=strlen(infoMsg);

							WRITE_CLIENT_CHECK(client,&msgSize,sizeof(int));
							WRITE_CLIENT_CHECK(client,infoMsg,msgSize);
							
							BYTE downloadFlag;
							READ_CLIENT_CHECK(client,&downloadFlag,sizeof(BYTE));
							if(downloadFlag==1){
							
								TransferBookContenent(clientBook.getLocation(),clientBook,client);
								addBookToUser(dbConnect,userID,clientBook);
							}


						}
						free(titleToSearch);

						break;
					}
					case 2:{/*BROWSE all or by genre,author*/

						int browseSize;
						READ_CLIENT_CHECK(client,&browseSize,sizeof(int));
						char * browseType=(char*)malloc(browseSize*sizeof(char));
						READ_CLIENT_CHECK(client,browseType,browseSize);
						browseType[browseSize]=0;

						if(strcmp(browseType,"all")==0){
							browseAll(dbConnect,client);
						}
						else
						if(strcmp(browseType,"genre")==0){


							int genreSize;
							READ_CLIENT_CHECK(client,&genreSize,sizeof(int));
							char * genre=(char*)malloc(genreSize*sizeof(char));
							READ_CLIENT_CHECK(client,genre,genreSize);
							genre[genreSize]='\0';

							browseByGenre(dbConnect,client,genre);
							free(genre);

						}
						else
						if(strcmp(browseType,"author")==0){
							int authorSize;
							READ_CLIENT_CHECK(client,&authorSize,sizeof(int));
							char * author=(char*)malloc(authorSize*sizeof(char));
							READ_CLIENT_CHECK(client,author,authorSize);
							author[authorSize]='\0';

							browseByAuthor(dbConnect,client,author);
							free(author);
						}

						free(browseType);
						break;
					}
					case 3:{//SUGGEST
						//first based on prevoius genre
						MYSQL_RES *res_set;	
						MYSQL_ROW row;
						char MYSQLcommand[500];
						sprintf(MYSQLcommand,"select fst_fav_author,scd_fav_author,fst_fav_genre,scd_fav_genre from users where id=%d;",userID);
						mysql_query (dbConnect,MYSQLcommand);
						res_set = mysql_store_result(dbConnect);
						row= mysql_fetch_row(res_set);
						string firstGenre=row[2];
						string secondGenre=row[3];
						string firstAuthor=row[0];
						string secondAuthor=row[1];
						srand(time(0));

						if((double)rand() / (double)RAND_MAX  > 0.3||secondGenre.empty())
							sprintf(MYSQLcommand,"select title,author from %s left join all_books on %s.id=all_books.id order by rand() limit 1;",firstGenre.c_str(),firstGenre.c_str());
						else
							sprintf(MYSQLcommand,"select title,author from %s left join all_books on %s.id=all_books.id order by rand() limit 1;",secondGenre.c_str(),secondGenre.c_str());
						
						suggestionDBquerys(dbConnect,client,MYSQLcommand);

						if((double)rand() / (double)RAND_MAX  > 0.3||secondAuthor.empty())
							sprintf(MYSQLcommand,"select title,author from all_books where author=\"%s\" order by rand() limit 1;",firstAuthor.c_str());
						else
							sprintf(MYSQLcommand,"select title,author from all_books where author=\"%s\" order by rand() limit 1;",secondAuthor.c_str());
						
						suggestionDBquerys(dbConnect,client,MYSQLcommand);

						sprintf(MYSQLcommand,"select book_name,author_name from users left join book_owned on users.id=book_owned.id  where book_name is not null and author_name is not null and (fst_fav_author=\"%s\" or fst_fav_genre=\"%s\" or scd_fav_author=\"%s\" or scd_fav_genre=\"%s\")  and users.id != %d order by rand() limit 1;",
							firstAuthor.c_str(),firstGenre.c_str(),firstAuthor.c_str(),firstGenre.c_str(),userID);
						
						suggestionDBquerys(dbConnect,client,MYSQLcommand);

						
					}
				}


			}
			
    		close (client);
    		exit(0);
    	}

    } /* while */
	mysql_close(dbConnect);				
}				/* main */