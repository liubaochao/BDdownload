#include "baidu.h"

char *currentDir;
char *selectedFile;
GtkWidget *textview_log;
gboolean is_downloading = FALSE;
int persents[THREAD_NUM];
GtkWidget *progressbar;

int main(int argc, char **argv) {
	init_env();
	init_ui(argc, argv);
	return 0;
}

void init_ui(int argc, char **argv) {
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *scrolled_window, *clist;
	GtkWidget *scrolled_text;
	GtkWidget *button_quota;
	GtkWidget *button_clear;
	GtkWidget *button_dir;
	GtkWidget *button_top;
	GtkWidget *button_save;
	GtkWidget *hbox_button;

//	ProgressData *pdata;

	gchar *titles[3] = { "Path", "Size", "Type" };

	if (!g_thread_supported())
		g_thread_init(NULL);
	gdk_threads_init();

	gtk_init(&argc, &argv);
	//init window
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_usize(GTK_WIDGET(window), 800, 600);
	gtk_window_set_title(GTK_WINDOW(window), "BaiduYunPan");
	gtk_signal_connect(GTK_OBJECT(window), "destroy",
			GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

	//vbox
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show(vbox);

	//scroll for list
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
	gtk_widget_show(scrolled_window);

	//listview
	clist = gtk_clist_new_with_titles(3, titles);
	gtk_clist_set_shadow_type(GTK_CLIST(clist), GTK_SHADOW_OUT);
	gtk_clist_set_column_width(GTK_CLIST(clist), 0, 400);
	gtk_clist_set_column_width(GTK_CLIST(clist), 1, 200);
	gtk_clist_set_column_width(GTK_CLIST(clist), 2, 200);
	gtk_clist_set_column_justification(GTK_CLIST(clist), 0, GTK_JUSTIFY_LEFT);
	gtk_clist_set_column_justification(GTK_CLIST(clist), 1, GTK_JUSTIFY_CENTER);
	gtk_clist_set_column_justification(GTK_CLIST(clist), 2, GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
	gtk_widget_show(clist);

	//hbox for button
	hbox_button = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_button, FALSE, TRUE, 0);
	gtk_widget_show(hbox_button);

	//quota button
	button_quota = gtk_button_new_with_label("Quota");
	gtk_box_pack_start(GTK_BOX(hbox_button), button_quota, TRUE, TRUE, 0);
	gtk_widget_show(button_quota);

	//dir button
	button_dir = gtk_button_new_with_label("Dir");
	gtk_box_pack_start(GTK_BOX(hbox_button), button_dir, TRUE, TRUE, 0);
	gtk_widget_show(button_dir);

	//top button
	button_top = gtk_button_new_with_label("Top");
	gtk_box_pack_start(GTK_BOX(hbox_button), button_top, TRUE, TRUE, 0);
	gtk_widget_show(button_top);

	//download button
	button_save = gtk_button_new_with_label("Save");
	gtk_box_pack_start(GTK_BOX(hbox_button), button_save, TRUE, TRUE, 0);
	gtk_widget_show(button_save);

	//progressbar
//	pdata = g_malloc(sizeof(ProgressData));
	progressbar = gtk_progress_bar_new();
	gtk_progress_set_format_string(progressbar, "%v");
	gtk_box_pack_start(GTK_BOX(vbox), progressbar, FALSE, FALSE, 0);
	gtk_widget_show(progressbar);

	//text view
	textview_log = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview_log), FALSE);

	//scroll for textview
	scrolled_text = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_text),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_text, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(scrolled_text), textview_log);
	gtk_widget_show(textview_log);
	gtk_widget_show(scrolled_text);

	//clear button
	button_clear = gtk_button_new_with_label("Clear");
	gtk_box_pack_start(GTK_BOX(vbox), button_clear, FALSE, FALSE, 0);
	gtk_widget_show(button_clear);

	//register event
	gtk_signal_connect_object(GTK_OBJECT(button_quota), "clicked",
			GTK_SIGNAL_FUNC(button_quota_clicked), NULL);
	gtk_signal_connect_object(GTK_OBJECT(button_clear), "clicked",
			GTK_SIGNAL_FUNC(button_text_clear), NULL);
	gtk_signal_connect_object(GTK_OBJECT(button_dir), "clicked",
			GTK_SIGNAL_FUNC(button_get_dir), clist);
	gtk_signal_connect(GTK_OBJECT(clist), "select_row",
			GTK_SIGNAL_FUNC(selection_made), NULL);
	gtk_signal_connect_object(GTK_OBJECT(button_save), "clicked",
			GTK_SIGNAL_FUNC(button_save_file), NULL);
	gtk_signal_connect_object(GTK_OBJECT(button_top), "clicked",
			GTK_SIGNAL_FUNC(button_top_dir), clist);

	gtk_widget_show(window);

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
}

