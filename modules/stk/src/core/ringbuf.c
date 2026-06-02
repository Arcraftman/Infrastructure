#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/ringbuf.h"

/* =========================================================================
 * 内部辅助函数
 * ========================================================================= */

/* 检查是否是2的幂 */
static bool is_power_of_two(size_t n) {
    return n && ((n & (n - 1)) == 0);
}

/* 计算下一个2的幂 */
static size_t next_power_of_two(size_t n) {
    size_t p = 1;
    while (p < n) {
        p <<= 1;
    }
    return p;
}

/* 获取 mask（用于位运算取模） */
static inline size_t ringbuf_mask(const stk_ringbuf* rb) {
    return rb->capacity - 1;
}

/* 计算可用空间（通用版本） */
static size_t ringbuf_available_generic(const stk_ringbuf* rb) {
    if (rb->full) {
        return 0;
    }
    if (rb->tail >= rb->head) {
        return rb->capacity - (rb->tail - rb->head);
    } else {
        return rb->head - rb->tail;
    }
}

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

STK_STATUS stk_ringbuf_init(stk_ringbuf* rb, size_t capacity) {
    STK_RETURN_IF(!rb, STK_EINVAL, "Ringbuf init: NULL ringbuf pointer");
    STK_RETURN_IF(capacity == 0, STK_EINVAL, "Ringbuf init: capacity must be > 0");
    
    /* 容量调整为2的幂，便于使用位运算 */
    if (!is_power_of_two(capacity)) {
        capacity = next_power_of_two(capacity);
        STK_LOG_DEBUG("Ringbuf init: adjusted capacity to %zu (next power of two)", capacity);
    }
    
    rb->buffer = (uint8_t*)malloc(capacity);
    if (!rb->buffer) {
        STK_LOG_ERROR("Ringbuf init: malloc failed (size=%zu)", capacity);
        return STK_ENOMEM;
    }
    
    rb->head = 0;
    rb->tail = 0;
    rb->capacity = capacity;
    rb->full = false;
    
    STK_LOG_DEBUG("Ringbuf initialized: capacity=%zu", capacity);
    return STK_OK;
}

STK_STATUS stk_ringbuf_free(stk_ringbuf* rb) {
    if (!rb) {
        STK_LOG_WARN("Ringbuf free: NULL ringbuf pointer");
        return STK_EINVAL;
    }
    
    if (rb->buffer) {
        free(rb->buffer);
        rb->buffer = NULL;
    }
    rb->head = rb->tail = rb->capacity = 0;
    rb->full = false;
    
    STK_LOG_DEBUG("Ringbuf freed");
    return STK_OK;
}

/* =========================================================================
 * 写入操作
 * ========================================================================= */

size_t stk_ringbuf_write(stk_ringbuf* rb, const void* data, size_t len) {
    if (!rb || !data || len == 0) {
        if (rb) STK_LOG_WARN("Ringbuf write: invalid parameters");
        return 0;
    }
    
    size_t available = ringbuf_available_generic(rb);
    size_t write_len = len > available ? available : len;
    
    if (write_len == 0) {
        STK_LOG_DEBUG("Ringbuf write: no space available");
        return 0;
    }
    
    const uint8_t* src = (const uint8_t*)data;
    size_t mask = ringbuf_mask(rb);
    size_t tail = rb->tail;
    
    /* 计算第一段可写长度（从 tail 到缓冲区末尾） */
    size_t first_chunk = rb->capacity - tail;
    
    if (first_chunk >= write_len) {
        memcpy(rb->buffer + tail, src, write_len);
    } else {
        memcpy(rb->buffer + tail, src, first_chunk);
        memcpy(rb->buffer, src + first_chunk, write_len - first_chunk);
    }
    
    rb->tail = (tail + write_len) & mask;
    rb->full = (rb->tail == rb->head);
    
    STK_LOG_DEBUG("Ringbuf write: wrote %zu bytes, used=%zu, available=%zu",
                  write_len, stk_ringbuf_used(rb), stk_ringbuf_available(rb));
    return write_len;
}

size_t stk_ringbuf_write_byte(stk_ringbuf* rb, uint8_t byte) {
    return stk_ringbuf_write(rb, &byte, 1);
}

