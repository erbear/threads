#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <curl/curl.h>
#include <regex.h>
#include <string.h>

char const *plik1, *plik2;

struct down_file {
  int id;
  int uniq;
  char url[1000];
  char error_msg[CURL_ERROR_SIZE];
  double total;
  double downloaded;
  char path[1000];
  bool error;
};

pthread_t * worker_threads;
int *worker_threads_args;
int threadsNumber = 2;
pthread_mutex_t files_queue_mutex = PTHREAD_MUTEX_INITIALIZER;


void * download_progress_func(down_file * f, double t, double d, double ultotal, double ulnow){
    f->total = t;
    f->downloaded = d;
    return NULL;
}

char * get_filename(char * str){
  char * name;
  int len = strlen(str);
  int i,k;
  for(i=len-1; i>= 0; i--){
      if(str[i] == '/'){
          name = new char[len-i + 10];
          for(k=0; k<(len-i); k++) name[k] = str[k+i+1];

          name[k] = '\0';
          return name;
      }
  }
  return NULL;
}

void *sort(void *params)
{
    while (true){

      down_file *threadFile;
      threadFile = new down_file;

      threadFile->id = *((int *)params);
      char tmp_path[40];
      char currentFile[256];
      char status[2];
      bool newUrlSaved = false;
      int numberOfLine = 0;
      int i = 0;
      pthread_mutex_lock(&files_queue_mutex);

      FILE * file = fopen(plik1, "r+");

      FILE * tmp_file = fopen("tmp_file1.txt", "w");

      if(file != NULL){
          char url[256];
          while(i++, fscanf(file, "%s", url) != EOF){
            if(url[0] == '-' || newUrlSaved) {
              fprintf(tmp_file, "%s\n", url);
              continue;
            }
            else {
              strcpy(threadFile->url, url);
              fprintf(tmp_file, "-%s\n", url);
              newUrlSaved = true;
              numberOfLine = i;
              threadFile->uniq = i;
            }

          }
          fclose(file);
          fclose(tmp_file);
          if (unlink("file1.txt")==0){
            rename("tmp_file1.txt", "file1.txt");
          }

      } else {
          printf("File '%s' not found\n", plik1);
      }

      pthread_mutex_unlock(&files_queue_mutex);
      printf("thread = %d, i = %d, numberOfLine = %d\n", threadFile->id, i, numberOfLine);

      // download
      CURL *curl = curl_easy_init();
      if(curl){
        sprintf(tmp_path, "/tmp/download-%d-%d.tmp", getpid(), threadFile->id);
        FILE *outfile = fopen(tmp_path, "w");
        char * full_url = NULL;

        curl_easy_setopt(curl, CURLOPT_URL, threadFile->url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, threadFile->error_msg);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_progress_func);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, threadFile);

        CURLcode url_res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &full_url);
        char * fname = get_filename(full_url);
        if (fname != NULL){
          sprintf(threadFile->path, "%d.%s", threadFile->uniq, fname);
          delete fname;

          CURLcode res = curl_easy_perform(curl);
          if(res == CURLE_OK){
            threadFile->error = false;
            rename(tmp_path, threadFile->path);
          } else {
            threadFile->error = true;
          }
        }

        fclose(outfile);
        curl_easy_cleanup(curl);

      }

      // if there is no new nodes
      if (i - 1 == numberOfLine || numberOfLine == 0)
        break;
    }
}

void wait_for_worker_threads(){
    for(int i=0; i<threadsNumber; i++){
        pthread_join(worker_threads[i], NULL);
    }
}

void create_threads(int number){
    worker_threads = new pthread_t[number];
    worker_threads_args = new int[number];

    for(int i=0; i<threadsNumber; i++){
      worker_threads_args[i] = i;
      pthread_create(worker_threads + i, NULL, sort, worker_threads_args + i);
    }
}

int main(int argc, char const *argv[]){
    plik1 = argv[1];
    plik2 = argv[2];

    create_threads(threadsNumber);
    wait_for_worker_threads();


    printf("\nzrobiłem już chyba wszystko...");
}

