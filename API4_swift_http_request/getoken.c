#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <curl.h>


#define URL "http://192.168.109.133:8080/auth/v1.0"
#define FILENAME "token.log"

#define MAX_SIZE (100)

char auth_token[MAX_SIZE];
char storage_token[MAX_SIZE];
char storage_URL[MAX_SIZE];

FILE *fp = NULL;

void static _get_token_callback(void *buffer, size_t size, size_t nmemb, void *userp) 
{
		static int count = 0;
		char *tmp = NULL;

		count++;

		//the line 4:autn token
		//the line 5:storage token
		//the line 4:storage URL
		switch(count)
		{
				case 4:
						{
								tmp = strstr((char *)buffer,"X-Auth-Token:");
								if (tmp != NULL)
								{
										strncpy(auth_token,tmp,strlen(tmp)-2);
#ifdef DEBUG
										printf("auth token length:%d\n",strlen(tmp));
										printf("%s\n",auth_token);
#endif
								}
								break;
						}
				case 5:
						{
								tmp = strstr((char *)buffer,"X-Storage-Token:");
								if (tmp != NULL)
								{
										strncpy(storage_token,tmp,strlen(tmp)-2);
#ifdef DEBUG
										printf("storage token length:%d\n",strlen(tmp));
										printf("%s\n",storage_token);
#endif
								}
								break;
						}
				case 6:
						{
								tmp = strstr((char *)buffer,"X-Storage-Url:");
								if (tmp != NULL)
								{
										strncpy(storage_URL,tmp+strlen("X-Storage-Url: "),strlen(tmp)-2-strlen("X-Storage-Url: "));
#ifdef DEBUG
										printf("storage URL length:%d\n",strlen(tmp));
										printf("%s\n",storage_URL);
#endif	
								}
								break;
						}

				default :
						break;
		}

		fwrite(buffer, size, nmemb,(FILE*)userp);
}


void get_token()
{
		CURL *curl = NULL;
		
		struct curl_slist *headerlist=NULL;

		fp = fopen(FILENAME, "w");
		assert(fp);

		//request header
		headerlist = curl_slist_append(headerlist,"X-Auth-User: test:tester\r");
		headerlist = curl_slist_append(headerlist,"X-Auth-Key: testing\r");

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL,URL);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,_get_token_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);	

#ifdef DEBUG
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif

		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
}

void static _query_all_containers_callback(void *buffer, size_t size, size_t nmemb, void *userp) 
{
		char *temp  = strtok(buffer,"\n");

		while(temp)
		{
#ifdef DEBUG
				printf("container:%s\n",temp);
#endif
				temp = strtok(NULL,"\n");
		}

		fwrite(buffer, size, nmemb,(FILE*)userp);
}

void query_all_containers()
{
		CURL *curl = NULL;
		struct curl_slist *headerlist=NULL;

		//request header
		headerlist = curl_slist_append(headerlist,auth_token);

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL,storage_URL);
		
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,_query_all_containers_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);


		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);	

#ifdef DEBUG
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif

		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
}

void query_container_files(char *container)
{
		CURL *curl = NULL;
		CURLcode res;
		struct curl_slist *headerlist=NULL;
		char tmp[100];

		strncpy(tmp,storage_URL,strlen(storage_URL));
		strncat(tmp,"/",1);
		strncat(tmp,container,strlen(container));

		//request header
		headerlist = curl_slist_append(headerlist,auth_token);

		curl = curl_easy_init();


		curl_easy_setopt(curl, CURLOPT_URL,tmp);


	//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);	


		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
}

void crate_container(char *container)
{
		CURL *curl = NULL;
		CURLcode res;
		struct curl_slist *headerlist=NULL;
		char tmp[MAX_SIZE] = {0};

		strncpy(tmp,storage_URL,strlen(storage_URL));
		strncat(tmp,"/",1);
		strncat(tmp,container,strlen(container));

		//request header
		headerlist = curl_slist_append(headerlist,auth_token);

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL,tmp);

		curl_easy_setopt(curl, CURLOPT_PUT, 1L);  

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);	


		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
}

void delete_container(char *container)
{
		CURL *curl = NULL;
		CURLcode res;
		struct curl_slist *headerlist=NULL;
		char tmp[MAX_SIZE] = {0};

		strncpy(tmp,storage_URL,strlen(storage_URL));
		strncat(tmp,"/",1);
		strncat(tmp,container,strlen(container));


		//request header
		headerlist = curl_slist_append(headerlist,auth_token);

		curl = curl_easy_init();
		
		curl_easy_setopt(curl, CURLOPT_URL,tmp);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);	
		curl_easy_setopt(curl,CURLOPT_CUSTOMREQUEST,"DELETE"); 


		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);


		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
}

void upload_file(const char *file_name,const char *container)
{
		CURL *curl = NULL;

		struct curl_slist *headerlist=NULL;
		char tmp[100];

		strncpy(tmp,storage_URL,strlen(storage_URL));
		strncat(tmp,"/",1);
		strncat(tmp,container,strlen(container));
		
		strncat(tmp,"/",1);
		strncat(tmp,container,strlen(file_name));
		
		printf("automan debug!!!file name is:%s\n",tmp);

		//request header
		headerlist = curl_slist_append(headerlist,auth_token);

		curl = curl_easy_init();


		curl_easy_setopt(curl, CURLOPT_URL,tmp);
		
				curl_easy_setopt(curl, CURLOPT_PUT, 1L);  
				curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);


	//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);	


		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	
	return ;
}


static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
		curl_off_t nread;
		/* in real-world cases, this would probablyget this data differently
		   as this fread() stuff is exactly what thelibrary already would do
		   by default internally */
		size_t retcode = fread(ptr, size, nmemb,(FILE*)stream);

		nread = (curl_off_t)retcode;

		fprintf(stderr, "*** We read %"CURL_FORMAT_CURL_OFF_T
						" bytes from file\n",nread);
		return retcode;
}

int download_file(const char *file_name,const char *container)
{
		CURL *curl = NULL;
		struct curl_slist *headerlist=NULL;
 		FILE *fp = fopen(file_name, "wb");  
 		char tmp[100] = {0};
 	
 		//构建请求的URL：指定要下载的文件绝对路径
		strncpy(tmp,storage_URL,strlen(storage_URL));
		strncat(tmp,"/",1);
		strncat(tmp,container,strlen(container));
		strncat(tmp,"/",1);
		strncat(tmp,file_name,strlen(file_name));

		//构建请求头
		headerlist = curl_slist_append(headerlist,auth_token);

		curl = curl_easy_init();
		assert(curl);

		curl_easy_setopt(curl, CURLOPT_URL,tmp);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);	

		//设置回调
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA,fp);  

		//指定要下载到本地的文件描述符
		curl_easy_setopt(curl, CURLOPT_READDATA,fp);

#ifdef DEBUG
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
#endif

		curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		return 0;
}


int main(int argc, char *argv[])
{
		get_token();

		query_all_containers();

		query_container_files("swtest");

		return 0;
}
