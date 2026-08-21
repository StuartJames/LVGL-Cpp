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

#include "widgets/EG_Button.h"
#if EG_USE_BTN != 0

#include "extra/layouts/EG_Flex.h"

///////////////////////////////////////////////////////////////////////////////////////

#define BUTTON_CLASS &c_ButtonClass

const EG_ClassType_t c_ButtonClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
  .IsEditable = 0,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGButton::EGButton(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= BUTTON_CLASS*/) : EGObject()
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButton::Configure(void)
{
  EGObject::Configure();
	ClearFlag(EG_OBJ_FLAG_SCROLLABLE);
	AddFlag(EG_OBJ_FLAG_SCROLL_ON_FOCUS);
}


#endif
