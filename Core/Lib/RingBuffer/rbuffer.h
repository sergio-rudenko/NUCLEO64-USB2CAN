/*
 * File:   rbuffer.h
 * Author: rudenkos
 *
 * Created on September 18, 2019, 11:20 AM
 */

#ifndef RBUFFER_H
#define RBUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        uint8_t* data;
        uint16_t size;
        volatile uint16_t readIndex;
        volatile uint16_t writeIndex;
        volatile bool overrun;
    } rBuffer_t;

    void 	 ring_buffer_init(rBuffer_t* rb, uint8_t* pdata, size_t size);
    void     ring_buffer_reset(rBuffer_t* rb);
    void     ring_buffer_flush(rBuffer_t* rb);

    size_t   ring_buffer_available(rBuffer_t* rb);
    size_t   ring_buffer_available_for_write(rBuffer_t* rb);

    size_t   ring_buffer_find(rBuffer_t* rb, uint8_t byte);
    void   	 ring_buffer_seek(rBuffer_t* rb, uint8_t byte);

    uint8_t  ring_buffer_at(rBuffer_t* rb, size_t i);
    uint8_t  ring_buffer_peek(rBuffer_t* rb);

    uint8_t  ring_buffer_read(rBuffer_t* rb);
    size_t   ring_buffer_read_bytes(rBuffer_t* rb, uint8_t* pdata, size_t size);

    void     ring_buffer_write(rBuffer_t* rb, uint8_t byte);
    size_t   ring_buffer_write_bytes(rBuffer_t* rb, uint8_t* pdata, size_t size);

    /* low level */
    void	 ring_buffer_move_read_index(rBuffer_t *rb, uint16_t offset);
    void 	 ring_buffer_move_write_index(rBuffer_t *rb, uint16_t offset);

    uint8_t* ring_buffer_get_read_ptr(rBuffer_t *rb);
    uint8_t* ring_buffer_get_write_ptr(rBuffer_t *rb);

    size_t 	 ring_buffer_get_read_linear_size(rBuffer_t *rb);
    size_t 	 ring_buffer_get_write_linear_size(rBuffer_t *rb);
#ifdef __cplusplus
}
#endif

#endif /* RBUFFER_H */
