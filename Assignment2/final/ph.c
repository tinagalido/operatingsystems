/**
 * Submitted by: Maria Cristina Galido (2092015); 
 * Shabnam Alizadeh (2084527); 
 * Romina Sharifi (2055127)
 * 
 * Compile by executing the Makefile, designed to work with the provided ph.c file
 * Run the following commands to run the code for 1,2 and 4 threads.
 * make run1
 * make run2
 * make run4
**/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#define SOL
#define NBUCKET 5
#define NKEYS 100000

// Global mutex declaration
pthread_mutex_t lock;

struct entry {
  int key;
  int value;
  struct entry *next;
};

struct entry *table[NBUCKET];
int keys[NKEYS];
int nthread = 1;
volatile int done;

double now() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}


static void
print(void)
{
  int i;
  struct entry *e;
  for (i = 0; i < NBUCKET; i++) {
    printf("%d: ", i);
    for (e = table[i]; e != 0; e = e->next) {
      printf("%d ", e->key);
    }
    printf("\n");
  }
}

static void 
insert(int key, int value, struct entry **p, struct entry *n)
{
  struct entry *e = malloc(sizeof(struct entry));
  e->key = key;
  e->value = value;
  e->next = n;
  *p = e;
}

static void put(int key, int value) {
  struct entry *n, **p;

  pthread_mutex_lock(&lock); // Lock before modifying the hash table

  for (p = &table[key%NBUCKET], n = table[key % NBUCKET]; n != 0; p = &n->next, n = n->next) {
    if (n->key == key) {
      // Key already exists, update value.
      n->value = value;
      pthread_mutex_unlock(&lock);
      return;
    }
    if (n->key > key) {
      break;
    }
  }
  insert(key, value, p, n);

  pthread_mutex_unlock(&lock); // Unlock after modifying the hash table
}

static struct entry* get(int key) {
  struct entry *e = 0;

  pthread_mutex_lock(&lock); // Lock before accessing the hash table

  for (e = table[key % NBUCKET]; e != 0; e = e->next) {
    if (e->key == key) {
      pthread_mutex_unlock(&lock);
      return e;
    }
  }

  pthread_mutex_unlock(&lock); // Unlock after accessing the hash table
  return NULL; // Return NULL if the key was not found
}


static void *thread(void *xa) {
  long n = (long) xa;
  int i;
  int b = NKEYS/nthread;
  int k = 0;
  double t1, t0;

  t0 = now();
  for (i = 0; i < b; i++) {
    put(keys[b*n + i], n);
  }
  t1 = now();
  printf("%ld: put time = %f\n", n, t1-t0);

  __sync_fetch_and_add(&done, 1);
  while (done < nthread);

  t0 = now();
  for (i = 0; i < NKEYS; i++) {
    struct entry *e = get(keys[i]);
    if (e == 0) k++;
  }
  t1 = now();
  printf("%ld: lookup time = %f\n", n, t1-t0);
  printf("%ld: %d keys missing\n", n, k);
}


int main(int argc, char *argv[]) {
  pthread_t *tha;
  void *value;
  long i;
  double t1, t0;

  if (argc < 2) {
    fprintf(stderr, "%s: %s nthread\n", argv[0], argv[0]);
    exit(-1);
  }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread);
  srandom(0);
  assert(NKEYS % nthread == 0);
  for (i = 0; i < NKEYS; i++) {
    keys[i] = random();
  }

  // Initialize the mutex lock
  pthread_mutex_init(&lock, NULL);

  t0 = now();
  for(i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, thread, (void *) i) == 0);
  }
  for(i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  t1 = now();

  // Destroy the mutex lock
  pthread_mutex_destroy(&lock);

  printf("completion time = %f\n", t1-t0);
}
