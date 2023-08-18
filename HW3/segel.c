#include "segel.h"

typedef struct my_queue_Node{
    int fd_of_request;
    struct timeval* curr_req_time;
    my_queue_Node* next_node;
    my_queue_Node* last_node;
} my_queue_Node;

typedef struct my_queue{
    my_queue_Node* first;
    my_queue_Node* last;
    int size_of_queue;
} my_queue;

my_queue_Node* my_queue_node_init(int fd, struct timeval* curr){
    my_queue_Node* n = malloc(sizeof(my_queue_Node));
    n->fd_of_request= fd;
    struct timeval* temp = malloc(sizeof(struct timeval));
    *temp = *curr;
    n->curr_req_time = temp;
    n->next_node = NULL;
    n->last_node = NULL;
    return n;
}

my_queue* my_queue_init(){
    my_queue* q = malloc(sizeof(my_queue));
    q->first = NULL;
    q->last = NULL;
    q->size_of_queue = 0;
    return q;
}


typedef struct my_pool_thread{
    int id_of_thread_in_pool;
    my_pool_thread* next_thread;
} my_pool_thread;

typedef struct my_pool{
    my_pool_thread* first_thread;
    my_pool_thread* last_thread;
    int id_placer;
    pthread_cond_t my_waker;
    pthread_cond_t my_main_thread_waker;
    int size_of_pool;
    int max_size_of_queue;
    int dynamic_max_size_of_queue;
    int number_of_workers;
    pthread_mutex_t my_mutex;
    my_queue* req_queue;
    my_queue* wait_queue;
    int sched_type;
} my_pool;

int get_size_of_pool(my_pool* p){
    return p->size_of_pool;
}
int get_size_of_active_queue(my_pool* p){
    return p->req_queue->size_of_queue;
}
int get_size_of_waiting_queue(my_pool* p){
    return p->wait_queue->size_of_queue;
}
void add_size_of_waiting_queue(my_pool* p){
    if (p->dynamic_max_size_of_queue > p->max_size_of_queue) {
        p->max_size_of_queue += 1;
    }
}

int get_max_size_of_queue(my_pool* p){
    return p->max_size_of_queue;
}

int get_number_of_workers(my_pool* p){
    return p->number_of_workers;
}
void set_number_of_workers(my_pool* p, int num){
    p->number_of_workers = num;
}
struct timeval* get_curr_time_of_last_in_queue(my_queue* q){
    return q->last->curr_req_time;
}

my_pool_thread* my_pool_thread_init(int id_of_thread){
    my_pool_thread* my_pool_thread_temp = malloc(sizeof(my_pool_thread));
    my_pool_thread_temp->id_of_thread_in_pool = id_of_thread;
    my_pool_thread_temp->next_thread = NULL;
    return my_pool_thread_temp;
}

void set_next_pool_thread(my_pool_thread* curr, my_pool_thread* next){
    curr->next_thread = next;
}

my_pool* my_pool_init(int size, int max_size_of_queue, int sched_type, int dyn_size_q){
    my_pool* p = malloc(sizeof(my_pool));
    pthread_t t;
    pthread_cond_init(&(p->my_waker), NULL);
    pthread_cond_init(&(p->my_main_thread_waker), NULL);
    p->first_thread = my_pool_thread_init(0);
    my_pool_thread* j = p->first_thread;
    my_pool_thread* k;
    p->id_placer = 0;
    p->last_thread = NULL;
    p->size_of_pool = size;
    p->max_size_of_queue = max_size_of_queue;
    p->dynamic_max_size_of_queue = dyn_size_q;
    p->number_of_workers = 0;
    p->req_queue = my_queue_init();
    p->wait_queue = my_queue_init();
    pthread_mutex_init(&(p->my_mutex), NULL);
    p->sched_type = sched_type;

    pthread_create(&t, NULL, wrap_request_handle, p);
    for (int i = 1; i < size; i++){
        k = my_pool_thread_init(i);
        set_next_pool_thread(j, k);
        j = k;
        pthread_create(&t, NULL, wrap_request_handle, p);
    }
    p->last_thread = j;
    return p;
}

int get_type_of_sched(my_pool* p){
    return p->sched_type;
}

pthread_cond_t* get_waker(my_pool* p){
    return &(p->my_waker);
}

