/**
 * monitor.c - 系统监控内核模块实现
 * 
 * 实现系统性能监控、进程管理和日志记录功能
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched/stat.h>
#include <linux/pid.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/timekeeping.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/version.h>
#include <linux/cred.h>
#include <linux/fdtable.h>
#include <linux/fs_struct.h>

#include "monitor.h"

/* ============================================================================
 * 全局变量
 * ============================================================================
 */
static struct monitor_device *g_monitor = NULL;

/* ============================================================================
 * 辅助函数
 * ============================================================================
 */

/**
 * 获取当前时间戳（秒）
 */
static inline unsigned long get_timestamp_sec(void)
{
    struct timespec64 ts;
    ktime_get_real_ts64(&ts);
    return (unsigned long)ts.tv_sec;
}

/**
 * 检查进程是否存在且可访问
 */
static int is_process_accessible(struct task_struct *task, int pid)
{
    const struct cred *cred;
    kuid_t uid;
    
    if (!task)
        return 0;
    
    /* root用户可以访问所有进程 */
    if (capable(CAP_SYS_ADMIN))
        return 1;
    
    /* 普通用户只能访问自己的进程 */
    cred = get_current_cred();
    uid = cred->uid;
    put_cred(cred);
    
    return uid_eq(uid, task->cred->uid);
}

/* ============================================================================
 * 日志系统实现
 * ============================================================================
 */

/**
 * 记录日志消息
 */
void monitor_log(struct monitor_device *dev, int level, const char *fmt, ...)
{
    va_list args;
    unsigned long flags;
    int head;
    struct log_entry *entry;
    
    if (!dev || !dev->is_running)
        return;
    
    if (level > dev->log_level)
        return;
    
    spin_lock_irqsave(&dev->log_lock, flags);
    
    head = dev->log_head;
    entry = &dev->log_buffer[head];
    
    entry->timestamp = get_timestamp_sec();
    entry->level = level;
    
    va_start(args, fmt);
    vsnprintf(entry->message, MAX_MSG_SIZE - 1, fmt, args);
    va_end(args);
    entry->message[MAX_MSG_SIZE - 1] = '\0';
    
    dev->log_head = (head + 1) % MAX_LOG_ENTRIES;
    
    if (dev->log_count < MAX_LOG_ENTRIES) {
        dev->log_count++;
    } else {
        dev->log_tail = (dev->log_tail + 1) % MAX_LOG_ENTRIES;
    }
    
    spin_unlock_irqrestore(&dev->log_lock, flags);
    
    /* 同时输出到内核日志 */
    switch (level) {
    case LOG_LEVEL_ERROR:
        pr_err("%s: %s\n", MONITOR_NAME, entry->message);
        break;
    case LOG_LEVEL_WARN:
        pr_warn("%s: %s\n", MONITOR_NAME, entry->message);
        break;
    case LOG_LEVEL_INFO:
        pr_info("%s: %s\n", MONITOR_NAME, entry->message);
        break;
    default:
        pr_debug("%s: %s\n", MONITOR_NAME, entry->message);
        break;
    }
}
EXPORT_SYMBOL(monitor_log);

/**
 * 获取日志条目
 */
int monitor_get_log(struct monitor_device *dev, struct log_entry *entry)
{
    unsigned long flags;
    int ret = -EAGAIN;
    int tail;
    
    if (!dev || !entry)
        return -EINVAL;
    
    spin_lock_irqsave(&dev->log_lock, flags);
    
    if (dev->log_count > 0) {
        tail = dev->log_tail;
        memcpy(entry, &dev->log_buffer[tail], sizeof(struct log_entry));
        dev->log_tail = (tail + 1) % MAX_LOG_ENTRIES;
        dev->log_count--;
        ret = 0;
    }
    
    spin_unlock_irqrestore(&dev->log_lock, flags);
    
    return ret;
}
EXPORT_SYMBOL(monitor_get_log);

/**
 * 清空日志缓冲区
 */
void monitor_clear_log(struct monitor_device *dev)
{
    unsigned long flags;
    
    if (!dev)
        return;
    
    spin_lock_irqsave(&dev->log_lock, flags);
    
    dev->log_head = 0;
    dev->log_tail = 0;
    dev->log_count = 0;
    
    spin_unlock_irqrestore(&dev->log_lock, flags);
    
    monitor_info(dev, "Log buffer cleared");
}
EXPORT_SYMBOL(monitor_clear_log);

