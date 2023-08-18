/*
 * client.c: A very, very primitive HTTP client.
 * 
 * To run, try: 
 *      ./client www.cs.technion.ac.il 80 /
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * HW3: For testing your server, you will want to modify this client.  
 * For example:
 * 
 * You may want to make this multi-threaded so that you can 
 * send many requests simultaneously to the server.
 *
 * You may also want to be able to request different URIs; 
 * you may want to get more URIs from the command line 
 * or read the list from a file. 
 *
 * When we test your server, we will be using modifications to this client.
 *
 */

#include "segel.h"

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
//  int given_thread_id = 0;
//  int given_thread_count = 0;
//  int given_thread_static_count = 0;
//  int given_thread_dynamic_count = 0;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

//    struct timeval arrival;
//    struct timeval dispatch;
    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
//    else if (sscanf(buf, "Stat-Req-Arrival:: %lu.%06lu ", &(arrival.tv_sec), &(arrival.tv_usec)) == 1) {
//        printf("Header: Stat-Req-Arrival:: %lu.%06lu\n", (arrival.tv_sec), (arrival.tv_usec));
//    }
//    else if (sscanf(buf, "Stat-Req-Dispatch:: %lu.%06lu ", &(dispatch.tv_sec), &(dispatch.tv_usec)) == 1) {
//        printf("Header: Stat-Req-Dispatch:: %lu.%06lu\n", (dispatch.tv_sec), (dispatch.tv_usec));
//    }
//    else if (sscanf(buf, "Stat-Thread-Id:: %d ", &given_thread_id) == 1) {
//        printf("Header: Stat-Thread-Id:: %d\n", given_thread_id);
//    }
//    else if (sscanf(buf, "Stat-Thread-Count:: %d ", &given_thread_count) == 1) {
//        printf("Header: Stat-Thread-Count:: %d\n", given_thread_count);
//    }
//    else if (sscanf(buf, "Stat-Thread-Static:: %d ", &given_thread_static_count) == 1) {
//        printf("Header: Stat-Thread-Static:: %d\n", given_thread_static_count);
//    }
//    else if (sscanf(buf, "Stat-Thread-Dynamic:: %d ", &given_thread_dynamic_count) == 1) {
//        printf("Header: Stat-Thread-Dynamic:: %d\n", given_thread_dynamic_count);
//    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

int main(int argc, char *argv[]) {
    char *host, *filename;
    int port;
    int clientfd;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
        exit(1);
    }

    host = argv[1];
    port = atoi(argv[2]);
    filename = argv[3];

    /* Open a single connection to the specified host and port */
    clientfd = Open_clientfd(host, port);

    clientSend(clientfd, filename);
    clientPrint(clientfd);

    Close(clientfd);

    exit(0);
}