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

#include "misc/EG_List.h"
#include "misc/EG_Memory.h"

/////////////////////////////////////////////////////////////////////////////

bool IsValidAddress(const void* lp,	int nBytes, bool bReadWrite /*= true*/){ return true; }; // TO DO

/////////////////////////////////////////////////////////////////////////////

EGList::EGList(void) :
	m_pNodeHead(nullptr),
  m_pNodeTail(nullptr),
	m_EntryCount(0)
{
}

/////////////////////////////////////////////////////////////////////////////

EGList::~EGList()
{
	RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////

void EGList::RemoveAll()
{
  while(m_EntryCount){
	  EG_Node_t *pNode = m_pNodeHead;
	  m_pNodeHead = pNode->m_pNext;
    EG_FreeMem(pNode);
    --m_EntryCount;
  }
	m_pNodeHead = m_pNodeTail = nullptr;
}

/////////////////////////////////////////////////////////////////////////////

EG_Node_t* EGList::NewNode(EG_Node_t *m_pPrev, EG_Node_t *m_pNext)
{
	EG_Node_t *pNode = (EG_Node_t*)EG_AllocMem(sizeof(EG_Node_t));
	pNode->m_pPrev = m_pPrev;
	pNode->m_pNext = m_pNext;
	m_EntryCount++;
	pNode->m_pData = nullptr;       
	return pNode;
}

/////////////////////////////////////////////////////////////////////////////

void EGList::FreeNode(EG_Node_t *pNode)
{
	if((pNode == nullptr) || (m_EntryCount == 0)) return;
	m_EntryCount--;
  EG_FreeMem(pNode);
}

/////////////////////////////////////////////////////////////////////////////

POSITION EGList::AddHead(void* pNewElement)
{
	EG_Node_t *pNewNode = NewNode(nullptr, m_pNodeHead);
	pNewNode->m_pData = pNewElement;
	if(m_pNodeHead != nullptr)	m_pNodeHead->m_pPrev = pNewNode;
	else m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;
	return (POSITION) pNewNode;
}

/////////////////////////////////////////////////////////////////////////////

POSITION EGList::AddTail(void* pNewElement)
{
	EG_Node_t *pNewNode = NewNode(m_pNodeTail, nullptr);
	pNewNode->m_pData = pNewElement;
	if(m_pNodeTail != nullptr) m_pNodeTail->m_pNext = pNewNode; 
	else	m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
	return (POSITION) pNewNode;
}

/////////////////////////////////////////////////////////////////////////////

void* EGList::RemoveHead()
{
  if((m_pNodeHead == nullptr) || (m_EntryCount == 0)) return nullptr;
	void *m_pData = m_pNodeHead->m_pData; // save contents
	EG_Node_t *pDeleteNode = m_pNodeHead;
	m_pNodeHead = pDeleteNode->m_pNext;
	if(m_pNodeHead != nullptr) m_pNodeHead->m_pPrev = nullptr; // Head node has no previous link
	else m_pNodeTail = nullptr;       // no nodes left
	FreeNode(pDeleteNode);
	return m_pData;
}

/////////////////////////////////////////////////////////////////////////////

void* EGList::RemoveTail()
{
  if((m_pNodeTail == nullptr) || (m_EntryCount == 0)) return nullptr;
	void* m_pData = m_pNodeTail->m_pData;
	EG_Node_t* pDeleteNode = m_pNodeTail;
	m_pNodeTail = pDeleteNode->m_pPrev;
	if(m_pNodeTail != nullptr) m_pNodeTail->m_pNext = nullptr; // Tail has no next link
	else m_pNodeHead = nullptr;       // no nodes left
	FreeNode(pDeleteNode);
	return m_pData;
}

/////////////////////////////////////////////////////////////////////////////

POSITION EGList::InsertBefore(POSITION Position, void* pNewElement)
{
	if(Position == 0) return AddHead(pNewElement); // insert before nothing -> head of the list
	EG_Node_t* pCurrentNode = (EG_Node_t*) Position;	// Insert it before position
	EG_Node_t* pNewNode = NewNode(pCurrentNode->m_pPrev, pCurrentNode);
	pNewNode->m_pData = pNewElement;
	if(pCurrentNode->m_pPrev != nullptr) pCurrentNode->m_pPrev->m_pNext = pNewNode;
	else m_pNodeHead = pNewNode;
	pCurrentNode->m_pPrev = pNewNode;
	return (POSITION) pNewNode;
}

/////////////////////////////////////////////////////////////////////////////

POSITION EGList::InsertAfter(POSITION Position, void* pNewElement)
{
	if(Position == 0) return AddTail(pNewElement); // insert after nothing -> tail of the list
	EG_Node_t* pCurrentNode = (EG_Node_t*)Position;	// Insert it before Position
	EG_Node_t* pNewNode = NewNode(pCurrentNode, pCurrentNode->m_pNext);
	pNewNode->m_pData = pNewElement;
	if(pCurrentNode->m_pNext != nullptr) pCurrentNode->m_pNext->m_pPrev = pNewNode;
	else m_pNodeTail = pNewNode;
	pCurrentNode->m_pNext = pNewNode;
	return (POSITION) pNewNode;
}

/////////////////////////////////////////////////////////////////////////////

void EGList::RemoveAt(POSITION &rPosition)
{
int Mode = 0;

	EG_Node_t* pDeleteNode = (EG_Node_t*)rPosition;
	if(pDeleteNode == nullptr)	return;
  if(pDeleteNode->m_pPrev == nullptr) Mode += 1;  // at the head
  if(pDeleteNode->m_pNext == nullptr) Mode += 2;  // at the tail
  switch(Mode){
    case 0:{    // delete node is between the tail and head
      pDeleteNode->m_pPrev->m_pNext = pDeleteNode->m_pNext;   // join the two ends
      pDeleteNode->m_pNext->m_pPrev = pDeleteNode->m_pPrev;
      rPosition = (POSITION)pDeleteNode->m_pNext;
      break;
    }
    case 1:{    // delete node is the head
      m_pNodeHead = pDeleteNode->m_pNext;   // reassign to the head
      m_pNodeHead->m_pPrev = nullptr;
      rPosition = (POSITION)m_pNodeHead;
      break;
    }
    case 2:{    // delete node is the tail
      m_pNodeTail = pDeleteNode->m_pPrev;   // reassign to the tail
      m_pNodeTail->m_pNext = nullptr;
      rPosition = (POSITION)m_pNodeTail;
      break;
    }
    case 3:{    // only this node present 
      m_pNodeTail = nullptr;
      m_pNodeHead = nullptr;
      rPosition = nullptr;
      break;
    }
  }  
	FreeNode(pDeleteNode);
}

/////////////////////////////////////////////////////////////////////////////

POSITION EGList::FindIndexed(int nIndex) const 
{
	if(nIndex >= m_EntryCount || nIndex < 0) return 0;  // 0 base index
	EG_Node_t *pNode = m_pNodeHead;
	while(nIndex--)	pNode = pNode->m_pNext;
	return (POSITION)pNode;
}

/////////////////////////////////////////////////////////////////////////////

POSITION EGList::Find(const void *pSearchValue, POSITION StartAfter) const
{
	EG_Node_t *pNode = (EG_Node_t*)StartAfter;
	if(pNode == nullptr){
		pNode = m_pNodeHead;  // start at head
	}
	else pNode = pNode->m_pNext;  // start after the one specified
	for(; pNode != nullptr; pNode = pNode->m_pNext)	if(pNode->m_pData == pSearchValue) return (POSITION)pNode;
	return 0;
}


