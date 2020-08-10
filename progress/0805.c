#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int num_threads = 1;
unsigned long memory = 1;
unsigned long bytes_per_thread = 1;

void set_ftrace_param() {
    int pid = getpid();
    int ret;
    char buff[200];
    sprintf(buff, "echo %d > /sys/kernel/debug/tracing/set_ftrace_pid", pid);

    printf("setting ftrace parameters\n");
    if ((ret = system("rm -f log")) == -1)
        goto error;
    if ((ret = system("echo 0 > /sys/kernel/debug/tracing/tracing_on")) == -1) /* tracing_on */
        goto error;
    if ((ret = system("echo 1 > /sys/kernel/debug/tracing/events/enable")) == -1) /* trace */
        goto error;
    if ((ret = system("echo function_graph > /sys/kernel/debug/tracing/current_tracer")) == -1) /* current_tracer */
        goto error;
    if ((ret = system("echo __do_page_fault > /sys/kernel/debug/tracing/set_ftrace_filter")) == -1) /* set_ftrace_filter */
        goto error;
    /* set buffer to 1GB */
    ret = system("echo 1048576 > /sys/kernel/debug/tracing/buffer_size_kb");

    if ((ret = system(buff)) == -1) /* set process id (make "echo pid(%d) > ..." string using sprintf & buff)*/
        goto error;
    if ((ret = system("echo 1 > /sys/kernel/debug/tracing/tracing_on")) == -1) /* start tracing_on */
        goto error;

    printf("ftrace setup. executing the workload...\n");
    return;

error:
    printf("Error occured in set_ftrace_param()\n");
    exit(EXIT_FAILURE);
}
  
void collect_and_clear_ftrace() {
    int ret;

    /*if ((ret = system("cat /sys/kernel/debug/tracing/trace | grep __do_page_fault"))==-1)
            goto error;*/
    if ((ret = system("cp /sys/kernel/debug/tracing/trace log")) == -1)
        goto error;
    if ((ret = system("echo 1408 > /sys/kernel/debug/tracing/buffer_size_kb")) == -1)
        goto error;

    system("sed 's/^.......//' log > tmp; rm -r log;\
                        cat tmp | grep __do_page_fault > log; rm -f tmp");
    /*system("cat log");*/
    printf("\n");
    system("avg=$(awk '{sum+=$1;n++} END {if(n>0) print(sum/n)}' log);\
                        echo Average Page Fault Time: $avg microsecs");
    printf("\n");
    printf("ftrace parameters cleared..\n");

error:
    //printf("Error occured in collect_and_clear_ftrace()\n");
    exit(EXIT_FAILURE);
}


void* thread_routine(void* arg) {
    pthread_t  tid;
    char* tmp, * start;
    int nr_pages;

    tid = pthread_self();

    printf("\ttid: %lx\n", tid);

    //      char* tmp = (char *)malloc(sizeof(char*) * 512);
    tmp = malloc(bytes_per_thread);
    if (!tmp)
        exit(EXIT_FAILURE);

    //      *tmp = 'A';

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

    while ((c = getopt(argc, argv, "m:t:")) != -1) {
        switch (c) {
        case 'm':
            //memory = 10;
            memory = atoi(optarg);
            break;
        case 't':
            //num_threads = 12;
            num_threads = atoi(optarg);
            break;
        default:
            printf(" >> %s [-m memoryGB] [-t num_threads]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    /* setting initial setup for ftrace */
    set_ftrace_param();

    thread = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
    if (!thread) exit(EXIT_FAILURE);

    bytes_per_thread = (memory * 1024 * 1024 * 1024) / num_threads;

    for (i = 0; i < num_threads; i++)
        pthread_create(&thread[i], NULL, thread_routine, NULL);

    for (i = 0; i < num_threads; i++)
        pthread_join(thread[i], NULL);

    collect_and_clear_ftrace();
    system("echo 1408 > /sys/kernel/debug/tracing/buffer_size_kb");

    printf("complete...\n");
    return 0;
}