pthread_cond_t* get_main_thread_waker(my_pool* p){
    return &(p->my_main_thread_waker);
}

pthread_mutex_t* get_mutex(my_pool* p){
    return &(p->my_mutex);
}

void enqueue(my_pool* p, int fd, int type, struct timeval* curr){
    my_queue* q;
    if (type == ACTIVE){
        q = p->req_queue;
    }
    if (type == WAITING){
        q = p->wait_queue;
    }
    if (q->size_of_queue == 0){
        q->first = my_queue_node_init(fd, curr);
        q->last = q->first;
        q->size_of_queue += 1;
        return;
    }
    my_queue_Node* temp = my_queue_node_init(fd, curr);
    temp->next_node = q->first;
    q->first->last_node = temp;
    q->first = temp;
    q->size_of_queue += 1;
}

int dequeue(my_pool* p, int type){
    my_queue* q;
    if (type == ACTIVE){
        q = p->req_queue;
    }
    if (type == WAITING){
        q = p->wait_queue;
    }
    if (q->size_of_queue == 0){
        return -1;                    //maybe there is a problem
    }
    if (q->size_of_queue == 1){
        int val = q->last->fd_of_request;
        free(q->first->curr_req_time);
        free(q->first);
        q->first = NULL;
        q->last = NULL;
        q->size_of_queue -= 1;
        return val;
    }
    if (q->size_of_queue == 2){
        int val = q->last->fd_of_request;
        q->first->next_node = NULL;
        free(q->last->curr_req_time);
        free(q->last);
        q->last = q->first;
        q->size_of_queue -= 1;
        return val;
    }
    int val = q->last->fd_of_request;
    my_queue_Node* temp = q->last->last_node;
    temp->next_node = NULL;
    free(q->last->curr_req_time);
    free(q->last);
    q->last = temp;
    q->size_of_queue -= 1;
    return val;
}

void delete_half_random(my_pool* p){
    int length = p->wait_queue->size_of_queue;
    int target = length - floor(length/2.0);
    int j = length;
    int i = 0;
    int chosen = 0;
    my_queue* result_queue = my_queue_init();
    my_queue_Node* t = p->wait_queue->first;
    my_queue_Node* r = result_queue->first;
    my_queue_Node* combine_left;
    while ((i < length) && (j > (target-chosen)) && (chosen == i) && (chosen < target)){
        int number_random = rand() % 2;
        if (number_random == 0){
            r = my_queue_node_init(t->fd_of_request, t->curr_req_time);
            result_queue->first = r;
            result_queue->size_of_queue++;
        }
        else{
            chosen++;
            Close(t->fd_of_request);
        }
        combine_left = t;
        t = t->next_node;
        free(combine_left);
        j--;
        i++;
    }

    while ((i < length) && (j > (target-chosen)) && (chosen < target)){
        int number_random = rand() % 2;
        if (number_random == 0){
            combine_left = r;
            r->next_node = my_queue_node_init(t->fd_of_request, t->curr_req_time);
            r = r->next_node;
            r->last_node = combine_left;
            result_queue->size_of_queue++;
        }
        else{
            chosen++;
            Close(t->fd_of_request);
        }
        combine_left = t;
        t = t->next_node;
        free(combine_left);
        j--;
        i++;
    }
    if (target == chosen){
        for (int k = 0; k < j; k++){
            if (r == NULL){
                r = my_queue_node_init(t->fd_of_request, t->curr_req_time);
                result_queue->first = r;
            }
            else {
                combine_left = r;
                r->next_node = my_queue_node_init(t->fd_of_request, t->curr_req_time);
                r = r->next_node;
                r->last_node = combine_left;
            }
            result_queue->size_of_queue++;
            combine_left = t;
            t = t->next_node;
            free(combine_left);
        }
    }
    else if ((target-chosen) == j){
        for (int k = i; k < length; k++){
            combine_left = t;
            Close(t->fd_of_request);
            t = t->next_node;
            free(combine_left);
        }
    }
    result_queue->last = r;
    p->wait_queue = result_queue;
}

