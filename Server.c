#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <mysql.h>
#include <functional>
#include <pthread.h>
#include <math.h>
#include <map>
#include <unistd.h>
#include <vector>
#include <signal.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <algorithm>
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

#define READ_CLIENT_CHECK(client,in,size)              		if(read(client,in,size)<0){perror("[SERVER]: Read error from client "); close (client); return(NULL);}
#define WRITE_CLIENT_CHECK(client,in,size)             		if(write(client,in,size)<0){perror("[SERVER]: Write error from client "); close (client);}
#define DATABASE_SERVER "localhost"
#define DATABASE_USER "bookstore"
#define DATABASE_PASSWORD "bookstore"
#define DATABASE_USED "books"
#define ROMANCE_ID 1
#define FANTASY_ID 2
#define SCIFI_ID 3
#define THRILLER_ID 4
#define MYSTERY_ID 5
#define SATIRA_ID 6
#define WEEK_LECTURE_ID  8
#define K_CLOSEST_NUMBER 0.3
typedef char BYTE;

#define PORT 1928

extern int errno;

typedef struct thData{
	int idThread;
	int client;
	MYSQL* dbConnect;
}thData;


int** userRating;

class book{
	private:
	struct{
		BYTE isNull=1;
	char * location;
	char* title;
	char* nickname;
	char* author;
	char* genres;
	char* subgenres;
	char * date;
	int id;
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
	char * getSubGenre()const{ return this->informations.subgenres;}
	char * getDate()const{ return this->informations.date;}
	int getBookID() const { return this->informations.id;}
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
	void setGenres( char * bookGenre){
		informations.genres=strdup(bookGenre);
	}
	void setSubGenres( char * bookSubGenre){
		informations.subgenres=strdup(bookSubGenre);
	}
	void setDate( char * bookDate){
		informations.date=strdup(bookDate);
	}
	void setBookID(int id){
		informations.id=id;
	}
	



};
bool sortInRev(const pair<double,int> &a,  const pair<double,int> &b) { 
       return (a.first > b.first); 
} 
int castBookGenres(string genres){
	if(strstr(genres.c_str(),"romance")||strstr(genres.c_str(),"Romance"))
		return ROMANCE_ID;
	if(strstr(genres.c_str(),"fantasy")||strstr(genres.c_str(),"Fantasy"))
		return FANTASY_ID;
	if(strstr(genres.c_str(),"mystery")||strstr(genres.c_str(),"Mystery"))
		return MYSTERY_ID;
	if(strstr(genres.c_str(),"thriller")||strstr(genres.c_str(),"Thriller"))
		return THRILLER_ID;
	if(strstr(genres.c_str(),"sci-fi")||strstr(genres.c_str(),"Sci-Fi")||strstr(genres.c_str(),"Sci-fi"))
		return SCIFI_ID;
	if(strstr(genres.c_str(),"satira")||strstr(genres.c_str(),"Satira"))
		return SATIRA_ID;
}
void populateRatingTable(MYSQL* dbConnect,int userNumber,int bookNumber,int ** userRating){

	for(int i=0;i<=userNumber;i++)
		for(int j=0;j<=bookNumber;j++)
			userRating[i][j]=0;
	
	MYSQL_RES *res_set;
	MYSQL_ROW row;
	mysql_query (dbConnect,"select user_id,book_id,stars from rating;");
	res_set = mysql_store_result(dbConnect);
	unsigned int numrows = mysql_num_rows(res_set);
	while (((row= mysql_fetch_row(res_set)) !=NULL )){
		int i=atoi(row[0]);
		int j=atoi(row[1]);
		int stars=atoi(row[2]);

		userRating[i][j]=stars;

	}


}
double cosine_similarity(int *A, int *B, unsigned int Vector_Length){
    double dot = 0.0, denom_a = 0.0, denom_b = 0.0 ;
     for(unsigned int i = 0u; i < Vector_Length; ++i) {
        dot += A[i] * B[i] ;
        denom_a += A[i] * A[i] ;
        denom_b += B[i] * B[i] ;
    }
	if(denom_a==0||denom_b==0)
		return 0;

    return dot / (sqrt(denom_a) * sqrt(denom_b)) ;
}
void userRatingCosine(int userID,int userNumber,int bookNumber,char * MYSQLcommand,MYSQL * dbConnect){
	int ** userRating= (int**) malloc((userNumber+1)*sizeof(int*));
	for (int i = 0; i <= userNumber; ++i)
    	userRating[i] = (int*) malloc((bookNumber+1)*sizeof(int));


	populateRatingTable(dbConnect,userNumber,bookNumber,userRating);
	 vector<pair<double,int>> neighbors;
	 vector<pair<double,int>> bookToSuggest;

	for(int i=1;i<=userNumber;i++){
		if(i!=userID){
		double similarities=cosine_similarity(userRating[userID],userRating[i],bookNumber);
		if(similarities>0)
			neighbors.push_back(pair<double,int>(similarities,i));
		}
	}
	if(neighbors.size()>0){

		sort(neighbors.begin(),neighbors.end(), sortInRev);


		for(int j=1;j<=bookNumber;j++){
			double score=0;
			int count=0;

			int maxNeighbors=K_CLOSEST_NUMBER*neighbors.size();
			if(maxNeighbors<=2){
				maxNeighbors=neighbors.size();
			}

			for(int i=0;i<maxNeighbors;i++){
				if(userRating[neighbors[i].second][j]!=0){
					score+=userRating[neighbors[i].second][j];
					count++;
				}
			}
			if(count>0){
				score=score/count;
				bookToSuggest.push_back(pair<double,int>(score,j));
			}
		}
		if(bookToSuggest.size()>0){
			sort(bookToSuggest.begin(),bookToSuggest.end(),sortInRev);
			sprintf(MYSQLcommand,"select title,name,genre_name from all_books left join all_genres on genre=id_genre natural join authors where id=%d and id not in (select id_book  from book_owned where id=%d ) order by rand() limit 1;",bookToSuggest[0].second,userID);
		}
		
	}
	else{
		sprintf(MYSQLcommand,"select");
	}
	for (int i = 0; i <= userNumber; ++i)
    	free(userRating[i]);
	free(userRating);	

		

}
int booksInDB(MYSQL* dbConnect){
	int number=0;
	MYSQL_RES *res_set;
	MYSQL_ROW row;
	mysql_query (dbConnect,"select AUTO_INCREMENT FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA='books' AND TABLE_NAME='all_books';");
	res_set = mysql_store_result(dbConnect);
	row=mysql_fetch_row(res_set);
	number=atoi(row[0])-1;
	return number;
}
int userInDB(MYSQL* dbConnect){
	int number=0;
	MYSQL_RES *res_set;
	MYSQL_ROW row;
	mysql_query (dbConnect,"select AUTO_INCREMENT FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA='books' AND TABLE_NAME='users';");
	res_set = mysql_store_result(dbConnect);
	row=mysql_fetch_row(res_set);
	number=atoi(row[0])-1;
	return number;
}
void addStarsToDB(MYSQL* dbConnect,const char * title,int stars,int userID){

	int starScore=stars;
	int book_id;
	MYSQL_RES *res_set;
	MYSQL_ROW row;
	mysql_query (dbConnect,"select id,title,stars from all_books;");
	unsigned int i =1;
	res_set = mysql_store_result(dbConnect);
	unsigned int numrows = mysql_num_rows(res_set);
	while (((row= mysql_fetch_row(res_set)) !=NULL )){
 		if(strcmp(row[i],title)==0){
			 if(row[i+1]!=NULL){
			 	starScore+=atoi(row[i+1]);
				starScore/=2;

				book_id=atoi(row[0]);
			 }
		 }
	}
	char MYSQLcommand[1000];
	sprintf(MYSQLcommand,"update all_books set stars=%d where title=\"%s\" ;",starScore,title);
	mysql_query (dbConnect,MYSQLcommand);

	sprintf(MYSQLcommand,"insert into rating(book_id,user_id,stars) VALUES ( %d, %d, %d) ",book_id,userID,stars);
	mysql_query (dbConnect,MYSQLcommand);
}
int addUserToDB(MYSQL * dbConnect, const char * username,const char * password){

	string userPass(password);
	int userHashedPass=hash<string>{}(userPass);

	MYSQL_RES *res_set;	
	MYSQL_ROW row;

	char MYSQLcommand[1000];
	sprintf(MYSQLcommand,"insert into users (username,password) VALUES (\"%s\",\"%d\");",username,userHashedPass);
	mysql_query (dbConnect,MYSQLcommand);

	mysql_query (dbConnect,"select id,username,password from users;");
	int i =0;
	res_set = mysql_store_result(dbConnect);
	int numrows = mysql_num_rows(res_set);
	while (((row= mysql_fetch_row(res_set)) !=NULL )){
 		if(strcmp(row[i+1],username)==0){
			 return atoi(row[i]);
		 }
	}
	return 0;
}
int logUserToDB(MYSQL * dbConnect, const char * username,const char * password){
	string userPass(password);
	int userHashedPass=hash<string>{}(userPass);
	MYSQL_RES *res_set;	
	MYSQL_ROW row;
	mysql_query (dbConnect,"select id,username,password from users;");
	unsigned int i =0;
	res_set = mysql_store_result(dbConnect);
	unsigned int numrows = mysql_num_rows(res_set);
	while (((row= mysql_fetch_row(res_set)) !=NULL )){
 		if(strcmp(row[i+1],username)==0&&(atoi(row[i+2])==userHashedPass)){\
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
	sprintf(MYSQLcommand,"update users set fst_fav_genre=\'%s\' where id=%d;",max1.str.c_str(),userID);
	mysql_query (dbConnect,MYSQLcommand);
	if(!max2.str.empty()){
		sprintf(MYSQLcommand,"update users set scd_fav_genre=\'%s\' where id=%d;",max2.str.c_str(),userID);
		mysql_query (dbConnect,MYSQLcommand);
	}

}
void addBookToUser(MYSQL * dbConnect,int userID,book& bookIN){
	char MYSQLcommand[2000];
	sprintf(MYSQLcommand,"insert into book_owned (id,book_name,author_name,genre,id_book) VALUES (%d,\"%s\",\"%s\",\"%s\",%d);",
	userID,bookIN.getTitle(),bookIN.getAuthor(),bookIN.getGenres(),bookIN.getBookID());
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
					
				case 'i':{
					char id[100];
					strcpy(id,line+3);
					id[strlen(id)-1]='\0';
					bookIN.setBookID(atoi(id));
					break;
				}
				case 'g':{
					char genre[500];
					strcpy(genre,line+3);
					genre[strlen(genre)-1]='\0';
					bookIN.setGenres(genre);
					break;
				}
				case 's':{
					char subgenre[500];
					strcpy(subgenre,line+3);
					subgenre[strlen(subgenre)-1]='\0';
					bookIN.setSubGenres(subgenre);
					break;
				}
				case 'd':{
					char date[500];
					strcpy(date,line+3);
					date[strlen(date)-1]='\0';
					bookIN.setDate(date);
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
void suggestionDBquerys(MYSQL * dbConnect, int client, const char * query,int userID){
	MYSQL_RES *res_set;	
	MYSQL_ROW row;
	int error=mysql_query (dbConnect,query);
	if(error==0){
		unsigned int i =0;
		res_set = mysql_store_result(dbConnect);
		unsigned int numrows = mysql_num_rows(res_set);
		WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
		while (((row= mysql_fetch_row(res_set)) !=NULL )){
			int firstRowSize,secondRowSize,thirdRowSize;
			if(row[i]!=NULL)
				firstRowSize=strlen(row[i]);
			WRITE_INSIDE_FUNC_CHECK(client,&firstRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i],firstRowSize);
			if(row[i+1]!=NULL)
				secondRowSize=strlen(row[i+1]);
			WRITE_INSIDE_FUNC_CHECK(client,&secondRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i+1],secondRowSize);
			if(row[i+2]!=NULL)
				thirdRowSize=strlen(row[i+2]);
			WRITE_INSIDE_FUNC_CHECK(client,&thirdRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i+2],thirdRowSize);
		}
	}
	else{
		char command[500];
		sprintf(command,"select title,name,genre_name from all_books left join all_genres on genre=id_genre natural join authors where id not in (select id_book from book_owned where id=%d) order by rand() limit 1",userID);
		mysql_query (dbConnect,command);
		res_set = mysql_store_result(dbConnect);
		int i=0;
		unsigned int numrows = mysql_num_rows(res_set);
		WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
		while (((row= mysql_fetch_row(res_set)) !=NULL )){
			int firstRowSize=strlen(row[i]);
			WRITE_INSIDE_FUNC_CHECK(client,&firstRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i],firstRowSize);

			int secondRowSize=strlen(row[i+1]);
			WRITE_INSIDE_FUNC_CHECK(client,&secondRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i+1],secondRowSize);

			int thirdRowSize=strlen(row[i+2]);
			WRITE_INSIDE_FUNC_CHECK(client,&thirdRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i+2],thirdRowSize);
		}
	}
	
	
}
void weekSuggestedlecture(MYSQL* dbConnect,int client,int userID){
	char query[200];
	sprintf(query,"select title,name,genre_name from all_books left join all_genres on all_books.genre=all_genres.id_genre natural join authors  where id=%d;",WEEK_LECTURE_ID);
	suggestionDBquerys(dbConnect,client,query,userID);
	
}
void browseAll(MYSQL* dbConnect,int client){

	MYSQL_RES *res_set;	
	MYSQL_ROW row;
	mysql_query (dbConnect,"select title,name,genre_name from all_books left join all_genres on genre=id_genre natural join authors;");
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
		int thirdRowSize=strlen(row[i+2]);
		WRITE_INSIDE_FUNC_CHECK(client,&thirdRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[i+2],thirdRowSize);

	}
	mysql_free_result(res_set); 
	

}

