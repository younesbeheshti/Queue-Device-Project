#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>

#define DEVICE "/dev/myQueue"
#define NUM_THREADS 5
#define QUEUE_SIZE 30

char data;
int size = 0;
double multicoreDuration;
double singlecoreDuration;
sem_t sem_empty;
sem_t sem_full;
pthread_mutex_t lock;

pthread_t reader_threads[NUM_THREADS - 1];
pthread_t writer_thread;

char randomChar();
int writeToQueue();
int readFromQueue();
void clearQueue();
double getTime();
void assignToThisCore(int core_id);
void singleCore();
void multiCore();
void *reader(void *arg);
void *writer(void *arg);

int main() {
    srand(time(NULL));
    clearQueue();

    if (pthread_mutex_init(&lock, NULL) != 0) {
        fprintf(stderr, "Failed to initialize mutex\n");
        return EXIT_FAILURE;
    }

    printf("Running single-core test:\n");
    singleCore();

    clearQueue();

    printf("Running multi-core test:\n");
    multiCore();

    pthread_mutex_destroy(&lock);

    printf("singlecore duration -> %.6f\n", singlecoreDuration);
    printf("mulitcore duration -> %.6f\n", multicoreDuration);

    return EXIT_SUCCESS;
}

void clearQueue() {
    system("cat /dev/myQueue > /dev/null");
    size = 0;
}

double getTime() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

void assignToThisCore(int core_id) {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id < 0 || core_id >= num_cores) {
        fprintf(stderr, "Invalid core ID: %d. Available cores: 0-%d\n", core_id, num_cores - 1);
        return;
    }

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core_id, &mask);

    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &mask) != 0) {
        perror("Failed to set processor affinity");
    } else {
        printf("Thread %lu assigned to core %d\n", current_thread, core_id);
    }
}

void singleCore() {
    sem_init(&sem_empty, 0, QUEUE_SIZE);
    sem_init(&sem_full, 0, 0);

    double start_time = getTime();
    assignToThisCore(0);

    int core = 0;
    pthread_create(&writer_thread, NULL, writer, &core);


    for (int i = 0; i < NUM_THREADS - 1; i++) {
        pthread_create(&reader_threads[i], NULL, reader, &core);
    }

    pthread_join(writer_thread, NULL);
    for (int i = 0; i < NUM_THREADS - 1; i++) {
        pthread_join(reader_threads[i], NULL);
    }

    double end_time = getTime();
    singlecoreDuration = end_time - start_time;
    printf("Single-core duration: %.6f seconds\n\n", singlecoreDuration);

    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
}

void multiCore() {
    sem_init(&sem_empty, 0, QUEUE_SIZE);
    sem_init(&sem_full, 0, 0);

    double start_time = getTime();

    int core_ids[NUM_THREADS];
    core_ids[0] = 0;
    pthread_create(&writer_thread, NULL, writer, &core_ids[0]);


    for (int i = 1; i < NUM_THREADS; i++) {
        core_ids[i] = i;
        pthread_create(&reader_threads[i - 1], NULL, reader, &core_ids[i]);
    }

    pthread_join(writer_thread, NULL);
    for (int i = 1; i < NUM_THREADS; i++) {
        pthread_join(reader_threads[i - 1], NULL);
    }

    double end_time = getTime();
    multicoreDuration = end_time - start_time;
    printf("Multi-core duration: %.6f seconds\n\n", multicoreDuration);

    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
}

void *writer(void *arg) {
    int core_id = *(int *)arg;
    assignToThisCore(core_id);

    for (int i = 0; i < QUEUE_SIZE; i++) {
        sem_wait(&sem_empty);
        pthread_mutex_lock(&lock);
        if (!writeToQueue()) {
            fprintf(stderr, "Error writing to queue\n");
        }
        pthread_mutex_unlock(&lock);
        sem_post(&sem_full);

        
        int x = 0;
        for (int i = 0; i < 1000000; i++)
        {
            x += i;
        }

    }

    printf("Writer thread on core %d finished\n", core_id);

    

    return NULL;
}

void *reader(void *arg) {
    int core_id = *(int *)arg;
    assignToThisCore(core_id);

    for (int i = 0; i < 5; i++) {
        sem_wait(&sem_full);
        pthread_mutex_lock(&lock);
        if (!readFromQueue()) {
            fprintf(stderr, "Error reading from queue\n");
        }
        pthread_mutex_unlock(&lock);
        sem_post(&sem_empty);



        int x = 0;
        for (int i = 0; i < 1000000; i++)
        {
            x += i;
        }
    }

    printf("Reader thread on core %d finished\n", core_id);

    return NULL;
}

int readFromQueue() {
    if (size == 0) {
        fprintf(stderr, "Queue is empty\n");
        return 0;
    }

    int fd = open(DEVICE, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return 0;
    }

    if (read(fd, &data, 1) < 0) {
        perror("Failed to read from the device");
        close(fd);
        return 0;
    }

    size--;
    printf("Read data: %c, Queue size: %d\n", data, size);

    close(fd);
    return 1;
}

int writeToQueue() {
    int fd = open(DEVICE, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return 0;
    }

    char ch = randomChar();
    if (write(fd, &ch, 1) < 0) {
        perror("Failed to write to the device");
        close(fd);
        return 0;
    }

    size++;
    printf("Wrote data: %c, Queue size: %d\n", ch, size);

    close(fd);
    return 1;
}

char randomChar() {
    return (rand() % 26) + 'a';
}