void* wrap_request_handle(void* pt){
    my_pool* temp_thread = (my_pool*)pt;
    int id_placer = temp_thread->id_placer;
    my_pool_thread* j = temp_thread->first_thread;
    while (id_placer > 0){
        j = j->next_thread;
        id_placer--;
    }
    temp_thread->id_placer++;
    int my_thread_id = j->id_of_thread_in_pool;
    int my_thread_count = 0;
    int my_thread_static_count = 0;
    int my_thread_dynamic_count = 0;
    struct timeval stop;
    while (1){
        pthread_mutex_lock(&(temp_thread->my_mutex));
        while (temp_thread->wait_queue->size_of_queue == 0){
            pthread_cond_wait(&(temp_thread->my_waker), &(temp_thread->my_mutex));
        }
        struct timeval arrival_time = *(get_curr_time_of_last_in_queue(temp_thread->wait_queue));
        struct timeval dis_time;
        my_thread_count++;

        int work_on_fd = dequeue(temp_thread, WAITING);
        enqueue(temp_thread, work_on_fd, ACTIVE, &dis_time);
        temp_thread->number_of_workers += 1;
        gettimeofday(&stop, NULL);
        timersub(&stop, &arrival_time, &dis_time);
        pthread_mutex_unlock(&(temp_thread->my_mutex));

        requestHandle(work_on_fd, my_thread_id, my_thread_count, &my_thread_static_count, &my_thread_dynamic_count, &arrival_time, &dis_time);
        Close(work_on_fd);

        pthread_mutex_lock(&(temp_thread->my_mutex));
        dequeue(temp_thread, ACTIVE);
        temp_thread->number_of_workers -= 1;
        pthread_mutex_unlock(&(temp_thread->my_mutex));

        if (temp_thread->sched_type == BLOCK){
            pthread_cond_signal(&(temp_thread->my_main_thread_waker));
        }
        if (temp_thread->sched_type == BLOCK_FLUSH){
            if (temp_thread->number_of_workers == 0){
                pthread_cond_signal(&(temp_thread->my_main_thread_waker));
            }
        }
    }
}


/************************** 
 * Error-handling functions
 **************************/
/* $begin errorfuns */
/* $begin unixerror */
void unix_error(char *msg) /* unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}
/* $end unixerror */

void posix_error(int code, char *msg) /* posix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(0);
}

void dns_error(char *msg) /* dns-style error */
{
    fprintf(stderr, "%s: DNS error %d\n", msg, h_errno);
    exit(0);
}

void app_error(char *msg) /* application error */
{
    fprintf(stderr, "%s\n", msg);
    exit(0);
}
/* $end errorfuns */


int Gethostname(char *name, size_t len) 
{
  int rc;

  if ((rc = gethostname(name, len)) < 0)
    unix_error("Setenv error");
  return rc;
}

int Setenv(const char *name, const char *value, int overwrite)
{
    int rc;

    if ((rc = setenv(name, value, overwrite)) < 0)
        unix_error("Setenv error");
    return rc;
}

/*********************************************
 * Wrappers for Unix process control functions
 ********************************************/

/* $begin forkwrapper */
pid_t Fork(void) 
{
    pid_t pid;

    if ((pid = fork()) < 0)
        unix_error("Fork error");
    return pid;
}
/* $end forkwrapper */

void Execve(const char *filename, char *const argv[], char *const envp[]) 
{
    if (execve(filename, argv, envp) < 0)
        unix_error("Execve error");
}

/* $begin wait */
pid_t Wait(int *status) 
{
    pid_t pid;

    if ((pid  = wait(status)) < 0)
        unix_error("Wait error");
    return pid;
}

pid_t WaitPid(pid_t pid, int *status, int options)
{
	if ((pid = waitpid(pid, status, options)) < 0) unix_error("Wait error");
	return pid;
}


/* $end wait */

/********************************
 * Wrappers for Unix I/O routines
 ********************************/

int Open(const char *pathname, int flags, mode_t mode) 
{
    int rc;

    if ((rc = open(pathname, flags, mode))  < 0)
        unix_error("Open error");
    return rc;
}

ssize_t Read(int fd, void *buf, size_t count) 
{
    ssize_t rc;

    if ((rc = read(fd, buf, count)) < 0) 
        unix_error("Read error");
    return rc;
}

ssize_t Write(int fd, const void *buf, size_t count) 
{
    ssize_t rc;

    if ((rc = write(fd, buf, count)) < 0)
        unix_error("Write error");
    return rc;
}