void browseByAuthor(MYSQL* dbConnect, int client,const char * author){
	MYSQL_RES *res_set;	
	MYSQL_ROW row;



	char MYSQLcommand[2000];
	sprintf(MYSQLcommand,"select name,description,genres,subgenres from authors where name=\"%s\";",author);
	int errors=mysql_query (dbConnect,MYSQLcommand);

	res_set = mysql_store_result(dbConnect);
	unsigned int numrows = mysql_num_rows(res_set);
	row= mysql_fetch_row(res_set);

	WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
	
	if(row!=NULL){
		int firstRowSize=strlen(row[0]);
		WRITE_INSIDE_FUNC_CHECK(client,&firstRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[0],firstRowSize);

		int secondRowSize=strlen(row[1]);
		WRITE_INSIDE_FUNC_CHECK(client,&secondRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[1],secondRowSize);

		int thirdRowSize=strlen(row[2]);
		WRITE_INSIDE_FUNC_CHECK(client,&thirdRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[2],thirdRowSize);

		int fourthRowSize=strlen(row[3]);
		WRITE_INSIDE_FUNC_CHECK(client,&fourthRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[3],fourthRowSize);



		sprintf(MYSQLcommand,"select title,name,genre_name from all_books left join all_genres on genre=id_genre natural join authors where name=\"%s\";",author);
		mysql_query (dbConnect,MYSQLcommand);
		unsigned int i =0;
		res_set = mysql_store_result(dbConnect);
		numrows = mysql_num_rows(res_set);
		WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
		while (((row= mysql_fetch_row(res_set)) !=NULL )){
			int firstRowSize=strlen(row[i]);
			WRITE_INSIDE_FUNC_CHECK(client,&firstRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i],firstRowSize);

			int secondRowSize=strlen(row[i+1]);
			WRITE_INSIDE_FUNC_CHECK(client,&secondRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i+1],secondRowSize);

			int thirdRowSize=strlen(row[i+2]);
			WRITE_INSIDE_FUNC_CHECK(client,&thirdRowSize,sizeof(int));
			WRITE_INSIDE_FUNC_CHECK(client,row[i+2],thirdRowSize);
		}
	}


	
}