void init_env() {
	currentDir = (char*) calloc(sizeof(char), MAX_BUFFER_SIZE);
	selectedFile = (char*) calloc(sizeof(char), MAX_BUFFER_SIZE);
	strcpy(currentDir, ROOT_PATH);
	strcpy(selectedFile, ROOT_PATH);
	curl_global_init(CURL_GLOBAL_ALL);
}

size_t readQuota(void *buffer, size_t size, size_t nmemb, void *userp) {
	char *ptr = (char*) calloc(sizeof(char), nmemb * size + 1);
	memcpy(ptr, buffer, nmemb * size);
	printf("quota:%s\n", ptr);
	parseQuota(ptr);
	FREE(ptr);
	return nmemb * size;
}

size_t readDir(void *buffer, size_t size, size_t nmemb, void *userp) {
	size_t realsize = nmemb * size;
	struct MemoryStruct *mem = (struct MemoryStruct *) userp;
	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		printf("not enough memory\n");
		return 0;
	}
	memcpy(&(mem->memory[mem->size]), buffer, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}

size_t saveFile(void *buffer, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	FILE *fp = (FILE*) userp;
	fwrite(buffer, size, nmemb, fp);
	return realsize;
}

int progress_callback(void *clientp, double dltotal, double dlnow,
		double ultotal, double ulnow) {
	struct SubTask *stp = (struct SubTask *) clientp;
	if (dltotal > 0 && dlnow >= 0 && dltotal >= dlnow) {
		persents[stp->index] = (int) (dlnow * 100 / dltotal);
	}
	return 0;
}

void parseQuota(char *msg) {
	if (msg == NULL)
		return;
	cJSON *pJson = cJSON_Parse(msg);
	if (NULL == pJson)
		return;
	char *buffer = (char*) calloc(sizeof(char), MAX_BUFFER_SIZE);
	snprintf(buffer, MAX_BUFFER_SIZE,
	SEP_LINE
	"quota:%.0lf\n"
	"used:%.0lf\n"
	"requestId:%.0lf\n"
	SEP_LINE, cJSON_GetObjectItem(pJson, "quota")->valuedouble,
			cJSON_GetObjectItem(pJson, "used")->valuedouble,
			cJSON_GetObjectItem(pJson, "request_id")->valuedouble);
	logMsg(buffer, TRUE);
	cJSON_Delete(pJson);
	FREE(buffer);
}

