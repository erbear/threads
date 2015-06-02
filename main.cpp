#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <curl/curl.h>
#include <regex.h>
#include <string.h>
#include <vector>
#include <ncurses.h>

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

pthread_t * worker_threads, screen_thread;
int *worker_threads_args;
int threadsNumber = 2;
pthread_mutex_t files_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<down_file*> allFiles;
pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t screen_cond = PTHREAD_COND_INITIALIZER;

bool running = true;


int screen_width;
int screen_height;
int screen_x;
int screen_y;


void * download_progress_func(down_file * f, double t, double d, double ultotal, double ulnow){
    pthread_mutex_lock(&screen_mutex);
    f->total = t;
    f->downloaded = d;
    running = true;
    pthread_cond_signal(&screen_cond);
    pthread_mutex_unlock(&screen_mutex);
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

      pthread_mutex_lock(&files_queue_mutex);
      pthread_mutex_lock(&screen_mutex);
      down_file *threadFile;
      threadFile = new down_file;

      threadFile->id = *((int *)params);
      allFiles.push_back(threadFile);
      char tmp_path[40];
      char currentFile[256];
      char status[2];
      bool newUrlSaved = false;
      int numberOfLine = 0;
      int i = 0;

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

      pthread_cond_signal(&screen_cond);
      pthread_mutex_unlock(&screen_mutex);
      pthread_mutex_unlock(&files_queue_mutex);
      // printf("thread = %d, i = %d, numberOfLine = %d\n", threadFile->id, i, numberOfLine);

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

          pthread_mutex_lock(&screen_mutex);
          sprintf(threadFile->path, "%d.%s", threadFile->uniq, fname);
          delete fname;
          pthread_mutex_unlock(&screen_mutex);

          CURLcode res = curl_easy_perform(curl);

          pthread_mutex_lock(&screen_mutex);
          if(res == CURLE_OK){
            threadFile->error = false;
            rename(tmp_path, threadFile->path);
          } else {
            threadFile->error = true;
          }
          running = true;
          pthread_mutex_unlock(&screen_mutex);
        }

        fclose(outfile);
        curl_easy_cleanup(curl);

      }
      // if there is no new nodes
      if (i - 1 == numberOfLine || numberOfLine == 0)
        break;
    }
}

void paint(){
  erase();
  int x = 0;
  int y = 0;
  double percent;
  for (int i = 0; i<allFiles.size(); i++)
  {
    clrtoeol();
    attron(COLOR_PAIR(1));
    mvprintw(y, x, "%3d. %-50s", i+1, allFiles[i]->url);
    attroff(COLOR_PAIR(1));
    if(allFiles[i]->total != 0.0)
      percent = allFiles[i]->downloaded*100/allFiles[i]->total;
    else
      percent = 0;

    //printf("t: %f\n", percent);
    attron(COLOR_PAIR(2));
    mvprintw(y, x+80, "%f",  percent);
    attroff(COLOR_PAIR(2));

    y++;
  }

  refresh();
}
void * screen_function(void * _arg) {
  initscr();

  getmaxyx(stdscr, screen_height, screen_width);

  start_color();
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);

  nodelay(stdscr, true);

  while (running){
    pthread_mutex_lock(&screen_mutex);
    paint();
    pthread_cond_wait(&screen_cond, &screen_mutex);
    pthread_mutex_unlock(&screen_mutex);
  }

  nodelay(stdscr, false);

  getch();

  endwin();

  return NULL;
}
void wait_for_worker_threads(){
    for(int i=0; i<threadsNumber; i++){
        pthread_join(worker_threads[i], NULL);
    }
    running = false;
}

void wait_for_screen_thread(){
    pthread_join(screen_thread, NULL);
}

void create_threads(int number){
    worker_threads = new pthread_t[number];
    worker_threads_args = new int[number];

    for(int i=0; i<threadsNumber; i++){
      worker_threads_args[i] = i;
      pthread_create(worker_threads + i, NULL, sort, worker_threads_args + i);
    }
}
void create_screen(){
    pthread_create(&screen_thread, NULL, screen_function, NULL);
}
int main(int argc, char const *argv[]){
    plik1 = argv[1];
    plik2 = argv[2];

    create_threads(threadsNumber);
    create_screen();

    wait_for_worker_threads();
    wait_for_screen_thread();


    // printf("\nzrobiłem już chyba wszystko...");
}

