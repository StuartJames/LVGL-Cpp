/**
 * @file EG_MonoTheme
 *
 */

#pragma once

#include "core/EG_Object.h"

//////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_THEME_MONO


class EGMonoTheme : public EGTheme
{
public:
                      EGMonoTheme(void) : EGTheme(){};
  void                Initialise(EGDisplay *pDisplay, bool Dark, const EG_Font_t *pFont);
  virtual void        ApplyTheme(EGObject *pObj);

  static EGTheme*     SetTheme(bool Dark, const EG_Font_t *pFont, EGDisplay *pDisplay = nullptr);
  static bool         IsCreated(void);
  static EGTheme*     GetTheme(void);

private:
  void                InitialiseStyles(bool Dark, const EG_Font_t *pFont);

  static EGDefTheme   *m_pTheme;

  EGStyle             m_Screen;
	EGStyle             m_Card;
	EGStyle             m_Scrollbar;
	EGStyle             m_Button;
	EGStyle             m_Pressed;
	EGStyle             m_Inverted;
	EGStyle             m_Disabled;
	EGStyle             m_Focus;
	EGStyle             m_Edit;
	EGStyle             m_PadGap;
	EGStyle             m_PadZero;
	EGStyle             m_NoRadius;
	EGStyle             m_RadiusCircle;
	EGStyle             m_LargeBorder;
	EGStyle             m_LargeLineSpace;
	EGStyle             m_Underline;
#if EG_USE_TEXTAREA
	EGStyle             m_TextCursor;
#endif
};

#endif
