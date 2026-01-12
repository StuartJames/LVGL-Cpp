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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "stdlib.h"

/////////////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------------------------
 *|        Head        |       Node n       |       Node 0       |        Tail        |
 *-------------------------------------------------------------------------------------
 *| Prev | Data | Next | Prev | Data | Next | Prev | Data | Next | Prev | Data | Next |
 *-------------------------------------------------------------------------------------
 *| Prev points to the next node towards the head. This will be null at the Head      |
 *| Next points to the next node towards the tail. This will be null at the Tail      |
 *-----------------------------------------------------------------------------------*/


/////////////////////////////////////////////////////////////////////////////

typedef struct EG_Node_t
{
	EG_Node_t *m_pNext;
	EG_Node_t *m_pPrev;
	void      *m_pData;
} EG_Node_t;

typedef EG_Node_t *POSITION;
//struct __POSITION {};
//typedef __POSITION* POSITION;

/////////////////////////////////////////////////////////////////////////////

bool IsValidAddress(const void* lp,	int nBytes, bool bReadWrite = true); 

/////////////////////////////////////////////////////////////////////////////

class EGList
{
public:
                  EGList(void);
                  ~EGList(void);
  void            Initialise(void);
 	uint32_t        GetCount(void) const;
  uint32_t        GetSize(void) const; 
	bool            IsEmpty(void) const;
	const void*     GetHead(POSITION *pPosition = nullptr);
	const void*     GetHead(POSITION &rPosition);
	const void*     GetTail(POSITION *pPosition = nullptr);
	const void*     GetTail(POSITION &rPosition);
	void*           RemoveHead(void);
	void*           RemoveTail(void);
	POSITION        AddHead(void *pNewElement);
	POSITION        AddTail(void *pNewElement);
	void            RemoveAll(void);
	POSITION        GetHeadPosition(void) const;
	POSITION        GetTailPosition(void) const;
  POSITION        GetNextPosition(POSITION &rPosition) const;
  POSITION        GetPrevPosition(POSITION &rPosition) const;
	const void*     GetNext(POSITION& rPosition) const; // return rPosition++
	const void*     GetNext(void *pElement) const;      
	const void*     GetPrev(POSITION& rPosition) const; // return rPosition--
	const void*     GetPrev(void *pElement) const;      
	const void*     GetAt(POSITION Position) const;
	void            SetAt(POSITION pos, void* pNewElement);
	void            RemoveAt(POSITION &rPosition);
	POSITION        InsertBefore(POSITION Position, void *pNewElement);
	POSITION        InsertAfter(POSITION Position, void *pNewElement);
	POSITION        Find(const void* pSearchValue, POSITION StartAfter = 0) const;
	POSITION        FindIndexed(int nIndex) const;

protected:  
	EG_Node_t*      NewNode(EG_Node_t *m_pPrev, EG_Node_t *m_pNext);
	void            FreeNode(EG_Node_t *pNode);

	EG_Node_t       *m_pNodeHead;
	EG_Node_t       *m_pNodeTail;
	uint32_t        m_EntryCount;
};

/////////////////////////////////////////////////////////////////////////////

inline void EGList::Initialise(void)
{
	m_EntryCount = 0;
	m_pNodeHead = m_pNodeTail = nullptr;
}

/////////////////////////////////////////////////////////////////////////////

inline uint32_t EGList::GetCount(void) const
{
  return m_EntryCount; 
}

/////////////////////////////////////////////////////////////////////////////

inline uint32_t EGList::GetSize(void) const
{ 
  return m_EntryCount; 
}

/////////////////////////////////////////////////////////////////////////////

inline bool EGList::IsEmpty(void) const
{ 
  return (m_EntryCount == 0) ? true : false;
}

/////////////////////////////////////////////////////////////////////////////

inline const void* EGList::GetHead(POSITION *pPosition /*= nullptr*/) 
{
  if(m_pNodeHead == nullptr) return nullptr;
  if(pPosition != nullptr) *pPosition = (POSITION)&m_pNodeHead->m_pNext;
	return m_pNodeHead->m_pData;
}

