

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#define TEST_FILE   "test_shared.txt"
#define LOG_FILE    "test_append.log"
#define BUFFER_SIZE 256

// ==================== Helper functions ====================

void reset_test_file(const char* path, const char* content)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("fd creation failed: ");
        return;
    }

    if (content != NULL) {
        write(fd, content, strlen(content));
    }

    close(fd);
}

void show_file(const char* path)
{
    char buffer[BUFFER_SIZE];
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open fd failed: ");
        return;
    }

    ssize_t read_data_n = read(fd, buffer, sizeof(buffer) - 1);
    if (read_data_n > 0) {
        buffer[read_data_n] = '\0';
        printf("File content: \"%s\"\n", buffer);
    }

    close(fd);
}

unsigned long get_inode(const char* path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_ino;
    }
    return 0;
}

void print_offset(int fd, const char* desc)
{
    off_t offset = lseek(fd, 0, SEEK_CUR);
    if (offset == -1) {
        perror("lseek failed: ");
    } else {
        printf("%s: current offset = %ld\n", desc, offset);
    }
}

void print_flags(int fd, const char* desc)
{
    int val = fcntl(fd, F_GETFL, 0);
    if (val == -1) {
        perror("fcntl failed: ");
        return;
    }

    printf("%s: file status flags = ", desc);
    switch (val & O_ACCMODE) {
        case O_RDONLY: printf("O_RDONLY"); break;
        case O_WRONLY: printf("O_WRONLY"); break;
        case O_RDWR:   printf("O_RDWR"); break;
        default:       printf("unknown");
    }

    if (val & O_APPEND) printf(" | O_APPEND");
    if (val & O_SYNC)   printf(" | O_SYNC");
    if (val & O_NONBLOCK) printf(" | O_NONBLOCK");
    printf("\n");
}

// ==================== Test 1: dup shares file table entry ====================

void test_dup_share(void)
{
    printf("\n========== Test 1: dup copies fd (shares file table entry) ==========\n");

    reset_test_file(TEST_FILE, "AAAAAA\n");

    int fd1 = open(TEST_FILE, O_RDWR);
    if (fd1 < 0) {
        perror("open fd1 failed: ");
        return;
    }

    int fd2 = dup(fd1);
    printf("open returned fd1 = %d, dup returned fd2 = %d\n", fd1, fd2);
    printf("Both descriptors point to the same file table entry\n\n");

    print_offset(fd1, "fd1 initial offset");
    print_offset(fd2, "fd2 initial offset");

    const char* data1 = "BBBBBB\n";
    write(fd1, data1, strlen(data1));
    printf("\nAfter writing \"%s\" via fd1:\n", data1);
    print_offset(fd1, "fd1 offset");
    print_offset(fd2, "fd2 offset");

    const char* data2 = "CCCCCC\n";
    write(fd2, data2, strlen(data2));
    printf("\nAfter writing \"%s\" via fd2:\n", data2);
    print_offset(fd1, "fd1 offset");
    print_offset(fd2, "fd2 offset");

    show_file(TEST_FILE);

    printf("\nConclusion: After dup, both descriptors share the same file table entry and offset\n");

    close(fd1);
    close(fd2);
}

// ==================== Test 2: fork shares file table entry ====================

void test_fork_share(void)
{
    printf("\n========== Test 2: parent and child share file table entry after fork ==========\n");

    reset_test_file(TEST_FILE, "START\n");

    int fd = open(TEST_FILE, O_RDWR);
    if (fd < 0) {
        perror("open fd failed: ");
        return;
    }

    printf("Parent opened file, fd = %d\n", fd);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed: ");
        close(fd);
        return;
    }

    if (pid == 0) {
        printf("\n[Child PID = %d]\n", getpid());
        print_offset(fd, "child initial offset");

        const char* child_data = "CHILD\n";
        write(fd, child_data, strlen(child_data));
        printf("Child wrote \"%s\"\n", child_data);
        print_offset(fd, "child offset after write");

        close(fd);
        exit(0);
    } else {
        wait(NULL);

        sleep(5);
        printf("\n[Parent PID = %d]\n", getpid());
        print_offset(fd, "parent offset");

        const char* parent_data = "PARENT\n";
        write(fd, parent_data, strlen(parent_data));
        printf("Parent wrote \"%s\"\n", parent_data);
        print_offset(fd, "parent offset after write");

        show_file(TEST_FILE);
        close(fd);
    }

    printf("\nConclusion: After fork, parent and child share the same file table entry, offsets affect each other\n");
}

// ==================== Test 3: multiple open creates independent entries ====================

void test_multiopen_independent(void)
{
    printf("\n========== Test 3: multiple open creates independent file table entries ==========\n");

    reset_test_file(TEST_FILE, "111111\n");

    int fd1 = open(TEST_FILE, O_RDWR);
    int fd2 = open(TEST_FILE, O_RDWR);

    if (fd1 < 0 || fd2 < 0) {
        perror("open failed: ");
        return;
    }

    printf("fd1 = %d, fd2 = %d\n", fd1, fd2);
    printf("Both descriptors point to different file table entries but share the same inode (inode = %lu)\n\n",
           get_inode(TEST_FILE));

    print_offset(fd1, "fd1 initial offset");
    print_offset(fd2, "fd2 initial offset");

    const char* data1 = "222222\n";
    write(fd1, data1, strlen(data1));
    printf("\nAfter writing \"%s\" via fd1:\n", data1);
    print_offset(fd1, "fd1 offset");
    print_offset(fd2, "fd2 offset (unchanged)");

    const char* data2 = "333333\n";
    write(fd2, data2, strlen(data2));
    printf("\nAfter writing \"%s\" via fd2:\n", data2);
    print_offset(fd1, "fd1 offset");
    print_offset(fd2, "fd2 offset");

    show_file(TEST_FILE);

    printf("\nConclusion: Each open creates an independent file table entry, offsets do not affect each other\n");

    close(fd1);
    close(fd2);
}

