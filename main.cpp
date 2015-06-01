#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_t * worker_threads;
int *worker_threads_args;
int *worker_threads_done;

int threadsNumber = 10;

void *show(void *params)
{
    int number = *((int *)params);
    printf("thread %d \n", number);
    for (int i = 0; i<100000; i++){
      if (i == 99999){
        printf("\n%d done!", number);
        worker_threads_done[number] = 1;
      }
    }
}

bool are_threads_finished()
{
  bool result = true;
  for (int i=0; i<threadsNumber; i++){
    if (worker_threads_done[i] == 0)
      result = false;
  }
  return result;
}

int main(int argc, char const *argv[]){
    worker_threads = new pthread_t[threadsNumber];
    worker_threads_args = new int[threadsNumber];
    worker_threads_done = new int[threadsNumber];

    for(int i=0; i<threadsNumber; i++){
      worker_threads_done[i] = 0;
      worker_threads_args[i] = i;
      pthread_create(worker_threads + i, NULL, show, worker_threads_args + i);
    }
    while (!are_threads_finished() == true);

    printf("\nzrobiłem już chyba wszystko...");
}