off_t Lseek(int fildes, off_t offset, int whence) 
{
    off_t rc;

    if ((rc = lseek(fildes, offset, whence)) < 0)
        unix_error("Lseek error");
    return rc;
}

void Close(int fd) 
{
    int rc;

    if ((rc = close(fd)) < 0)
        unix_error("Close error");
}

int Select(int  n, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout) 
{
    int rc;

    if ((rc = select(n, readfds, writefds, exceptfds, timeout)) < 0)
        unix_error("Select error");
    return rc;
}

int Dup2(int fd1, int fd2) 
{
    int rc;

    if ((rc = dup2(fd1, fd2)) < 0)
        unix_error("Dup2 error");
    return rc;
}

void Stat(const char *filename, struct stat *buf) 
{
    if (stat(filename, buf) < 0)
        unix_error("Stat error");
}

void Fstat(int fd, struct stat *buf) 
{
    if (fstat(fd, buf) < 0)
        unix_error("Fstat error");
}

/***************************************
 * Wrappers for memory mapping functions
 ***************************************/
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset) 
{
    void *ptr;

    if ((ptr = mmap(addr, len, prot, flags, fd, offset)) == ((void *) -1))
        unix_error("mmap error");
    return(ptr);
}

void Munmap(void *start, size_t length) 
{
    if (munmap(start, length) < 0)
        unix_error("munmap error");
}

/**************************** 
 * Sockets interface wrappers
 ****************************/

int Socket(int domain, int type, int protocol) 
{
    int rc;

    if ((rc = socket(domain, type, protocol)) < 0)
        unix_error("Socket error");
    return rc;
}

void Setsockopt(int s, int level, int optname, const void *optval, int optlen) 
{
    int rc;

    if ((rc = setsockopt(s, level, optname, optval, optlen)) < 0)
        unix_error("Setsockopt error");
}

void Bind(int sockfd, struct sockaddr *my_addr, int addrlen) 
{
    int rc;

    if ((rc = bind(sockfd, my_addr, addrlen)) < 0)
        unix_error("Bind error");
}

void Listen(int s, int backlog) 
{
    int rc;

    if ((rc = listen(s,  backlog)) < 0)
        unix_error("Listen error");
}

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen) 
{
    int rc;

    if ((rc = accept(s, addr, addrlen)) < 0)
        unix_error("Accept error");
    return rc;
}

void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen) 
{
    int rc;

    if ((rc = connect(sockfd, serv_addr, addrlen)) < 0)
        unix_error("Connect error");
}

/************************
 * DNS interface wrappers 
 ***********************/

/* $begin gethostbyname */
struct hostent *Gethostbyname(const char *name) 
{
    struct hostent *p;

    if ((p = gethostbyname(name)) == NULL)
        dns_error("Gethostbyname error");
    return p;
}
/* $end gethostbyname */

struct hostent *Gethostbyaddr(const char *addr, int len, int type) 
{
    struct hostent *p;

    if ((p = gethostbyaddr(addr, len, type)) == NULL)
        dns_error("Gethostbyaddr error");
    return p;
}

/*********************************************************************
 * The Rio package - robust I/O functions
 **********************************************************************/
/*
 * rio_readn - robustly read n bytes (unbuffered)
 */
/* $begin rio_readn */
ssize_t rio_readn(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR) /* interrupted by sig handler return */
                nread = 0;      /* and call read() again */
            else
                return -1;      /* errno set by read() */ 
        } 
        else if (nread == 0)
            break;              /* EOF */
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}
/* $end rio_readn */

/*
 * rio_writen - robustly write n bytes (unbuffered)
 */
/* $begin rio_writen */
ssize_t rio_writen(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR)  /* interrupted by sig handler return */
                nwritten = 0;    /* and call write() again */
            else
                return -1;       /* errorno set by write() */
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}
/* $end rio_writen */


/* 
 * rio_read - This is a wrapper for the Unix read() function that
 *    transfers min(n, rio_cnt) bytes from an internal buffer to a user
 *    buffer, where n is the number of bytes requested by the user and
 *    rio_cnt is the number of unread bytes in the internal buffer. On
 *    entry, rio_read() refills the internal buffer via a call to
 *    read() if the internal buffer is empty.
 */
