/*
 *                LVGL++ 2026 HydraSystems.
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
 *  Based on LVGL
 *
 * =====================================================================
 *
 * Edit     Date     Version       Edit Description
 * ====  ==========  ======= ===========================================
 * SJ    2026/05/18   8.4.0    Original by LVGL 9.5.0
 *
 */

#include "misc/EG_Array.h"
#include "misc/EG_Memory.h"
//#include "stdlib/EG_string.h"

#include "misc/EG_Assert.h"

////////////////////////////////////////////////////////////////////////////////

EGArray::EGArray(uint32_t Capacity, uint32_t ElementSize) : m_Size(0)
{
	m_Capacity = Capacity;
	m_ElementSize = ElementSize;
	m_pData = (uint8_t*)EG_AllocMem(Capacity * ElementSize);
	m_LocalAllocation = true;
	EG_ASSERT_MALLOC(m_pData);
}

////////////////////////////////////////////////////////////////////////////////

EGArray::EGArray(void *pBuffer, uint32_t Capacity, uint32_t ElementSize) : m_Size(0)
{
	EG_ASSERT_NULL(pBuffer);
	m_Capacity = Capacity;
	m_ElementSize = ElementSize;
	m_pData = (uint8_t*)pBuffer;
	m_LocalAllocation = false;
}

////////////////////////////////////////////////////////////////////////////////

EGArray::~EGArray(void)
{
	if(m_pData) {
		if(m_LocalAllocation) EG_FreeMem(m_pData);
		m_pData = nullptr;
	}
	m_Size = 0;
	m_Capacity = 0;
}

////////////////////////////////////////////////////////////////////////////////

void EGArray::CopyTo(EGArray *pTarget)
{
	if(pTarget->m_pData) {
		if(pTarget->m_LocalAllocation) EG_FreeMem(pTarget->m_pData);
		pTarget->m_pData = nullptr;
	}
	pTarget->m_Capacity = m_Capacity;
	pTarget->m_ElementSize = m_ElementSize;
	m_pData = (uint8_t*)EG_AllocMem(m_Capacity * m_ElementSize);
	EG_CopyMem(pTarget->m_pData, m_pData, m_Size * m_ElementSize);
	pTarget->m_Size = m_Size;
}

//////////////////////////////////////////////////////////////////////////////////

void EGArray::operator = (const EGArray &rval)
{
	if(m_pData) {
		if(m_LocalAllocation) EG_FreeMem(m_pData);
		m_pData = nullptr;
	}
	m_Capacity = rval.m_Capacity;
	m_ElementSize = rval.m_ElementSize;
	m_pData = (uint8_t*)EG_AllocMem(m_Capacity * m_ElementSize);
	EG_CopyMem(m_pData, rval.m_pData, rval.m_Size * rval.m_ElementSize);
	m_Size = rval.m_Size;
	m_LocalAllocation = false;
}

////////////////////////////////////////////////////////////////////////////////

void EGArray::Shrink(void)
{
	if(m_Size <= m_Capacity / EG_ARRAY_DEFAULT_SHRINK_RATIO) {
		Resize(m_Size);
	}
}

////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGArray::RemoveAt(uint32_t Index)
{
	if(Index >= m_Size) return EG_RES_INVALID;
	if(Index == m_Size - 1) {	// Shortcut
		m_Size--;
		Shrink();
		return EG_RES_OK;
	}
	uint8_t *pStart = (uint8_t*)GetAt(Index);
	uint8_t *pRemaining = pStart + m_ElementSize;
	uint32_t RemainingSize = (m_Size - Index - 1) * m_ElementSize;
	EG_MoveMem(pStart, pRemaining, RemainingSize);
	m_Size--;
	Shrink();
	return EG_RES_OK;
}

////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGArray::RemoveUnordered(uint32_t Index)
{
	if(Index >= m_Size) return EG_RES_INVALID;
	if(Index == m_Size - 1) {	// Shortcut
		m_Size--;
		Shrink();
		return EG_RES_OK;
	}
	uint8_t *pDst = (uint8_t*)GetAt(Index);	// Copy the last element into the position to remove
	uint8_t *pSrc = (uint8_t*)GetAt(m_Size - 1);
	EG_CopyMem(pDst, pSrc, m_ElementSize);
	m_Size--;
	Shrink();
	return EG_RES_OK;
}

////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGArray::Erase(uint32_t Start, uint32_t End)
{
	if(End > m_Size) End = m_Size;
	if(Start >= End) return EG_RES_INVALID;
	if(End == m_Size) {	// Shortcut
		m_Size = Start;
		Shrink();
		return EG_RES_OK;
	}
	uint8_t *pStart = (uint8_t*)GetAt(Start);
	uint8_t *pRemaining = pStart + (End - Start) * m_ElementSize;
	uint32_t RemainingSize = (m_Size - End) * m_ElementSize;
	EG_MoveMem(pStart, pRemaining, RemainingSize);
	m_Size -= (End - Start);
	Shrink();
	return EG_RES_OK;
}

////////////////////////////////////////////////////////////////////////////////

bool EGArray::Resize(uint32_t NewCapacity)
{
	if(m_LocalAllocation == false) {
		EG_LOG_WARN("Cannot resize array with external buffer");
		return false;
	}
	uint8_t *pData = (uint8_t*)EG_ReallocMem(m_pData, NewCapacity * m_ElementSize);
	EG_ASSERT_NULL(pData);
	if(pData == nullptr) return false;
	m_pData = pData;
	m_Capacity = NewCapacity;
	if(m_Size > NewCapacity) {
		m_Size = NewCapacity;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGArray::Append(const EGArray *pArray)
{
	EG_ASSERT_NULL(m_pData);
	uint32_t Size = pArray->m_Size;
	if(m_Size + Size > m_Capacity) {		// array is full
		if(Resize(m_Size + Size) == false) return EG_RES_INVALID;
	}
	uint8_t *pData = m_pData + m_Size * m_ElementSize;
	EG_CopyMem(pData, pArray->m_pData, m_ElementSize * Size);
	m_Size += Size;
	return EG_RES_OK;
}

////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGArray::PushBack(const void *pElement)
{
	EG_ASSERT_NULL(m_pData);
	if(m_Size == m_Capacity) {	// array is full
		if(Resize(m_Capacity + EG_ARRAY_DEFAULT_CAPACITY) == false) {
			return EG_RES_INVALID;
		}
	}
	// * When the element is nullptr, it means that the user wants to add an empty element.
	uint8_t *pData = m_pData + m_Size * m_ElementSize;
	if(pElement) EG_CopyMem(pData, pElement, m_ElementSize);
	else EG_ZeroMem(pData, m_ElementSize);
	m_Size++;
	return EG_RES_OK;
}

////////////////////////////////////////////////////////////////////////////////

void* EGArray::GetAt( uint32_t Index)
{
	if(Index >= m_Size) return nullptr;
	EG_ASSERT_NULL(m_pData);
	return m_pData + Index * m_ElementSize;
}

////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGArray::SetAt(uint32_t Index, const void *value)
{
	uint8_t *pData = (uint8_t*)GetAt(Index);
	if(pData == nullptr) return EG_RES_INVALID;
	EG_CopyMem(pData, value, m_ElementSize);
	return EG_RES_OK;
}


