#ifndef BAIDU_H_
#define BAIDU_H_ 

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <float.h>

#include "encodeUtils.h"
#include "cJSON.h"

#define u_long unsigned long

#define MAX_BUFFER_SIZE 4096
#define MAX_LOG_SIZE 1024
#define u_long unsigned long
#define THREAD_NUM 3
#define TYPE_FILE "File"
#define TYPE_DIR "Directory"
#define TYPE_NONE "N/A"
#define DOWNLOAD_DIR "/home/loulei/netdisk"
#define ROOT_PATH "/apps/bypy"
//https://pcs.baidu.com/rest/2.0/pcs/quota?method=info&access_token=21.6c72d1a679357c99852476bf5385db05.2592000.1492403737.1580293205-1572671&app_id=250528
#define ACCESS_TOKEN "21.e427453d75f3c2d7ca2b67759df081d9.2592000.1516082814.1580293205-1572671"
#define QUOTA_URL  "https://pcs.baidu.com/rest/2.0/pcs/quota?method=info&access_token="ACCESS_TOKEN
#define FILE_BASE_URL "https://pcs.baidu.com/rest/2.0/pcs/file?method=list&access_token="ACCESS_TOKEN"&path="
#define DOWNLOAD_BASE_URL "https://d.pcs.baidu.com/rest/2.0/pcs/file?method=download&access_token="ACCESS_TOKEN"&path="

#define SEP_LINE "====================================================\n"

#define FREE(p) \
{ \
	printf("%s:%d:%s:free(0x%lx)\n", __FILE__, __LINE__, \
	__FUNCTION__, (unsigned long)p); \
	free(p); \
}

struct MemoryStruct {
	char *memory;
	size_t size;
};

struct SubTask {
	u_long start;
	u_long end;
	char *url;
	char *filename;
	char *subfile;
	FILE *subfp;
	int index;
};

//typedef struct _ProgressData{
//	GtkWidget *pbar;
//	int timer;
//	gboolean activity_mode;
//}ProgressData;

typedef struct _DownloadFileDesc{
	long size;
	char *downloadUrl;
} DownloadFileDesc;

typedef struct _SubTask {
	u_long start;
	u_long end;
	char *url;
	char *filename;
	char *subfile;
	FILE *subfp;
	int index;
}SubTask;

void logMsg(const char *pmsg, gboolean isThread);
void clearMsg();
size_t readQuota(void *buffer, size_t size, size_t nmemb, void *userp);
size_t readDir(void *buffer, size_t size, size_t nmemb, void *userp);
void get_file_list(gpointer data);
void get_quota(gpointer data);
void parseQuota(char *msg);
void parseDir(char *msg, GtkCList *list);
void button_quota_clicked();
void button_text_clear();
void button_get_dir(gpointer data);
void button_save_file();
void button_top_dir(gpointer data);
void selection_made(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data);
gint progress_timeout(gpointer data);
//void destroy_progress(ProgressData *pdata);

void download(char *filename);
void* subDownload(void *psubtask);
size_t saveFile(void *buffer, size_t size, size_t nmemb, void *userp);
int progress_callback(void *clientp, double dltotal, double dlnow,
		double ultotal, double ulnow);
void* show_progress(void *data);
u_long get_file_size(const char *filename);

void buffer_copy(char *res, char *dest);

void init_ui(int argc, char **argv);
void init_env();

char *_(char *c){
    return(g_locale_to_utf8(c,-1,0,0,0));
}



#endif /* BAIDU_H_ */
