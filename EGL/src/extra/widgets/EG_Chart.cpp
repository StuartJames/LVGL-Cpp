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
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "extra/widgets/EG_Chart.h"
#if EG_USE_CHART != 0

#include "misc/EG_Assert.h"

#define EG_CHART_HDIV_DEF 3
#define EG_CHART_VDIV_DEF 5
#define EG_CHART_POINT_CNT_DEF 10
#define EG_CHART_LABEL_MAX_TEXT_LENGTH 16

EGDrawRect *g_pDrawRect;
///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_ChartClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGChart::EventCB,
	.WidthDef = _EG_PCT(100),
	.HeightDef = EG_DPI_DEF * 2,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGChart::EGChart(void)
{
	m_MinY[0] = 0;
	m_MinX[0] = 0;
	m_MinY[1] = 0;
	m_MinX[1] = 0;
	m_MaxY[0] = 100;
	m_MaxX[0] = 100;
	m_MaxY[1] = 100;
	m_MaxX[1] = 100;
}

///////////////////////////////////////////////////////////////////////////////////////

EGChart::EGChart(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_ChartClass*/) : EGObject()
{
	m_MinY[0] = 0;
	m_MinX[0] = 0;
	m_MinY[1] = 0;
	m_MinX[1] = 0;
	m_MaxY[0] = 100;
	m_MaxX[0] = 100;
	m_MaxY[1] = 100;
	m_MaxX[1] = 100;
  Attach(this, pParent, pClassCnfg);
	Initialise();

}

///////////////////////////////////////////////////////////////////////////////////////

