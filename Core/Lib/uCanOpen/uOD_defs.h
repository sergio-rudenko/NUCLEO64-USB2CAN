/*
 * uOD_defs.h
 *
 *  Created on: Oct 26, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UOD_DEFS_H_
#define LIB_UCANOPEN_UOD_DEFS_H_

typedef enum uOD_ItemType
{
	UNDEFINED = 0,
	UNSIGNED8,
	UNSIGNED16,
	UNSIGNED32,
	UNSIGNED64,
	VISIBLE_STRING,
	OCTET_STRING,
	SUBARRAY,
} uOD_ItemType_t;

typedef enum uOD_ItemAccess
{
	READ_ONLY = 0,
	WRITE_ONLY,
	READ_WRITE,
} uOD_ItemAccess_t;

typedef struct uOD_Item
{
	uint16_t index;
	uOD_ItemType_t Type;
	uOD_ItemAccess_t Access;
	void *address;
	size_t size;
} uOD_Item_t;

#endif /* LIB_UCANOPEN_UOD_DEFS_H_ */