/* ============================================================================
 * 系统信息获取实现
 * ============================================================================
 */

/**
 * 获取CPU信息
 */
int monitor_get_cpu_info(struct cpu_info *info)
{
    int cpu;
    struct kernel_cpustat kcpustat;
    
    if (!info)
        return -EINVAL;
    
    memset(info, 0, sizeof(struct cpu_info));
    
    /* 获取CPU核心数 */
    info->cpu_count = num_online_cpus();
    if (info->cpu_count > MAX_CPU_CORES)
        info->cpu_count = MAX_CPU_CORES;
    
    /* 获取总体CPU时间 */
    for_each_online_cpu(cpu) {
        kcpustat = kcpustat_cpu(cpu);
        info->user += kcpustat.cpustat[CPUTIME_USER];
        info->nice += kcpustat.cpustat[CPUTIME_NICE];
        info->system += kcpustat.cpustat[CPUTIME_SYSTEM];
        info->idle += kcpustat.cpustat[CPUTIME_IDLE];
        info->iowait += kcpustat.cpustat[CPUTIME_IOWAIT];
        info->irq += kcpustat.cpustat[CPUTIME_IRQ];
        info->softirq += kcpustat.cpustat[CPUTIME_SOFTIRQ];
        info->steal += kcpustat.cpustat[CPUTIME_STEAL];
        info->guest += kcpustat.cpustat[CPUTIME_GUEST];
    }
    
    info->total = info->user + info->nice + info->system + info->idle +
                  info->iowait + info->irq + info->softirq + info->steal;
    
    /* 计算使用率百分比 */
    if (info->total > 0) {
        unsigned long idle_total = info->idle + info->iowait;
        info->usage_percent = (info->total - idle_total) * 100 / info->total;
    }
    
    return 0;
}
EXPORT_SYMBOL(monitor_get_cpu_info);

/**
 * 获取内存信息
 */
int monitor_get_memory_info(struct mem_info *info)
{
    struct sysinfo si;
    
    if (!info)
        return -EINVAL;
    
    memset(info, 0, sizeof(struct mem_info));
    
    si_meminfo(&si);
    si_swapinfo(&si);
    
    /* 系统内存信息 */
    info->total_kb = si.totalram << (PAGE_SHIFT - 10);
    info->free_kb = si.freeram << (PAGE_SHIFT - 10);
    info->swap_total_kb = si.totalswap << (PAGE_SHIFT - 10);
    info->swap_free_kb = si.freeswap << (PAGE_SHIFT - 10);
    
    /* 获取更详细的内存信息 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    info->available_kb = si_availram(&si) << (PAGE_SHIFT - 10);
#else
    info->available_kb = info->free_kb;
#endif
    
    /* 计算使用率 */
    if (info->total_kb > 0) {
        unsigned long used_kb = info->total_kb - info->free_kb;
        info->usage_percent = (used_kb * 100) / info->total_kb;
    }
    
    if (info->swap_total_kb > 0) {
        unsigned long swap_used_kb = info->swap_total_kb - info->swap_free_kb;
        info->swap_usage_percent = (swap_used_kb * 100) / info->swap_total_kb;
    }
    
    return 0;
}
EXPORT_SYMBOL(monitor_get_memory_info);

/**
 * 获取系统负载
 */
int monitor_get_system_load(struct sys_load *load)
{
    struct task_struct *p;
    unsigned long running = 0, total = 0;
    
    if (!load)
        return -EINVAL;
    
    memset(load, 0, sizeof(struct sys_load));
    
    /* 获取平均负载 */
    load->load_avg_1 = avenrun[0] * 100 / FIXED_1;
    load->load_avg_5 = avenrun[1] * 100 / FIXED_1;
    load->load_avg_15 = avenrun[2] * 100 / FIXED_1;
    
    /* 统计进程数量 */
    rcu_read_lock();
    for_each_process(p) {
        total++;
        if (task_is_running(p))
            running++;
    }
    rcu_read_unlock();
    
    load->running_tasks = running;
    load->total_tasks = total;
    load->latest_pid = task_pid_nr(current);
    
    return 0;
}
EXPORT_SYMBOL(monitor_get_system_load);

/* ============================================================================
 * 进程管理实现
 * ============================================================================
 */

/**
 * 获取进程列表
 */