void parseDir(char *msg, GtkCList *list) {
	if (msg == NULL) {
		return;
	}
	printf("msg:%s\n", msg);
	cJSON *pJson = cJSON_Parse(msg);
	if (NULL == pJson) {
		printf("parse error\n");
		char *errormsg = (char*) calloc(sizeof(char), MAX_LOG_SIZE);
		snprintf(errormsg, MAX_LOG_SIZE, "[%d] [%s] parse error\n", __LINE__,
				__FUNCTION__);
		logMsg(errormsg, TRUE);
		FREE(errormsg);
		return;
	}
	cJSON *pArray = cJSON_GetObjectItem(pJson, "list");
	int count = cJSON_GetArraySize(pArray);
	gchar *filelist[count][3];
	int i;
	for (i = 0; i < count; i++) {
		cJSON *pitem = cJSON_GetArrayItem(pArray, i);
		char *path = cJSON_GetObjectItem(pitem, "path")->valuestring;
		double size = cJSON_GetObjectItem(pitem, "size")->valuedouble;
		int isDir = cJSON_GetObjectItem(pitem, "isdir")->valueint;
		filelist[i][0] = path;
		if (isDir) {
//			printf("%s\n", path);
			filelist[i][1] = TYPE_NONE;
			filelist[i][2] = TYPE_DIR;
		} else {
			int flag = 0;
			while (size > 1024) {
				size /= 1024;
				flag++;
			}
			char *unit = "unknowen size unit";
			if (flag == 0)
				unit = "byte";
			else if (flag == 1)
				unit = "K";
			else if (flag == 2)
				unit = "M";
			else if (flag == 3)
				unit = "G";
			else if (flag == 4)
				unit = "T";
//			printf("%s\t\tsize:%.2lf%s\n", path, size, unit);
			char *ssize = (char*) calloc(sizeof(char), 50);
			snprintf(ssize, 50, "%.2lf%s", size, unit);
			filelist[i][1] = ssize;
			filelist[i][2] = TYPE_FILE;
		}
	}

	gdk_threads_enter();
	gtk_clist_clear(list);
	for (i = 0; i < count; i++) {
		gtk_clist_append(list, filelist[i]);
	}
	gdk_threads_leave();
	memset(currentDir, 0, MAX_BUFFER_SIZE);
	memcpy(currentDir, selectedFile, MAX_BUFFER_SIZE);
	memset(selectedFile, 0, MAX_BUFFER_SIZE);
	cJSON_Delete(pJson);
}

void get_quota(gpointer data) {
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, QUOTA_URL);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &readQuota);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			GtkWidget *text = GTK_TEXT_VIEW(data);
			char *errormsg = (char*) calloc(sizeof(char), MAX_LOG_SIZE);
			snprintf(errormsg, MAX_LOG_SIZE, "[%d] [%s] %s\n", __LINE__,
					__FUNCTION__, curl_easy_strerror(res));
			logMsg(errormsg, TRUE);
			FREE(errormsg);
		}
	}
	curl_easy_cleanup(curl);
}

void get_file_list(gpointer data) {
	if (!selectedFile)
		return;
	GtkCList *list = GTK_CLIST(data);
	unsigned int len = strlen(selectedFile);
	char encodeStr[MAX_BUFFER_SIZE] = { 0 };
	urlencode(selectedFile, len, encodeStr, MAX_BUFFER_SIZE);
	char *url = malloc(strlen(FILE_BASE_URL) + strlen(encodeStr) + 1);
	strcpy(url, FILE_BASE_URL);
	strcat(url, encodeStr);

	CURL *curl;
	CURLcode res;

	struct MemoryStruct chunk;
	chunk.memory = malloc(1);
	chunk.size = 0;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
	}
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &readDir);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void * )&chunk);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		char *errormsg = (char*) calloc(sizeof(char), MAX_LOG_SIZE);
		snprintf(errormsg, MAX_LOG_SIZE, "[%d] [%s] %s\n", __LINE__,
				__FUNCTION__, curl_easy_strerror(res));
		logMsg(errormsg, TRUE);
		FREE(errormsg);
	} else {
		parseDir(chunk.memory, list);
		FREE(chunk.memory);
	}
	curl_easy_cleanup(curl);
	FREE(url);
}

void button_quota_clicked() {
	g_thread_create((GThreadFunc) get_quota, NULL, FALSE, NULL);
}

void button_text_clear() {
	g_thread_create((GThreadFunc) clearMsg, NULL, FALSE, NULL);
}

void button_get_dir(gpointer data) {
	g_thread_create((GThreadFunc) get_file_list, data, FALSE, NULL);
}

void button_save_file() {
	printf("save file ...\n");
//	ProgressData *pdata = (ProgressData*)data;
//	pdata->activity_mode = TRUE;
//	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pdata->pbar), 0.0);
//	pdata->timer = gtk_timeout_add(100, progress_timeout, pdata);
	if (selectedFile && strlen(selectedFile)) {
//		printf("selected:%s\n", selectedFile);
		g_thread_create((GThreadFunc) download, selectedFile, FALSE, NULL);
	} else {
		logMsg("must select download file !\n", FALSE);
	}

}

