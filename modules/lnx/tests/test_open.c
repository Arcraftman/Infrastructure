#define _GNU_SOURCE         // 启用 GNU 扩展功能

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <utime.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>       // 添加 wait() 函数


int main(int argc, char *argv[])
{
    int fd1, fd2, fd3;
    ssize_t n;
    off_t offset;
    pid_t pid;
    
    printf("\n========== 丰富的文件 I/O 系统调用演示 ==========\n\n");
    
    // ==================== 1. open() - 打开文件 ====================
    printf("【1. open() - 打开文件】\n");
    fd1 = open("test1.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd1 == -1) {
        perror("   open失败");
        return 1;
    }
    printf("   打开/创建文件 test1.txt, fd = %d\n", fd1);
    
    // ==================== 2. write() - 写入数据 ====================
    printf("\n【2. write() - 写入数据】\n");
    const char *data = "Hello from write() system call!\n";
    n = write(fd1, data, strlen(data));
    printf("   写入 %ld 字节: %s", n, data);
    
    // ==================== 3. read() - 读取数据 ====================
    printf("\n【3. read() - 读取数据】\n");
    lseek(fd1, 0, SEEK_SET);  // 重置到开头
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    n = read(fd1, buffer, sizeof(buffer) - 1);
    printf("   读取 %ld 字节: %s", n, buffer);
    
    // ==================== 4. lseek() - 移动文件指针 ====================
    printf("\n【4. lseek() - 移动文件指针】\n");
    offset = lseek(fd1, 0, SEEK_CUR);
    printf("   当前位置: %ld\n", offset);
    offset = lseek(fd1, 0, SEEK_END);
    printf("   文件大小: %ld\n", offset);
    lseek(fd1, 0, SEEK_SET);
    printf("   重置到开头\n");
    
    // ==================== 5. dup() - 复制文件描述符 ====================
    printf("\n【5. dup() - 复制文件描述符】\n");
    fd2 = dup(fd1);
    printf("   复制 fd %d -> fd %d\n", fd1, fd2);
    // 通过新描述符写入
    const char *dup_data = "Data from duplicated fd!\n";
    write(fd2, dup_data, strlen(dup_data));
    printf("   通过复制的fd写入数据\n");
    
    // ==================== 6. dup2() - 复制到指定描述符 ====================
    printf("\n【6. dup2() - 复制到指定描述符】\n");
    fd3 = dup2(fd1, 100);
    printf("   复制 fd %d 到 fd 100, 返回 %d\n", fd1, fd3);
    const char *dup2_data = "Data from dup2()!\n";
    write(100, dup2_data, strlen(dup2_data));
    printf("   通过 fd 100 写入数据\n");
    
    // ==================== 7. close() - 关闭文件描述符 ====================
    printf("\n【7. close() - 关闭文件描述符】\n");
    close(fd2);
    printf("   关闭 fd %d\n", fd2);
    close(100);
    printf("   关闭 fd 100\n");
    
    // ==================== 8. stat() / fstat() - 获取文件状态 ====================
    printf("\n【8. stat()/fstat() - 获取文件状态】\n");
    struct stat file_stat;
    if (fstat(fd1, &file_stat) == 0) {
        printf("   文件大小: %ld 字节\n", file_stat.st_size);
        printf("   权限: %o\n", file_stat.st_mode & 0777);
        printf("   链接数: %ld\n", file_stat.st_nlink);
        printf("   UID: %d, GID: %d\n", file_stat.st_uid, file_stat.st_gid);
        printf("   设备号: %ld\n", file_stat.st_dev);
        printf("   inode号: %ld\n", file_stat.st_ino);
    }
    
    // ==================== 9. access() - 检查文件权限 ====================
    printf("\n【9. access() - 检查文件权限】\n");
    if (access("test1.txt", R_OK) == 0)
        printf("   文件可读\n");
    if (access("test1.txt", W_OK) == 0)
        printf("   文件可写\n");
    if (access("test1.txt", X_OK) == 0)
        printf("   文件可执行\n");
    
    // ==================== 10. chmod() / fchmod() - 修改权限 ====================
    printf("\n【10. chmod() - 修改文件权限】\n");
    if (chmod("test1.txt", 0600) == 0)
        printf("   修改权限为 0600 (rw-------)\n");
    
    // ==================== 11. truncate() / ftruncate() - 截断文件 ====================
    printf("\n【11. ftruncate() - 截断文件】\n");
    if (ftruncate(fd1, 50) == 0) {
        printf("   截断文件到 50 字节\n");
        struct stat new_stat;
        fstat(fd1, &new_stat);
        printf("   新文件大小: %ld 字节\n", new_stat.st_size);
    }
    
    // ==================== 12. link() - 创建硬链接 ====================
    printf("\n【12. link() - 创建硬链接】\n");
    if (link("test1.txt", "test1_hardlink.txt") == 0) {
        printf("   创建硬链接 test1_hardlink.txt\n");
        struct stat link_stat;
        stat("test1.txt", &link_stat);
        printf("   链接数增加到: %ld\n", link_stat.st_nlink);
    }
    
    // ==================== 13. symlink() - 创建符号链接 ====================
    printf("\n【13. symlink() - 创建符号链接】\n");
    if (symlink("test1.txt", "test1_symlink.txt") == 0)
        printf("   创建符号链接 test1_symlink.txt -> test1.txt\n");
    
    // ==================== 14. readlink() - 读取符号链接 ====================
    printf("\n【14. readlink() - 读取符号链接】\n");
    char link_target[256];
    n = readlink("test1_symlink.txt", link_target, sizeof(link_target) - 1);
    if (n != -1) {
        link_target[n] = '\0';
        printf("   符号链接指向: %s\n", link_target);
    }
    
    // ==================== 15. unlink() - 删除文件/链接 ====================
    printf("\n【15. unlink() - 删除文件/链接】\n");
    unlink("test1_hardlink.txt");
    printf("   删除硬链接文件\n");
    unlink("test1_symlink.txt");
    printf("   删除符号链接文件\n");
    
    // ==================== 16. rename() - 重命名文件 ====================
    printf("\n【16. rename() - 重命名文件】\n");
    if (rename("test1.txt", "test_renamed.txt") == 0)
        printf("   重命名 test1.txt -> test_renamed.txt\n");
    
    // ==================== 17. mkdir() - 创建目录 ====================
    printf("\n【17. mkdir() - 创建目录】\n");
    if (mkdir("test_dir", 0755) == 0)
        printf("   创建目录 test_dir\n");
    
    // ==================== 18. rmdir() - 删除目录 ====================
    printf("\n【18. rmdir() - 删除目录】\n");
    if (rmdir("test_dir") == 0)
        printf("   删除目录 test_dir\n");
    
    // ==================== 19. opendir() / readdir() / closedir() - 遍历目录 ====================
    printf("\n【19. opendir()/readdir()/closedir() - 遍历目录】\n");
    mkdir("test_dir", 0755);  // 重新创建用于演示
    DIR *dir = opendir(".");
    if (dir) {
        struct dirent *entry;
        int count = 0;
        printf("   当前目录内容:\n");
        while ((entry = readdir(dir)) != NULL && count < 10) {
            printf("     - %s (类型: %d)\n", entry->d_name, entry->d_type);
            count++;
        }
        closedir(dir);
        printf("   关闭目录流\n");
    }
    rmdir("test_dir");
    
    // ==================== 20. chdir() / getcwd() - 改变/获取工作目录 ====================
    printf("\n【20. chdir()/getcwd() - 改变/获取工作目录】\n");
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("   当前目录: %s\n", cwd);
    }
    mkdir("temp_dir", 0755);
    if (chdir("temp_dir") == 0) {
        char new_cwd[1024];
        getcwd(new_cwd, sizeof(new_cwd));
        printf("   切换到: %s\n", new_cwd);
        chdir("..");  // 切换回去
    }
    rmdir("temp_dir");
    
    // ==================== 21. utime() - 修改文件时间戳 ====================
    printf("\n【21. utime() - 修改文件时间戳】\n");
    struct utimbuf times;
    times.actime = 1000000000;  // 访问时间
    times.modtime = 1000000000;  // 修改时间
    if (utime("test_renamed.txt", &times) == 0)
        printf("   修改文件时间戳\n");
    
    // ==================== 22. fsync() - 同步文件数据到磁盘 ====================
    printf("\n【22. fsync() - 同步到磁盘】\n");
    if (fsync(fd1) == 0)
        printf("   同步文件数据到磁盘\n");
    
    // ==================== 23. fdatasync() - 同步数据（不含元数据） ====================
    printf("\n【23. fdatasync() - 同步数据】\n");
    if (fdatasync(fd1) == 0)
        printf("   同步文件数据（不含元数据）\n");
    
    // ==================== 24. fork() - 创建子进程 ====================
    printf("\n【24. fork() - 创建子进程】\n");
    pid = fork();
    if (pid == 0) {
        // 子进程
        printf("   子进程 (PID: %d) 写入数据\n", getpid());
        const char *child_data = "Data from child process\n";
        write(fd1, child_data, strlen(child_data));
        exit(0);
    } else if (pid > 0) {
        // 父进程
        wait(NULL);  // 等待子进程
        printf("   父进程等待子进程完成\n");
    }
    
    // ==================== 25. getpid() / getppid() - 获取进程ID ====================
    printf("\n【25. getpid()/getppid() - 获取进程ID】\n");
    printf("   当前进程PID: %d\n", getpid());
    printf("   父进程PPID: %d\n", getppid());
    
    // ==================== 26. sleep() - 睡眠 ====================
    printf("\n【26. sleep() - 进程睡眠】\n");
    printf("   睡眠1秒...\n");
    sleep(1);
    printf("   醒来继续执行\n");
    
    // ==================== 27. pipe() - 创建管道 ====================
    printf("\n【27. pipe() - 创建管道】\n");
    int pipefd[2];
    if (pipe(pipefd) == 0) {
        printf("   创建管道: 读端%d, 写端%d\n", pipefd[0], pipefd[1]);
        const char *pipe_data = "Data through pipe";
        write(pipefd[1], pipe_data, strlen(pipe_data) + 1);
        char pipe_buf[100];
        read(pipefd[0], pipe_buf, sizeof(pipe_buf));
        printf("   通过管道传输: %s\n", pipe_buf);
        close(pipefd[0]);
        close(pipefd[1]);
    }
    
    // ==================== 28. select() - I/O多路复用 ====================
    printf("\n【28. select() - I/O多路复用】\n");
    fd_set read_fds;
    struct timeval tv;
    FD_ZERO(&read_fds);
    FD_SET(fd1, &read_fds);
    tv.tv_sec = 0;
    tv.tv_usec = 100000;  // 100ms
    int ret = select(fd1 + 1, &read_fds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(fd1, &read_fds))
        printf("   文件描述符 %d 可读\n", fd1);
    else if (ret == 0)
        printf("   超时，无可读数据\n");
    
    // ==================== 29. ioctl() - 设备控制 ====================
    printf("\n【29. ioctl() - 设备控制】\n");
    unsigned long bytes_available;
    if (ioctl(fd1, FIONREAD, &bytes_available) == 0)
        printf("   可读字节数: %ld\n", bytes_available);
    else
        printf("   ioctl 不支持此文件类型\n");
    
    // ==================== 30. dup2 重定向标准输出 ====================
    printf("\n【30. 重定向标准输出】\n");
    int stdout_save = dup(STDOUT_FILENO);
    int new_fd = open("redirect_output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(new_fd, STDOUT_FILENO);
    printf("这行文字会写入文件 redirect_output.txt\n");
    fflush(stdout);
    dup2(stdout_save, STDOUT_FILENO);
    printf("   恢复标准输出，上面一行已重定向到文件\n");
    close(stdout_save);
    close(new_fd);
    
    // ==================== 31. fcntl() - 文件控制 ====================
    printf("\n【31. fcntl() - 文件控制】\n");
    int flags = fcntl(fd1, F_GETFL);
    printf("   当前文件状态标志: 0x%x\n", flags);
    printf("   访问模式: %s\n", (flags & O_ACCMODE) == O_RDWR ? "读写" : 
                                (flags & O_ACCMODE) == O_RDONLY ? "只读" : "只写");
    // 设置非阻塞标志
    fcntl(fd1, F_SETFL, flags | O_NONBLOCK);
    printf("   设置非阻塞模式\n");
    // 恢复
    fcntl(fd1, F_SETFL, flags);
    
    // ==================== 32. lstat() - 获取链接状态 ====================
    printf("\n【32. lstat() - 获取链接状态】\n");
    struct stat link_stat;
    symlink("test_renamed.txt", "test_link.tmp");
    if (lstat("test_link.tmp", &link_stat) == 0) {
        printf("   符号链接大小: %ld 字节\n", link_stat.st_size);
        if (S_ISLNK(link_stat.st_mode))
            printf("   确认这是一个符号链接\n");
    }
    unlink("test_link.tmp");
    
    // ==================== 33. sysconf() - 获取系统配置 ====================
    printf("\n【33. sysconf() - 获取系统配置】\n");
    long page_size = sysconf(_SC_PAGESIZE);
    long open_max = sysconf(_SC_OPEN_MAX);
    printf("   系统页大小: %ld 字节\n", page_size);
    printf("   最大打开文件数: %ld\n", open_max);
    
    // ==================== 34. pathconf() - 获取文件配置 ====================
    printf("\n【34. pathconf() - 获取文件配置】\n");
    long name_max = pathconf("test_renamed.txt", _PC_NAME_MAX);
    long pipe_buf = pathconf("test_renamed.txt", _PC_PIPE_BUF);
    printf("   文件名最大长度: %ld\n", name_max);
    printf("   管道缓冲区大小: %ld\n", pipe_buf);
    
    // ==================== 35. 文件锁 (fcntl) ====================
    printf("\n【35. fcntl() 文件锁】\n");
    struct flock fl;
    fl.l_type = F_WRLCK;      // 写锁
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 10;             // 锁前10字节
    if (fcntl(fd1, F_SETLK, &fl) == 0) {
        printf("   对文件前10字节加写锁成功\n");
        // 解锁
        fl.l_type = F_UNLCK;
        fcntl(fd1, F_SETLK, &fl);
        printf("   解锁\n");
    } else {
        printf("   加锁失败: %s\n", strerror(errno));
    }
    
    // ==================== 36. 文件描述符偏移保存/恢复 ====================
    printf("\n【36. 文件描述符偏移操作】\n");
    off_t saved_offset = lseek(fd1, 0, SEEK_CUR);
    printf("   保存当前偏移: %ld\n", saved_offset);
    lseek(fd1, 0, SEEK_END);
    printf("   移动到末尾: %ld\n", lseek(fd1, 0, SEEK_CUR));
    lseek(fd1, saved_offset, SEEK_SET);
    printf("   恢复偏移: %ld\n", lseek(fd1, 0, SEEK_CUR));
    
    // ==================== 37. 预读建议 (readahead) ====================
    printf("\n【37. readahead() - 预读建议】\n");
    if (readahead(fd1, 0, 1024) == 0)
        printf("   建议内核预读 1024 字节\n");
    
    // ==================== 38. 同步整个文件系统 ====================
    printf("\n【38. sync() - 同步文件系统】\n");
    sync();  // 调度所有缓冲区写入磁盘
    printf("   调度所有文件系统缓冲区写入\n");
    
    // ==================== 清理工作 ====================
    printf("\n【清理工作】\n");
    close(fd1);
    printf("   关闭文件描述符 %d\n", fd1);
    
    // 删除测试文件
    unlink("test_renamed.txt");
    unlink("redirect_output.txt");
    printf("   删除测试文件\n");
    
    printf("\n========== 演示结束，共使用约 40 个系统调用 ==========\n\n");
    
    return 0;
}