/////////////////////////////////////////////////////////////////////////////

inline const void* EGList::GetHead(POSITION &rPosition)
{
  if(m_pNodeHead == nullptr) return nullptr;
  rPosition = (POSITION)m_pNodeHead->m_pNext;
	return m_pNodeHead->m_pData;
}

/////////////////////////////////////////////////////////////////////////////

inline const void* EGList::GetTail(POSITION *pPosition /*= nullptr*/) 
{
  if(m_pNodeTail == nullptr) return nullptr;
  if(pPosition != nullptr) *pPosition = (POSITION)&m_pNodeTail->m_pPrev;
	return m_pNodeTail->m_pData;
}

/////////////////////////////////////////////////////////////////////////////

inline const void* EGList::GetTail(POSITION &rPosition) 
{
  if(m_pNodeTail == nullptr) return nullptr;
  rPosition = (POSITION)m_pNodeTail->m_pPrev;
	return m_pNodeTail->m_pData;
}

/////////////////////////////////////////////////////////////////////////////

inline POSITION EGList::GetHeadPosition(void) const
{ 
  if(m_pNodeHead == nullptr) return nullptr;
  return (POSITION)m_pNodeHead;
}

/////////////////////////////////////////////////////////////////////////////

inline POSITION EGList::GetTailPosition(void) const
{ 
  if(m_pNodeTail == nullptr) return nullptr;
  return (POSITION)m_pNodeTail;
}

/////////////////////////////////////////////////////////////////////////////

inline POSITION EGList::GetNextPosition(POSITION &rPosition) const
{ 
  EG_Node_t *pNode = (EG_Node_t*)rPosition;
	if(pNode == nullptr) rPosition = nullptr;
	else rPosition = (POSITION)pNode->m_pNext;
	return rPosition;
}

/////////////////////////////////////////////////////////////////////////////

inline POSITION EGList::GetPrevPosition(POSITION &rPosition) const
{ 
  EG_Node_t *pNode = (EG_Node_t*)rPosition;
	if(pNode == nullptr) rPosition = nullptr;
	else rPosition = (POSITION)pNode->m_pPrev;
	return rPosition;
}

/////////////////////////////////////////////////////////////////////////////

inline const void* EGList::GetNext(POSITION &rPosition) const// return rPosition++
{
  EG_Node_t *pNode = (EG_Node_t*)rPosition;
	if(pNode == nullptr) return nullptr;
	rPosition = (POSITION)pNode->m_pNext;
	return pNode->m_pData;
}

/////////////////////////////////////////////////////////////////////////////

inline const void* EGList::GetNext(void *pElement) const
{
  EG_Node_t *pNode = (EG_Node_t*)Find(pElement);
	if((pNode == nullptr) || (pNode->m_pNext == nullptr)) return nullptr;
  return pNode->m_pNext->m_pData;
}

/////////////////////////////////////////////////////////////////////////////

inline const void* EGList::GetPrev(POSITION& rPosition) const 
{
  EG_Node_t* pNode = (EG_Node_t*)rPosition;
	if(pNode == nullptr) return nullptr;
	rPosition = (POSITION)pNode->m_pPrev;
	return pNode->m_pData; 
}

/////////////////////////////////////////////////////////////////////////////

inline const void* EGList::GetPrev(void *pElement) const
{
  EG_Node_t* pNode = (EG_Node_t*)Find(pElement);
	if((pNode == nullptr) || (pNode->m_pPrev == nullptr)) return nullptr;
  return pNode->m_pPrev->m_pData;
}

/////////////////////////////////////////////////////////////////////////////

inline const void* EGList::GetAt(POSITION Position) const
{ 
  EG_Node_t* pNode = (EG_Node_t*)Position;
	if(pNode == nullptr) return nullptr;
	return pNode->m_pData;
}

/////////////////////////////////////////////////////////////////////////////

inline void EGList::SetAt(POSITION Position, void *pElement)
{ 
  EG_Node_t* pNode = (EG_Node_t*)Position;
	if(pNode != nullptr) pNode->m_pData = pElement;
}