void button_top_dir(gpointer data) {
	if (!strcmp(ROOT_PATH, currentDir)) {
		logMsg("root path already !\n", FALSE);
	} else {
		char *tail = strrchr(currentDir, '/');
		if (tail) {
			memset(selectedFile, 0, MAX_BUFFER_SIZE);
			memcpy(selectedFile, currentDir, strlen(currentDir) - strlen(tail));
			button_get_dir(data);
		}
	}
}

void download(char *filename) {
	char *msg_log = (char*)calloc(sizeof(char), MAX_BUFFER_SIZE);
	char encodeStr[MAX_BUFFER_SIZE] = { 0 };
	urlencode(filename, strlen(filename), encodeStr, MAX_BUFFER_SIZE);
	char *singlename = strrchr(filename, '/');
	char *localfile = calloc(strlen(DOWNLOAD_DIR) + strlen(singlename) + 1,
			sizeof(char));
	strcpy(localfile, DOWNLOAD_DIR);
	strcat(localfile, singlename);
	logMsg(localfile, TRUE);
	logMsg("\n", TRUE);

	char *url = calloc(strlen(DOWNLOAD_BASE_URL) + strlen(encodeStr) + 1,
			sizeof(char));
	strcpy(url, DOWNLOAD_BASE_URL);
	strcat(url, encodeStr);

	DownloadFileDesc *pDesc = (DownloadFileDesc *) malloc(
			sizeof(DownloadFileDesc));
	memset(pDesc, 0, sizeof(DownloadFileDesc));

	getDownloadFileLength(url, pDesc);
	char *download_url = pDesc->downloadUrl;

	long filesize = pDesc->size;
	long tail = filesize % THREAD_NUM;
	long page = filesize / THREAD_NUM;

	memset(msg_log, 0, MAX_BUFFER_SIZE);
	snprintf(msg_log, MAX_BUFFER_SIZE, SEP_LINE
	"filesize:%ld\n"
	"pagesize:%ld\n"
	"tailsize:%ld\n"
	SEP_LINE, filesize, page, tail);
	logMsg(msg_log, TRUE);

	time_t startp, endp;
	time(&startp);

	SubTask subtasks[THREAD_NUM];
	int i;
	for (i = 0; i < THREAD_NUM; i++) {
		subtasks[i].start = i * page;
		subtasks[i].end = (i + 1) * page - 1;
		subtasks[i].filename = localfile;
		subtasks[i].url = download_url;
		subtasks[i].index = i;
	}
	subtasks[THREAD_NUM - 1].end += tail;

	is_downloading = TRUE;
	pthread_t pts[THREAD_NUM];
//	GThread* gts[THREAD_NUM];
	pthread_t pprogress;
	for (i = 0; i < THREAD_NUM; i++) {
		int pindex = i;
		pthread_create(&pts[pindex], NULL, subDownload, &subtasks[pindex]);
//		gts[i] = g_thread_create(subDownload, &subtasks[pindex], TRUE, NULL);
	}
	pthread_create(&pprogress, NULL, show_progress, (void*) NULL);
//	g_thread_create(show_progress, NULL, FALSE, NULL);
	for (i = 0; i < THREAD_NUM; i++) {
		pthread_join(pts[i], NULL);
//		g_thread_join(gts[i]);
	}
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), 1.0000000);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar), "Progress 100%");

	is_downloading = FALSE;

	FREE(url);
	time(&endp);
	memset(msg_log, 0, MAX_BUFFER_SIZE);
//	printf("%ld %ld %ld\n", filesize, endp, startp);
	long cost = (endp - startp) == 0 ? 1 : (endp - startp);
	snprintf(msg_log, MAX_BUFFER_SIZE, SEP_LINE
			"download finish and cost : %zu seconds\n"
			"average speed : %ld bps\n"
			"start combination ...\n"
			SEP_LINE
			, cost, filesize / cost);
	logMsg(msg_log, TRUE);

	do {
		for(i=0; i<THREAD_NUM; i++){
			buffer_copy(subtasks[i].subfile, localfile);
		}
	} while (0);

	logMsg("combination completely\n", TRUE);
	for (i = 0; i < THREAD_NUM; i++) {
		persents[i] = 0;
		unlink(subtasks[i].subfile);
		FREE(subtasks[i].subfile);
	}
	FREE(msg_log);
	FREE(localfile);

}