STK_STATUS stk_ringbuf_write_force(stk_ringbuf* rb, const void* data, size_t len) {
    STK_RETURN_IF(!rb, STK_EINVAL, "Ringbuf write_force: NULL ringbuf pointer");
    STK_RETURN_IF(!data && len > 0, STK_EINVAL, "Ringbuf write_force: NULL data with len>0");
    
    if (len == 0) return STK_OK;
    
    const uint8_t* src = (const uint8_t*)data;
    size_t mask = ringbuf_mask(rb);
    
    /* 如果数据长度超过容量，只保留最后 capacity 字节 */
    if (len > rb->capacity) {
        src = src + (len - rb->capacity);
        len = rb->capacity;
    }
    
    size_t available = ringbuf_available_generic(rb);
    
    /* 如果空间不足，需要覆盖旧数据 */
    if (len > available) {
        size_t overflow = len - available;
        rb->head = (rb->head + overflow) & mask;
        rb->full = (rb->head == rb->tail);
    }
    
    /* 写入数据 */
    size_t written = stk_ringbuf_write(rb, src, len);
    
    STK_LOG_DEBUG("Ringbuf write_force: forced write %zu bytes", written);
    return (written == len) ? STK_OK : STK_ERROR;
}

/* =========================================================================
 * 读取操作
 * ========================================================================= */

size_t stk_ringbuf_read(stk_ringbuf* rb, void* buffer, size_t len) {
    if (!rb || !buffer) {
        if (rb) STK_LOG_WARN("Ringbuf read: invalid parameters");
        return 0;
    }
    
    size_t used;
    if (rb->full) {
        used = rb->capacity;
    } else if (rb->tail >= rb->head) {
        used = rb->tail - rb->head;
    } else {
        used = rb->capacity - (rb->head - rb->tail);
    }
    
    size_t read_len = len > used ? used : len;
    
    if (read_len == 0) {
        return 0;
    }
    
    uint8_t* dst = (uint8_t*)buffer;
    size_t mask = ringbuf_mask(rb);
    size_t head = rb->head;
    
    /* 计算第一段可读长度（从 head 到缓冲区末尾） */
    size_t first_chunk = rb->capacity - head;
    
    if (first_chunk >= read_len) {
        memcpy(dst, rb->buffer + head, read_len);
    } else {
        memcpy(dst, rb->buffer + head, first_chunk);
        memcpy(dst + first_chunk, rb->buffer, read_len - first_chunk);
    }
    
    rb->head = (head + read_len) & mask;
    rb->full = false;
    
    STK_LOG_DEBUG("Ringbuf read: read %zu bytes, used=%zu, available=%zu",
                  read_len, stk_ringbuf_used(rb), stk_ringbuf_available(rb));
    return read_len;
}

size_t stk_ringbuf_read_byte(stk_ringbuf* rb, uint8_t* byte) {
    if (!rb || !byte) return 0;
    return stk_ringbuf_read(rb, byte, 1);
}

STK_STATUS stk_ringbuf_peek(const stk_ringbuf* rb, void* buffer, size_t len, size_t offset) {
    STK_RETURN_IF(!rb, STK_EINVAL, "Ringbuf peek: NULL ringbuf pointer");
    STK_RETURN_IF(!buffer, STK_EINVAL, "Ringbuf peek: NULL buffer");
    
    size_t used;
    if (rb->full) {
        used = rb->capacity;
    } else if (rb->tail >= rb->head) {
        used = rb->tail - rb->head;
    } else {
        used = rb->capacity - (rb->head - rb->tail);
    }
    
    if (offset >= used) {
        STK_LOG_WARN("Ringbuf peek: offset %zu out of range (used=%zu)", offset, used);
        return STK_ERANGE;
    }
    
    size_t peek_len = len;
    if (offset + peek_len > used) {
        peek_len = used - offset;
    }
    
    if (peek_len == 0) return STK_OK;
    
    size_t mask = ringbuf_mask(rb);
    size_t start = (rb->head + offset) & mask;
    uint8_t* dst = (uint8_t*)buffer;
    
    size_t first_chunk = rb->capacity - start;
    
    if (first_chunk >= peek_len) {
        memcpy(dst, rb->buffer + start, peek_len);
    } else {
        memcpy(dst, rb->buffer + start, first_chunk);
        memcpy(dst + first_chunk, rb->buffer, peek_len - first_chunk);
    }
    
    STK_LOG_DEBUG("Ringbuf peek: peeked %zu bytes at offset %zu", peek_len, offset);
    return STK_OK;
}

/* =========================================================================
 * 管理操作
 * ========================================================================= */

STK_STATUS stk_ringbuf_clear(stk_ringbuf* rb) {
    STK_RETURN_IF(!rb, STK_EINVAL, "Ringbuf clear: NULL ringbuf pointer");
    
    rb->head = 0;
    rb->tail = 0;
    rb->full = false;
    
    STK_LOG_DEBUG("Ringbuf cleared");
    return STK_OK;
}

STK_STATUS stk_ringbuf_reset(stk_ringbuf* rb) {
    return stk_ringbuf_clear(rb);
}