int monitor_get_process_list(struct proc_list *list)
{
    struct task_struct *p;
    int count = 0;
    
    if (!list)
        return -EINVAL;
    
    memset(list, 0, sizeof(struct proc_list));
    
    rcu_read_lock();
    for_each_process(p) {
        if (count >= MAX_PID_LIST - 1)
            break;
        list->pids[count++] = task_pid_nr(p);
    }
    rcu_read_unlock();
    
    list->count = count;
    
    return 0;
}
EXPORT_SYMBOL(monitor_get_process_list);

/**
 * 获取进程详细信息
 */
int monitor_get_process_info(int pid, struct proc_info *info)
{
    struct task_struct *task;
    struct pid *pid_struct;
    struct mm_struct *mm;
    int ret = 0;
    
    if (!info || pid <= 0)
        return -EINVAL;
    
    memset(info, 0, sizeof(struct proc_info));
    
    rcu_read_lock();
    
    /* 查找进程 */
    pid_struct = find_get_pid(pid);
    if (!pid_struct) {
        ret = -ESRCH;
        goto out;
    }
    
    task = get_pid_task(pid_struct, PIDTYPE_PID);
    if (!task) {
        ret = -ESRCH;
        goto out_put;
    }
    
    /* 检查权限 */
    if (!is_process_accessible(task, pid)) {
        ret = -EACCES;
        goto out_task;
    }
    
    /* 基本信息 */
    info->pid = pid;
    info->ppid = task_tgid_nr(task->real_parent);
    info->pgid = task_pgrp_nr(task);
    info->sid = task_session_nr(task);
    
    /* 用户信息 */
    info->uid = from_kuid(&init_user_ns, task_uid(task));
    info->euid = from_kuid(&init_user_ns, task_euid(task));
    info->gid = from_kgid(&init_user_ns, task_gid(task));
    info->egid = from_kgid(&init_user_ns, task_egid(task));
    
    /* 进程名称 */
    get_task_comm(info->name, task);
    info->state = task_state_index(task);
    
    /* 优先级信息 */
    info->nice = task_nice(task);
    info->priority = task_prio(task);
    info->rt_priority = task->rt_priority;
    
    /* 内存信息 */
    mm = get_task_mm(task);
    if (mm) {
        info->vm_size_kb = mm->total_vm << (PAGE_SHIFT - 10);
        info->vm_rss_kb = get_mm_rss(mm) << (PAGE_SHIFT - 10);
        mmput(mm);
    }
    
    /* CPU时间 */
    info->cpu_time_user = task->utime;
    info->cpu_time_sys = task->stime;
    
    /* 运行时间 */
    info->uptime_sec = jiffies_to_msecs(jiffies - task->start_time) / 1000;
    info->start_time = task->start_time;
    
    /* 上下文切换 */
    info->voluntary_ctxt_switches = task->nvcsw;
    info->nonvoluntary_ctxt_switches = task->nivcsw;
    
    /* 线程数 */
    info->threads = atomic_read(&task->signal->live);
    
out_task:
    put_task_struct(task);
out_put:
    put_pid(pid_struct);
out:
    rcu_read_unlock();
    
    return ret;
}
EXPORT_SYMBOL(monitor_get_process_info);

/**
 * 获取进程统计信息
 */
int monitor_get_process_stats(struct proc_stats *stats)
{
    struct task_struct *p;
    
    if (!stats)
        return -EINVAL;
    
    memset(stats, 0, sizeof(struct proc_stats));
    
    rcu_read_lock();
    for_each_process(p) {
        stats->total_processes++;
        
        switch (p->__state) {
        case TASK_RUNNING:
            stats->running_processes++;
            break;
        case TASK_INTERRUPTIBLE:
        case TASK_UNINTERRUPTIBLE:
            stats->sleeping_processes++;
            break;
        case TASK_STOPPED:
            stats->stopped_processes++;
            break;
        case TASK_DEAD:
        case TASK_TRACED:
            break;
        }
        
        if (task_is_zombie(p))
            stats->zombie_processes++;
        
        if (atomic_read(&p->signal->live) > 1)
            stats->threaded_processes++;
    }
    rcu_read_unlock();
    
    return 0;
}
EXPORT_SYMBOL(monitor_get_process_stats);

/**
 * 终止进程
 */
