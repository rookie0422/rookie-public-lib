#ifndef VLIST_H
#define VLIST_H
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 通用队列数据结构
 *
 */

typedef struct vlist_item_t {
	struct vlist_item_t*    next;///< 始终指向链表的头部
	struct vlist_item_t*    pre;///< 始终指向链表的尾部
	void*                   object;///< 链表数据
	
} vlist_item_t;

typedef struct vlist_t {
	vlist_item_t *  header;
	vlist_item_t *  tail;
	vlist_item_t *	 iterator;
	int             itemcount;
} vlist_t;

vlist_t* vlist__create(void);
vlist_item_t* vlist__append(vlist_t * list, void* object);
void* vlist__pop(vlist_t * list);
int vlist__count(vlist_t * list);

/**
 * @brief  虚拟弹出，不实际弹出，只返回第一个对象指针
 * @note   
 * @param  list: 
 * @retval None
 */
void* vlist__virtual_pop(vlist_t * list);


void* vlist__remove(vlist_t* list, void* object);
vlist_item_t* vlist__search(vlist_t* list, void* object);

void vlist__clear(vlist_t* list);

void* vlist__begin_iterator(vlist_t* list, bool *hasnext);
void* vlist__next(vlist_t* list, bool *hasnext);

#endif
