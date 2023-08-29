#ifndef _LIST_H
#define _LIST_H

#include "main.h"

#define LIST_MAX_NUM  1000
#define LIST_U32_MAX_CNT   0

struct xLIST_ITEM
{
	volatile u16  xItemValue;
	volatile u32  value;
	struct xLIST_ITEM* volatile pxNext;
	struct xLIST_ITEM* volatile pxPrevious;
	void* pvOwner;
	void* volatile pvContainer;
};
typedef struct xLIST_ITEM ListItem_t;

struct xMINI_LIST_ITEM
{
	volatile u16 xItemValue;
	volatile u32 value;
	struct xLIST_ITEM* volatile pxNext;
	struct xLIST_ITEM* volatile pxPrevious;
};

typedef struct xMINI_LIST_ITEM MiniListItem_t;

typedef struct xLIST
{
	u16 uxNumberOfItems;
	ListItem_t* volatile pxIndex;
	MiniListItem_t xListEnd;
} List_t;


extern List_t   pkg_list;       //unload car list

void vListInitialise(List_t* const pxList);
void vListInitialiseItem(ListItem_t* pxItem);
void vListInsert(List_t* const pxList, ListItem_t* const pxNewListItem); // sort form small to big  according  to  xItemvalue;
void vListInsertsort(List_t* const pxList, ListItem_t* const pxNewListItem); //sort from big to small according to value;
void vListInsertEnd(List_t* const pxList, ListItem_t* const pxNewListItem);
void uxListRemove(ListItem_t* pxItemToRemove);
#endif