void* subDownload(gpointer psubtask) {
	char *log_msg = (char*)malloc(MAX_BUFFER_SIZE);
	struct SubTask *stp = (struct SubTask *) psubtask;

	char range[50];
	sprintf(range, "%ld-%ld", stp->start, stp->end);

	char *subfile = calloc(strlen(stp->filename) + 200, sizeof(char));
	strcpy(subfile, stp->filename);
	strcat(subfile, "_");
	strcat(subfile, range);
	stp->subfile = subfile;


	CURL *curl;
	CURLcode res;
	FILE *fp = NULL;
start:
	if(access(stp->subfile, F_OK | W_OK)){
		fp = fopen(stp->subfile, "wb+");
	}else{
		fp = fopen(stp->subfile, "ab+");
	}
	if(!fp){
		printf("[Error] task%d : %s\n", stp->index, strerror(errno));
		return NULL;
	}

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, stp->url);
	}




	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &saveFile);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, stp);

	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

	//set range
	u_long current_size = get_file_size(stp->subfile);
	snprintf(log_msg, MAX_BUFFER_SIZE, "[INFO] task%d : current size:%ld\n", stp->index, current_size);
	logMsg(log_msg, TRUE);
	u_long start = stp->start + current_size;
	if(start > stp->end){
		memset(log_msg, 0, MAX_BUFFER_SIZE);
		snprintf(log_msg, MAX_BUFFER_SIZE, "[INFO] task%d : complete\n", stp->index);
		logMsg(log_msg, TRUE);
		curl_easy_cleanup(curl);
		persents[stp->index] = 100;
		return NULL;
	}
	sprintf(range, "%ld-%ld", start, stp->end);

	memset(log_msg, 0, MAX_BUFFER_SIZE);
	snprintf(log_msg, MAX_BUFFER_SIZE, "[INFO] task%d : %s\n", stp->index, range);
	logMsg(log_msg, TRUE);

	curl_easy_setopt(curl, CURLOPT_RANGE, range);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		memset(log_msg, 0, MAX_BUFFER_SIZE);
		snprintf(log_msg, MAX_BUFFER_SIZE, "[Error] task%d : %s\n", stp->index, curl_easy_strerror(res));
		logMsg(log_msg, TRUE);
	}
	fclose(fp);
	memset(log_msg, 0, MAX_BUFFER_SIZE);
	snprintf(log_msg, MAX_BUFFER_SIZE, "[INFO] task%d : subtask %d finish\n", stp->index, stp->index);
	logMsg(log_msg, TRUE);
	curl_easy_cleanup(curl);

	u_long expectsize = stp->end - stp->start + 1;
	u_long realsize = get_file_size(stp->subfile);
	memset(log_msg, 0, MAX_BUFFER_SIZE);
	snprintf(log_msg, MAX_BUFFER_SIZE, "[INFO] task%d : expect:%ld, real:%ld\n", stp->index, expectsize, realsize);
	logMsg(log_msg, TRUE);
	if (expectsize == realsize) {
		memset(log_msg, 0, MAX_BUFFER_SIZE);
		snprintf(log_msg, MAX_BUFFER_SIZE, log_msg, "[INFO] task%d : check ok\n", stp->index);
		logMsg(log_msg, TRUE);
	} else {
		memset(log_msg, 0, MAX_BUFFER_SIZE);
		snprintf(log_msg, MAX_BUFFER_SIZE, "[Error] task%d : check fail , again !!! \n", stp->index);
		logMsg(log_msg, TRUE);
		sleep(stp->index * 3);
		goto start;
	}
	FREE(log_msg);
	return NULL;
}

u_long get_file_size(const char *filename) {
	u_long filesize = -1;
	struct stat statbuff;
	if (stat(filename, &statbuff) < 0) {
		return filesize;
	} else {
		filesize = statbuff.st_size;
	}
	return filesize;
}

