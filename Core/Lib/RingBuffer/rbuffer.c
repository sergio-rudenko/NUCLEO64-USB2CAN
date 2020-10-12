/*
 * File:   rbuffer_c
 * Author: rudenkos
 *
 * Created on September 18, 2019, 11:29 AM
 */

#ifndef RBUFFER_C
#define RBUFFER_C

#include <rbuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @param rb
 * @param offset
 */
void ring_buffer_move_read_index(rBuffer_t *rb, uint16_t offset) {
	while ((offset--) && ring_buffer_available(rb)) {
		rb->readIndex = (rb->readIndex + 1) % rb->size;
		rb->overrun = false;
	}
}

/**
 *
 * @param rb
 * @param offset
 */
void ring_buffer_move_write_index(rBuffer_t *rb, uint16_t offset) {
	while (offset--) {
		rb->writeIndex = (rb->writeIndex + 1) % rb->size;

		if (rb->overrun) {
			rb->readIndex = rb->writeIndex;
		} else {
			rb->overrun = (rb->writeIndex == rb->readIndex);
		}
	}
}

/**
 *
 * @param rb
 * @return
 */
uint8_t* ring_buffer_get_read_ptr(rBuffer_t *rb) {
	return (rb) ? (rb->data + rb->readIndex) : NULL;
}

/**
 *
 * @param rb
 * @return
 */
uint8_t* ring_buffer_get_write_ptr(rBuffer_t *rb) {
	return (rb) ? (rb->data + rb->writeIndex) : NULL;
}

/**
 *
 * @param rb
 * @return
 */
size_t ring_buffer_get_read_linear_size(rBuffer_t *rb) {
	size_t size = 0;

	if (rb) {
		size = (rb->data + rb->size) - ring_buffer_get_read_ptr(rb);
		size = (size < ring_buffer_available(rb)) ?
				size : ring_buffer_available(rb);
	}
	return size;
}

/**
 *
 * @param rb
 * @return
 */
size_t ring_buffer_get_write_linear_size(rBuffer_t *rb) {
	size_t size = 0;

	if (rb) {
		size = (rb->data + rb->size) - ring_buffer_get_write_ptr(rb);
		size = (size < ring_buffer_available_for_write(rb)) ?
				size : ring_buffer_available_for_write(rb);
	}
	return size;
}

/**
 *
 * @param rb
 * @param pdata
 * @param size
 * @return
 */
void ring_buffer_init(rBuffer_t *rb, uint8_t *pdata, size_t size) {
	if (rb && pdata && size) {
		rb->data = pdata;
		rb->size = size;
		ring_buffer_reset(rb);
	}
}

/**
 *
 * @param rb
 */
void ring_buffer_reset(rBuffer_t *rb) {
	if (rb) {
		rb->readIndex = 0;
		rb->writeIndex = 0;
		rb->overrun = false;
		memset(rb->data, 0x00, rb->size);
	}
}

/**
 *
 * @param rb
 * @return
 */
size_t ring_buffer_available(rBuffer_t *rb) {
	size_t bytes = 0;

	if (rb) {
		if (rb->writeIndex > rb->readIndex) {
			bytes = rb->writeIndex - rb->readIndex;
		} else if (rb->writeIndex < rb->readIndex) {
			bytes = rb->size - rb->readIndex + rb->writeIndex;
		} else if (rb->overrun) {
			bytes = rb->size;
		} else {
			bytes = 0;
		}
	}
	return bytes;
}

/**
 *
 * @param rb
 * @return
 */
size_t ring_buffer_available_for_write(rBuffer_t *rb) {
	size_t bytes = 0;

	if (rb) {
		bytes = rb->size - ring_buffer_available(rb);
	}
	return bytes;
}

/**
 * @breaf Attempt to find specific byte in buffer, readIndex remain unchanged
 * @param rb
 * @param byte
 * @return index of target byte in buffer or 0 if not found
 */
size_t ring_buffer_find(rBuffer_t *rb, uint8_t byte) {
	size_t result = 0;

	for (size_t i = 0; i < ring_buffer_available(rb); i++) {
		if (ring_buffer_at(rb, i) == byte) {
			result = i;
			break;
		}
	}
	return result;
}

/**
 * @breaf Move readIndex to specific byte or until buffer empty
 * @param rb
 * @param byte
 */
void ring_buffer_seek(rBuffer_t *rb, uint8_t byte) {
	while (ring_buffer_available(rb)) {
		if (ring_buffer_peek(rb) != byte) {
			ring_buffer_move_read_index(rb, 1);
		}
		else {
			break;
		}
	}
}

/**
 *
 * @param rb
 * @param i
 * @return
 */
uint8_t ring_buffer_at(rBuffer_t *rb, size_t i) {
	uint8_t result = 0x00;

	if (rb && i < ring_buffer_available(rb)) {
		result = rb->data[(rb->readIndex + i) % rb->size];
	}
	return result;
}

/**
 *
 * @param rb
 * @return
 */
uint8_t ring_buffer_peek(rBuffer_t *rb) {
	uint8_t result = 0x00;

	if (rb) {
		result = rb->data[rb->readIndex];
	}
	return result;
}

/**
 *
 * @param rb
 */
void ring_buffer_flush(rBuffer_t *rb) {
	while (ring_buffer_available(rb)) {
		ring_buffer_move_read_index(rb, 1);
	}
}

/**
 *
 * @param rb
 * @return
 */
uint8_t ring_buffer_read(rBuffer_t *rb) {
	uint8_t result = 0x00;

	if (ring_buffer_available(rb)) {
		result = rb->data[rb->readIndex];
		ring_buffer_move_read_index(rb, 1);
	}
	return result;
}

/**
 *
 * @param rb
 * @param pdata
 * @param size
 * @return
 */
size_t ring_buffer_read_bytes(rBuffer_t *rb, uint8_t *pdata, size_t size) {
	size_t count = 0;

	while (ring_buffer_available(rb) && (count < size)) {
		pdata[count++] = ring_buffer_read(rb);
	}
	return count;
}

/**
 *
 * @param rb
 * @param data
 */
void ring_buffer_write(rBuffer_t *rb, uint8_t data) {
	if (rb) {
		rb->data[rb->writeIndex] = data;
		ring_buffer_move_write_index(rb, 1);
	}
}

/**
 *
 * @param rb
 * @param pdata
 * @param size
 * @return
 */
size_t ring_buffer_write_bytes(rBuffer_t *rb, uint8_t *pdata, size_t size) {
	size_t count = 0;

	if (rb) {
		//TODO? Prevent overrun
		while (count < size) {
			ring_buffer_write(rb, pdata[count++]);
		}
	}
	return count;
}

#ifdef __cplusplus
}
#endif

#endif /* RBUFFER_C */