// ==================== Test 4: O_APPEND atomic append ====================

void test_append_atomic(void)
{
    printf("\n========== Test 4: O_APPEND atomic append ==========\n");

    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (fd < 0) {
        perror("open failed: ");
        return;
    }

    print_flags(fd, "opened with O_APPEND");

    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            for (int j = 0; j < 5; j++) {
                char buf[64];
                snprintf(buf, sizeof(buf), "pid=%d, seq=%d\n", getpid(), j);
                write(fd, buf, strlen(buf));
                usleep(10000);
            }
            close(fd);
            exit(0);
        }
    }

    while (wait(NULL) > 0);
    close(fd);

    printf("\n3 child processes each write 5 lines. O_APPEND guarantees atomicity, no overwrites\n");

    FILE* fp = fopen(LOG_FILE, "r");
    if (fp) {
        char line[BUFFER_SIZE];
        int count = 0;
        while (fgets(line, sizeof(line), fp)) count++;
        printf("Lines actually written: %d\n", count);
        fclose(fp);
    }

    printf("\nConclusion: O_APPEND makes write atomically seek to end then write\n");
}

// ==================== Test 5: lseek+write race condition ====================

void test_race_condition(void)
{
    printf("\n========== Test 5: Non-atomic lseek+write race condition ==========\n");

    reset_test_file(LOG_FILE, NULL);

    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed: ");
        return;
    }

    printf("Without O_APPEND, using two separate calls lseek+write\n");

    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            for (int j = 0; j < 5; j++) {
                lseek(fd, 0, SEEK_END);
                char buf[64];
                snprintf(buf, sizeof(buf), "pid=%d, seq=%d\n", getpid(), j);
                write(fd, buf, strlen(buf));
                usleep(10000);
            }
            close(fd);
            exit(0);
        }
    }

    while (wait(NULL) > 0);
    close(fd);

    printf("\nlseek+write is not atomic, data may be overwritten\n");

    FILE* fp = fopen(LOG_FILE, "r");
    if (fp) {
        char line[BUFFER_SIZE];
        int count = 0;
        while (fgets(line, sizeof(line), fp)) count++;
        printf("Lines actually written: %d (expected 15, may be less)\n", count);
        fclose(fp);
    }

    printf("\nConclusion: Concurrent lseek+write from multiple processes causes data overwrite\n");
}

// ==================== Test 6: fcntl get/set flags ====================

void test_fcntl_flags(void)
{
    printf("\n========== Test 6: fcntl get/set file status flags ==========\n");

    int fd = open(TEST_FILE, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed: ");
        return;
    }

    print_flags(fd, "initial flags");

    int val = fcntl(fd, F_GETFL, 0);
    val |= O_APPEND;
    fcntl(fd, F_SETFL, val);
    print_flags(fd, "after setting O_APPEND");

    val = fcntl(fd, F_GETFL, 0);
    val |= O_SYNC;
    fcntl(fd, F_SETFL, val);
    print_flags(fd, "after setting O_SYNC");

    val = fcntl(fd, F_GETFL, 0);
    val &= ~O_APPEND;
    fcntl(fd, F_SETFL, val);
    print_flags(fd, "after clearing O_APPEND");

    printf("\nConclusion: fcntl can modify status flags without closing the file\n");

    close(fd);
}

// ==================== Test 7: fsync delayed write ====================

void test_fsync_delayed_write(void)
{
    printf("\n========== Test 7: Delayed write and fsync ==========\n");

    reset_test_file(TEST_FILE, NULL);

    int fd = open(TEST_FILE, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed: ");
        return;
    }

    const char* data = "Data in buffer cache\n";
    size_t len = strlen(data);

    printf("1. write: data goes to kernel buffer cache\n");
    clock_t start = clock();
    write(fd, data, len);
    clock_t end = clock();
    printf("   write took: %.6f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    printf("\n2. fsync: force to disk and wait\n");
    start = clock();
    if (fsync(fd) == 0) {
        end = clock();
        printf("   fsync took: %.6f s\n", (double)(end - start) / CLOCKS_PER_SEC);
    }

    printf("\n3. sync: enqueue, do not wait\n");
    start = clock();
    sync();
    end = clock();
    printf("   sync took: %.6f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    printf("\nConclusion: write is delayed, fsync flushes to disk, sync only enqueues\n");

    close(fd);
}

// ==================== Main ====================

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage: %s [dup|fork|multiopen|append|race|fcntl|fsync|all]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "dup") == 0) {
        test_dup_share();
    } else if (strcmp(argv[1], "fork") == 0) {
        test_fork_share();
    } else if (strcmp(argv[1], "multiopen") == 0) {
        test_multiopen_independent();
    } else if (strcmp(argv[1], "append") == 0) {
        test_append_atomic();
    } else if (strcmp(argv[1], "race") == 0) {
        test_race_condition();
    } else if (strcmp(argv[1], "fcntl") == 0) {
        test_fcntl_flags();
    } else if (strcmp(argv[1], "fsync") == 0) {
        test_fsync_delayed_write();
    } else if (strcmp(argv[1], "all") == 0) {
        test_dup_share();
        test_fork_share();
        test_multiopen_independent();
        test_append_atomic();
        test_race_condition();
        test_fcntl_flags();
        test_fsync_delayed_write();
        printf("\n========== All tests completed ==========\n");
    } else {
        printf("Unknown option: %s\n", argv[1]);
    }

    unlink(TEST_FILE);
    unlink(LOG_FILE);

    return 0;
}