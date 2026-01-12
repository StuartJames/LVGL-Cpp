/*
 *                LEGL 2025-2026 HydraSystems.
 *
 *  This program is free software; you can redistribute it and/or   
 *  modify it under the terms of the GNU General Public License as  
 *  published by the Free Software Foundation; either version 2 of  
 *  the License, or (at your option) any later version.             
 *                                                                  
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of  
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   
 *  GNU General Public License for more details.                    
 * 
 *  Based on a design by LVGL Kft
 * 
 * =====================================================================
 *
 * Edit     Date     Version       Edit Description
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "misc/EG_LinkList.h"
#include "misc/EG_Memory.h"

/////////////////////////////////////////////////////////////////////////////////////////

#define LL_NODE_META_SIZE (sizeof(EG_ListNode_t *) + sizeof(EG_ListNode_t *))
#define LL_PREV_P_OFFSET(pList) (pList->n_size)
#define LL_NEXT_P_OFFSET(pList) (pList->n_size + sizeof(EG_ListNode_t *))

/////////////////////////////////////////////////////////////////////////////////////////

static void NodeSetPrev(lv_ll_t *pList, EG_ListNode_t *act, EG_ListNode_t *prev);
static void NodeSetNext(lv_ll_t *pList, EG_ListNode_t *act, EG_ListNode_t *next);

/////////////////////////////////////////////////////////////////////////////////////////

void _lv_ll_init(lv_ll_t *pList, uint32_t node_size)
{
	pList->head = NULL;
	pList->tail = NULL;
#ifdef LV_ARCH_64
	/*Round the size up to 8*/
	node_size = (node_size + 7) & (~0x7);
#else
	/*Round the size up to 4*/
	node_size = (node_size + 3) & (~0x3);
#endif

	pList->n_size = node_size;
}

/////////////////////////////////////////////////////////////////////////////////////////

void* _lv_ll_ins_head(lv_ll_t *pList)
{
	EG_ListNode_t *NewNode;

	NewNode = (EG_ListNode_t *)EG_AllocMem(pList->n_size + LL_NODE_META_SIZE);
	if(NewNode != nullptr) {
		NodeSetPrev(pList, NewNode, nullptr);       /*No prev. before the new head*/
		NodeSetNext(pList, NewNode, pList->head); /*After new comes the old head*/
		if(pList->head != nullptr) {                /*If there is old head then before it goes the new*/
			NodeSetPrev(pList, pList->head, NewNode);
		}
		pList->head = NewNode;    /*Set the new head in the dsc.*/
		if(pList->tail == nullptr) { /*If there is no tail (1. node) set the tail too*/
			pList->tail = NewNode;
		}
	}
	return NewNode;
}

/////////////////////////////////////////////////////////////////////////////////////////

void* _lv_ll_ins_prev(lv_ll_t *pList, void *pNode)
{
EG_ListNode_t *NewNode;

	if((pList == nullptr) || (pNode == nullptr)) return nullptr;
	if(pList->head == pNode) {
		NewNode = (EG_ListNode_t *)_lv_ll_ins_head(pList);
		if(NewNode == nullptr) return nullptr;
	}
	else {
		NewNode = (EG_ListNode_t *)EG_AllocMem(pList->n_size + LL_NODE_META_SIZE);
		if(NewNode == nullptr) return nullptr;
		EG_ListNode_t *pPrevNode;
		pPrevNode = (EG_ListNode_t *)_lv_ll_get_prev(pList, pNode);
		NodeSetNext(pList, pPrevNode, NewNode);
		NodeSetPrev(pList, NewNode, pPrevNode);
		NodeSetPrev(pList, (EG_ListNode_t *)pNode, NewNode);
		NodeSetNext(pList, NewNode, (EG_ListNode_t *)pNode);
	}
	return NewNode;
}

/////////////////////////////////////////////////////////////////////////////////////////