/* $begin rio_read */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0) {  /* refill if buf is empty */
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, 
                           sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0) {
            if (errno != EINTR) /* interrupted by sig handler return */
                return -1;
        }
        else if (rp->rio_cnt == 0)  /* EOF */
            return 0;
        else 
            rp->rio_bufptr = rp->rio_buf; /* reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;          
    if (rp->rio_cnt < n)   
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}
/* $end rio_read */

/*
 * rio_readinitb - Associate a descriptor with a read buffer and reset buffer
 */
/* $begin rio_readinitb */
void rio_readinitb(rio_t *rp, int fd) 
{
    rp->rio_fd = fd;  
    rp->rio_cnt = 0;  
    rp->rio_bufptr = rp->rio_buf;
}
/* $end rio_readinitb */

/*
 * rio_readnb - Robustly read n bytes (buffered)
 */
/* $begin rio_readnb */
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;
    
    while (nleft > 0) {
        if ((nread = rio_read(rp, bufp, nleft)) < 0) {
            if (errno == EINTR) /* interrupted by sig handler return */
                nread = 0;      /* call read() again */
            else
                return -1;      /* errno set by read() */ 
        } 
        else if (nread == 0)
            break;              /* EOF */
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}
/* $end rio_readnb */

/* 
 * rio_readlineb - robustly read a text line (buffered)
 */
/* $begin rio_readlineb */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) { 
        if ((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return 0; /* EOF, no data read */
            else
                break;    /* EOF, some data was read */
        } else
            return -1;    /* error */
    }
    *bufp = 0;
    return n;
}
/* $end rio_readlineb */

/**********************************
 * Wrappers for robust I/O routines
 **********************************/
ssize_t Rio_readn(int fd, void *ptr, size_t nbytes) 
{
    ssize_t n;
  
    if ((n = rio_readn(fd, ptr, nbytes)) < 0)
        unix_error("Rio_readn error");
    return n;
}

void Rio_writen(int fd, void *usrbuf, size_t n) 
{
    if (rio_writen(fd, usrbuf, n) != n)
        unix_error("Rio_writen error");
}

void Rio_readinitb(rio_t *rp, int fd)
{
    rio_readinitb(rp, fd);
} 

ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n) 
{
    ssize_t rc;

    if ((rc = rio_readnb(rp, usrbuf, n)) < 0)
        unix_error("Rio_readnb error");
    return rc;
}

ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    ssize_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
        unix_error("Rio_readlineb error");
    return rc;
} 

/******************************** 
 * Client/server helper functions
 ********************************/
/*
 * open_clientfd - open connection to server at <hostname, port> 
 *   and return a socket descriptor ready for reading and writing.
 *   Returns -1 and sets errno on Unix error. 
 *   Returns -2 and sets h_errno on DNS (gethostbyname) error.
 */
/* $begin open_clientfd */
int open_clientfd(char *hostname, int port) 
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1; /* check errno for cause of error */

    /* Fill in the server's IP address and port */
    if ((hp = gethostbyname(hostname)) == NULL)
        return -2; /* check h_errno for cause of error */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr, 
          (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    /* Establish a connection with the server */
    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}
/* $end open_clientfd */

/*  
 * open_listenfd - open and return a listening socket on port
 *     Returns -1 and sets errno on Unix error.
 */
/* $begin open_listenfd */
int open_listenfd(int port) 
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
  
    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      fprintf(stderr, "socket failed\n");
      return -1;
    }
 
    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                   (const void *)&optval , sizeof(int)) < 0) {
      fprintf(stderr, "setsockopt failed\n");
      return -1;
    }

    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0) {
      fprintf(stderr, "bind failed\n");
      return -1;
    }

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0) {
      fprintf(stderr, "listen failed\n");
      return -1;
    }
    return listenfd;
}
/* $end open_listenfd */

/******************************************
 * Wrappers for the client/server helper routines 
 ******************************************/
int Open_clientfd(char *hostname, int port) 
{
    int rc;

    if ((rc = open_clientfd(hostname, port)) < 0) {
        if (rc == -1)
            unix_error("Open_clientfd Unix error");
        else        
            dns_error("Open_clientfd DNS error");
    }
    return rc;
}

int Open_listenfd(int port) 
{
    int rc;

    if ((rc = open_listenfd(port)) < 0)
        unix_error("Open_listenfd error");
    return rc;
}


