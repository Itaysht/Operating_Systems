#include "segel.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    getargs(&port, argc, argv);

    // 
    // HW3: Create some threads...
    //
    int type_of_sched = 0;
    int dyn_size_q = -1;
    if (!strcmp(argv[4], "block")){
        type_of_sched = BLOCK;
    }
    else if (!strcmp(argv[4], "dt")){
        type_of_sched = DROP_TAIL;
    }
    else if (!strcmp(argv[4], "dh")){
        type_of_sched = DROP_HEAD;
    }
    else if (!strcmp(argv[4], "bf")){
        type_of_sched = BLOCK_FLUSH;
    }
    else if (!strcmp(argv[4], "dynamic")){
        type_of_sched = DYNAMIC;
        dyn_size_q = atoi(argv[5]);
    }
    else if (!strcmp(argv[4], "random")){
        type_of_sched = RANDOM;
    }
    my_pool* pooler = my_pool_init(atoi(argv[2]), atoi(argv[3]), type_of_sched, dyn_size_q);

    listenfd = Open_listenfd(port);
    while (1) {
        struct timeval curr_time;
	clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *) &clientaddr, (socklen_t *) &clientlen);
    gettimeofday(&curr_time, NULL);

    pthread_mutex_lock(get_mutex(pooler));
    if (get_size_of_active_queue(pooler) + get_size_of_waiting_queue(pooler) < get_max_size_of_queue(pooler)){
        enqueue(pooler, connfd, WAITING, &curr_time);
        if (get_number_of_workers(pooler) < get_size_of_pool(pooler)) {
            pthread_cond_signal(get_waker(pooler));
        }
        pthread_mutex_unlock(get_mutex(pooler));
    }
    else{
        if (get_type_of_sched(pooler) == BLOCK){
            pthread_cond_wait(get_main_thread_waker(pooler), get_mutex(pooler));
            enqueue(pooler, connfd, WAITING, &curr_time);
            pthread_cond_signal(get_waker(pooler));
            pthread_mutex_unlock(get_mutex(pooler));
        }
        else if (get_type_of_sched(pooler) == DROP_TAIL){
            pthread_mutex_unlock(get_mutex(pooler));
            Close(connfd);
        }
        else if (get_type_of_sched(pooler) == DROP_HEAD){
            int oldest_req = dequeue(pooler, WAITING);
            if (oldest_req != -1) {
                Close(oldest_req);
                enqueue(pooler, connfd, WAITING, &curr_time);
            }
            else{
                Close(connfd);
            }
            pthread_mutex_unlock(get_mutex(pooler));
        }
        else if (get_type_of_sched(pooler) == BLOCK_FLUSH){
            pthread_cond_wait(get_main_thread_waker(pooler), get_mutex(pooler));
            Close(connfd);
            pthread_mutex_unlock(get_mutex(pooler));
        }
        else if (get_type_of_sched(pooler) == DYNAMIC){
            Close(connfd);
            add_size_of_waiting_queue(pooler);
            pthread_mutex_unlock(get_mutex(pooler));
        }
        else if (get_type_of_sched(pooler) == RANDOM){
            if (get_size_of_waiting_queue(pooler) == 0){
                Close(connfd);
            }
            else {
                delete_half_random(pooler);
                enqueue(pooler, connfd, WAITING, &curr_time);
            }
            pthread_mutex_unlock(get_mutex(pooler));
        }
    }
	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	// 
//	requestHandle(connfd);
//
//	Close(connfd);
    }

}


    


 
