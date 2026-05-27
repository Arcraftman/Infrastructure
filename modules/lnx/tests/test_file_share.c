

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

// ==================== 辅助函数 ====================

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
        printf("文件内容: \"%s\"\n", buffer);
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
        printf("%s: 当前文件偏移量 = %ld\n", desc, offset);
    }
}

void print_flags(int fd, const char* desc)
{
    int val = fcntl(fd, F_GETFL, 0);
    if (val == -1) {
        perror("fcntl failed: ");
        return;
    }

    printf("%s: 文件状态标志 = ", desc);
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

// ==================== 测试1: dup 共享文件表项 ====================

void test_dup_share(void)
{
    printf("\n========== 测试1: dup 复制文件描述符（共享文件表项） ==========\n");

    reset_test_file(TEST_FILE, "AAAAAA\n");

    int fd1 = open(TEST_FILE, O_RDWR);
    if (fd1 < 0) {
        perror("open fd1 failed: ");
        return;
    }

    int fd2 = dup(fd1);
    printf("open 返回 fd1 = %d, dup 返回 fd2 = %d\n", fd1, fd2);
    printf("两个描述符指向同一个文件表项\n\n");

    print_offset(fd1, "fd1 初始偏移");
    print_offset(fd2, "fd2 初始偏移");

    const char* data1 = "BBBBBB\n";
    write(fd1, data1, strlen(data1));
    printf("\n通过 fd1 写入 \"%s\" 后:\n", data1);
    print_offset(fd1, "fd1 偏移");
    print_offset(fd2, "fd2 偏移");

    const char* data2 = "CCCCCC\n";
    write(fd2, data2, strlen(data2));
    printf("\n通过 fd2 写入 \"%s\" 后:\n", data2);
    print_offset(fd1, "fd1 偏移");
    print_offset(fd2, "fd2 偏移");

    show_file(TEST_FILE);

    printf("\n结论: dup 后两个描述符指向同一个文件表项，共享偏移量\n");

    close(fd1);
    close(fd2);
}

// ==================== 测试2: fork 共享文件表项 ====================

void test_fork_share(void)
{
    printf("\n========== 测试2: fork 后父子进程共享文件表项 ==========\n");

    reset_test_file(TEST_FILE, "START\n");

    int fd = open(TEST_FILE, O_RDWR);
    if (fd < 0) {
        perror("open fd failed: ");
        return;
    }

    printf("父进程打开文件，fd = %d\n", fd);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed: ");
        close(fd);
        return;
    }

    if (pid == 0) {
        printf("\n[子进程 PID = %d]\n", getpid());
        print_offset(fd, "子进程初始偏移");

        const char* child_data = "CHILD\n";
        write(fd, child_data, strlen(child_data));
        printf("子进程写入 \"%s\"\n", child_data);
        print_offset(fd, "子进程写入后偏移");

        close(fd);
        exit(0);
    } else {
        wait(NULL);

        sleep(5);
        printf("\n[父进程 PID = %d]\n", getpid());
        print_offset(fd, "父进程偏移");

        const char* parent_data = "PARENT\n";
        write(fd, parent_data, strlen(parent_data));
        printf("父进程写入 \"%s\"\n", parent_data);
        print_offset(fd, "父进程写入后偏移");

        show_file(TEST_FILE);
        close(fd);
    }

    printf("\n结论: fork 后父子进程共享同一个文件表项，偏移量互相影响\n");
}

// ==================== 测试3: 多次 open 独立文件表项 ====================

void test_multiopen_independent(void)
{
    printf("\n========== 测试3: 多次 open 获得独立的文件表项 ==========\n");

    reset_test_file(TEST_FILE, "111111\n");

    int fd1 = open(TEST_FILE, O_RDWR);
    int fd2 = open(TEST_FILE, O_RDWR);

    if (fd1 < 0 || fd2 < 0) {
        perror("open failed: ");
        return;
    }

    printf("fd1 = %d, fd2 = %d\n", fd1, fd2);
    printf("两个描述符指向不同的文件表项，但共享同一个 inode (inode = %lu)\n\n",
           get_inode(TEST_FILE));

    print_offset(fd1, "fd1 初始偏移");
    print_offset(fd2, "fd2 初始偏移");

    const char* data1 = "222222\n";
    write(fd1, data1, strlen(data1));
    printf("\n通过 fd1 写入 \"%s\" 后:\n", data1);
    print_offset(fd1, "fd1 偏移");
    print_offset(fd2, "fd2 偏移（不变）");

    const char* data2 = "333333\n";
    write(fd2, data2, strlen(data2));
    printf("\n通过 fd2 写入 \"%s\" 后:\n", data2);
    print_offset(fd1, "fd1 偏移");
    print_offset(fd2, "fd2 偏移");

    show_file(TEST_FILE);

    printf("\n结论: 每次 open 创建独立的文件表项，偏移量互不影响\n");

    close(fd1);
    close(fd2);
}