void* _lv_ll_ins_tail(lv_ll_t *pList)
{
	EG_ListNode_t *NewNode;

	NewNode = (EG_ListNode_t *)EG_AllocMem(pList->n_size + LL_NODE_META_SIZE);

	if(NewNode != nullptr) {
		NodeSetNext(pList, NewNode, nullptr);       /*No next after the new tail*/
		NodeSetPrev(pList, NewNode, pList->tail); /*The prev. before new is the old tail*/
		if(pList->tail != nullptr) {                /*If there is old tail then the new comes after it*/
			NodeSetNext(pList, pList->tail, NewNode);
		}
		pList->tail = NewNode;    /*Set the new tail in the dsc.*/
		if(pList->head == nullptr) { /*If there is no head (1. node) set the head too*/
			pList->head = NewNode;
		}
	}

	return NewNode;
}

/////////////////////////////////////////////////////////////////////////////////////////

void _lv_ll_remove(lv_ll_t *pList, void *pNode)
{
	if(pList == nullptr) return;
	if(pList->head == pNode) {	// The new head will be the node after 'n_act'
		pList->head = (EG_ListNode_t *)_lv_ll_get_next(pList, pNode);
		if(pList->head == nullptr) {
			pList->tail = nullptr;
		}
		else {
			NodeSetPrev(pList, pList->head, nullptr);
		}
	}
	else if(pList->tail == pNode) {	// The new tail will be the node before 'n_act'
		pList->tail = (EG_ListNode_t *)_lv_ll_get_prev(pList, pNode);
		if(pList->tail == nullptr) {
			pList->head = nullptr;
		}
		else{
			NodeSetNext(pList, pList->tail, nullptr);
		}
	}
	else{
		EG_ListNode_t *pPrevNode = (EG_ListNode_t *)_lv_ll_get_prev(pList, pNode);
		EG_ListNode_t *pNextNode = (EG_ListNode_t *)_lv_ll_get_next(pList, pNode);
		NodeSetNext(pList, pPrevNode, pNextNode);
		NodeSetPrev(pList, pNextNode, pPrevNode);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void _lv_ll_clear(lv_ll_t *pList)
{
	void *i;
	void *i_next;

	i = _lv_ll_get_head(pList);
	i_next = nullptr;

	while(i != nullptr) {
		i_next = _lv_ll_get_next(pList, i);

		_lv_ll_remove(pList, i);
		EG_FreeMem(i);

		i = i_next;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void _lv_ll_chg_list(lv_ll_t *ll_ori_p, lv_ll_t *ll_new_p, void *node, bool head)
{
	_lv_ll_remove(ll_ori_p, node);

	if(head) {
		/*Set node as head*/
		NodeSetPrev(ll_new_p, (EG_ListNode_t *)node, nullptr);
		NodeSetNext(ll_new_p, (EG_ListNode_t *)node, ll_new_p->head);

		if(ll_new_p->head != nullptr) { /*If there is old head then before it goes the new*/
			NodeSetPrev(ll_new_p, ll_new_p->head, (EG_ListNode_t *)node);
		}

		ll_new_p->head = (EG_ListNode_t *)node;       /*Set the new head in the dsc.*/
		if(ll_new_p->tail == nullptr) { /*If there is no tail (first node) set the tail too*/
			ll_new_p->tail = (EG_ListNode_t *)node;
		}
	}
	else {
		/*Set node as tail*/
		NodeSetPrev(ll_new_p, (EG_ListNode_t *)node, ll_new_p->tail);
		NodeSetNext(ll_new_p, (EG_ListNode_t *)node, nullptr);

		if(ll_new_p->tail != nullptr) { /*If there is old tail then after it goes the new*/
			NodeSetNext(ll_new_p, ll_new_p->tail, (EG_ListNode_t *)node);
		}

		ll_new_p->tail = (EG_ListNode_t *)node;       /*Set the new tail in the dsc.*/
		if(ll_new_p->head == nullptr) { /*If there is no head (first node) set the head too*/
			ll_new_p->head = (EG_ListNode_t *)node;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void* _lv_ll_get_head(const lv_ll_t *pList)
{
	if(pList == nullptr) return nullptr;
	return pList->head;
}

/////////////////////////////////////////////////////////////////////////////////////////

void* _lv_ll_get_tail(const lv_ll_t *pList)
{
	if(pList == NULL) return nullptr;
	return pList->tail;
}

/////////////////////////////////////////////////////////////////////////////////////////

void* _lv_ll_get_next(const lv_ll_t *pList, const void *n_act)
{
	// Pointer to the next node is stored in the end of this node.
  // Go there and return the address found there
	const EG_ListNode_t *n_act_d = (EG_ListNode_t *)n_act;
	n_act_d += LL_NEXT_P_OFFSET(pList);
	return *((EG_ListNode_t **)n_act_d);
}

/////////////////////////////////////////////////////////////////////////////////////////

void* _lv_ll_get_prev(const lv_ll_t *pList, const void *n_act)
{
	// Pointer to the prev. node is stored in the end of this node.
  // Go there and return the address found there
	const EG_ListNode_t *n_act_d = (EG_ListNode_t *)n_act;
	n_act_d += LL_PREV_P_OFFSET(pList);
	return *((EG_ListNode_t **)n_act_d);
}

/////////////////////////////////////////////////////////////////////////////////////////

uint32_t _lv_ll_get_len(const lv_ll_t *pList)
{
	uint32_t len = 0;
	void *node;

	for(node = _lv_ll_get_head(pList); node != nullptr; node = _lv_ll_get_next(pList, node)) {
		len++;
	}
	return len;
}

/////////////////////////////////////////////////////////////////////////////////////////

void _lv_ll_move_before(lv_ll_t *pList, void *n_act, void *n_after)
{
	if(n_act == n_after) return; /*Can't move before itself*/
	void *n_before;
	if(n_after != nullptr)	n_before = _lv_ll_get_prev(pList, n_after);
	else n_before = _lv_ll_get_tail(pList); /*if `n_after` is NULL `n_act` should be the new tail*/
	if(n_act == n_before) return; /*Already before `n_after`*/
	/*It's much easier to remove from the list and add again*/
	_lv_ll_remove(pList, n_act);
	/*Add again by setting the prev. and next nodes*/
	NodeSetNext(pList, (EG_ListNode_t *)n_before, (EG_ListNode_t *)n_act);
	NodeSetPrev(pList, (EG_ListNode_t *)n_act, (EG_ListNode_t *)n_before);
	NodeSetPrev(pList, (EG_ListNode_t *)n_after, (EG_ListNode_t *)n_act);
	NodeSetNext(pList, (EG_ListNode_t *)n_act, (EG_ListNode_t *)n_after);
	/*If `n_act` was moved before NULL then it become the new tail*/
	if(n_after == nullptr) pList->tail = (EG_ListNode_t *)n_act;
	/*If `n_act` was moved before `NULL` then it's the new head*/
	if(n_before == nullptr) pList->head = (EG_ListNode_t *)n_act;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool _lv_ll_is_empty(lv_ll_t *pList)
{
	if(pList == nullptr) return true;
	if(pList->head == nullptr && pList->tail == nullptr) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void NodeSetPrev(lv_ll_t *pList, EG_ListNode_t *act, EG_ListNode_t *prev)
{
	if(act == nullptr) return; /*Can't set the prev node of `NULL`*/
	uint8_t *act8 = (uint8_t *)act;
	act8 += LL_PREV_P_OFFSET(pList);
	EG_ListNode_t **act_node_p = (EG_ListNode_t **)act8;
	EG_ListNode_t **prev_node_p = (EG_ListNode_t **)&prev;
	*act_node_p = *prev_node_p;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void NodeSetNext(lv_ll_t *pList, EG_ListNode_t *act, EG_ListNode_t *next)
{
	if(act == nullptr) return; /*Can't set the next node of `NULL`*/
	uint8_t *act8 = (uint8_t *)act;
	act8 += LL_NEXT_P_OFFSET(pList);
	EG_ListNode_t **act_node_p = (EG_ListNode_t **)act8;
	EG_ListNode_t **next_node_p = (EG_ListNode_t **)&next;

	*act_node_p = *next_node_p;
}