STK_STATUS stk_ringbuf_skip(stk_ringbuf* rb, size_t len) {
    STK_RETURN_IF(!rb, STK_EINVAL, "Ringbuf skip: NULL ringbuf pointer");
    
    size_t used;
    if (rb->full) {
        used = rb->capacity;
    } else if (rb->tail >= rb->head) {
        used = rb->tail - rb->head;
    } else {
        used = rb->capacity - (rb->head - rb->tail);
    }
    
    size_t skip_len = len > used ? used : len;
    
    if (skip_len == 0) return STK_OK;
    
    size_t mask = ringbuf_mask(rb);
    rb->head = (rb->head + skip_len) & mask;
    rb->full = false;
    
    STK_LOG_DEBUG("Ringbuf skip: skipped %zu bytes", skip_len);
    return STK_OK;
}

STK_STATUS stk_ringbuf_advance(stk_ringbuf* rb, size_t len) {
    return stk_ringbuf_skip(rb, len);
}

/* =========================================================================
 * 查询接口
 * ========================================================================= */

size_t stk_ringbuf_used(const stk_ringbuf* rb) {
    if (!rb) return 0;
    if (rb->full) return rb->capacity;
    if (rb->tail >= rb->head) return rb->tail - rb->head;
    return rb->capacity - (rb->head - rb->tail);
}

size_t stk_ringbuf_available(const stk_ringbuf* rb) {
    if (!rb) return 0;
    return rb->capacity - stk_ringbuf_used(rb);
}

size_t stk_ringbuf_capacity(const stk_ringbuf* rb) {
    return rb ? rb->capacity : 0;
}

bool stk_ringbuf_empty(const stk_ringbuf* rb) {
    return rb ? (!rb->full && (rb->head == rb->tail)) : true;
}

bool stk_ringbuf_full(const stk_ringbuf* rb) {
    return rb ? rb->full : false;
}

/* =========================================================================
 * 线程安全操作 (SPSC)
 * ========================================================================= */

size_t stk_ringbuf_write_spsc(stk_ringbuf* rb, const void* data, size_t len) {
    if (!rb || !data || len == 0) return 0;
    
    size_t tail = rb->tail;
    size_t head = rb->head;
    size_t capacity = rb->capacity;
    size_t mask = capacity - 1;
    
    /* 计算可用空间（无锁，使用本地副本） */
    size_t available;
    if (tail >= head) {
        available = capacity - (tail - head);
    } else {
        available = head - tail;
    }
    if (rb->full) available = 0;
    
    size_t write_len = len > available ? available : len;
    if (write_len == 0) return 0;
    
    const uint8_t* src = (const uint8_t*)data;
    size_t first_chunk = capacity - tail;
    
    if (first_chunk >= write_len) {
        memcpy(rb->buffer + tail, src, write_len);
    } else {
        memcpy(rb->buffer + tail, src, first_chunk);
        memcpy(rb->buffer, src + first_chunk, write_len - first_chunk);
    }
    
    rb->tail = (tail + write_len) & mask;
    rb->full = (rb->tail == head);
    
    return write_len;
}

size_t stk_ringbuf_read_spsc(stk_ringbuf* rb, void* buffer, size_t len) {
    if (!rb || !buffer) return 0;
    
    size_t tail = rb->tail;
    size_t head = rb->head;
    size_t capacity = rb->capacity;
    size_t mask = capacity - 1;
    
    /* 计算已用空间（无锁，使用本地副本） */
    size_t used;
    if (tail >= head) {
        used = tail - head;
    } else {
        used = capacity - (head - tail);
    }
    if (rb->full) used = capacity;
    
    size_t read_len = len > used ? used : len;
    if (read_len == 0) return 0;
    
    uint8_t* dst = (uint8_t*)buffer;
    size_t first_chunk = capacity - head;
    
    if (first_chunk >= read_len) {
        memcpy(dst, rb->buffer + head, read_len);
    } else {
        memcpy(dst, rb->buffer + head, first_chunk);
        memcpy(dst + first_chunk, rb->buffer, read_len - first_chunk);
    }
    
    rb->head = (head + read_len) & mask;
    rb->full = false;
    
    return read_len;
}

size_t stk_ringbuf_available_spsc(const stk_ringbuf* rb) {
    if (!rb) return 0;
    
    size_t tail = rb->tail;
    size_t head = rb->head;
    size_t capacity = rb->capacity;
    
    if (rb->full) return 0;
    if (tail >= head) return capacity - (tail - head);
    return head - tail;
}

size_t stk_ringbuf_used_spsc(const stk_ringbuf* rb) {
    if (!rb) return 0;
    
    size_t tail = rb->tail;
    size_t head = rb->head;
    size_t capacity = rb->capacity;
    
    if (rb->full) return capacity;
    if (tail >= head) return tail - head;
    return capacity - (head - tail);
}