int monitor_kill_process(int pid)
{
    struct pid *pid_struct;
    struct task_struct *task;
    int ret = 0;
    
    if (pid <= 0)
        return -EINVAL;
    
    rcu_read_lock();
    
    pid_struct = find_get_pid(pid);
    if (!pid_struct) {
        ret = -ESRCH;
        goto out;
    }
    
    task = get_pid_task(pid_struct, PIDTYPE_PID);
    if (!task) {
        ret = -ESRCH;
        goto out_put;
    }
    
    if (!is_process_accessible(task, pid)) {
        ret = -EACCES;
        goto out_task;
    }
    
    /* 发送SIGTERM信号 */
    ret = send_sig_info(SIGTERM, SEND_SIG_PRIV, task);
    
out_task:
    put_task_struct(task);
out_put:
    put_pid(pid_struct);
out:
    rcu_read_unlock();
    
    return ret;
}
EXPORT_SYMBOL(monitor_kill_process);

/* ============================================================================
 * 设备文件操作实现
 * ============================================================================
 */

/**
 * 打开设备
 */
static int monitor_open(struct inode *inode, struct file *filp)
{
    struct monitor_device *dev;
    
    dev = container_of(inode->i_cdev, struct monitor_device, cdev);
    filp->private_data = dev;
    
    mutex_lock(&dev->lock);
    atomic_inc(&dev->open_count);
    mutex_unlock(&dev->lock);
    
    monitor_info(dev, "Device opened (opens: %d)", atomic_read(&dev->open_count));
    
    return 0;
}

/**
 * 释放设备
 */
static int monitor_release(struct inode *inode, struct file *filp)
{
    struct monitor_device *dev = filp->private_data;
    
    mutex_lock(&dev->lock);
    atomic_dec(&dev->open_count);
    mutex_unlock(&dev->lock);
    
    monitor_info(dev, "Device closed (remaining: %d)", atomic_read(&dev->open_count));
    
    return 0;
}

/**
 * 读取设备数据
 */
static ssize_t monitor_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    struct monitor_device *dev = filp->private_data;
    char msg[256];
    size_t len;
    
    atomic_inc(&dev->read_count);
    
    /* 读取日志或返回统计信息 */
    snprintf(msg, sizeof(msg), "System Monitor v%s\n", MONITOR_VERSION);
    len = strlen(msg);
    
    if (*offset >= len)
        return 0;
    
    if (count > len - *offset)
        count = len - *offset;
    
    if (copy_to_user(buf, msg + *offset, count)) {
        atomic_inc(&dev->error_count);
        return -EFAULT;
    }
    
    *offset += count;
    
    return count;
}

/**
 * 写入设备数据
 */
static ssize_t monitor_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
    struct monitor_device *dev = filp->private_data;
    char cmd[64];
    
    if (count >= sizeof(cmd))
        count = sizeof(cmd) - 1;
    
    if (copy_from_user(cmd, buf, count)) {
        atomic_inc(&dev->error_count);
        return -EFAULT;
    }
    
    cmd[count] = '\0';
    
    atomic_inc(&dev->write_count);
    monitor_debug(dev, "Received command: %s", cmd);
    
    return count;
}

/**
 * IOCTL 控制
 */
