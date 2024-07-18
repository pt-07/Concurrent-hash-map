
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"
#include "ring_buffer.h"
#define READY 1

typedef struct bucket
{
    key_type key;
    value_type value;
    struct bucket *next;
    pthread_mutex_t lock;
} bucket_t;

struct bucket *hashtable;
int hashtableSize = 1000;
int numThreads;
char filename[] = "shmem_file";
struct ring *ring = NULL;
char *shmem_area = NULL;
int fd;
// int shm_size = 1024; //might be wrong
int ring_size;
char *mem;
// pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_spinlock_t *slocksArray;

// Function to initialize the key-value store
void init_kvstore(int size)
{
    hashtable = malloc(size * sizeof(bucket_t));
    slocksArray = malloc(size * sizeof(pthread_spinlock_t));
    for (int i = 0; i < size; i++)
    {
        pthread_spin_init(&slocksArray[i], PTHREAD_PROCESS_PRIVATE);
    }
    int fd = open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }
    struct stat sb;
    if (stat(filename, &sb) == -1)
    {
        perror("stat");
        exit(1);
    }
    int shm_size = sb.st_size;

    // Map the shared file into memory
    mem = mmap(NULL, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    close(fd);
    ring = (struct ring *)mem;
    shmem_area = mem;
    if (init_ring(ring) < 0)
    {
        // printf("Ring initialization failed\n");
        exit(EXIT_FAILURE);
    }
}
void put(key_type k, value_type v)
{
    int index = hash_function(k, hashtableSize);

    pthread_spin_lock(&slocksArray[index]);

    struct bucket *current = &hashtable[index];
    struct bucket *prev = NULL;

    // Find the bucket with the given key or the last bucket in the chain
    while (current != NULL && current->key != k)
    {
        prev = current;
        current = current->next;
    }

    if (current != NULL)
    {
        // Key found, update the value
        current->value = v;
    }
    else
    {
        // Key not found, add a new bucket to the chain
        current = malloc(sizeof(bucket_t));
        if (current == NULL)
        {
            perror("malloc");
            exit(1);
        }

        current->key = k;
        current->value = v;
        current->next = NULL;

        if (prev != NULL)
        {
            // Add the new bucket to the end of the chain
            prev->next = current;
        }
        else
        {
            // The chain is empty, add the new bucket as the first bucket
            hashtable[index] = *current;
        }
    }

    pthread_spin_unlock(&slocksArray[index]);
}

value_type get(key_type k)
{
    int index = hash_function(k, hashtableSize);
    // pthread_spin_lock(&slocksArray[index]);
    struct bucket *current = &hashtable[index];
    while (current != NULL)
    {
        if (current->key == k)
        {
            // pthread_spin_unlock(&slocksArray[index]);
            return current->value;
        }
        current = current->next;
    }
    // pthread_spin_unlock(&slocksArray[index]);
    return 0;
}

// Function to process a request
void request(struct buffer_descriptor *bd)
{
    switch (bd->req_type)
    {
    case PUT:
        put(bd->k, bd->v);
        //  printf("PUT: %u %u\n", bd->k, bd->v);
        memcpy((struct buffer_descriptor *)(bd->res_off + mem), bd, sizeof(*bd));
        break;
    case GET:
        bd->v = get(bd->k);
        //  printf("GET: %u %u\n", bd->k, bd->v);
        memcpy((struct buffer_descriptor *)(bd->res_off + mem), bd, sizeof(*bd));
        break;
    default:
        break;
    }
}

// Thread function
void *thread_func(void *arg)
{
    struct ring *r = (struct ring *)arg;
    struct buffer_descriptor bd;

    while (1)
    {
        // printf("In server thread_func, about to call ring_get\n");
        ring_get(r, &bd);
        struct buffer_descriptor *bd_status_board = (struct buffer_descriptor *)(bd.res_off + mem);
        // printf("Called ring_get, about to call request\n");
        request(&bd);
        bd_status_board->ready = READY;
        // printf("Called request, set bd_status_board->ready to READY\n");
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int opt;
    int nvalue = 0;
    int svalue = 0;

    while ((opt = getopt(argc, argv, "n:s:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            numThreads = atoi(optarg);
            break;
        case 's':
            hashtableSize = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s -n <num_threads> -s <hashtable_size>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    init_kvstore(hashtableSize);

    // Start nvalue threads
    pthread_t threads[numThreads];
    for (int i = 0; i < numThreads; i++)
    {
        if (pthread_create(&threads[i], NULL, thread_func, ring) != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < numThreads; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            perror("pthread_join");
            return 1;
        }
    }
    return 0;
}
