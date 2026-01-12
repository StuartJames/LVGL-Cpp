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

#include <stddef.h>

#include "misc/EG_Utilities.h"

/////////////////////////////////////////////////////////////////////////////

/** Searches pBase[0] to pBase[ElementCount - 1] for an item that matches *pKey.
 *
 * @note The function CompareFunc must return negative if its first
 *  argument (the search pKey) is less than its second (a table entry),
 *  zero if equal, and positive if greater.
 *
 *  @note Items in the array must be in ascending order.
 *
 * @param pKey    Pointer to item being searched for
 * @param pBase   Pointer to first element to search
 * @param ElementCount      Number of elements
 * @param ElementSize   Size of each element
 * @param CompareFunc    Pointer to comparison function (see #unicode_list_compare as a comparison function
 * example)
 *
 * @return a pointer to a matching item, or NULL if none exists.
 */
void* _lv_utils_bsearch(const void *pKey, const void *pBase, uint32_t ElementCount, uint32_t ElementSize, int32_t (*CompareFunc)(const void *pRef, const void *pElement))
{
const char *pMiddle;
int32_t c;

	for(pMiddle = (char *)pBase; ElementCount != 0;) {
		pMiddle += (ElementCount / 2) * ElementSize;
		if((c = (*CompareFunc)(pKey, pMiddle)) > 0) {
			ElementCount = (ElementCount / 2) - ((ElementCount & 1) == 0);
			pBase = (pMiddle += ElementSize);
		}
		else if(c < 0) {
			ElementCount /= 2;
			pMiddle = (char *)pBase;
		}
		else {
			return (char *)pMiddle;
		}
	}
	return NULL;
}