EGChart::~EGChart()
{
	EG_CharSeries_t *pSeries;
	while(!m_Series.IsEmpty()){
    pSeries = (EG_CharSeries_t*)m_Series.GetHead();
		if(!pSeries->ExtBufferUsedY) EG_FreeMem(pSeries->pPointsY);
		m_Series.RemoveHead();
		EG_FreeMem(pSeries);
	}
	EG_CharCursor_t *pCursor;
	while(!m_Cursor.IsEmpty()) {
		pCursor = (EG_CharCursor_t*)m_Cursor.GetHead();
		m_Cursor.RemoveHead();
		EG_FreeMem(pCursor);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::Configure(void)
{
  EGObject::Configure();
	m_MinY[0] = 0;
	m_MinX[0] = 0;
	m_MinY[1] = 0;
	m_MinX[1] = 0;
	m_MaxY[0] = 100;
	m_MaxX[0] = 100;
	m_MaxY[1] = 100;
	m_MaxX[1] = 100;
	m_HorizontalDivCount = EG_CHART_HDIV_DEF;
	m_VerticalDivCount = EG_CHART_VDIV_DEF;
	m_PointCount = EG_CHART_POINT_CNT_DEF;
	m_SelectPointIndex = EG_CHART_POINT_NONE;
	m_ChartType = EG_CHART_TYPE_LINE;
	m_UpdateMode = EG_CHART_UPDATE_MODE_SHIFT;
	m_ZoomX = EG_SCALE_NONE;
	m_ZoomY = EG_SCALE_NONE;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetType(EG_ChartType_e Type)
{
POSITION Position;

	if(m_ChartType == Type) return;
	if(m_ChartType == EG_CHART_TYPE_SCATTER) {
		EG_CharSeries_t *pSeries;
		for(pSeries = (EG_CharSeries_t*)m_Series.GetTail(Position); pSeries != nullptr; pSeries = (EG_CharSeries_t*)m_Series.GetPrev(Position)){
			EG_FreeMem(pSeries->pPointsX);
			pSeries->pPointsX = nullptr;
		}
	}
	if(Type == EG_CHART_TYPE_SCATTER) {
		EG_CharSeries_t *pSeries;
		for(pSeries = (EG_CharSeries_t*)m_Series.GetTail(Position); pSeries != nullptr; pSeries = (EG_CharSeries_t*)m_Series.GetPrev(Position)){
			pSeries->pPointsX = (EG_Coord_t*)EG_AllocMem(sizeof(EGPoint) * m_PointCount);
			EG_ASSERT_MALLOC(pSeries->pPointsX);
			if(pSeries->pPointsX == nullptr) return;
		}
	}
	m_ChartType = Type;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetPointCount(uint16_t Count)
{
POSITION Position;

	if(m_PointCount == Count) return;
	EG_CharSeries_t *pSeries;
	if(Count < 1) Count = 1;
	for(pSeries = (EG_CharSeries_t*)m_Series.GetTail(Position); pSeries != nullptr; pSeries = (EG_CharSeries_t*)m_Series.GetPrev(Position)){
		if(m_ChartType == EG_CHART_TYPE_SCATTER) {
			if(!pSeries->ExtBufferUsedX) AllocateNewPoints(pSeries, Count, &pSeries->pPointsX);
		}
		if(!pSeries->ExtBufferUsedY) AllocateNewPoints(pSeries, Count, &pSeries->pPointsY);
		pSeries->StartPoint = 0;
	}
	m_PointCount = Count;

	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetRange(EG_ChartAxis_e Axis, EG_Coord_t Min, EG_Coord_t Max)
{
	Max = Max == Min ? Max + 1 : Max;
	switch(Axis) {
		case EG_CHART_AXIS_PRIMARY_Y:
			m_MinY[0] = Min;
			m_MaxY[0] = Max;
			break;
		case EG_CHART_AXIS_SECONDARY_Y:
			m_MinY[1] = Min;
			m_MaxY[1] = Max;
			break;
		case EG_CHART_AXIS_PRIMARY_X:
			m_MinX[0] = Min;
			m_MaxX[0] = Max;
			break;
		case EG_CHART_AXIS_SECONDARY_X:
			m_MinX[1] = Min;
			m_MaxX[1] = Max;
			break;
		default:
			EG_LOG_WARN("Invalid axis: %d", Axis);
			return;
	}
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetUpdateMode(EG_ChartMode_e UpdateMode)
{
	if(m_UpdateMode == UpdateMode) return;
	m_UpdateMode = UpdateMode;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetDivLineCount(uint8_t HorizontalDiv, uint8_t VerticalDiv)
{
	if(m_HorizontalDivCount == HorizontalDiv && m_VerticalDivCount == VerticalDiv) return;
	m_HorizontalDivCount = HorizontalDiv;
	m_VerticalDivCount = VerticalDiv;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetZoomX(uint16_t ZoomX)
{
	if(m_ZoomX == ZoomX) return;

	m_ZoomX = ZoomX;
	RefreshSelfSize();
	ReadjustScroll(EG_ANIM_OFF);	// Be the chart doesn't remain scrolled out
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetZoomY(uint16_t ZoomY)
{
	if(m_ZoomY == ZoomY) return;
	m_ZoomY = ZoomY;
	RefreshSelfSize();
	ReadjustScroll(EG_ANIM_OFF);	// Be the chart doesn't remain scrolled out
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGChart::GetZoomX(void)
{
	return m_ZoomX;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGChart::GetZoomY(void)
{
	return m_ZoomY;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetAxisTick(EG_ChartAxis_e Axis, EG_Coord_t MajorLength, EG_Coord_t MinorLength,
                              EG_Coord_t MajorCount, EG_Coord_t MinorCount, bool LabelEnable, EG_Coord_t DrawSize)
{
	EG_CharTickDSC_t *pTick = GetTickDSC(Axis);
	pTick->MajorLength = MajorLength;
	pTick->MinorLength = MinorLength;
	pTick->MajorCount = MajorCount;
	pTick->MinorCount = MinorCount;
	pTick->LabelEnable = LabelEnable;
	pTick->DrawSize = DrawSize;
	RefreshExtDrawSize();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_ChartType_e EGChart::GetType(void)
{
	return m_ChartType;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGChart::GetPointCount(void)
{
	return m_PointCount;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGChart::GetStartPointX(EG_CharSeries_t *pSeries)
{
	return m_UpdateMode == EG_CHART_UPDATE_MODE_SHIFT ? pSeries->StartPoint : 0;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::GetPointPosByIndex(EG_CharSeries_t *pSeries, uint16_t Index, EGPoint *pPoint)
{
	EG_ASSERT_NULL(pSeries);
	if(Index >= m_PointCount) {
		EG_LOG_WARN("Invalid index: %d", Index);
		pPoint->m_X = 0;
		pPoint->m_Y = 0;
		return;
	}
  EG_Coord_t ChartWidth = ((int32_t)GetContentWidth() * m_ZoomX) >> 8;
	EG_Coord_t ChartHeight = ((int32_t)GetContentHeight() * m_ZoomY) >> 8;
	if(m_ChartType == EG_CHART_TYPE_LINE) {
		pPoint->m_X = (ChartWidth * Index) / (m_PointCount - 1);
	}
	else if(m_ChartType == EG_CHART_TYPE_SCATTER) {
		pPoint->m_X = EG_Map(pSeries->pPointsX[Index], m_MinX[pSeries->AxisSecX], m_MaxX[pSeries->AxisSecX], 0, ChartWidth);
	}
	else if(m_ChartType == EG_CHART_TYPE_BAR) {
		uint32_t SeriesCount = m_Series.GetSize();
		int32_t SeriesGap = ((int32_t)GetStylePadColumn(EG_PART_ITEMS) * m_ZoomX) >> 8;		// Gap between the column on the X m_Tick
		int32_t BlockGap = ((int32_t)GetStylePadColumn(EG_PART_MAIN) * m_ZoomX) >> 8;	// Gap between the columns on adjacent X ticks
		EG_Coord_t BlockWidth = (ChartWidth - ((m_PointCount - 1) * BlockGap)) / m_PointCount;
		EG_CharSeries_t *ser_i = nullptr;
		uint32_t ser_idx = 0;
    POSITION Pos;
		for(ser_i = (EG_CharSeries_t*)m_Series.GetTail(Pos); ser_i != nullptr; ser_i = (EG_CharSeries_t*)m_Series.GetPrev(Pos)){
			if(ser_i == pSeries) break;
			ser_idx++;
		}
		pPoint->m_X = (int32_t)((int32_t)(ChartWidth + BlockGap) * Index) / m_PointCount;
		pPoint->m_X += BlockWidth * ser_idx / SeriesCount;
		EG_Coord_t ColumnWidth = (BlockWidth - (SeriesGap * (SeriesCount - 1))) / SeriesCount;
		pPoint->m_X += ColumnWidth / 2;
	}

	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	pPoint->m_X += GetStylePadLeft(EG_PART_MAIN) + BorderWidth;
	pPoint->m_X -= GetScrollLeft();
	uint32_t StartPoint = GetStartPointX(pSeries);
	Index = ((int32_t)StartPoint + Index) % m_PointCount;
	int32_t temp_y = 0;
	temp_y = (int32_t)((int32_t)pSeries->pPointsY[Index] - m_MinY[pSeries->AxisSecY]) * ChartHeight;
	temp_y = temp_y / (m_MaxY[pSeries->AxisSecY] - m_MinY[pSeries->AxisSecY]);
	pPoint->m_Y = ChartHeight - temp_y;
	pPoint->m_Y += GetStylePadTop(EG_PART_MAIN) + BorderWidth;
	pPoint->m_Y -= GetScrollTop();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::Refresh(void)
{
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_CharSeries_t* EGChart::AddSeries(EG_Color_t Color, EG_ChartAxis_e Axis)
{
  EG_CharSeries_t *pSeries = (EG_CharSeries_t*)EG_AllocMem(sizeof(EG_CharSeries_t));
	EG_ASSERT_MALLOC(pSeries);
	if(pSeries == nullptr) return nullptr;
	pSeries->Color = Color;
	pSeries->pPointsY = (EG_Coord_t*)EG_AllocMem(sizeof(EG_Coord_t) * m_PointCount);
	EG_ASSERT_MALLOC(pSeries->pPointsY);
	if(m_ChartType == EG_CHART_TYPE_SCATTER) {
		pSeries->pPointsX = (EG_Coord_t*)EG_AllocMem(sizeof(EG_Coord_t) * m_PointCount);
		EG_ASSERT_MALLOC(pSeries->pPointsX);
	}
	if(pSeries->pPointsY == nullptr) {
		EG_FreeMem(pSeries);
		return nullptr;
	}
	pSeries->StartPoint = 0;
	pSeries->ExtBufferUsedY = false;
	pSeries->Hidden = 0;
	pSeries->AxisSecX = Axis & EG_CHART_AXIS_SECONDARY_X ? 1 : 0;
	pSeries->AxisSecY = Axis & EG_CHART_AXIS_SECONDARY_Y ? 1 : 0;
	m_Series.AddHead(pSeries);
	EG_Coord_t *pTemp = pSeries->pPointsY;
	EG_Coord_t DefValue = EG_CHART_POINT_NONE;
	for(uint16_t i = 0; i < m_PointCount; i++) {
		*pTemp = DefValue;
		pTemp++;
	}
	return pSeries;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::RemoveSeries(EG_CharSeries_t *pSeries)
{
	EG_ASSERT_NULL(pSeries);
	if(!pSeries->ExtBufferUsedY && pSeries->pPointsY) EG_FreeMem(pSeries->pPointsY);
	if(!pSeries->ExtBufferUsedX && pSeries->pPointsX) EG_FreeMem(pSeries->pPointsX);
  POSITION Pos = m_Series.Find(pSeries);
  if(Pos != nullptr) m_Series.RemoveAt(Pos);
	EG_FreeMem(pSeries);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::HideSeries(EG_CharSeries_t *pSeries, bool Hide)
{
	EG_ASSERT_NULL(pSeries);
	pSeries->Hidden = Hide ? 1 : 0;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetSeriesColor(EG_CharSeries_t *pSeries, EG_Color_t Color)
{
	EG_ASSERT_NULL(pSeries);
	pSeries->Color = Color;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetStartPointX(EG_CharSeries_t *pSeries, uint16_t Index)
{
	EG_ASSERT_NULL(pSeries);
	if(Index >= m_PointCount) return;
	pSeries->StartPoint = Index;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_CharSeries_t* EGChart::GetSeriesNext(const EG_CharSeries_t *pSeries)
{
POSITION Pos;

	if(pSeries != nullptr){
	  if((Pos = m_Series.Find(pSeries)) != 0 ) return (EG_CharSeries_t*)m_Series.GetNext(Pos);
  }
  return (EG_CharSeries_t*)m_Series.GetHead();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_CharCursor_t* EGChart::AddCursor(EG_Color_t Color, EG_DirType_e Direction)
{
	EG_CharCursor_t *pCursor = (EG_CharCursor_t*)m_Cursor.GetHead();
	EG_ASSERT_MALLOC(pCursor);
	if(pCursor == nullptr) return nullptr;
	pCursor->Position.m_X = EG_CHART_POINT_NONE;
	pCursor->Position.m_Y = EG_CHART_POINT_NONE;
	pCursor->PointIndex = EG_CHART_POINT_NONE;
	pCursor->PositionSet = 0;
	pCursor->Color = Color;
	pCursor->Direction = Direction;
	return pCursor;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetCursorPos(EG_CharCursor_t *pCursor, EGPoint *pPosition)
{
	EG_ASSERT_NULL(pCursor);
	pCursor->Position.m_X = pPosition->m_X;
	pCursor->Position.m_Y = pPosition->m_Y;
	pCursor->PositionSet = 1;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetCursorPoint(EG_CharCursor_t *pCursor, EG_CharSeries_t *pSeries, uint16_t PointIndex)
{
	EG_ASSERT_NULL(pCursor);
	pCursor->PointIndex = PointIndex;
	pCursor->PositionSet = 0;
	if(pSeries == nullptr) pSeries = GetSeriesNext(nullptr);
	pCursor->pSeries = pSeries;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

EGPoint EGChart::GetCursorPoint(EG_CharCursor_t *pCursor)
{
	EG_ASSERT_NULL(pCursor);
	return pCursor->Position;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetAllValue(EG_CharSeries_t *pSeries, EG_Coord_t Value)
{
	EG_ASSERT_NULL(pSeries);
	for(uint16_t i = 0; i < m_PointCount; i++) {
		pSeries->pPointsY[i] = Value;
	}
	pSeries->StartPoint = 0;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetNextValue(EG_CharSeries_t *pSeries, EG_Coord_t Value)
{
	EG_ASSERT_NULL(pSeries);
	pSeries->pPointsY[pSeries->StartPoint] = Value;
	InvalidatePoint(pSeries->StartPoint);
	pSeries->StartPoint = (pSeries->StartPoint + 1) % m_PointCount;
	InvalidatePoint(pSeries->StartPoint);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetNextValues(EG_CharSeries_t *pSeries, EG_Coord_t ValueX, EG_Coord_t ValueY)
{
	EG_ASSERT_NULL(pSeries);
	if(m_ChartType != EG_CHART_TYPE_SCATTER) {
		EG_LOG_WARN("Type must be EG_CHART_TYPE_SCATTER");
		return;
	}
	pSeries->pPointsX[pSeries->StartPoint] = ValueX;
	pSeries->pPointsY[pSeries->StartPoint] = ValueY;
	pSeries->StartPoint = (pSeries->StartPoint + 1) % m_PointCount;
	InvalidatePoint(pSeries->StartPoint);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetValueByIndex(EG_CharSeries_t *pSeries, uint16_t Index, EG_Coord_t Value)
{
	EG_ASSERT_NULL(pSeries);
	if(Index >= m_PointCount) return;
	pSeries->pPointsY[Index] = Value;
	InvalidatePoint(Index);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetValuesByIndex(EG_CharSeries_t *pSeries, uint16_t Index, EG_Coord_t ValueX, EG_Coord_t ValueY)
{
	EG_ASSERT_NULL(pSeries);
	if(m_ChartType != EG_CHART_TYPE_SCATTER) {
		EG_LOG_WARN("Type must be EG_CHART_TYPE_SCATTER");
		return;
	}
	if(Index >= m_PointCount) return;
	pSeries->pPointsX[Index] = ValueX;
	pSeries->pPointsY[Index] = ValueY;
	InvalidatePoint(Index);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetExtArrayY(EG_CharSeries_t *pSeries, EG_Coord_t Array[])
{
	EG_ASSERT_NULL(pSeries);
	if(!pSeries->ExtBufferUsedY && pSeries->pPointsY) EG_FreeMem(pSeries->pPointsY);
	pSeries->ExtBufferUsedY = true;
	pSeries->pPointsY = Array;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::SetExtArrayX(EG_CharSeries_t *pSeries, EG_Coord_t Array[])
{
	EG_ASSERT_NULL(pSeries);
	if(!pSeries->ExtBufferUsedX && pSeries->pPointsX) EG_FreeMem(pSeries->pPointsX);
	pSeries->ExtBufferUsedX = true;
	pSeries->pPointsX = Array;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t* EGChart::GetArrayY(EG_CharSeries_t *pSeries)
{
	EG_ASSERT_NULL(pSeries);
	return pSeries->pPointsY;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t* EGChart::GetArrayX(EG_CharSeries_t *pSeries)
{
	EG_ASSERT_NULL(pSeries);
	return pSeries->pPointsX;
}

///////////////////////////////////////////////////////////////////////////////////////

uint32_t EGChart::GetSelectPoint(void)
{
	return m_SelectPointIndex;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(&c_ChartClass) != EG_RES_OK) return;// Call the ancestor's event handler
	EGChart *pChart = (EGChart*)pEvent->GetTarget();
  pChart->Event(pEvent);  // Dereference once
}

void EGChart::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_PRESSED: {
      EGInputDevice *pInput = EGInputDevice::GetActive();
      EGPoint Point;
      pInput->GetPoint(&Point);
      Point.m_X -= m_Rect.GetX1();
      uint32_t Index = GetIndexFromX(Point.m_X + GetScrollLeft());
      if(Index != (uint32_t)m_SelectPointIndex) {
        InvalidatePoint(Index);
        InvalidatePoint(m_SelectPointIndex);
        m_SelectPointIndex = Index;
        EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr);
      }
      break;
    }
    case EG_EVENT_RELEASED: {
      InvalidatePoint(m_SelectPointIndex);
      m_SelectPointIndex = EG_CHART_POINT_NONE;
      break;
    }
    case EG_EVENT_SIZE_CHANGED: {
      RefreshSelfSize();
      break;
    }
    case EG_EVENT_REFR_EXT_DRAW_SIZE: {
      pEvent->SetExtDrawSize(EG_MAX4(m_Tick[0].DrawSize, m_Tick[1].DrawSize, m_Tick[2].DrawSize, m_Tick[3].DrawSize));
      break;
    }
    case EG_EVENT_GET_SELF_SIZE: {
      EGPoint *Point = (EGPoint*)pEvent->GetParam();
      Point->m_X = ((int32_t)GetContentWidth() * m_ZoomX) >> 8;
      Point->m_Y = ((int32_t)GetContentHeight() * m_ZoomY) >> 8;
      break;
    }
    case EG_EVENT_DRAW_MAIN: {
      EGDrawContext *pContext = pEvent->GetDrawContext();
      DrawTickLines(pContext);
      DrawAxis(pContext);
      if(!m_Series.IsEmpty()) {
        if(m_ChartType == EG_CHART_TYPE_LINE) DrawLineSeries(pContext);
        else if(m_ChartType == EG_CHART_TYPE_BAR) DrawBarSeries(pContext);
        else if(m_ChartType == EG_CHART_TYPE_SCATTER) DrawScatterSeries(pContext);
      }
      DrawCursors(pContext);
      break;
    }
    default:{
      break;  // do nothing
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::DrawTickLines(EGDrawContext *pContext)
{
EGRect SeriesClip;
int16_t i, TickStart, TickEnd;

	if(!SeriesClip.Intersect(&m_Rect, pContext->m_pClipRect)) return;
	const EGRect *pOriginalClipRect = pContext->m_pClipRect;
	pContext->m_pClipRect = &SeriesClip;
	EGPoint Point1, Point2;
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_BorderSide_t BorderSide = GetStyleBorderSide(EG_PART_MAIN);
	EG_OPA_t BorderOPA = GetStyleBorderOPA(EG_PART_MAIN);
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t ChartWidth = ((int32_t)GetContentWidth() * m_ZoomX) >> 8;
	EG_Coord_t ChartHeight = ((int32_t)GetContentHeight() * m_ZoomY) >> 8;

	EGDrawLine DrawLine;
	InititialseDrawLine(EG_PART_MAIN, &DrawLine);
	EGDrawDiscriptor DrawDiscriptor;
	InitDrawDescriptor(&DrawDiscriptor, pContext);
	DrawDiscriptor.m_Part = EG_PART_MAIN;
	DrawDiscriptor.m_pClass = &c_ChartClass;
	DrawDiscriptor.m_Type = EG_CHART_DRAW_PART_DIV_LINE_INIT;
	DrawDiscriptor.m_pDrawLine = &DrawLine;
	DrawDiscriptor.m_Index = 0xFFFFFFFF;
	DrawDiscriptor.m_pPoint1 = nullptr;
	DrawDiscriptor.m_pPoint2 = nullptr;
	EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);

	EG_Coord_t ScrollLeft = GetScrollLeft();
	EG_Coord_t ScrollTop = GetScrollTop();
	if(m_HorizontalDivCount != 0) {
		EG_Coord_t OffsetY = m_Rect.GetY1() + PadTop - ScrollTop;
		Point1.m_X = m_Rect.GetX1();
		Point2.m_X = m_Rect.GetX2();
		TickStart = 0;
		TickEnd = m_HorizontalDivCount;
		if(BorderOPA > EG_OPA_MIN && BorderWidth > 0) {
			if((BorderSide & EG_BORDER_SIDE_TOP) && (GetStylePadTop(EG_PART_MAIN) == 0)) TickStart++;
			if((BorderSide & EG_BORDER_SIDE_BOTTOM) && (GetStylePadBottom(EG_PART_MAIN) == 0)) TickEnd--;
		}
		for(i = TickStart; i < TickEnd; i++) {
			Point1.m_Y = (int32_t)((int32_t)ChartHeight * i) / (m_HorizontalDivCount - 1);
			Point1.m_Y += OffsetY;
			Point2.m_Y = Point1.m_Y;
			DrawDiscriptor.m_pClass = &c_ChartClass;
			DrawDiscriptor.m_Type = EG_CHART_DRAW_PART_DIV_LINE_HOR;
			DrawDiscriptor.m_pPoint1 = &Point1;
			DrawDiscriptor.m_pPoint2 = &Point2;
			DrawDiscriptor.m_Index = i;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
			DrawLine.Draw(pContext, &Point1, &Point2);
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
		}
	}
	if(m_VerticalDivCount != 0) {
		EG_Coord_t OffsetX = m_Rect.GetX1() + PadLeft - ScrollLeft;
		Point1.m_Y = m_Rect.GetY1();
		Point2.m_Y = m_Rect.GetY2();
		TickStart = 0;
		TickEnd = m_VerticalDivCount;
		if(BorderOPA > EG_OPA_MIN && BorderWidth > 0) {
			if((BorderSide & EG_BORDER_SIDE_LEFT) && (GetStylePadLeft(EG_PART_MAIN) == 0)) TickStart++;
			if((BorderSide & EG_BORDER_SIDE_RIGHT) && (GetStylePadRight(EG_PART_MAIN) == 0)) TickEnd--;
		}
		for(i = TickStart; i < TickEnd; i++) {
			Point1.m_X = (int32_t)((int32_t)ChartWidth * i) / (m_VerticalDivCount - 1);
			Point1.m_X += OffsetX;
			Point2.m_X = Point1.m_X;
			DrawDiscriptor.m_pClass = &c_ChartClass;
			DrawDiscriptor.m_Type = EG_CHART_DRAW_PART_DIV_LINE_VER;
			DrawDiscriptor.m_pPoint1 = &Point1;
			DrawDiscriptor.m_pPoint2 = &Point2;
			DrawDiscriptor.m_Index = i;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
			DrawLine.Draw(pContext, &Point1, &Point2);
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
		}
	}
	DrawDiscriptor.m_Index = 0xFFFFFFFF;
	DrawDiscriptor.m_pPoint1 = nullptr;
	DrawDiscriptor.m_pPoint2 = nullptr;
	EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
	pContext->m_pClipRect = pOriginalClipRect;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::DrawAxis(EGDrawContext *pContext)
{
	DrawAxisY(pContext, EG_CHART_AXIS_PRIMARY_Y);
	DrawAxisY(pContext, EG_CHART_AXIS_SECONDARY_Y);
	DrawAxisX(pContext, EG_CHART_AXIS_PRIMARY_X);
	DrawAxisX(pContext, EG_CHART_AXIS_SECONDARY_X);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::DrawLineSeries(EGDrawContext *pContext)
{
EGRect SeriesClip;

	if(!SeriesClip.Intersect(&m_Rect, pContext->m_pClipRect)) return;
	const EGRect *pOriginalClipRect = pContext->m_pClipRect;
	pContext->m_pClipRect = &SeriesClip;
	if(m_PointCount < 2) return;
	uint16_t i;
	EGPoint Point1, Point2;
	EG_CharSeries_t *pSeries;
	EGDrawLine DrawLine;
	EGDrawRect DrawRect;

  EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t ChartWidth = ((int32_t)GetContentWidth() * m_ZoomX) >> 8;
	EG_Coord_t ChartHeight = ((int32_t)GetContentHeight() * m_ZoomY) >> 8;
	EG_Coord_t OffsetX = m_Rect.GetX1() + PadLeft - GetScrollLeft();
	EG_Coord_t OffsetY = m_Rect.GetY1() + PadTop - GetScrollTop();

	InititialseDrawLine(EG_PART_ITEMS, &DrawLine);
	InititialseDrawRect(EG_PART_INDICATOR, &DrawRect);
	EG_Coord_t PointWidth = GetStyleWidth(EG_PART_INDICATOR) / 2;
	EG_Coord_t PointHeight = GetStyleHeight(EG_PART_INDICATOR) / 2;
	// Do not bother with line ending if the point will be over it
	if(EG_MIN(PointWidth, PointHeight) > DrawLine.m_Width / 2) DrawLine.m_RawEnd = 1;
	if(DrawLine.m_Width == 1) DrawLine.m_RawEnd = 1;
	// If there are at least as many points as pixels then draw only vertical lines
	bool DenseMode = m_PointCount >= ChartWidth ? true : false;
	// Go through all data lines
  POSITION Pos;
//  ESP_LOGI("[Chart ]", "Clip Y1:%d,Y2:%d, Series Y1:%d,Y2:%d", pOriginalClipRect->GetY1(), pOriginalClipRect->GetY2(), SeriesClip.GetY1(), SeriesClip.GetY2());
	for(pSeries = (EG_CharSeries_t*)m_Series.GetTail(Pos); pSeries != nullptr; pSeries = (EG_CharSeries_t*)m_Series.GetPrev(Pos)){
		if(pSeries->Hidden) continue;
		DrawLine.m_Color = pSeries->Color;
		DrawRect.m_BackgroundColor = pSeries->Color;
		EG_Coord_t StartPoint = GetStartPointX(pSeries);
		Point1.m_X = OffsetX;
		Point2.m_X = OffsetX;
		EG_Coord_t PointIndex = StartPoint;
		EG_Coord_t PreviousPoint = StartPoint;
		int32_t CalcY = (int32_t)((int32_t)pSeries->pPointsY[PreviousPoint] - m_MinY[pSeries->AxisSecY]) * ChartHeight;
		CalcY = CalcY / (m_MaxY[pSeries->AxisSecY] - m_MinY[pSeries->AxisSecY]);
		Point2.m_Y = ChartHeight - CalcY + OffsetY;
		EGDrawDiscriptor DrawDiscriptor;
		InitDrawDescriptor(&DrawDiscriptor, pContext);
		DrawDiscriptor.m_pClass = &c_ChartClass;
		DrawDiscriptor.m_Type = EG_CHART_DRAW_PART_LINE_AND_POINT;
		DrawDiscriptor.m_Part = EG_PART_ITEMS;
		DrawDiscriptor.m_pDrawLine = &DrawLine;
		DrawDiscriptor.m_pDrawRect = &DrawRect;
		DrawDiscriptor.m_pSubPart = pSeries;
		EG_Coord_t MinY = Point2.m_Y;
		EG_Coord_t MaxY = Point2.m_Y;
		for(i = 0; i < m_PointCount; i++) {
			Point1.m_X = Point2.m_X;
			Point1.m_Y = Point2.m_Y;
			if(Point1.m_X > pOriginalClipRect->GetX2() + PointWidth + 1) break;
			Point2.m_X = ((ChartWidth * i) / (m_PointCount - 1)) + OffsetX;
			PointIndex = (StartPoint + i) % m_PointCount;
			CalcY = (int32_t)((int32_t)pSeries->pPointsY[PointIndex] - m_MinY[pSeries->AxisSecY]) * ChartHeight;
			CalcY = CalcY / (m_MaxY[pSeries->AxisSecY] - m_MinY[pSeries->AxisSecY]);
			Point2.m_Y = ChartHeight - CalcY + OffsetY;
			if(Point2.m_X < pOriginalClipRect->GetX1() - PointWidth - 1) {
				PreviousPoint = PointIndex;
				continue;
			}
			if(i != 0) {		// Don't draw the first point. A second point is also required to draw the line
				if(DenseMode) {
					if(pSeries->pPointsY[PreviousPoint] != EG_CHART_POINT_NONE && pSeries->pPointsY[PointIndex] != EG_CHART_POINT_NONE) {
						// Draw only one vertical line between the Min and Max y-values on the same x-value
						MaxY = EG_MAX(MaxY, Point2.m_Y);
						MinY = EG_MIN(MinY, Point2.m_Y);
						if(Point1.m_X != Point2.m_X) {
							EG_Coord_t LastY = Point2.m_Y;
							Point2.m_X--; // It's already on the next x value
							Point1.m_X = Point2.m_X;
							Point1.m_Y = MinY;
							Point2.m_Y = MaxY;
							if(Point1.m_Y == Point2.m_Y) Point2.m_Y++; // If they are the same no line will be drawn
							DrawLine.Draw(pContext, &Point1, &Point2);
							Point2.m_X++;        // Compensate the previous x--
							MinY = LastY; // Start the line of the next x from the current last y
							MaxY = LastY;
						}
					}
				}
				else {
					EGRect PointRect;
					PointRect.Set(Point1.m_X - PointWidth, Point1.m_Y - PointHeight, Point1.m_X + PointWidth, Point1.m_Y + PointHeight);
					DrawDiscriptor.m_Index = i - 1;
					DrawDiscriptor.m_pPoint1 = pSeries->pPointsY[PreviousPoint] != EG_CHART_POINT_NONE ? &Point1 : nullptr;
					DrawDiscriptor.m_pPoint2 = pSeries->pPointsY[PointIndex] != EG_CHART_POINT_NONE ? &Point2 : nullptr;
					DrawDiscriptor.m_pRect = &PointRect;
					DrawDiscriptor.m_Value = pSeries->pPointsY[PreviousPoint];
					EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
					if(pSeries->pPointsY[PreviousPoint] != EG_CHART_POINT_NONE && pSeries->pPointsY[PointIndex] != EG_CHART_POINT_NONE) {
						DrawLine.Draw(pContext, &Point1, &Point2);
					}
					if(PointWidth && PointHeight && pSeries->pPointsY[PreviousPoint] != EG_CHART_POINT_NONE) {
//    ESP_LOGI("[Chart ]", "RY1:%d, RY2:%d, CY1:%d, CY2:%d", PointRect.GetY1(), PointRect.GetY2(), pOriginalClipRect->GetY1(), pOriginalClipRect->GetY2());
        g_pItemObj = this;
						DrawRect.Draw(pContext, &PointRect);
					}
					EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
				}
			}
			PreviousPoint = PointIndex;
		}
		// Draw the last point
		if(!DenseMode && i == m_PointCount) {
			if(pSeries->pPointsY[PointIndex] != EG_CHART_POINT_NONE) {
				EGRect PointRect(Point2.m_X - PointWidth, Point2.m_Y - PointHeight, Point2.m_X + PointWidth, Point2.m_Y + PointHeight);
				DrawDiscriptor.m_Index = i - 1;
				DrawDiscriptor.m_pPoint1 = nullptr;
				DrawDiscriptor.m_pPoint2 = nullptr;
				DrawDiscriptor.m_pRect = &PointRect;
				DrawDiscriptor.m_Value = pSeries->pPointsY[PointIndex];
				EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
				DrawRect.Draw(pContext, &PointRect);
				EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
			}
		}
	}
	pContext->m_pClipRect = pOriginalClipRect;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::DrawScatterSeries(EGDrawContext *pContext)
{
	EGRect ClipRect;
	if(ClipRect.Intersect(&m_Rect, pContext->m_pClipRect) == false) return;
	const EGRect *pOriginalClipRect = pContext->m_pClipRect;
	pContext->m_pClipRect = &ClipRect;
	uint16_t i;
	EGPoint Point1, Point2;
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t ChartWidth = ((int32_t)GetContentWidth() * m_ZoomX) >> 8;
	EG_Coord_t ChartHeight = ((int32_t)GetContentHeight() * m_ZoomY) >> 8;
	EG_Coord_t OffsetX = m_Rect.GetX1() + PadLeft + BorderWidth - GetScrollLeft();
	EG_Coord_t OffsetY = m_Rect.GetY1() + PadTop + BorderWidth - GetScrollTop();
	EG_CharSeries_t *pSeries;
	EGDrawLine DrawLine;
	InititialseDrawLine(EG_PART_ITEMS, &DrawLine);
	EGDrawRect DrawRect;
	InititialseDrawRect(EG_PART_INDICATOR, &DrawRect);
	EG_Coord_t PointWidth = GetStyleWidth(EG_PART_INDICATOR) / 2;
	EG_Coord_t PointHeight = GetStyleHeight(EG_PART_INDICATOR) / 2;
	// Do not bother with line ending is the point will over it
	if(EG_MIN(PointWidth, PointHeight) > DrawLine.m_Width / 2) DrawLine.m_RawEnd = 1;
	if(DrawLine.m_Width == 1) DrawLine.m_RawEnd = 1;

	// Go through all data lines
  POSITION Pos;
	for(pSeries = (EG_CharSeries_t*)m_Series.GetTail(Pos); pSeries != nullptr; pSeries = (EG_CharSeries_t*)m_Series.GetPrev(Pos)){
		if(pSeries->Hidden) continue;
		DrawLine.m_Color = pSeries->Color;
		DrawRect.m_BackgroundColor = pSeries->Color;
		EG_Coord_t StartPoint = GetStartPointX(pSeries);
		Point1.m_X = OffsetX;
		Point2.m_X = OffsetX;
		EG_Coord_t PointIndex = StartPoint;
		EG_Coord_t PreviousPoint = StartPoint;
		if(pSeries->pPointsY[PointIndex] != EG_CHART_POINT_CNT_DEF) {
			Point2.m_X = EG_Map(pSeries->pPointsX[PointIndex], m_MinX[pSeries->AxisSecX], m_MaxX[pSeries->AxisSecX], 0, ChartWidth);
			Point2.m_X += OffsetX;
			Point2.m_Y = EG_Map(pSeries->pPointsY[PointIndex], m_MinY[pSeries->AxisSecY], m_MaxY[pSeries->AxisSecY], 0, ChartHeight);
			Point2.m_Y = ChartHeight - Point2.m_Y;
			Point2.m_Y += OffsetY;
		}
		else {
			Point2.m_X = EG_COORD_MIN;
			Point2.m_Y = EG_COORD_MIN;
		}
		EGDrawDiscriptor DrawDiscriptor;
		InitDrawDescriptor(&DrawDiscriptor, pContext);
		DrawDiscriptor.m_Part = EG_PART_ITEMS;
		DrawDiscriptor.m_pClass = &c_ChartClass;
		DrawDiscriptor.m_Type = EG_CHART_DRAW_PART_LINE_AND_POINT;
		DrawDiscriptor.m_pDrawLine = &DrawLine;
		DrawDiscriptor.m_pDrawRect = &DrawRect;
		DrawDiscriptor.m_pSubPart = pSeries;
		for(i = 0; i < m_PointCount; i++) {
			Point1.m_X = Point2.m_X;
			Point1.m_Y = Point2.m_Y;
			PointIndex = (StartPoint + i) % m_PointCount;
			if(pSeries->pPointsY[PointIndex] != EG_CHART_POINT_NONE) {
				Point2.m_Y = EG_Map(pSeries->pPointsY[PointIndex], m_MinY[pSeries->AxisSecY], m_MaxY[pSeries->AxisSecY], 0, ChartHeight);
				Point2.m_Y = ChartHeight - Point2.m_Y;
				Point2.m_Y += OffsetY;

				Point2.m_X = EG_Map(pSeries->pPointsX[PointIndex], m_MinX[pSeries->AxisSecX], m_MaxX[pSeries->AxisSecX], 0, ChartWidth);
				Point2.m_X += OffsetX;
			}
			else {
				PreviousPoint = PointIndex;
				continue;
			}
			// Don't draw the first point. A second point is also required to draw the line
			if(i != 0) {
				EGRect PointRect(Point1.m_X - PointWidth, Point1.m_Y - PointHeight, Point1.m_X + PointWidth, Point1.m_Y + PointHeight);
				DrawDiscriptor.m_Index = i - 1;
				DrawDiscriptor.m_pPoint1 = pSeries->pPointsY[PreviousPoint] != EG_CHART_POINT_NONE ? &Point1 : nullptr;
				DrawDiscriptor.m_pPoint2 = pSeries->pPointsY[PointIndex] != EG_CHART_POINT_NONE ? &Point2 : nullptr;
				DrawDiscriptor.m_pRect = &PointRect;
				DrawDiscriptor.m_Value = pSeries->pPointsY[PreviousPoint];
				EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
				if(pSeries->pPointsY[PreviousPoint] != EG_CHART_POINT_NONE && pSeries->pPointsY[PointIndex] != EG_CHART_POINT_NONE) {
					DrawLine.Draw(pContext, &Point1, &Point2);
					if(PointWidth && PointHeight) {
						DrawRect.Draw(pContext, &PointRect);
					}
				}
				EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
			}
			PreviousPoint = PointIndex;
		}
		// Draw the last point
		if(i == m_PointCount) {
			if(pSeries->pPointsY[PointIndex] != EG_CHART_POINT_NONE) {
				EGRect PointRect(Point2.m_X - PointWidth, Point2.m_Y - PointHeight, Point2.m_X + PointWidth, Point2.m_Y + PointHeight);
				DrawDiscriptor.m_Index = i - 1;
				DrawDiscriptor.m_pPoint1 = nullptr;
				DrawDiscriptor.m_pPoint2 = nullptr;
				DrawDiscriptor.m_pRect = &PointRect;
				DrawDiscriptor.m_Value = pSeries->pPointsY[PointIndex];
				EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
				DrawRect.Draw(pContext, &PointRect);
				EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
			}
		}
	}
	pContext->m_pClipRect = pOriginalClipRect;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::DrawBarSeries(EGDrawContext *pContext)
{
EGRect ClipRect, ColumnRect;
EG_CharSeries_t *pSeries;

	if(ClipRect.Intersect(&m_Rect, pContext->m_pClipRect) == false) return;
	const EGRect *pOriginalClipRect = pContext->m_pClipRect;
	pContext->m_pClipRect = &ClipRect;
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t ChartWidth = ((int32_t)GetContentWidth() * m_ZoomX) >> 8;
	EG_Coord_t ChartHeight = ((int32_t)GetContentHeight() * m_ZoomY) >> 8;

	uint32_t SeriesCount = m_Series.GetSize();
	int32_t BlockGap = ((int32_t)GetStylePadColumn(EG_PART_MAIN) * m_ZoomX) >>	8; // Gap between the column on ~adjacent X
	EG_Coord_t BlockWidth = (ChartWidth - ((m_PointCount - 1) * BlockGap)) / m_PointCount;
	int32_t SeriesGap = ((int32_t)GetStylePadColumn(EG_PART_ITEMS) * m_ZoomX) >> 8; // Gap between the columns on the ~same X
	EG_Coord_t ColumnWidth = (BlockWidth - (SeriesCount - 1) * SeriesGap) / SeriesCount;
	if(ColumnWidth < 1) ColumnWidth = 1;
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t OffsetX = PadLeft - GetScrollLeft() + BorderWidth;
	EG_Coord_t OffsetY = PadTop - GetScrollTop() + BorderWidth;
	EGDrawRect DrawColumn;
	InititialseDrawRect(EG_PART_ITEMS, &DrawColumn);
	DrawColumn.m_BackgroundGrad.dir = EG_GRAD_DIR_NONE;
	DrawColumn.m_BackgroundOPA = EG_OPA_COVER;
	ColumnRect.SetY2(m_Rect.GetY2() + DrawColumn.m_Radius);	// Make the cols longer with `radius` to clip the rounding from the bottom
	EGDrawDiscriptor DrawDiscriptor;
	InitDrawDescriptor(&DrawDiscriptor, pContext);
	DrawDiscriptor.m_Part = EG_PART_ITEMS;
	DrawDiscriptor.m_pClass = &c_ChartClass;
	DrawDiscriptor.m_Type = EG_CHART_DRAW_PART_BAR;
	for(uint16_t i = 0; i < m_PointCount; i++) {	// Go through all points
		EG_Coord_t RefX = (int32_t)((int32_t)(ChartWidth - BlockWidth) * i) / (m_PointCount - 1) + m_Rect.GetX1() + OffsetX;
		DrawDiscriptor.m_Index = i;
		// Draw the current point of all data line
    POSITION Pos;
		for(pSeries = (EG_CharSeries_t*)m_Series.GetTail(Pos); pSeries != nullptr; pSeries = (EG_CharSeries_t*)m_Series.GetPrev(Pos)){
			if(pSeries->Hidden) continue;
			EG_Coord_t StartPoint = GetStartPointX(pSeries);
			ColumnRect.SetX1(RefX);
			ColumnRect.SetX2(RefX + ColumnWidth - 1);
			RefX += ColumnWidth + SeriesGap;
			if(ColumnRect.GetX2() < ClipRect.GetX1()) continue;
			if(ColumnRect.GetX1() > ClipRect.GetX2()) break;
			DrawColumn.m_BackgroundColor = pSeries->Color;
			EG_Coord_t PointIndex = (StartPoint + i) % m_PointCount;
			int32_t CalcY = (int32_t)((int32_t)pSeries->pPointsY[PointIndex] - m_MinY[pSeries->AxisSecY]) * ChartHeight;
			CalcY = CalcY / (m_MaxY[pSeries->AxisSecY] - m_MinY[pSeries->AxisSecY]);
			ColumnRect.SetY1(ChartHeight - CalcY + m_Rect.GetY1() + OffsetY);
			if(pSeries->pPointsY[PointIndex] != EG_CHART_POINT_NONE) {
				DrawDiscriptor.m_pRect = &ColumnRect;
				DrawDiscriptor.m_pDrawRect = &DrawColumn;
				DrawDiscriptor.m_pSubPart = pSeries;
				DrawDiscriptor.m_Value = pSeries->pPointsY[PointIndex];
				EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
				DrawColumn.Draw(pContext, &ColumnRect);
				EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
			}
		}
	}
	pContext->m_pClipRect = pOriginalClipRect;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::DrawCursors(EGDrawContext *pContext)
{
	if(m_Cursor.IsEmpty()) return;
	EGRect ClipRect;
	if(ClipRect.Intersect(&m_Rect, pContext->m_pClipRect) == false) return;
	const EGRect *pOriginalClipRect = pContext->m_pClipRect;
	pContext->m_pClipRect = &ClipRect;
	EGPoint Point1, Point2;
	EG_CharCursor_t *pCursor;
	EGDrawLine DrawLine;
	InititialseDrawLine(EG_PART_CURSOR, &DrawLine);
	EGDrawRect DrawCursor;
	DrawCursor.m_BackgroundOPA = DrawLine.m_OPA;
	DrawCursor.m_Radius = EG_RADIUS_CIRCLE;
	EGDrawLine DrawLineTemp;
	EGDrawRect DrawCursorTemp;
	EG_Coord_t PointWidth = GetStyleWidth(EG_PART_CURSOR) / 2;
	EG_Coord_t PointHeight = GetStyleWidth(EG_PART_CURSOR) / 2;
	EGDrawDiscriptor DrawDiscriptor;
	InitDrawDescriptor(&DrawDiscriptor, pContext);
	DrawDiscriptor.m_pDrawLine = &DrawLineTemp;
	DrawDiscriptor.m_pDrawRect = &DrawCursorTemp;
	DrawDiscriptor.m_Part = EG_PART_CURSOR;
	DrawDiscriptor.m_pClass = &c_ChartClass;
	DrawDiscriptor.m_Type = EG_CHART_DRAW_PART_CURSOR;
	// Go through all cursor lines
  POSITION Pos;
	for(pCursor = (EG_CharCursor_t*)m_Cursor.GetTail(Pos); pCursor != nullptr; pCursor = (EG_CharCursor_t*)m_Cursor.GetPrev(Pos)){
		DrawLineTemp = DrawLine;
		DrawCursorTemp = DrawCursor;
		DrawLineTemp.m_Color = pCursor->Color;
		DrawCursorTemp.m_BackgroundColor = pCursor->Color;
		DrawDiscriptor.m_pPoint1 = &Point1;
		DrawDiscriptor.m_pPoint2 = &Point2;
		EG_Coord_t cx;
		EG_Coord_t cy;
		if(pCursor->PositionSet) {
			cx = pCursor->Position.m_X;
			cy = pCursor->Position.m_Y;
		}
		else {
			if(pCursor->PointIndex == EG_CHART_POINT_NONE) continue;
			EGPoint Point;
			GetPointPosByIndex(pCursor->pSeries, pCursor->PointIndex, &Point);
			cx = Point.m_X;
			cy = Point.m_Y;
		}
		cx += m_Rect.GetX1();
		cy += m_Rect.GetY1();
		EGRect PointRect;
		bool DrawPoint = PointWidth && PointHeight;
		if(DrawPoint) {
			PointRect.Set(cx - PointWidth, cy - PointHeight, cx + PointWidth, cy + PointHeight);
			DrawDiscriptor.m_pRect = &PointRect;
		}
		else DrawDiscriptor.m_pRect = nullptr;
		if(pCursor->Direction & EG_DIR_HOR) {
			Point1.m_X = pCursor->Direction & EG_DIR_LEFT ? m_Rect.GetX1() : cx;
			Point1.m_Y = cy;
			Point2.m_X = pCursor->Direction & EG_DIR_RIGHT ? m_Rect.GetX2() : cx;
			Point2.m_Y = Point1.m_Y;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
			DrawLineTemp.Draw(pContext, &Point1, &Point2);
			if(DrawPoint) {
				DrawCursorTemp.Draw(pContext, &PointRect);
			}
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
		}
		if(pCursor->Direction & EG_DIR_VER) {
			Point1.m_X = cx;
			Point1.m_Y = pCursor->Direction & EG_DIR_TOP ? m_Rect.GetY1() : cy;
			Point2.m_X = Point1.m_X;
			Point2.m_Y = pCursor->Direction & EG_DIR_BOTTOM ? m_Rect.GetY2() : cy;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
			DrawLineTemp.Draw(pContext, &Point1, &Point2);

			if(DrawPoint) {
				DrawCursorTemp.Draw(pContext, &PointRect);
			}

			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
		}
	}

	pContext->m_pClipRect = pOriginalClipRect;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::DrawAxisY(EGDrawContext *pContext, EG_ChartAxis_e Axis)
{
	EG_CharTickDSC_t *pTick = GetTickDSC(Axis);
	if(!pTick->LabelEnable && !pTick->MajorLength && !pTick->MinorLength) return;
	if(pTick->MajorCount <= 1) return;
	uint32_t total_tick_num = (pTick->MajorCount - 1) * (pTick->MinorCount);
	if(total_tick_num == 0) return;
	uint8_t sec_axis = (Axis == EG_CHART_AXIS_PRIMARY_Y) ? 0 : 1;
	uint32_t i;
	EGPoint Point1, Point2;
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t ChartHeight = ((int32_t)GetContentHeight() * m_ZoomY) >> 8;
	EG_Coord_t OffsetY = m_Rect.GetY1() + PadTop + BorderWidth - GetScrollTop();
	EG_Coord_t label_gap;
	EG_Coord_t OffsetX;
	if(Axis == EG_CHART_AXIS_PRIMARY_Y) {
		label_gap = GetStylePadLeft(EG_PART_TICKS);
		OffsetX = m_Rect.GetX1();
	}
	else {
		label_gap = GetStylePadRight(EG_PART_TICKS);
		OffsetX = m_Rect.GetX2();
	}
	EG_Coord_t MajorLength = pTick->MajorLength;
	EG_Coord_t MinorLength = pTick->MinorLength;
	// Tick lines on secondary y axis are drawn in other direction
	if(Axis == EG_CHART_AXIS_SECONDARY_Y) {
		MajorLength *= -1;
		MinorLength *= -1;
	}
	EGDrawLine DrawLine;
	InititialseDrawLine(EG_PART_TICKS, &DrawLine);
	EGDrawLabel DrawLabel;
	InititialseDrawLabel(EG_PART_TICKS, &DrawLabel);
	EGDrawDiscriptor DrawDiscriptor;
	InitDrawDescriptor(&DrawDiscriptor, pContext);
	DrawDiscriptor.m_pClass = &c_ChartClass;
	DrawDiscriptor.m_Type = EG_CHART_DRAW_PART_TICK_LABEL;
	DrawDiscriptor.m_Index = Axis;
	DrawDiscriptor.m_Part = EG_PART_TICKS;
	DrawDiscriptor.m_pDrawLine = &DrawLine;
	DrawDiscriptor.m_pDrawLabel = &DrawLabel;
	for(i = 0; i <= total_tick_num; i++) {
		Point2.m_Y = Point1.m_Y = OffsetY + (int32_t)((int32_t)(ChartHeight - DrawLine.m_Width) * i) / total_tick_num;	// draw a line at moving y position
		Point1.m_X = OffsetX;		// first point of the tick
		if(Axis == EG_CHART_AXIS_PRIMARY_Y)	Point1.m_X--;	// move extra pixel out of chart boundary
		else Point1.m_X++;
		bool major = false;		// second point of the m_Tick
		if(i % pTick->MinorCount == 0) major = true;
		if(major)	Point2.m_X = Point1.m_X - MajorLength; // major tick
		else Point2.m_X = Point1.m_X - MinorLength; // minor tick
		DrawDiscriptor.m_pPoint1 = &Point1;
		DrawDiscriptor.m_pPoint2 = &Point2;
		int32_t tick_value = EG_Map(total_tick_num - i, 0, total_tick_num, m_MinY[sec_axis], m_MaxY[sec_axis]);
		DrawDiscriptor.m_Value = tick_value;
		if(major && pTick->LabelEnable) {		// add text only to major tick
			char buf[EG_CHART_LABEL_MAX_TEXT_LENGTH];
			eg_snprintf(buf, sizeof(buf), "%" EG_PRId32, tick_value);
			DrawDiscriptor.m_pDrawLabel = &DrawLabel;
			DrawDiscriptor.m_pText = buf;
			DrawDiscriptor.m_TextLength = EG_CHART_LABEL_MAX_TEXT_LENGTH;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
			EGPoint Size;			// reserve appropriate area
			EG_GetTextSize(&Size, DrawDiscriptor.m_pText, DrawLabel.m_pFont, DrawLabel.m_Kerning, DrawLabel.m_LineSpace, EG_COORD_MAX,	EG_TEXT_FLAG_NONE);
			EGRect Rect;		// set the area at some distance of the major tick len left of the m_Tick
			Rect.SetY1(Point2.m_Y - Size.m_Y / 2);
			Rect.SetY2(Point2.m_Y + Size.m_Y / 2);
			if(!sec_axis) {
				Rect.SetX1(Point2.m_X - Size.m_X - label_gap);
				Rect.SetX2(Point2.m_X - label_gap);
			}
			else {
				Rect.SetX1(Point2.m_X + label_gap);
				Rect.SetX2(Point2.m_X + Size.m_X + label_gap);
			}
			if(Rect.GetY2() >= m_Rect.GetY1() && Rect.GetY1() <= m_Rect.GetY2()) {
				DrawLabel.Draw(pContext, &Rect, DrawDiscriptor.m_pText, nullptr);
			}
		}
		else {
			DrawDiscriptor.m_pDrawLabel = nullptr;
			DrawDiscriptor.m_pText = nullptr;
			DrawDiscriptor.m_TextLength = 0;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
		}
		if(Point1.m_Y + DrawLine.m_Width / 2 >= m_Rect.GetY1() &&
			 Point2.m_Y - DrawLine.m_Width / 2 <= m_Rect.GetY2()) {
			DrawLine.Draw(pContext, &Point1, &Point2);
		}
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::DrawAxisX(EGDrawContext *pContext, EG_ChartAxis_e Axis)
{
	EG_CharTickDSC_t *pTick = GetTickDSC(Axis);
	if(pTick->MajorCount <= 1) return;
	if(!pTick->LabelEnable && !pTick->MajorLength && !pTick->MinorLength) return;
	uint32_t total_tick_num = (pTick->MajorCount - 1) * (pTick->MinorCount);
	if(total_tick_num == 0) return;
	uint32_t i;
	EGPoint Point1, Point2;
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN) + GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t ChartWidth = ((int32_t)GetContentWidth() * m_ZoomX) >> 8;
	EGDrawLabel DrawLabel;
	InititialseDrawLabel(EG_PART_TICKS, &DrawLabel);
	EG_Coord_t OffsetX = m_Rect.GetX1() + PadLeft - GetScrollLeft();
	EG_Coord_t OffsetY;
	EG_Coord_t label_gap;
	if(Axis == EG_CHART_AXIS_PRIMARY_X) {
		label_gap = pTick->LabelEnable ? GetStylePadBottom(EG_PART_TICKS) : 0;
		OffsetY = m_Rect.GetY2() + 1;
	}
	else {
		label_gap = pTick->LabelEnable ? GetStylePadTop(EG_PART_TICKS) : 0;
		OffsetY = m_Rect.GetY1() - 1;
	}

	if(Axis == EG_CHART_AXIS_PRIMARY_X) {
		if(OffsetY > pContext->m_pClipRect->GetY2()) return;
		if(OffsetY + label_gap + DrawLabel.m_pFont->LineHeight + pTick->MajorLength < pContext->m_pClipRect->GetY1()) return;
	}
	EGDrawLine DrawLine;
	InititialseDrawLine(EG_PART_TICKS, &DrawLine);
	DrawLine.m_DashGap = 0;
	DrawLine.m_DashWidth = 0;
	EGDrawDiscriptor DrawDiscriptor;
	InitDrawDescriptor(&DrawDiscriptor, pContext);
	DrawDiscriptor.m_pClass = &c_ChartClass;
	DrawDiscriptor.m_Type = EG_CHART_DRAW_PART_TICK_LABEL;
	DrawDiscriptor.m_Index = EG_CHART_AXIS_PRIMARY_X;
	DrawDiscriptor.m_Part = EG_PART_TICKS;
	DrawDiscriptor.m_pDrawLabel = &DrawLabel;
	DrawDiscriptor.m_pDrawLine = &DrawLine;
	uint8_t sec_axis = Axis == EG_CHART_AXIS_PRIMARY_X ? 0 : 1;

	// The columns ticks should be aligned to the center of blocks
	if(m_ChartType == EG_CHART_TYPE_BAR) {
		int32_t BlockGap = ((int32_t)GetStylePadColumn(EG_PART_MAIN) * m_ZoomX) >>	8; // Gap between the columns on ~adjacent X
		EG_Coord_t BlockWidth = (ChartWidth + BlockGap) / (m_PointCount);
		OffsetX += (BlockWidth - BlockGap) / 2;
		ChartWidth -= BlockWidth - BlockGap;
	}
	Point1.m_Y = OffsetY;
	for(i = 0; i <= total_tick_num; i++) { // one extra loop - it may not exist in the list, empty label
		bool major = false;
		if(i % pTick->MinorCount == 0) major = true;
		Point2.m_X = Point1.m_X = OffsetX + (int32_t)((int32_t)(ChartWidth - DrawLine.m_Width) * i) / total_tick_num;		// draw a line at moving x position
		if(sec_axis) Point2.m_Y = Point1.m_Y - (major ? pTick->MajorLength : pTick->MinorLength);
		else Point2.m_Y = Point1.m_Y + (major ? pTick->MajorLength : pTick->MinorLength);
		DrawDiscriptor.m_pPoint1 = &Point1;
		DrawDiscriptor.m_pPoint2 = &Point2;
		int32_t tick_value;		// add text only to major tick
		if(m_ChartType == EG_CHART_TYPE_SCATTER) {
			tick_value = EG_Map(i, 0, total_tick_num, m_MinX[sec_axis], m_MaxX[sec_axis]);
		}
		else {
			tick_value = i / pTick->MinorCount;
		}
		DrawDiscriptor.m_Value = tick_value;
		if(major && pTick->LabelEnable) {
			char buf[EG_CHART_LABEL_MAX_TEXT_LENGTH];
			eg_snprintf(buf, sizeof(buf), "%" EG_PRId32, tick_value);
			DrawDiscriptor.m_pDrawLabel = &DrawLabel;
			DrawDiscriptor.m_pText = buf;
			DrawDiscriptor.m_TextLength = EG_CHART_LABEL_MAX_TEXT_LENGTH;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
			EGPoint Size;			// reserve appropriate area
			EG_GetTextSize(&Size, DrawDiscriptor.m_pText, DrawLabel.m_pFont, DrawLabel.m_Kerning, DrawLabel.m_LineSpace, EG_COORD_MAX,	EG_TEXT_FLAG_NONE);
			EGRect Rect;			// set the area at some distance of the major tick len under of the tick
			Rect.SetX1(Point2.m_X - Size.m_X / 2);
			Rect.SetX2(Point2.m_X + Size.m_X / 2);
			if(sec_axis) {
				Rect.SetY2(Point2.m_Y - label_gap);
				Rect.SetY1(Rect.GetY2() - Size.m_Y);
			}
			else {
				Rect.SetY1(Point2.m_Y + label_gap);
				Rect.SetY2(Rect.GetY1() + Size.m_Y);
			}

			if(Rect.GetX2() >= m_Rect.GetX1() &&
				 Rect.GetX1() <= m_Rect.GetX2()) {
				DrawLabel.Draw(pContext, &Rect, DrawDiscriptor.m_pText, nullptr);
			}
		}
		else {
			DrawDiscriptor.m_pDrawLabel = nullptr;
			DrawDiscriptor.m_pText = nullptr;
			DrawDiscriptor.m_TextLength = 0;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
		}
		if(Point1.m_X + DrawLine.m_Width / 2 >= m_Rect.GetX1() && Point2.m_X - DrawLine.m_Width / 2 <= m_Rect.GetX2()) {
			DrawLine.Draw(pContext, &Point1, &Point2);
		}
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

uint32_t EGChart::GetIndexFromX(EG_Coord_t X)
{
	EG_Coord_t ChartWidth = ((int32_t)GetContentWidth() * m_ZoomX) >> 8;
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);
	X -= PadLeft;
	if(X < 0) return 0;
	if(X > ChartWidth) return m_PointCount - 1;
	if(m_ChartType == EG_CHART_TYPE_LINE) return (X * (m_PointCount - 1) + ChartWidth / 2) / ChartWidth;
	if(m_ChartType == EG_CHART_TYPE_BAR) return (X * m_PointCount) / ChartWidth;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::InvalidatePoint(uint16_t Index)
{
	if(Index >= m_PointCount) return;
	EG_Coord_t ChartWidth = ((int32_t)GetContentWidth() * m_ZoomX) >> 8;
	EG_Coord_t ScrollLeft = GetScrollLeft();
	if(m_UpdateMode == EG_CHART_UPDATE_MODE_SHIFT) {	// In shift mode the whole chart changes so the whole object
		Invalidate();
		return;
	}
	if(m_ChartType == EG_CHART_TYPE_LINE) {
		EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
		EG_Coord_t pleft = GetStylePadLeft(EG_PART_MAIN);
		EG_Coord_t OffsetX = m_Rect.GetX1() + pleft + BorderWidth - ScrollLeft;
		EG_Coord_t LineWidth = GetStyleLineWidth(EG_PART_ITEMS);
		EG_Coord_t PointWidth = GetStyleWidth(EG_PART_INDICATOR);
		EGRect Rect(m_Rect);
		Rect.DecY1(LineWidth + PointWidth);
		Rect.IncY2(LineWidth + PointWidth);
		if(Index < m_PointCount - 1) {
			Rect.SetX1(((ChartWidth * Index) / (m_PointCount - 1)) + OffsetX - LineWidth - PointWidth);
			Rect.SetX2(((ChartWidth * (Index + 1)) / (m_PointCount - 1)) + OffsetX + LineWidth + PointWidth);
			InvalidateArea(&Rect);
		}
		if(Index > 0) {
			Rect.SetX1(((ChartWidth * (Index - 1)) / (m_PointCount - 1)) + OffsetX - LineWidth - PointWidth);
			Rect.SetX2(((ChartWidth * Index) / (m_PointCount - 1)) + OffsetX + LineWidth + PointWidth);
			InvalidateArea(&Rect);
		}
	}
	else if(m_ChartType == EG_CHART_TYPE_BAR) {
		EGRect ColumnRect;
		int32_t BlockGap = ((int32_t)GetStylePadColumn(EG_PART_MAIN) * m_ZoomX) >>	8; // Gap between the column on ~adjacent X
		EG_Coord_t BlockWidth = (ChartWidth + BlockGap) / m_PointCount;
		EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
		EG_Coord_t RefX;
		RefX = (int32_t)((int32_t)(BlockWidth) * Index);
		RefX += m_Rect.GetX1() + BorderWidth + GetStylePadLeft(EG_PART_MAIN);
		m_Rect.Copy(&ColumnRect);	// Save the original coordinates
	  ColumnRect.SetX1(RefX - ScrollLeft);
		ColumnRect.SetX2(ColumnRect.GetX1() + BlockWidth);
		ColumnRect.DecX1(BlockGap);
		InvalidateArea(&ColumnRect);
	}
	else {
		Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGChart::AllocateNewPoints(EG_CharSeries_t *pSeries, uint32_t Count, EG_Coord_t **pPoints)
{
	if((*pPoints) == nullptr) return;
	uint32_t point_cnt_old = m_PointCount;
	uint32_t i;
	if(pSeries->StartPoint != 0) {
		EG_Coord_t *new_points = (EG_Coord_t*)EG_AllocMem(sizeof(EG_Coord_t) * Count);
		EG_ASSERT_MALLOC(new_points);
		if(new_points == nullptr) return;

		if(Count >= point_cnt_old) {
			for(i = 0; i < point_cnt_old; i++) {
				new_points[i] =
					(*pPoints)[(i + pSeries->StartPoint) % point_cnt_old]; // Copy old contents to new array
			}
			for(i = point_cnt_old; i < Count; i++) {
				new_points[i] = EG_CHART_POINT_NONE; // Fill up the rest with default value
			}
		}
		else {
			for(i = 0; i < Count; i++) {
				new_points[i] =
					(*pPoints)[(i + pSeries->StartPoint) % point_cnt_old]; // Copy old contents to new array
			}
		}

		// Switch over pointer from old to new
		EG_FreeMem((*pPoints));
		(*pPoints) = new_points;
	}
	else {
		(*pPoints) = (EG_Coord_t*)EG_ReallocMem((*pPoints), sizeof(EG_Coord_t) * Count);
		EG_ASSERT_MALLOC((*pPoints));
		if((*pPoints) == nullptr) return;
		// Initialize the new points
		if(Count > point_cnt_old) {
			for(i = point_cnt_old - 1; i < Count; i++) {
				(*pPoints)[i] = EG_CHART_POINT_NONE;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EG_CharTickDSC_t* EGChart::GetTickDSC(EG_ChartAxis_e Axis)
{
	switch(Axis){
		case EG_CHART_AXIS_PRIMARY_Y:
			return &m_Tick[0];
		case EG_CHART_AXIS_PRIMARY_X:
			return &m_Tick[1];
		case EG_CHART_AXIS_SECONDARY_Y:
			return &m_Tick[2];
		case EG_CHART_AXIS_SECONDARY_X:
			return &m_Tick[3];
		default:
			return nullptr;
	}
	return nullptr; // keep compiler happy
}

#endif