// ==================== 测试4: O_APPEND 原子追加 ====================

void test_append_atomic(void)
{
    printf("\n========== 测试4: O_APPEND 原子追加 ==========\n");

    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (fd < 0) {
        perror("open failed: ");
        return;
    }

    print_flags(fd, "打开时设置 O_APPEND");

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

    printf("\n3个子进程各写入5行，O_APPEND 保证原子性，数据不会覆盖\n");

    FILE* fp = fopen(LOG_FILE, "r");
    if (fp) {
        char line[BUFFER_SIZE];
        int count = 0;
        while (fgets(line, sizeof(line), fp)) count++;
        printf("实际写入行数: %d\n", count);
        fclose(fp);
    }

    printf("\n结论: O_APPEND 使 write 原子地移到文件末尾再写\n");
}

// ==================== 测试5: lseek+write 竞争条件 ====================

void test_race_condition(void)
{
    printf("\n========== 测试5: 非原子 lseek+write 竞争条件 ==========\n");

    reset_test_file(LOG_FILE, NULL);

    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed: ");
        return;
    }

    printf("未使用 O_APPEND，使用 lseek+write 两个独立调用\n");

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

    printf("\nlseek+write 不是原子操作，数据可能被覆盖\n");

    FILE* fp = fopen(LOG_FILE, "r");
    if (fp) {
        char line[BUFFER_SIZE];
        int count = 0;
        while (fgets(line, sizeof(line), fp)) count++;
        printf("实际写入行数: %d (预期15行，可能少于15)\n", count);
        fclose(fp);
    }

    printf("\n结论: 多个进程并发 lseek+write 会导致数据覆盖\n");
}

// ==================== 测试6: fcntl 获取/设置标志 ====================

void test_fcntl_flags(void)
{
    printf("\n========== 测试6: fcntl 获取/设置文件状态标志 ==========\n");

    int fd = open(TEST_FILE, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed: ");
        return;
    }

    print_flags(fd, "初始标志");

    int val = fcntl(fd, F_GETFL, 0);
    val |= O_APPEND;
    fcntl(fd, F_SETFL, val);
    print_flags(fd, "设置 O_APPEND 后");

    val = fcntl(fd, F_GETFL, 0);
    val |= O_SYNC;
    fcntl(fd, F_SETFL, val);
    print_flags(fd, "设置 O_SYNC 后");

    val = fcntl(fd, F_GETFL, 0);
    val &= ~O_APPEND;
    fcntl(fd, F_SETFL, val);
    print_flags(fd, "清除 O_APPEND 后");

    printf("\n结论: fcntl 可以在不关闭文件的情况下修改状态标志\n");

    close(fd);
}

// ==================== 测试7: fsync 延迟写 ====================

void test_fsync_delayed_write(void)
{
    printf("\n========== 测试7: 延迟写与 fsync ==========\n");

    reset_test_file(TEST_FILE, NULL);

    int fd = open(TEST_FILE, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed: ");
        return;
    }

    const char* data = "Data in buffer cache\n";
    size_t len = strlen(data);

    printf("1. write: 数据进入内核缓冲区\n");
    clock_t start = clock();
    write(fd, data, len);
    clock_t end = clock();
    printf("   write 耗时: %.6f 秒\n", (double)(end - start) / CLOCKS_PER_SEC);

    printf("\n2. fsync: 强制写入磁盘并等待\n");
    start = clock();
    if (fsync(fd) == 0) {
        end = clock();
        printf("   fsync 耗时: %.6f 秒\n", (double)(end - start) / CLOCKS_PER_SEC);
    }

    printf("\n3. sync: 排入队列，不等待\n");
    start = clock();
    sync();
    end = clock();
    printf("   sync 耗时: %.6f 秒\n", (double)(end - start) / CLOCKS_PER_SEC);

    printf("\n结论: write 延迟写，fsync 强制落盘，sync 仅排入队列\n");

    close(fd);
}

// ==================== 主函数 ====================

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("用法: %s [dup|fork|multiopen|append|race|fcntl|fsync|all]\n", argv[0]);
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
        printf("\n========== 所有测试完成 ==========\n");
    } else {
        printf("未知选项: %s\n", argv[1]);
    }

    unlink(TEST_FILE);
    unlink(LOG_FILE);

    return 0;
}