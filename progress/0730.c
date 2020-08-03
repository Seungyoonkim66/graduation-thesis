#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>      // 파일 제어 : http://neosrtos.com/docs/posix_api/fcntl.html
#include <unistd.h>     // 표준 심볼 상수 및 자료형: http://neosrtos.com/docs/posix_api/unistd.html
#include <sys/mman.h>   // 메모리 관리 : https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/mman.h.html

int num_threads = 1;
unsigned long memory = 1;
unsigned long bytes_per_thread = 1;

void* thread_routine(void* arg) {
    pthread_t  tid;
    char* tmp, * start;
    int nr_pages;

    tid = pthread_self();

    printf("\ttid: %lx\n", tid);

    tmp = malloc(bytes_per_thread);
    if (!tmp)
        exit(EXIT_FAILURE);

    start = tmp;
    nr_pages = bytes_per_thread / 4096;
    while (nr_pages--) {
        *start = 'A';
        start += 4096;
    }

    free(tmp);

    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t* thread;
    int i, c;

    while ((c = getopt(argc, argv, "m:t:")) != -1) {    // getopt : 프록그램 실행 명령행 인자 파싱 : https://www.joinc.co.kr/w/man/3/getopt
        switch (c) {
        case 'm':
            memory = atoi(optarg);  // atoi : char to int <stdlib.h>
            break;
        case 't':
            num_threads = atoi(optarg);
            break;
        default:
            printf(" >> %s [-m memoryGB] [-t num_threads]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    thread = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
    if (!thread) exit(EXIT_FAILURE);

    bytes_per_thread = (memory * 1024 * 1024 * 1024) / num_threads;

    for (i = 0; i < num_threads; i++)
        pthread_create(&thread[i], NULL, thread_routine, NULL);
        // int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg)
        // 1. thread : 생성된 thread의 ID
        // 2. att : thread의 특성 (default = NULL) 
        //    thread 속성 지정하려면 pthread_attr_init으로 초기화해야 한다.
        // 3. start_routine : 어떤 로직을 할지 함수 포인터를 매개변수로 받는다.
        // 4. arg : start_routine에 전달될 인자 
        // thread 생성 성공시 0 반환 실패시 1 반환  
        // http://contents.kocw.or.kr/document/10_Thread.pdf

    for (i = 0; i < num_threads; i++)
        pthread_join(thread[i], NULL);

    printf("complete...\n");
    return 0;
}

