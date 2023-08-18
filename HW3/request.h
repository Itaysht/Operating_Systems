#ifndef __REQUEST_H__

#include "segel.h"

void requestHandle(int fd, int my_thread_id, int my_thread_count, int* my_thread_static_count, int* my_thread_dynamic_count, struct timeval* arrival, struct timeval* dis);

#endif