static long monitor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct monitor_device *dev = filp->private_data;
    void __user *argp = (void __user *)arg;
    int ret = 0;
    int level, pid;
    struct log_entry log_entry;
    struct monitor_stats stats;
    struct cpu_info cpu_info;
    struct mem_info mem_info;
    struct sys_load sys_load;
    struct proc_list proc_list;
    struct proc_info proc_info;
    struct proc_stats proc_stats;
    
    atomic_inc(&dev->ioctl_count);
    
    if (_IOC_TYPE(cmd) != MONITOR_MAGIC)
        return -ENOTTY;
    
    switch (cmd) {
    /* 系统信息命令 */
    case MONITOR_GET_CPU_INFO:
        ret = monitor_get_cpu_info(&cpu_info);
        if (ret == 0 && copy_to_user(argp, &cpu_info, sizeof(cpu_info)))
            ret = -EFAULT;
        break;
        
    case MONITOR_GET_MEM_INFO:
        ret = monitor_get_memory_info(&mem_info);
        if (ret == 0 && copy_to_user(argp, &mem_info, sizeof(mem_info)))
            ret = -EFAULT;
        break;
        
    case MONITOR_GET_SYS_LOAD:
        ret = monitor_get_system_load(&sys_load);
        if (ret == 0 && copy_to_user(argp, &sys_load, sizeof(sys_load)))
            ret = -EFAULT;
        break;
        
    /* 进程管理命令 */
    case MONITOR_GET_PROC_LIST:
        ret = monitor_get_process_list(&proc_list);
        if (ret == 0 && copy_to_user(argp, &proc_list, sizeof(proc_list)))
            ret = -EFAULT;
        break;
        
    case MONITOR_GET_PROC_INFO:
        if (copy_from_user(&proc_info, argp, sizeof(proc_info))) {
            ret = -EFAULT;
            break;
        }
        ret = monitor_get_process_info(proc_info.pid, &proc_info);
        if (ret == 0 && copy_to_user(argp, &proc_info, sizeof(proc_info)))
            ret = -EFAULT;
        break;
        
    case MONITOR_GET_PROC_STATS:
        ret = monitor_get_process_stats(&proc_stats);
        if (ret == 0 && copy_to_user(argp, &proc_stats, sizeof(proc_stats)))
            ret = -EFAULT;
        break;
        
    case MONITOR_KILL_PROC:
        if (get_user(pid, (int __user *)argp)) {
            ret = -EFAULT;
            break;
        }
        ret = monitor_kill_process(pid);
        break;
        
    /* 日志控制命令 */
    case MONITOR_SET_LOG_LEVEL:
        if (get_user(level, (int __user *)argp)) {
            ret = -EFAULT;
            break;
        }
        if (level < LOG_LEVEL_NONE || level > LOG_LEVEL_TRACE) {
            ret = -EINVAL;
            break;
        }
        dev->log_level = level;
        monitor_info(dev, "Log level changed to %d", level);
        break;
        
    case MONITOR_GET_LOG_LEVEL:
        if (put_user(dev->log_level, (int __user *)argp))
            ret = -EFAULT;
        break;
        
    case MONITOR_GET_LOG:
        ret = monitor_get_log(dev, &log_entry);
        if (ret == 0 && copy_to_user(argp, &log_entry, sizeof(log_entry)))
            ret = -EFAULT;
        break;
        
    case MONITOR_CLEAR_LOG:
        monitor_clear_log(dev);
        break;
        
    /* 统计信息命令 */
    case MONITOR_GET_STATS:
        ret = monitor_get_stats(dev, &stats);
        if (ret == 0 && copy_to_user(argp, &stats, sizeof(stats)))
            ret = -EFAULT;
        break;
        
    default:
        ret = -ENOTTY;
        break;
    }
    
    return ret;
}

/* ============================================================================
 * 统计信息
 * ============================================================================
 */

/**
 * 获取模块统计信息
 */
int monitor_get_stats(struct monitor_device *dev, struct monitor_stats *stats)
{
    if (!dev || !stats)
        return -EINVAL;
    
    memset(stats, 0, sizeof(struct monitor_stats));
    
    stats->start_time = dev->start_time;
    stats->open_count = atomic_read(&dev->open_count);
    stats->read_count = atomic_read(&dev->read_count);
    stats->write_count = atomic_read(&dev->write_count);
    stats->ioctl_count = atomic_read(&dev->ioctl_count);
    stats->error_count = atomic_read(&dev->error_count);
    stats->log_count = dev->log_count;
    stats->log_level = dev->log_level;
    stats->is_active = dev->is_running;
    
    return 0;
}
EXPORT_SYMBOL(monitor_get_stats);

/* ============================================================================
 * 设备初始化和清理
 * ============================================================================
 */

/**
 * 文件操作结构体
 */
static const struct file_operations monitor_fops = {
    .owner          = THIS_MODULE,
    .open           = monitor_open,
    .release        = monitor_release,
    .read           = monitor_read,
    .write          = monitor_write,
    .unlocked_ioctl = monitor_ioctl,
};

/**
 * 初始化设备
 */