void browseByGenre(MYSQL* dbConnect, int client,const char * genre){
	MYSQL_RES *res_set;	
	MYSQL_ROW row;

	char MYSQLcommand[1000];
	sprintf(MYSQLcommand,"select title,name,genre_name from all_books left join all_genres on genre=id_genre natural join authors where genre_name=\"%s\";",genre);
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

		int thirdRowSize=strlen(row[i+2]);
		WRITE_INSIDE_FUNC_CHECK(client,&thirdRowSize,sizeof(int));
		WRITE_INSIDE_FUNC_CHECK(client,row[i+2],thirdRowSize);
	}

	}
	else{
		int numrows=0;
		WRITE_INSIDE_FUNC_CHECK(client,&numrows,sizeof(int));
	}
}

void * CLIENT_CMD(void* arg){

	thData ThreadData; 
	ThreadData= *((struct thData*)arg);

	MYSQL * dbConnect=ThreadData.dbConnect;
	int client=ThreadData.client;
	int ThreadIndex=ThreadData.idThread;

	int usernameSize,passwordSize,userID=0;

	while(userID==0){//Login and register part

		BYTE loginOrregister;

		READ_CLIENT_CHECK(client,&loginOrregister,sizeof(BYTE));

		READ_CLIENT_CHECK(client,&usernameSize,sizeof(int));
		char * username=(char*)malloc(usernameSize*sizeof(char));
		READ_CLIENT_CHECK(client,username,usernameSize);

		READ_CLIENT_CHECK(client,&passwordSize,sizeof(int));
		char * password=(char*)malloc(passwordSize*sizeof(char));
		READ_CLIENT_CHECK(client,password,passwordSize);

		if(loginOrregister==1){
			userID=logUserToDB(dbConnect,username,password);
		}
		else{
			userID=addUserToDB(dbConnect,username,password);
		}
		WRITE_CLIENT_CHECK(client,&userID,sizeof(int));
		free(username);
		free(password);


	}
	

    int pid; 
	bool EXIT=false;
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

							char infoMsg[2000];
							

							sprintf(infoMsg,"Title: %s\nAuthor: %s\nGenres: %s\nSubgenres: %s\nDate:%s\nExcelent Choice!\n",clientBook.getTitle(),
									clientBook.getAuthor(),clientBook.getGenres(),clientBook.getSubGenre(),clientBook.getDate());

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
						char * browseType=(char*)malloc((browseSize+1)*sizeof(char));
						READ_CLIENT_CHECK(client,browseType,browseSize);
						browseType[browseSize]=0;

						if(strcmp(browseType,"all")==0){
							browseAll(dbConnect,client);
						}
						else
						if(strcmp(browseType,"genre")==0){


							int genreSize;
							READ_CLIENT_CHECK(client,&genreSize,sizeof(int));
							char * genre=(char*)malloc((genreSize+1)*sizeof(char));
							READ_CLIENT_CHECK(client,genre,genreSize);
							genre[genreSize]='\0';

							browseByGenre(dbConnect,client,genre);
							free(genre);

						}
						else
						if(strcmp(browseType,"author")==0){

							int authorSize;
							READ_CLIENT_CHECK(client,&authorSize,sizeof(int));
							char * author=(char*)malloc((authorSize+1)*sizeof(char));
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
						BYTE hasPreviousBooks=1;
						MYSQL_RES *res_set;	
						MYSQL_ROW row;
						
						char MYSQLcommand[2000];
						sprintf(MYSQLcommand,"select fst_fav_author,scd_fav_author,fst_fav_genre,scd_fav_genre from users where id=%d ;",userID);
						mysql_query (dbConnect,MYSQLcommand);
						
						res_set = mysql_store_result(dbConnect);
						row= mysql_fetch_row(res_set);
						if(row[0]==NULL){
							hasPreviousBooks=0;
						}

						WRITE_CLIENT_CHECK(client,&hasPreviousBooks,sizeof(BYTE));

						weekSuggestedlecture(dbConnect,client,userID);//SUGGEST THE WEEK LECTURE



						if(hasPreviousBooks==1){
							char  rowchar[300];
							string firstAuthor;
							string secondAuthor;
							string firstGenre;
							string secondGenre;
							if(row[0]!=NULL){
								sprintf(rowchar,"%s",row[0]);
								firstAuthor=rowchar;
							}
							if(row[1]!=NULL){
								sprintf(rowchar,"%s",row[1]);
								secondAuthor=rowchar;
							}
							if(row[2]!=NULL){
								sprintf(rowchar,"%s",row[2]);
								firstGenre=rowchar;
							}
							if(row[3]!=NULL){
								sprintf(rowchar,"%s",row[3]);
								secondGenre=rowchar;
							}

							int firstGenreID=castBookGenres(firstGenre);
							int secondGenreID=castBookGenres(secondGenre);
				
							srand(time(0));
							double probability=(double)rand() / (double)RAND_MAX;

							if(probability>=0.6){//BASED ON RATING-COSINE-SIMILIRATY
								int userNumber=userInDB(dbConnect);
								int bookNumber=booksInDB(dbConnect);
								sprintf(MYSQLcommand,"ala bala");
								userRatingCosine(userID, userNumber, bookNumber,MYSQLcommand,dbConnect);
							}
							else if(probability>=0.3){//BASED ON USER WITH SAME GENRE AND AUTHOR PREFERENCES
								sprintf(MYSQLcommand,"select book_name,author_name,genre from book_owned left join users on users.id=book_owned.id  where users.id!=%d and (fst_fav_author = \"%s\" or fst_fav_genre = \"%s\" or scd_fav_author= \"%s\" or scd_fav_genre= \"%s\")  and id_book not in (select id_book from book_owned where id=%d) order by rand() limit 1;",
									userID,firstAuthor.c_str(),firstGenre.c_str(),firstAuthor.c_str(),firstGenre.c_str(),userID);

							}
							else if(probability>=0.1){//BASED ON AUTHOR PREFERENCES
								if((double)rand() / (double)RAND_MAX  > 0.3||secondAuthor.empty())
									sprintf(MYSQLcommand,"select title,name,genre_name from all_books left join all_genres on all_books.genre=all_genres.id_genre natural join authors where name=\"%s\" and id not in (select id_book from book_owned where id=%d ) order by rand() limit 1;",firstAuthor.c_str(),userID);
								else
									sprintf(MYSQLcommand,"select title,name,genre_name from all_books left join all_genres on all_books.genre=all_genres.id_genre natural join authors where name=\"%s\" and id not in (select id_book  from book_owned where id=%d ) order by rand() limit 1;",secondAuthor.c_str(),userID);
						
							}
							else{//BASED ON GENRE PREFERENCES
								if((double)rand() / (double)RAND_MAX  > 0.3||secondGenre.empty())
									sprintf(MYSQLcommand,"select title,name,genre_name from all_books left join all_genres on all_books.genre=all_genres.id_genre natural join authors where id_genre=%d and id not in (select id_book from book_owned where id=%d ) order by rand() limit 1;",firstGenreID,userID);
								else
									sprintf(MYSQLcommand,"select title,name,genre_name from all_books left join all_genres on all_books.genre=all_genres.id_genre natural join authors where id_genre=%d and id not in (select id_book from book_owned where id=%d ) order by rand() limit 1;",secondGenreID,userID);
							}	
							suggestionDBquerys(dbConnect,client,MYSQLcommand,userID);

						}
						
						suggestionDBquerys(dbConnect,client,"select title,name,genre_name from all_books left join all_genres on genre=id_genre natural join authors where stars=(select MAX(stars) from all_books);",userID); //SUGGESTION based on stars
						break;

						
					}
					case 4: {//RATE
						BYTE errorFlag;
						READ_CLIENT_CHECK(client,&errorFlag,sizeof(BYTE));
						if(errorFlag==1)
							break;

						int titleSize;
						READ_CLIENT_CHECK(client,&titleSize,sizeof(int));
						char * title=(char*)malloc(titleSize*sizeof(char));
						READ_CLIENT_CHECK(client,title,titleSize);
						title[titleSize]='\0';
						
						int stars;
						READ_CLIENT_CHECK(client,&stars,sizeof(int));
						addStarsToDB(dbConnect,title,stars,userID);


						free(title);
						break;
					}
			}


		}
    close (client);
}

int main ()
{	
	thData* ThreadData;
	int ThreadIndex=0;

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
	

		ThreadData=(struct thData*)malloc(sizeof(struct thData));
		ThreadData->idThread=ThreadIndex++;
		ThreadData->client=client;
		ThreadData->dbConnect=dbConnect;



		pthread_t CLIENT_COMMANDS_THREAD;
		pthread_create(&CLIENT_COMMANDS_THREAD, NULL, CLIENT_CMD, ThreadData);


    } /* while */
	mysql_close(dbConnect);	

			
}				/* main */