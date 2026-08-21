/*
 *                EGL 2025-2026 HydraSystems.
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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#pragma once

#include "EG_Types.h"

/////////////////////////////////////////////////////////////////////////////

#ifndef EG_ARRAY_DEFAULT_CAPACITY
#define EG_ARRAY_DEFAULT_CAPACITY 4
#endif

#ifndef EG_ARRAY_DEFAULT_SHRINK_RATIO
#define EG_ARRAY_DEFAULT_SHRINK_RATIO 2
#endif

/////////////////////////////////////////////////////////////////////////////

class EGArray
{
public:
	 								EGArray(uint32_t Capacity, uint32_t ElementSize);
	 								EGArray(void *pBuffer, uint32_t Capacity, uint32_t ElementSize);
								 ~EGArray(void);

	bool 						Resize(uint32_t new_capacity);
	void 						DeInit(void);
	void 						Shrink(void);
	EG_Result_t 		RemoveAt(uint32_t index);
	EG_Result_t 		RemoveUnordered(uint32_t index);
	EG_Result_t 		Erase(uint32_t start, uint32_t end);
	EG_Result_t 		Append(const EGArray *other);
	EG_Result_t 		PushBack(const void *element);
	EG_Result_t 		SetAt(uint32_t index, const void *value);
	void* 					GetAt(const uint32_t index);
	void 						CopyTo(EGArray *target);
  void            operator = (const EGArray &rval);

	uint32_t 				GetSize(void);
	uint32_t 				Capacity(void);
	bool 						IsEmpty(void);
	bool 						IsFull(void);
	void 						Clear(void);
	void*						Front(void);
	void*						Back(void);

private:
  uint8_t 			 *m_pData;
	uint32_t 				m_Size;
	uint32_t 				m_Capacity;
	uint32_t 				m_ElementSize;
	bool 						m_LocalAllocation; /* true: data is allocated by the array; false: data is allocated by the user */
};

/////////////////////////////////////////////////////////////////////////////

inline uint32_t EGArray::GetSize(void)
{
	return m_Size;
}

/////////////////////////////////////////////////////////////////////////////

inline uint32_t EGArray::Capacity(void)
{
	return m_Capacity;
}

/////////////////////////////////////////////////////////////////////////////

inline bool EGArray::IsEmpty(void)
{
	return m_Size == 0;
}

/////////////////////////////////////////////////////////////////////////////

inline bool EGArray::IsFull(void)
{
	return m_Size == m_Capacity;
}

/////////////////////////////////////////////////////////////////////////////

inline void EGArray::Clear(void)
{
	m_Size = 0;
}

/////////////////////////////////////////////////////////////////////////////

inline void* EGArray::Front(void)
{
	return GetAt(0);
}

/////////////////////////////////////////////////////////////////////////////

inline void* EGArray::Back(void)
{
	return GetAt(GetSize() - 1);
}

