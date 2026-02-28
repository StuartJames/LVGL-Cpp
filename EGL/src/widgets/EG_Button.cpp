/**
 * @file lv_btn.c
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
