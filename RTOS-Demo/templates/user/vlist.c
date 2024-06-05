#include "vlist.h"
#include "dyn_mem.h"
#include <string.h>
#include <stdbool.h>

/**
 * @brief 动态生成一个队列
 *
 * @return vlist_t* 队列指针
 */

vlist_t * vlist__create()
{
	vlist_t* plist = dm_alloc(sizeof(vlist_t));
	memset(plist, 0, sizeof(vlist_t));
	return plist;
}


vlist_item_t* vlist__append(vlist_t * list, void* object)
{

	vlist_item_t* pnode;
	if (list == NULL)
		return NULL;
	
	pnode = (vlist_item_t *)dm_alloc(sizeof(vlist_item_t));
	if (pnode == NULL)
		return NULL;

	memset(pnode, 0, sizeof(vlist_item_t));

	pnode->object = object;

	if(list->header == NULL) {
		list->tail = pnode;
		list->header = pnode;		
	} else {
		list->tail->next = pnode;
		pnode->pre = list->tail;
		list->tail = pnode;
	}
	list->itemcount++;
	return pnode;
}

vlist_item_t* vlist__search(vlist_t* list, void* object)
{
	vlist_item_t* pnode;
	if (list == NULL)
		return NULL;

	pnode = list->header;

	while (pnode != NULL) {
		if (pnode->object == object) {
			return pnode;
		}
		pnode = pnode->next;
	}

	return NULL;
}

void* vlist__remove(vlist_t* list, void* object)
{
	vlist_item_t* pnode;
	if (list == NULL)
		return NULL;

	pnode = vlist__search(list, object);

	if (pnode == NULL) {
		return NULL;
	}

	if (list->tail == pnode || list->header == pnode) {
		if (list->tail == pnode) {
			list->tail =  pnode->pre;
			if (pnode->pre != NULL)
				pnode->pre->next = NULL;
		}

		if (list->header == pnode) {
			list->header = pnode->next;
			if (pnode->next != NULL)
				pnode->next->pre = NULL;
		}
	} else {
		if (pnode->pre != NULL)
			pnode->pre->next = pnode->next;
		if (pnode->next != NULL)
			pnode->next->pre = pnode->pre;
	}

	dm_free(pnode);
	list->itemcount--;
	return object;
}

void vlist__clear(vlist_t* list)
{
    vlist_item_t* pnode;
	vlist_item_t* nextnode;

	if (list == NULL)
		return;

	pnode = list->header;
    
	while (pnode != NULL) {
		nextnode = pnode->next;
		dm_free(pnode);
		pnode = nextnode;
	}

	list->header = NULL;
	list->tail = NULL;
	list->itemcount = 0;
}

void* vlist__begin_iterator(vlist_t* list, bool *hasnext)
{
	if (list == NULL)
		return NULL;

	list->iterator = list->header;
	if (list->iterator == NULL) {
		*hasnext = false;
		return NULL;
	}
		
	
	if (list->iterator->next == NULL)
		*hasnext = false;
	else
		*hasnext = true;
	
	return list->iterator->object;
}

void* vlist__next(vlist_t* list, bool *hasnext)
{
	if (list == NULL)
		return NULL;

	list->iterator = list->iterator->next;
	if (list->iterator == NULL) {
		*hasnext = false;
		return NULL;
	}
		
	
	if (list->iterator->next == NULL)
		*hasnext = false;
	else
		*hasnext = true;
	
	return list->iterator->object;
}

void* vlist__pop(vlist_t * list)
{
	vlist_item_t* pnode;
	void *object;
	if (list == NULL)
		return NULL;

	pnode = list->header;

	if (pnode == NULL) {
		return NULL;
	}

	object = pnode->object;

	if (list->tail == pnode || list->header == pnode) {
		if (list->tail == pnode) {
			list->tail =  pnode->pre;
			if (pnode->pre != NULL)
				pnode->pre->next = NULL;
		}

		if (list->header == pnode) {
			list->header = pnode->next;
			if (pnode->next != NULL)
				pnode->next->pre = NULL;
		}
	} else {
		if (pnode->pre != NULL)
			pnode->pre->next = pnode->next;
		if (pnode->next != NULL)
			pnode->next->pre = pnode->pre;
	}

	dm_free(pnode);

	list->itemcount--;
	return object;
}
/**
 * @brief  虚拟弹出，不实际弹出，只返回第一个对象指针
 * @note   
 * @param  list: 
 * @retval None
 */
void* vlist__virtual_pop(vlist_t * list)
{
	vlist_item_t* pnode;
	void *object;
	if (list == NULL)
		return NULL;

	pnode = list->header;

	if (pnode == NULL) {
		return NULL;
	}

	object = pnode->object;

	return object;
}


int vlist__count(vlist_t * list)
{
	if (list == NULL)
		return 0;

	return list->itemcount;
}