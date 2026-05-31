#define _GNU_SOURCE         // Enable GNU extensions

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
#include <sys/wait.h>       // wait() function


int main(int argc, char *argv[])
{
    int fd1, fd2, fd3;
    ssize_t n;
    off_t offset;
    pid_t pid;
    
    printf("\n========== File I/O System Call Demo ==========\n\n");
    
    // ==================== 1. open() - Open file ====================
    printf("[1. open() - Open file]\n");
    fd1 = open("test1.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd1 == -1) {
        perror("   open failed");
        return 1;
    }
    printf("   Opened/created file test1.txt, fd = %d\n", fd1);
    
    // ==================== 2. write() - Write data ====================
    printf("\n[2. write() - Write data]\n");
    const char *data = "Hello from write() system call!\n";
    n = write(fd1, data, strlen(data));
    printf("   Wrote %ld bytes: %s", n, data);
    
    // ==================== 3. read() - Read data ====================
    printf("\n[3. read() - Read data]\n");
    lseek(fd1, 0, SEEK_SET);  // Reset to beginning
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    n = read(fd1, buffer, sizeof(buffer) - 1);
    printf("   Read %ld bytes: %s", n, buffer);
    
    // ==================== 4. lseek() - Move file pointer ====================
    printf("\n[4. lseek() - Move file pointer]\n");
    offset = lseek(fd1, 0, SEEK_CUR);
    printf("   Current position: %ld\n", offset);
    offset = lseek(fd1, 0, SEEK_END);
    printf("   File size: %ld\n", offset);
    lseek(fd1, 0, SEEK_SET);
    printf("   Reset to beginning\n");
    
    // ==================== 5. dup() - Duplicate fd ====================
    printf("\n[5. dup() - Duplicate file descriptor]\n");
    fd2 = dup(fd1);
    printf("   Duplicated fd %d -> fd %d\n", fd1, fd2);
    const char *dup_data = "Data from duplicated fd!\n";
    write(fd2, dup_data, strlen(dup_data));
    printf("   Wrote data through duplicated fd\n");
    
    // ==================== 6. dup2() - Duplicate to specific fd ====================
    printf("\n[6. dup2() - Duplicate to specific descriptor]\n");
    fd3 = dup2(fd1, 100);
    printf("   Duplicated fd %d to fd 100, returned %d\n", fd1, fd3);
    const char *dup2_data = "Data from dup2()!\n";
    write(100, dup2_data, strlen(dup2_data));
    printf("   Wrote data through fd 100\n");
    
    // ==================== 7. close() - Close fd ====================
    printf("\n[7. close() - Close file descriptor]\n");
    close(fd2);
    printf("   Closed fd %d\n", fd2);
    close(100);
    printf("   Closed fd 100\n");
    
    // ==================== 8. stat() / fstat() - File status ====================
    printf("\n[8. stat()/fstat() - Get file status]\n");
    struct stat file_stat;
    if (fstat(fd1, &file_stat) == 0) {
        printf("   File size: %ld bytes\n", file_stat.st_size);
        printf("   Permissions: %o\n", file_stat.st_mode & 0777);
        printf("   Link count: %ld\n", file_stat.st_nlink);
        printf("   UID: %d, GID: %d\n", file_stat.st_uid, file_stat.st_gid);
        printf("   Device: %ld\n", file_stat.st_dev);
        printf("   inode: %ld\n", file_stat.st_ino);
    }
    
    // ==================== 9. access() - Check permissions ====================
    printf("\n[9. access() - Check file permissions]\n");
    if (access("test1.txt", R_OK) == 0)
        printf("   File is readable\n");
    if (access("test1.txt", W_OK) == 0)
        printf("   File is writable\n");
    if (access("test1.txt", X_OK) == 0)
        printf("   File is executable\n");
    
    // ==================== 10. chmod() / fchmod() - Change permissions ====================
    printf("\n[10. chmod() - Change file permissions]\n");
    if (chmod("test1.txt", 0600) == 0)
        printf("   Changed permissions to 0600 (rw-------)\n");
    
    // ==================== 11. truncate() / ftruncate() - Truncate file ====================
    printf("\n[11. ftruncate() - Truncate file]\n");
    if (ftruncate(fd1, 50) == 0) {
        printf("   Truncated file to 50 bytes\n");
        struct stat new_stat;
        fstat(fd1, &new_stat);
        printf("   New file size: %ld bytes\n", new_stat.st_size);
    }
    
    // ==================== 12. link() - Create hard link ====================
    printf("\n[12. link() - Create hard link]\n");
    if (link("test1.txt", "test1_hardlink.txt") == 0) {
        printf("   Created hard link test1_hardlink.txt\n");
        struct stat link_stat;
        stat("test1.txt", &link_stat);
        printf("   Link count increased to: %ld\n", link_stat.st_nlink);
    }
    
    // ==================== 13. symlink() - Create symbolic link ====================
    printf("\n[13. symlink() - Create symbolic link]\n");
    if (symlink("test1.txt", "test1_symlink.txt") == 0)
        printf("   Created symlink test1_symlink.txt -> test1.txt\n");
    
    // ==================== 14. readlink() - Read symbolic link ====================
    printf("\n[14. readlink() - Read symbolic link]\n");
    char link_target[256];
    n = readlink("test1_symlink.txt", link_target, sizeof(link_target) - 1);
    if (n != -1) {
        link_target[n] = '\0';
        printf("   Symlink points to: %s\n", link_target);
    }
    
    // ==================== 15. unlink() - Delete file/link ====================
    printf("\n[15. unlink() - Delete file/link]\n");
    unlink("test1_hardlink.txt");
    printf("   Deleted hard link\n");
    unlink("test1_symlink.txt");
    printf("   Deleted symbolic link\n");
    
    // ==================== 16. rename() - Rename file ====================
    printf("\n[16. rename() - Rename file]\n");
    if (rename("test1.txt", "test_renamed.txt") == 0)
        printf("   Renamed test1.txt -> test_renamed.txt\n");
    
    // ==================== 17. mkdir() - Create directory ====================
    printf("\n[17. mkdir() - Create directory]\n");
    if (mkdir("test_dir", 0755) == 0)
        printf("   Created directory test_dir\n");
    
    // ==================== 18. rmdir() - Remove directory ====================
    printf("\n[18. rmdir() - Remove directory]\n");
    if (rmdir("test_dir") == 0)
        printf("   Removed directory test_dir\n");
    
    // ==================== 19. opendir()/readdir()/closedir() - List directory ====================
    printf("\n[19. opendir()/readdir()/closedir() - List directory]\n");
    mkdir("test_dir", 0755);
    DIR *dir = opendir(".");
    if (dir) {
        struct dirent *entry;
        int count = 0;
        printf("   Current directory contents:\n");
        while ((entry = readdir(dir)) != NULL && count < 10) {
            printf("     - %s (type: %d)\n", entry->d_name, entry->d_type);
            count++;
        }
        closedir(dir);
        printf("   Closed directory stream\n");
    }
    rmdir("test_dir");
    
    // ==================== 20. chdir()/getcwd() - Change/show work directory ====================
    printf("\n[20. chdir()/getcwd() - Change/show working directory]\n");
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("   Current dir: %s\n", cwd);
    }
    mkdir("temp_dir", 0755);
    if (chdir("temp_dir") == 0) {
        char new_cwd[1024];
        getcwd(new_cwd, sizeof(new_cwd));
        printf("   Switched to: %s\n", new_cwd);
        chdir("..");
    }
    rmdir("temp_dir");
    
    // ==================== 21. utime() - Change file timestamps ====================
    printf("\n[21. utime() - Change file timestamps]\n");
    struct utimbuf times;
    times.actime = 1000000000;  // Access time
    times.modtime = 1000000000;  // Modification time
    if (utime("test_renamed.txt", &times) == 0)
        printf("   Changed file timestamps\n");
    
    // ==================== 22. fsync() - Sync file to disk ====================
    printf("\n[22. fsync() - Sync to disk]\n");
    if (fsync(fd1) == 0)
        printf("   Synced file data to disk\n");
    
    // ==================== 23. fdatasync() - Sync data (no metadata) ====================
    printf("\n[23. fdatasync() - Sync data]\n");
    if (fdatasync(fd1) == 0)
        printf("   Synced file data (no metadata)\n");
    
    // ==================== 24. fork() - Create child process ====================
    printf("\n[24. fork() - Create child process]\n");
    pid = fork();
    if (pid == 0) {
        // Child process
        printf("   Child (PID: %d) writing data\n", getpid());
        const char *child_data = "Data from child process\n";
        write(fd1, child_data, strlen(child_data));
        exit(0);
    } else if (pid > 0) {
        // Parent process
        wait(NULL);  // Wait for child
        printf("   Parent waited for child to finish\n");
    }
    
    // ==================== 25. getpid()/getppid() - Get process IDs ====================
    printf("\n[25. getpid()/getppid() - Get process IDs]\n");
    printf("   Current PID: %d\n", getpid());
    printf("   Parent PPID: %d\n", getppid());
    
    // ==================== 26. sleep() - Sleep ====================
    printf("\n[26. sleep() - Process sleep]\n");
    printf("   Sleeping 1 second...\n");
    sleep(1);
    printf("   Awake, continuing\n");
    
    // ==================== 27. pipe() - Create pipe ====================
    printf("\n[27. pipe() - Create pipe]\n");
    int pipefd[2];
    if (pipe(pipefd) == 0) {
        printf("   Created pipe: read %d, write %d\n", pipefd[0], pipefd[1]);
        const char *pipe_data = "Data through pipe";
        write(pipefd[1], pipe_data, strlen(pipe_data) + 1);
        char pipe_buf[100];
        read(pipefd[0], pipe_buf, sizeof(pipe_buf));
        printf("   Through pipe: %s\n", pipe_buf);
        close(pipefd[0]);
        close(pipefd[1]);
    }
    
    // ==================== 28. select() - I/O multiplexing ====================
    printf("\n[28. select() - I/O multiplexing]\n");
    fd_set read_fds;
    struct timeval tv;
    FD_ZERO(&read_fds);
    FD_SET(fd1, &read_fds);
    tv.tv_sec = 0;
    tv.tv_usec = 100000;  // 100ms
    int ret = select(fd1 + 1, &read_fds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(fd1, &read_fds))
        printf("   fd %d is readable\n", fd1);
    else if (ret == 0)
        printf("   Timeout, no data readable\n");
    
    // ==================== 29. ioctl() - Device control ====================
    printf("\n[29. ioctl() - Device control]\n");
    unsigned long bytes_available;
    if (ioctl(fd1, FIONREAD, &bytes_available) == 0)
        printf("   Readable bytes: %ld\n", bytes_available);
    else
        printf("   ioctl not supported for this file type\n");
    
    // ==================== 30. Redirect stdout with dup2 ====================
    printf("\n[30. Redirect stdout]\n");
    int stdout_save = dup(STDOUT_FILENO);
    int new_fd = open("redirect_output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(new_fd, STDOUT_FILENO);
    printf("This line goes to redirect_output.txt\n");
    fflush(stdout);
    dup2(stdout_save, STDOUT_FILENO);
    printf("   Restored stdout, line above was redirected to file\n");
    close(stdout_save);
    close(new_fd);
    
    // ==================== 31. fcntl() - File control ====================
    printf("\n[31. fcntl() - File control]\n");
    int flags = fcntl(fd1, F_GETFL);
    printf("   Current file status flags: 0x%x\n", flags);
    printf("   Access mode: %s\n", (flags & O_ACCMODE) == O_RDWR ? "read-write" : 
                                (flags & O_ACCMODE) == O_RDONLY ? "read-only" : "write-only");
    fcntl(fd1, F_SETFL, flags | O_NONBLOCK);
    printf("   Set non-blocking mode\n");
    fcntl(fd1, F_SETFL, flags);
    
    // ==================== 32. lstat() - Get link status ====================
    printf("\n[32. lstat() - Get link status]\n");
    struct stat link_stat;
    symlink("test_renamed.txt", "test_link.tmp");
    if (lstat("test_link.tmp", &link_stat) == 0) {
        printf("   Symlink size: %ld bytes\n", link_stat.st_size);
        if (S_ISLNK(link_stat.st_mode))
            printf("   Confirmed it is a symbolic link\n");
    }
    unlink("test_link.tmp");
    
    // ==================== 33. sysconf() - Get system config ====================
    printf("\n[33. sysconf() - Get system configuration]\n");
    long page_size = sysconf(_SC_PAGESIZE);
    long open_max = sysconf(_SC_OPEN_MAX);
    printf("   Page size: %ld bytes\n", page_size);
    printf("   Max open files: %ld\n", open_max);
    
    // ==================== 34. pathconf() - Get path config ====================
    printf("\n[34. pathconf() - Get path configuration]\n");
    long name_max = pathconf("test_renamed.txt", _PC_NAME_MAX);
    long pipe_buf = pathconf("test_renamed.txt", _PC_PIPE_BUF);
    printf("   Max filename length: %ld\n", name_max);
    printf("   Pipe buffer size: %ld\n", pipe_buf);
    
    // ==================== 35. File locking (fcntl) ====================
    printf("\n[35. fcntl() file locking]\n");
    struct flock fl;
    fl.l_type = F_WRLCK;      // Write lock
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 10;             // Lock first 10 bytes
    if (fcntl(fd1, F_SETLK, &fl) == 0) {
        printf("   Acquired write lock on first 10 bytes\n");
        fl.l_type = F_UNLCK;
        fcntl(fd1, F_SETLK, &fl);
        printf("   Unlocked\n");
    } else {
        printf("   Lock failed: %s\n", strerror(errno));
    }
    
    // ==================== 36. File offset save/restore ====================
    printf("\n[36. File descriptor offset operations]\n");
    off_t saved_offset = lseek(fd1, 0, SEEK_CUR);
    printf("   Saved current offset: %ld\n", saved_offset);
    lseek(fd1, 0, SEEK_END);
    printf("   Moved to end: %ld\n", lseek(fd1, 0, SEEK_CUR));
    lseek(fd1, saved_offset, SEEK_SET);
    printf("   Restored offset: %ld\n", lseek(fd1, 0, SEEK_CUR));
    
    // ==================== 37. Readahead hint ====================
    printf("\n[37. readahead() - Pre-read hint]\n");
    if (readahead(fd1, 0, 1024) == 0)
        printf("   Advised kernel to pre-read 1024 bytes\n");
    
    // ==================== 38. Sync entire filesystem ====================
    printf("\n[38. sync() - Sync filesystem]\n");
    sync();
    printf("   Scheduled all filesystem buffers for write\n");
    
    // ==================== Cleanup ====================
    printf("\n[Cleanup]\n");
    close(fd1);
    printf("   Closed fd %d\n", fd1);
    
    unlink("test_renamed.txt");
    unlink("redirect_output.txt");
    printf("   Deleted test files\n");
    
    printf("\n========== Demo complete, ~40 system calls used ==========\n\n");
    
    return 0;
}