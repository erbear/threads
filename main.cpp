#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>

char const *plik1, *plik2;

pthread_t * worker_threads;
int *worker_threads_args;
int *worker_threads_status;

int threadsNumber = 2;

pthread_mutex_t files_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

void *sort(void *params)
{
    while (true){
      int number = *((int *)params);
      char currentFile[256];
      char status[2];
      bool newUrlSaved = false;

      pthread_mutex_lock(&files_queue_mutex);

      FILE * file = fopen(plik1, "r+");

      FILE * tmp_file = fopen("tmp_file1.txt", "w");

      if(file != NULL){
          char url[256];
          int i = 0;

          while(fscanf(file, "%s %s", status, url) != EOF){
            if(status[0] == '!') {
              fprintf(tmp_file, "! %s\n", url);
              continue;
            }
            if (newUrlSaved) {
              fprintf(tmp_file, "- %s\n", url);
            }
            else {
              strcpy(currentFile, url);
              fprintf(tmp_file, "! %s\n", url);
              newUrlSaved = true;
              printf("%s\n", url);
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
      worker_threads_status[number] = 1;
      pthread_mutex_unlock(&files_queue_mutex);
      break;
    }
}

bool are_threads_finished()
{
  bool result = true;
  for (int i=0; i<threadsNumber; i++){
    if (worker_threads_status[i] == 0)
      result = false;
  }
  return result;
}
void wait_for_worker_threads(){
    for(int i=0; i<threadsNumber; i++){
        pthread_join(worker_threads[i], NULL);
    }
}

void create_threads(int number){
    worker_threads = new pthread_t[number];
    worker_threads_args = new int[number];
    worker_threads_status = new int[number];

    for(int i=0; i<threadsNumber; i++){
      worker_threads_status[i] = 0;
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

