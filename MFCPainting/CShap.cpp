#include "pch.h"
#include "CShap.h"


static CPoint RotatePointAround(const CPoint& pt, const CPoint& center, double rad)
{
	double dx = static_cast<double>(pt.x - center.x);
	double dy = static_cast<double>(pt.y - center.y);
	double cosv = std::cos(rad);
	double sinv = std::sin(rad);
	double rx = dx * cosv - dy * sinv;
	double ry = dx * sinv + dy * cosv;
	return CPoint(static_cast<int>(std::round(center.x + rx)), static_cast<int>(std::round(center.y + ry)));
}

LineShap::LineShap(CPoint start, CPoint end)
{
	startPoint = start;
	endPoint = end;
}

void LineShap::Draw(CPaintDC* pdc)
{
	pdc->MoveTo(startPoint);
	pdc->LineTo(endPoint);
}

bool LineShap::IsSelected(CPoint point)
{
	const double tolerance = 5.0;

	double ax = static_cast<double>(startPoint.x);
	double ay = static_cast<double>(startPoint.y);
	double bx = static_cast<double>(endPoint.x);
	double by = static_cast<double>(endPoint.y);
	double px = static_cast<double>(point.x);
	double py = static_cast<double>(point.y);

	double dx = bx - ax;
	double dy = by - ay;
	double len2 = dx * dx + dy * dy;

	double dist = 0.0;

	if (len2 <= 1e-9) {
		double ux = px - ax;
		double uy = py - ay;
		dist = std::sqrt(ux * ux + uy * uy);
	}
	else {
		double t = ((px - ax) * dx + (py - ay) * dy) / len2;
		if (t < 0.0) t = 0.0;
		else if (t > 1.0) t = 1.0;

		double cx = ax + t * dx;
		double cy = ay + t * dy;

		double ux = px - cx;
		double uy = py - cy;
		dist = std::sqrt(ux * ux + uy * uy);
	}

	return dist <= tolerance;
}

void LineShap::DrawSelection(CPaintDC* pdc)
{
	if (!Selected) return;

	// 使用虚线笔绘制
	CPen pen(PS_DASH, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&pen);
	this->Draw(pdc);
}

void LineShap::Move(CSize delta)
{
	startPoint.Offset(delta);
	endPoint.Offset(delta);
}

void LineShap::Rotate(double degrees)
{
	double rad = degrees * (acos(-1.0) / 180.0);
	CPoint center = GetCenter();
	startPoint = RotatePointAround(startPoint, center, rad);
	endPoint = RotatePointAround(endPoint, center, rad);
}

CPoint LineShap::GetCenter() const
{
	return CPoint((startPoint.x + endPoint.x) / 2, (startPoint.y + endPoint.y) / 2);

}

CircleShap::CircleShap(CPoint center, CPoint r)
{
	centerPoint = center;
	rPoint = r;
}

void CircleShap::Draw(CPaintDC* pdc)
{
	CBrush* pOld = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
	double dx = static_cast<double>(rPoint.x - centerPoint.x);
	double dy = static_cast<double>(rPoint.y - centerPoint.y);
	int r = static_cast<int>(std::sqrt(dx * dx + dy * dy) + 0.5); // 半径
	pdc->Ellipse(centerPoint.x - r, centerPoint.y - r, centerPoint.x + r + 1, centerPoint.y + r + 1);
	pdc->SelectObject(pOld);
}

bool CircleShap::IsSelected(CPoint point)
{
	const double tolerance = 5.0;

	double dx = static_cast<double>(point.x - centerPoint.x);
	double dy = static_cast<double>(point.y - centerPoint.y);
	double dist = std::sqrt(dx * dx + dy * dy);

	double rx = static_cast<double>(rPoint.x - centerPoint.x);
	double ry = static_cast<double>(rPoint.y - centerPoint.y);
	double radius = std::sqrt(rx * rx + ry * ry);

	// 当点距圆周的距离在容差范围内，认为命中
	 return  std::abs(dist - radius) <= tolerance;
}

void CircleShap::DrawSelection(CPaintDC* pdc)
{
	if (!Selected) return;

	// 计算半径
	double dx = static_cast<double>(rPoint.x - centerPoint.x);
	double dy = static_cast<double>(rPoint.y - centerPoint.y);
	int r = static_cast<int>(std::sqrt(dx * dx + dy * dy) + 0.5); // 半径

	// 为了不覆盖线条（避免填充覆盖），在绘制选中圆时选择 NULL_BRUSH（只绘制轮廓）
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
	CPen pen(PS_DASH, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&pen);

	pdc->Ellipse(centerPoint.x - r, centerPoint.y - r, centerPoint.x + r + 1, centerPoint.y + r + 1);

	pdc->SelectObject(pOldPen);
}

void CircleShap::Move(CSize delta)
{
	centerPoint.Offset(delta);
	rPoint.Offset(delta);
}

void CircleShap::Rotate(double degrees)
{
	double rad = degrees * (acos(-1.0) / 180.0);
	rPoint = RotatePointAround(rPoint, centerPoint, rad);
}

CPoint CircleShap::GetCenter() const
{
	return centerPoint;
}



void CShap::ChangeSelected(CPoint point)
{
	Selected = IsSelected(point);
}

void CShap::Destroy()
{
	this->~CShap();
}