int monitor_device_init(struct monitor_device *dev)
{
    int ret;
    
    if (!dev)
        return -EINVAL;
    
    /* 动态分配设备号 */
    ret = alloc_chrdev_region(&dev->device_number, 0, 1, MONITOR_NAME);
    if (ret < 0) {
        pr_err("%s: Failed to allocate device number\n", MONITOR_NAME);
        return ret;
    }
    
    dev->major_number = MAJOR(dev->device_number);
    dev->minor_number = MINOR(dev->device_number);
    
    /* 初始化字符设备 */
    cdev_init(&dev->cdev, &monitor_fops);
    dev->cdev.owner = THIS_MODULE;
    
    ret = cdev_add(&dev->cdev, dev->device_number, 1);
    if (ret < 0) {
        pr_err("%s: Failed to add cdev\n", MONITOR_NAME);
        unregister_chrdev_region(dev->device_number, 1);
        return ret;
    }
    
    /* 创建设备类 */
    dev->class = class_create(THIS_MODULE, DEVICE_CLASS);
    if (IS_ERR(dev->class)) {
        ret = PTR_ERR(dev->class);
        pr_err("%s: Failed to create class\n", MONITOR_NAME);
        cdev_del(&dev->cdev);
        unregister_chrdev_region(dev->device_number, 1);
        return ret;
    }
    
    /* 创建设备文件 */
    dev->device = device_create(dev->class, NULL, dev->device_number, NULL, DEVICE_NAME);
    if (IS_ERR(dev->device)) {
        ret = PTR_ERR(dev->device);
        pr_err("%s: Failed to create device\n", MONITOR_NAME);
        class_destroy(dev->class);
        cdev_del(&dev->cdev);
        unregister_chrdev_region(dev->device_number, 1);
        return ret;
    }
    
    return 0;
}
EXPORT_SYMBOL(monitor_device_init);

/**
 * 清理设备
 */
void monitor_device_exit(struct monitor_device *dev)
{
    if (!dev)
        return;
    
    if (dev->device)
        device_destroy(dev->class, dev->device_number);
    
    if (dev->class)
        class_destroy(dev->class);
    
    cdev_del(&dev->cdev);
    unregister_chrdev_region(dev->device_number, 1);
}
EXPORT_SYMBOL(monitor_device_exit);

/* ============================================================================
 * 模块初始化和退出
 * ============================================================================
 */

/**
 * 模块初始化
 */
static int __init monitor_init(void)
{
    int ret;
    
    pr_info("========================================\n");
    pr_info("  System Monitor Module v%s\n", MONITOR_VERSION);
    pr_info("========================================\n");
    
    /* 分配设备结构 */
    g_monitor = kzalloc(sizeof(struct monitor_device), GFP_KERNEL);
    if (!g_monitor) {
        pr_err("%s: Failed to allocate device structure\n", MONITOR_NAME);
        return -ENOMEM;
    }
    
    /* 初始化锁 */
    mutex_init(&g_monitor->lock);
    spin_lock_init(&g_monitor->log_lock);
    
    /* 初始化原子变量 */
    atomic_set(&g_monitor->open_count, 0);
    atomic_set(&g_monitor->read_count, 0);
    atomic_set(&g_monitor->write_count, 0);
    atomic_set(&g_monitor->ioctl_count, 0);
    atomic_set(&g_monitor->error_count, 0);
    
    /* 初始化日志系统 */
    g_monitor->log_head = 0;
    g_monitor->log_tail = 0;
    g_monitor->log_count = 0;
    g_monitor->log_level = LOG_LEVEL_INFO;
    
    /* 设置运行状态 */
    g_monitor->start_time = get_timestamp_sec();
    g_monitor->is_running = true;
    
    /* 初始化设备 */
    ret = monitor_device_init(g_monitor);
    if (ret < 0) {
        pr_err("%s: Failed to initialize device\n", MONITOR_NAME);
        kfree(g_monitor);
        return ret;
    }
    
    pr_info("✓ Module loaded successfully\n");
    pr_info("✓ Device: /dev/%s\n", DEVICE_NAME);
    pr_info("✓ Major: %d, Minor: %d\n", g_monitor->major_number, g_monitor->minor_number);
    pr_info("========================================\n");
    
    return 0;
}

/**
 * 模块退出
 */
static void __exit monitor_exit(void)
{
    if (!g_monitor)
        return;
    
    pr_info("========================================\n");
    pr_info("  Unloading System Monitor Module\n");
    pr_info("========================================\n");
    
    g_monitor->is_running = false;
    
    monitor_device_exit(g_monitor);
    kfree(g_monitor);
    g_monitor = NULL;
    
    pr_info("✓ Module unloaded successfully\n");
    pr_info("========================================\n");
}

module_init(monitor_init);
module_exit(monitor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("System Monitor Team");
MODULE_DESCRIPTION("System Monitor Kernel Module - Real-time system monitoring");
MODULE_VERSION(MONITOR_VERSION);
MODULE_ALIAS("sysmon");