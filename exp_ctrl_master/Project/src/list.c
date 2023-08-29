#include <stdlib.h>
#include "list.h"

List_t   pkg_list;       //pkg transted list


void vListInitialise(List_t* const pxList)
{

	pxList->pxIndex = (ListItem_t*)&(pxList->xListEnd);

	pxList->xListEnd.xItemValue = LIST_MAX_NUM;
	pxList->xListEnd.value = LIST_U32_MAX_CNT;

	pxList->xListEnd.pxNext = (ListItem_t*)&(pxList->xListEnd);
	pxList->xListEnd.pxPrevious = (ListItem_t*)&(pxList->xListEnd);

	pxList->uxNumberOfItems = 0;
}
/*-----------------------------------------------------------*/

void vListInitialiseItem(ListItem_t* pxItem)
{
	pxItem->pvContainer = NULL;
}
/*-----------------------------------------------------------*/

void vListInsertEnd(List_t* const pxList, ListItem_t* const pxNewListItem)
{
	ListItem_t* const pxIndex = pxList->pxIndex;

	pxNewListItem->pxNext = pxIndex;
	pxNewListItem->pxPrevious = pxIndex->pxPrevious;

	pxIndex->pxPrevious->pxNext = pxNewListItem;
	pxIndex->pxPrevious = pxNewListItem;

	pxNewListItem->pvContainer = (void*)pxList;

	(pxList->uxNumberOfItems)++;
}
/*-----------------------------------------------------------*/

void vListInsert(List_t* const pxList, ListItem_t* const pxNewListItem)  // sort form small to big  according  to  xItemvalue;
{
	ListItem_t* pxIterator;
	const u16 xValueOfInsertion = pxNewListItem->xItemValue;


	if (xValueOfInsertion == LIST_MAX_NUM)
	{
		pxIterator = pxList->xListEnd.pxPrevious;
	}
	else
	{

		for (pxIterator = (ListItem_t*)&(pxList->xListEnd); pxIterator->pxNext->xItemValue <= xValueOfInsertion; pxIterator = pxIterator->pxNext)
		{
		}
	}

	pxNewListItem->pxNext = pxIterator->pxNext;
	pxNewListItem->pxNext->pxPrevious = pxNewListItem;
	pxNewListItem->pxPrevious = pxIterator;
	pxIterator->pxNext = pxNewListItem;

	pxNewListItem->pvContainer = (void*)pxList;
	(pxList->uxNumberOfItems)++;
}

void vListInsertsort(List_t* const pxList, ListItem_t* const pxNewListItem)  //sort from big to small according to value;
{
	ListItem_t* pxIterator;

	//	pxNewListItem->value = pxNewListItem->xItemValue;
	const u16 xValueOfInsertion = pxNewListItem->value;



	for (pxIterator = (ListItem_t*)&(pxList->xListEnd); pxIterator->pxNext->value > xValueOfInsertion; pxIterator = pxIterator->pxNext)
	{

	}

	pxNewListItem->pxNext = pxIterator->pxNext;
	pxNewListItem->pxNext->pxPrevious = pxNewListItem;
	pxNewListItem->pxPrevious = pxIterator;
	pxIterator->pxNext = pxNewListItem;

	pxNewListItem->pvContainer = (void*)pxList;
	(pxList->uxNumberOfItems)++;
}


void uxListRemove(ListItem_t* pxItemToRemove)
{

	List_t* pxList = (List_t*)pxItemToRemove->pvContainer;

	pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
	pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;

	if (pxList->pxIndex == pxItemToRemove)
	{
		pxList->pxIndex = pxItemToRemove->pxPrevious;
	}

	pxItemToRemove->pvContainer = NULL;
	//(pxList->uxNumberOfItems)--;

	//	return pxList->uxNumberOfItems;
}