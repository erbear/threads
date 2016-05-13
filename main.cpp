#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <curl/curl.h>
#include <string.h>
#include <vector>
#include <ncurses.h>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

char const *plik1, *plik2;

regex pattern("[\"'](https{0,1}:\\/\\/\\S*?)[\"']");

struct fileObj {
  int id;
  int uniq;
  char url[1000];
  char error_msg[CURL_ERROR_SIZE];
  double total;
  double downloaded;
  char path[1000];
  bool error;
  bool done;
};


struct linkEl
{
  int uniq;
  char url[1000];
  int rank;
  bool parsed;
};

struct parseEl
{
  char path[1000];
  bool parsed;
};

struct downloadStatus {
  int id;
  char status[1000];
};

struct parseStatus {
  int id;
  char status[1000];
};


bool sort_function(linkEl *pierwszy, linkEl *drugi){
  return pierwszy->rank > drugi->rank;
}

pthread_t * worker_threads, screen_thread, *parse_threads;
int *worker_threads_args, *parse_threads_args;
int threadsNumber = 10;
pthread_mutex_t files_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t parsed_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t screen_cond = PTHREAD_COND_INITIALIZER;

int screen_width;
int screen_height;
int screen_x;
int screen_y;

std::vector<fileObj*> allFiles;
std::vector<linkEl*> sortedFiles;
std::vector<parseEl*> parseElements;
std::vector<downloadStatus*> downloadStatuses;
std::vector<parseStatus*> parseStatuses;

void add_link(int uniq, char *url){
  bool appears = false;
  for (int i=0; i<sortedFiles.size(); i++){
    int isEqual = strcmp(sortedFiles[i]->url, url);
    if (isEqual == 0){
      sortedFiles[i]->rank++;
      appears = true;
      sort( sortedFiles.begin(), sortedFiles.end(), sort_function );
      break;
    }
  }
  if (!appears){
    linkEl *newLink;
    newLink = new linkEl;
    strcpy(newLink->url, url);
    newLink->rank = 1;
    sortedFiles.push_back(newLink);
  }
}
void * download_progress_func(fileObj * f, double t, double d, double ultotal, double ulnow){
    pthread_mutex_lock(&screen_mutex);
    f->total = t;
    f->downloaded = d;
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
void *parse(void *params){
  int id = *((int *)params);
  strcpy(parseStatuses[id]->status, "Created");
  while (true){
    int elementNumber;
    bool founded = false;


    pthread_mutex_lock(&screen_mutex);
    strcpy(parseStatuses[id]->status, "File search");
    pthread_mutex_unlock(&screen_mutex);
    for (int i=0; i<parseElements.size(); i++){
      if (!parseElements[i]->parsed){
        elementNumber = i;
        founded = true;

        pthread_mutex_lock(&parsed_file_mutex);
        parseElements[i]->parsed = true;
        pthread_mutex_unlock(&parsed_file_mutex);
      }
    }

    if (founded){
      pthread_mutex_lock(&screen_mutex);
      strcpy(parseStatuses[id]->status, "Founded");
      pthread_mutex_unlock(&screen_mutex);
      ifstream inFile;

      //read from file

      pthread_mutex_lock(&parsed_file_mutex);
      inFile.open(parseElements[elementNumber]->path);//open the input file
      pthread_mutex_unlock(&parsed_file_mutex);

      stringstream strStream;
      strStream << inFile.rdbuf();//read the file
      string str = strStream.str();//str holds the content of the file

      int line;
      string tekst, tekstOut = "";
      std::ofstream out;

      pthread_mutex_lock(&screen_mutex);
      strcpy(parseStatuses[id]->status, "Parsing...");
      pthread_mutex_unlock(&screen_mutex);

      while( getline( strStream, tekst ) )
      {
        smatch wynik; // tutaj będzie zapisany wynik
        ++line;
        if( regex_search( tekst, wynik, pattern ) ){
          string tad = wynik.str();
          tad.erase(tad.begin());
          tad.erase(tad.end() - 1);
          tekstOut.append(tad.append("\n"));
        }
      }
      unlink(parseElements[elementNumber]->path);
      pthread_mutex_lock(&screen_mutex);
      strcpy(parseStatuses[id]->status, "Saving new links");
      pthread_mutex_unlock(&screen_mutex);
      pthread_mutex_lock(&files_queue_mutex);
      out.open(plik1, ios::app);
      out << tekstOut << "\n";
      pthread_mutex_unlock(&files_queue_mutex);
    }
    else {
      strcpy(parseStatuses[id]->status, "Waiting");
    }
  }
}
void *download(void *params)
{
  int id = *((int *)params);
  strcpy(downloadStatuses[id]->status, "Created");
    while (true){

      fileObj *threadFile;
      char tmp_path[1000];
      char currentFile[1000];
      char status[2];
      bool newUrlSaved = false;
      int numberOfLine = 0;
      int i = 0;
      bool founded = false;

      pthread_mutex_lock(&screen_mutex);
      strcpy(downloadStatuses[id]->status, "Searching for links");
      pthread_mutex_unlock(&screen_mutex);

      pthread_mutex_lock(&files_queue_mutex);
      pthread_mutex_lock(&screen_mutex);

      FILE * file = fopen(plik1, "r+");
      FILE * tmp_file = fopen("tmp_file1.txt", "w");

      if(file != NULL){
        char url[1000];
        while(i++, fscanf(file, "%s", url) != EOF){
          if(url[0] == '-' || newUrlSaved) {
            fprintf(tmp_file, "%s\n", url);
            continue;
          }
          else {
            threadFile = new fileObj;
            threadFile->id = *((int *)params);
            strcpy(threadFile->url, url);
            threadFile->done = false;
            fprintf(tmp_file, "-%s\n", url);
            newUrlSaved = true;
            numberOfLine = i;
            threadFile->uniq = i;
            allFiles.push_back(threadFile);
            founded = true;
          }
        }
      } else {
      }

      fclose(file);
      fclose(tmp_file);
      if (unlink("file1.txt")==0){
        rename("tmp_file1.txt", "file1.txt");
      }
      pthread_cond_signal(&screen_cond);
      pthread_mutex_unlock(&screen_mutex);
      pthread_mutex_unlock(&files_queue_mutex);

      if (!founded) {

        pthread_mutex_lock(&screen_mutex);
        strcpy(downloadStatuses[id]->status, "Waaiting");
        pthread_mutex_unlock(&screen_mutex);
      }
      else {

        pthread_mutex_lock(&screen_mutex);
        strcpy(downloadStatuses[id]->status, "Downloading");
        pthread_mutex_unlock(&screen_mutex);

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
            sprintf(threadFile->path, "files/%d.%s", threadFile->uniq, fname);
            delete fname;
            pthread_mutex_unlock(&screen_mutex);

            CURLcode res = curl_easy_perform(curl);

            pthread_mutex_lock(&files_queue_mutex);
            pthread_mutex_lock(&screen_mutex);
            if(res == CURLE_OK){
              threadFile->error = false;
              rename(tmp_path, threadFile->path);
              threadFile->done = true;
              add_link(threadFile->uniq, threadFile->url);
            } else {

              strcpy(downloadStatuses[id]->status, "ERROR OCCURED");
              threadFile->error = true;
            }
            pthread_mutex_unlock(&screen_mutex);
            pthread_mutex_unlock(&files_queue_mutex);
          }


          fclose(outfile);
          curl_easy_cleanup(curl);

          parseEl *toParse;
          toParse = new parseEl;
          strcpy(toParse->path, threadFile->path);
          toParse->parsed = false;

          pthread_mutex_lock(&screen_mutex);
          strcpy(downloadStatuses[id]->status, "Adding to parse");
          pthread_mutex_unlock(&screen_mutex);

          pthread_mutex_lock(&parsed_file_mutex);
          parseElements.push_back(toParse);
          pthread_mutex_unlock(&parsed_file_mutex);

        }
      }
    }
}

