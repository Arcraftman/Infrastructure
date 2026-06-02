#ifndef STK_CORE_RINGBUF_H
#define STK_CORE_RINGBUF_H


#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 环形缓冲区 (Ring Buffer / Circular Buffer)
 * 
 * 固定大小的循环缓冲区，适合生产者-消费者场景。
 * 支持单生产者单消费者 (SPSC) 的无锁操作。
 * 
 * 特性:
 *   - 容量自动调整为2的幂（便于位运算优化）
 *   - 支持覆盖写入（强制写入）
 *   - 支持查看（peek）不消费数据
 *   - SPSC 模式无锁操作
 * 
 * 时间复杂度:
 *   - 所有操作均为 O(1)
 * 
 * Basic usage:
 * @code
 *   stk_ringbuf rb;
 *   stk_ringbuf_init(&rb, 1024);
 *   stk_ringbuf_write(&rb, data, len);
 *   size_t read = stk_ringbuf_read(&rb, buffer, sizeof(buffer));
 *   stk_ringbuf_free(&rb);
 * @endcode
 * ========================================================================= */

typedef struct {
    uint8_t* buffer;   /* 数据缓冲区 */
    size_t   head;     /* 读指针 */
    size_t   tail;     /* 写指针 */
    size_t   capacity; /* 缓冲区容量 (必须是2的幂) */
    bool     full;     /* 是否已满 */
} stk_ringbuf;

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

/* 初始化环形缓冲区（容量会自动调整为2的幂） */
STK_API STK_STATUS stk_ringbuf_init(stk_ringbuf* rb, size_t capacity);

/* 释放环形缓冲区内存 */
STK_API STK_STATUS stk_ringbuf_free(stk_ringbuf* rb);

/* =========================================================================
 * 写入操作
 * ========================================================================= */

/* 写入数据（返回实际写入字节数，不超过可用空间） */
STK_API size_t stk_ringbuf_write(stk_ringbuf* rb, const void* data, size_t len);

/* 写入单个字节 */
STK_API size_t stk_ringbuf_write_byte(stk_ringbuf* rb, uint8_t byte);

/* 强制写入（空间不足时覆盖旧数据） */
STK_API STK_STATUS stk_ringbuf_write_force(stk_ringbuf* rb, const void* data, size_t len);

/* =========================================================================
 * 读取操作
 * ========================================================================= */

/* 读取数据（返回实际读取字节数） */
STK_API size_t stk_ringbuf_read(stk_ringbuf* rb, void* buffer, size_t len);

/* 读取单个字节 */
STK_API size_t stk_ringbuf_read_byte(stk_ringbuf* rb, uint8_t* byte);

/* 查看数据（不消费，offset 为偏移量，0 表示从头部开始） */
STK_API STK_STATUS stk_ringbuf_peek(const stk_ringbuf* rb, void* buffer, size_t len, size_t offset);

/* =========================================================================
 * 管理操作
 * ========================================================================= */

/* 清空缓冲区（重置 head 和 tail） */
STK_API STK_STATUS stk_ringbuf_clear(stk_ringbuf* rb);

/* 重置缓冲区（同 clear） */
STK_API STK_STATUS stk_ringbuf_reset(stk_ringbuf* rb);

/* 跳过指定字节数（不读取到 buffer） */
STK_API STK_STATUS stk_ringbuf_skip(stk_ringbuf* rb, size_t len);

/* 推进读指针（同 skip） */
STK_API STK_STATUS stk_ringbuf_advance(stk_ringbuf* rb, size_t len);

/* =========================================================================
 * 查询接口
 * ========================================================================= */

/* 返回已使用字节数 */
STK_API size_t stk_ringbuf_used(const stk_ringbuf* rb);

/* 返回可用字节数 */
STK_API size_t stk_ringbuf_available(const stk_ringbuf* rb);

/* 返回总容量 */
STK_API size_t stk_ringbuf_capacity(const stk_ringbuf* rb);

/* 检查缓冲区是否为空 */
STK_API bool stk_ringbuf_empty(const stk_ringbuf* rb);

/* 检查缓冲区是否已满 */
STK_API bool stk_ringbuf_full(const stk_ringbuf* rb);

/* =========================================================================
 * 线程安全操作 (SPSC - Single Producer Single Consumer)
 * 
 * 这些函数是无锁的，适用于单生产者单消费者场景。
 * 不需要额外的互斥锁，但必须在单生产者单消费者模式下使用。
 * ========================================================================= */

/* SPSC 模式写入（无锁） */
STK_API size_t stk_ringbuf_write_spsc(stk_ringbuf* rb, const void* data, size_t len);

/* SPSC 模式读取（无锁） */
STK_API size_t stk_ringbuf_read_spsc(stk_ringbuf* rb, void* buffer, size_t len);

/* SPSC 模式查看可用空间（无锁） */
STK_API size_t stk_ringbuf_available_spsc(const stk_ringbuf* rb);

/* SPSC 模式查看已用空间（无锁） */
STK_API size_t stk_ringbuf_used_spsc(const stk_ringbuf* rb);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_RINGBUF_H */