#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>

#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#endif

#include <string.h>

//These are also located in dvio_pds.c
#define HTTP_PREFIX  "http://"
#define HTTPS_PREFIX  "https://"
#define FTP_PREFIX "ftp://"
#define SFTP_PREFIX "sftp://"
#define FILE_PREFIX "file:///"




FILE *url_file; /* declare a FILE pointer */
int unsigned long  url_data_counter = 0;

size_t	url_callback(char *, size_t , size_t, void *);


/*
 * try_remote_load - will identify if the filename is remote, 
 * and loads it if it has been downloaded properly. The filename
 * changes
 */


char * try_remote_load(const char * filename){
	char * tmpfilename = NULL;
       char * rtnfilename = NULL; 
#ifdef HAVE_LIBCURL	

	if(filename != NULL){
		//checks if libcurls URL
		if(	(strncasecmp(filename, FILE_PREFIX, strlen(FILE_PREFIX)) == 0) ||
		  	(strncasecmp(filename, HTTP_PREFIX, strlen(HTTP_PREFIX)) == 0) ||  
			(strncasecmp(filename, HTTPS_PREFIX, strlen(HTTPS_PREFIX)) == 0) ||
			(strncasecmp(filename, FTP_PREFIX, strlen(FTP_PREFIX)) == 0) ||
			(strncasecmp(filename, SFTP_PREFIX, strlen(SFTP_PREFIX)) == 0)){
			
				tmpfilename = make_temp_file_path();
 				//Now filename means URL
				if(tmpfilename != NULL  && url_create_file(tmpfilename, filename) == 0){
				   rtnfilename = strdup(tmpfilename);
				}				

				if(tmpfilename != NULL){
					free(tmpfilename);
				}
	   	}
	}  

#endif
	if (rtnfilename == NULL) rtnfilename = strdup(filename);
        return rtnfilename;
}

#ifdef HAVE_LIBCURL	

/** Return urlencoded string  (applied only to the characters <=32 and >=123 **/
char * get_loose_urlencoded(const char *url){

   int i,j = 0;
   int len = strlen(url);
   int MAX_ENCODED_LENGTH = 4; //i.e. space = %20 (3 characters)

   //length of 1  character = length of 3 encoded characters
   char ret[len*MAX_ENCODED_LENGTH];
      
   strcpy(ret,""); //Initial 
   for(i=0; i<len; i++){	      
      char enc[MAX_ENCODED_LENGTH];
      if(url[i] > 32 && url[i] < 123){
		sprintf(enc, "%c",url[i]);
      }else{
		sprintf(enc, "%%%x",url[i]);
      }
      strcat(ret, enc);
   }
   return strdup(ret);
}

int  url_create_file(const char * filename, const char * url){
	CURL *curl;
	CURLcode res;
	char errorbuff[CURL_ERROR_SIZE];
	curl = curl_easy_init();
	char *url_escaped = NULL;
	      
	if(curl) {
		//Open file for writing
		url_file = fopen(filename, "wb"); 
		if(url_file==NULL) {
			parse_error("Error: can't open file for writing.\n");
			return 1;
		}


		//Encode the URL (excluding special characters)
		if((strncasecmp(url, HTTP_PREFIX, strlen(HTTP_PREFIX)) == 0) ||
		   (strncasecmp(url, HTTPS_PREFIX, strlen(HTTPS_PREFIX)) == 0)){
			url_escaped = get_loose_urlencoded(url);
			if(url_escaped == NULL){
				parse_error("Error: could not escape url.\n");
				return 1;			
			}
			curl_easy_setopt(curl, CURLOPT_URL, url_escaped); 
		}else{
			curl_easy_setopt(curl, CURLOPT_URL, url); 
		}

//		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorbuff);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "davinci-curl");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, url_callback);

		parse_error2("Downloading.");
		res = curl_easy_perform(curl);


		/* always cleanup */
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		fclose(url_file);
		url_file = NULL;

		if(url_escaped != NULL){
			free(url_escaped);
		}

		if(res != CURLE_OK ){
		   parse_error("Download error: %s", errorbuff);
		   return 1;
		}else{
			parse_error2("done.\n");
		}
	}else{
	   return 1;
	}
	return 0;
}



size_t	url_callback(char *buffer, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	char c;
	int i=0;
	while(i < realsize) {     /* keep looping... */
		c =  buffer[i];
		fputc(c, url_file);  
		i++;
	}

	url_data_counter += realsize;
	//Show a dot every 200KB
	if(url_data_counter  >= 200000 ){
	   parse_error2(".");
	   url_data_counter = 0;
	}
	

	return realsize;
}

#endif