void paint(){
  erase();
  int x = 0;
  int y = 0;
  double percent;

  mvprintw(y, x, "PARSING THREADS");
  mvprintw(y, x+50, "DOWNLOADING THREADS");
  y += 2;
  for (int i=0; i<parseStatuses.size(); i++){
    mvprintw(y, x+1, "%d. %-50s", i, parseStatuses[i]->status);
    mvprintw(y, x+51, "%d. %-50s", i, downloadStatuses[i]->status);
    y += 1;
  }

  y += 5;
  mvprintw(y, x, "%-50s", "LIST OF DOWNLOADS");
  y += 2;

  for (int i = 0; i<allFiles.size(); i++)
  {
    clrtoeol();

    if (allFiles[i]->error){
      attron(COLOR_PAIR(3));
      mvprintw(y, x+10, "%-50s",  "ERROR!");
      attroff(COLOR_PAIR(3));
    }
    else{

      if (!allFiles[i]->done){
        if(allFiles[i]->total != 0.0)
          percent = allFiles[i]->downloaded*100/allFiles[i]->total;
        else
          percent = 0;

        attron(COLOR_PAIR(2));
        mvprintw(y, x, "%3.0f%%",  percent);
        mvprintw(y, x+10, "%3d. %-50s", i+1, allFiles[i]->url);
        attroff(COLOR_PAIR(2));
        y++;
      }

    }
  }

  y = threadsNumber * 2 + 10;

  mvprintw(y, x, "%-50s", "TOP 10");
  y += 2;
  for (int i = 0; i<sortedFiles.size() && i<10; i++){
    mvprintw(y, x, "%3d. %-50s", sortedFiles[i]->rank, sortedFiles[i]->url);
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

  while (true){
    pthread_mutex_lock(&screen_mutex);
    paint();
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
      pthread_join(parse_threads[i], NULL);
    }
}

void wait_for_screen_thread(){
    pthread_join(screen_thread, NULL);
}

void create_threads(int number){
    worker_threads = new pthread_t[number];
    worker_threads_args = new int[number];
    parse_threads = new pthread_t[number];
    parse_threads_args = new int[number];

    for(int i=0; i<threadsNumber; i++){
      worker_threads_args[i] = i;
      downloadStatus * dst = new downloadStatus;
      downloadStatuses.push_back(dst);
      pthread_create(worker_threads + i, NULL, download, worker_threads_args + i);


      parse_threads_args[i] = i;
      parseStatus * pst = new parseStatus;
      parseStatuses.push_back(pst);
      pthread_create(parse_threads + i, NULL, parse, parse_threads_args + i);
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

}

