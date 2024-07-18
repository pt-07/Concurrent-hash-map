#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ring_buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <stdatomic.h>


#define READY 1
#define NOT_READY 0
pthread_mutex_t getLock;
pthread_mutex_t submitLock;



int init_ring(struct ring *r) {
    // Initialize mutex
    if (pthread_mutex_init(&getLock, NULL) != 0) {
        return -1;
    }
    if (pthread_mutex_init(&submitLock, NULL) != 0) {
        return -1;
    }
    // Initialize buffer descriptors
    // for (int i = 0; i < RING_SIZE; ++i) {
    //     r->buffer[i].ready = NOT_READY;
    // }

    // r->p_head = r->p_tail = r->c_head = r->c_tail = 0;

    return 0;
}

void ring_submit(struct ring *r, struct buffer_descriptor *bd){
    pthread_mutex_lock(&submitLock);
    // Use atomic_fetch_add to increment p_head and p_tail atomically
    while((atomic_load(&r->p_head) - atomic_load(&r->c_tail) >= RING_SIZE)){
        
    }
    memcpy(&r->buffer[atomic_load(&r->p_head) % RING_SIZE], bd, sizeof(struct buffer_descriptor));
    atomic_fetch_add(&r->p_head, 1);
    atomic_fetch_add(&r->p_tail, 1);
    pthread_mutex_unlock(&submitLock);
}

void ring_get(struct ring *r, struct buffer_descriptor *bd){
   pthread_mutex_lock(&getLock);
    //checking if the ring buffer is empty
    while(atomic_load(&r->c_head) >= atomic_load(&r->p_tail)){
        //spin
    }
    memcpy(bd, &r->buffer[atomic_load(&r->c_head) % RING_SIZE], sizeof(struct buffer_descriptor));
    atomic_fetch_add(&r->c_head, 1);
    atomic_fetch_add(&r->c_tail, 1);
   pthread_mutex_unlock(&getLock);
}
// void ring_submit(struct ring *r, struct buffer_descriptor *bd){
//     pthread_mutex_lock(&submitLock);
//     // Wait until there is space in the ring buffer
//     while((r->p_head - r->c_tail) >= RING_SIZE){
        
//     }
//     memcpy(&r->buffer[r->p_head % RING_SIZE], bd, sizeof(struct buffer_descriptor));
//     r->p_head++;
//     r->p_tail++;
//     pthread_mutex_unlock(&submitLock);
// }

// void ring_get(struct ring *r, struct buffer_descriptor *bd){
//    pthread_mutex_lock(&getLock);
//     while(r->c_head >= r->p_tail){
        
//     }
//     memcpy(bd, &r->buffer[r->c_head % RING_SIZE], sizeof(struct buffer_descriptor));
//     r->c_head++;
//     r->c_tail++;
//    pthread_mutex_unlock(&getLock);
// }


// #define NUM_PRODUCERS 10
// #define NUM_CONSUMERS 5
// #define NUM_SUBMITS 100

// // Producer thread function
// void* producer_thread(void* arg) {
//     struct ring *r = (struct ring*)arg;
//     struct buffer_descriptor bd;
//     bd.req_type = PUT;
//     bd.k = 12345;
//     bd.v = 67890;
//     bd.res_off = 0;
//     bd.ready = 0;

//     struct buffer_descriptor bd1;
//     bd1.req_type = PUT;
//     bd1.k = 12;
//     bd1.v = 34;
//     bd1.res_off = 0;
//     bd1.ready = 0;

//     for (int i = 0; i < NUM_SUBMITS; i++) {
//         if(i == 0){
//             ring_submit(r, &bd);
//         }
//         else{
//             ring_submit(r,&bd1);
//         }
//     }

//     return NULL;
// }

// // Consumer thread function
// void* consumer_thread(void* arg) {
//     struct ring *r = (struct ring*)arg;
//     struct buffer_descriptor bd;

//     for (int i = 0; i < NUM_SUBMITS; i++) {
//         ring_get(r, &bd);

//         // Check if the retrieved buffer descriptor is correct
//         if (bd.req_type != PUT || bd.k != 12345 || bd.v != 67890 || bd.res_off != 0 || bd.ready != 0) {
//             printf("Retrieved buffer descriptor is incorrect\n");
//             exit(-1);
//         }
//         printf("success on the first");
//     }

//     return NULL;
// }

// // int main() {
// //     // Initialize the ring buffer
// //     int init_result = init_ring();
// //     if (init_result != 0) {
// //         printf("Failed to initialize the ring buffer\n");
// //         return -1;
// //     }

// //     // Create a ring buffer
// //     struct ring *r = malloc(sizeof(struct ring));
// //     memset(r, 0, sizeof(struct ring));
// //     pthread_mutex_init(&r->mutex, NULL);
// //     pthread_cond_init(&r->cond, NULL);

// //     // Create and start producer threads
// //     pthread_t producer_threads[NUM_PRODUCERS];
// //     for (int i = 0; i < NUM_PRODUCERS; i++) {
// //         pthread_create(&producer_threads[i], NULL, producer_thread, r);
// //     }

// //     // Create and start consumer threads
// //     pthread_t consumer_threads[NUM_CONSUMERS];
// //     for (int i = 0; i < NUM_CONSUMERS; i++) {
// //         pthread_create(&consumer_threads[i], NULL, consumer_thread, r);
// //     }

// //     // Wait for all threads to finish
// //     for (int i = 0; i < NUM_PRODUCERS; i++) {
// //         pthread_join(producer_threads[i], NULL);
// //     }
// //     for (int i = 0; i < NUM_CONSUMERS; i++) {
// //         pthread_join(consumer_threads[i], NULL);
// //     }

// //     printf("All tests passed! \n");
// //     return 0;
// // }