void* show_progress(gpointer data) {
	while (is_downloading) {
		int i;
		int totalPersent = 0;
		for (i = 0; i < THREAD_NUM; i++) {
			char progress_msg[MAX_BUFFER_SIZE];
			snprintf(progress_msg, MAX_BUFFER_SIZE, "thread%d:%d\t", (i + 1),
					persents[i]);
			logMsg(progress_msg, TRUE);
			totalPersent += persents[i];
		}
		logMsg("\n", TRUE);

		int persent = totalPersent / THREAD_NUM;
		if(persent < 0) persent = 0;
		if(persent > 100) persent = 100;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), persent*1.0000000/100);
		char str_persent[MAX_BUFFER_SIZE];
		snprintf(str_persent, 20, "Progress %d%%", persent);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar), str_persent);
		sleep(5);
	}
	return NULL;
}

void getDownloadFileLength(const char *url, DownloadFileDesc *pdesc) {
	double downloadFileLenght = 0;
	CURL *curl;
	CURLcode res;
	char* new_url = NULL;

	while (!pdesc->downloadUrl || downloadFileLenght <= 0) {

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);


		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			printf("failed:%s\n", curl_easy_strerror(res));
			downloadFileLenght = -1;
		} else {
			curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &new_url);
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
					&downloadFileLenght);
			char *url = calloc(sizeof(char), strlen(new_url) + 1);
			strcpy(url, new_url);
			if (url) {
				logMsg(url, TRUE);
				logMsg("\n", TRUE);
//				if (!strstr(url, "ws.cdn.baidupcs.com")
//						&& !strstr(url, "qd2.cache.baidupcs.com")
//						&& !strstr(url, "qdcu01.baidupcs.com")
//						&& !strstr(url, "nj02all01.baidupcs.com")
//						&& !strstr(url, "nj02.poms.baidupcs.com")) {
//					logMsg("bad url, try again !!!\n", TRUE);
//					free(url);
//				} else {
				if(strstr(url, "lx.cdn.baidupcs.com")){
					FREE(pdesc->downloadUrl);
					downloadFileLenght = -1;
				}
				pdesc->downloadUrl = url;
				pdesc->size = (long) downloadFileLenght;
			}
		}
		curl_easy_cleanup(curl);
		sleep(5);
	}


}

void selection_made(GtkWidget *clist, gint row, gint column,
		GdkEventButton *event, gpointer data) {
	gchar *text;
	gtk_clist_get_text(GTK_CLIST(clist), row, 0, &text);
	logMsg(text, FALSE);
	logMsg("\n", FALSE);
	memset(selectedFile, 0, MAX_BUFFER_SIZE);
	strcpy(selectedFile, text);
	return;
}

void logMsg(const char *pmsg, gboolean isThread) {
	if (textview_log) {
		if (isThread)
			gdk_threads_enter();
		GtkTextBuffer *buffer;
		GtkTextIter iter;
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_log));
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert(buffer, &iter, pmsg, strlen(pmsg));
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(textview_log), &iter, 0, 1,
		TRUE, TRUE);
		while (gtk_events_pending())
			gtk_main_iteration();
		if (isThread)
			gdk_threads_leave();
	}

}

void clearMsg() {
	if (textview_log) {
		gdk_threads_enter();
		GtkTextBuffer *buffer;
		buffer = gtk_text_view_get_buffer(textview_log);
		gtk_text_buffer_set_text(buffer, "", strlen(""));
		gdk_threads_leave();
	}
}


void buffer_copy(char *res, char *dest){
	time_t start, end;
	start = time(NULL);
	char buf[MAX_BUFFER_SIZE];
	FILE *fd_r, *fd_w;
	int bread = 0;
	int bwrite = 0;
	char *tempbuf;

	fd_r = fopen(res, "rb");
	fd_w = fopen(dest, "ab+");

	while((bread=fread(buf, sizeof(char), MAX_BUFFER_SIZE, fd_r))){
		if(bread == -1 && errno != EINTR){
			perror("read");
			exit(1);
		}else if(bread > 0){
			tempbuf = buf;
			while((bwrite = fwrite(buf, sizeof(char), bread, fd_w))){
				if(bwrite == -1 && errno != EINTR){
					perror("write");
					exit(1);
				}else if(bread == bwrite){
					break;
				}else if(bwrite > 0){
					tempbuf+=bwrite;
					bread-=bwrite;
				}
			}
		}
	}

	fclose(fd_r);
	fclose(fd_w);
	end = time(NULL);
	time_t cost = end - start;
	printf("buffer cost:%ld\n", cost);
}
