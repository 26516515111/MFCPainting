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

void LineShap::Scale(double factor, CPoint center)
{
	auto scalePoint = [&](CPoint& pt) {
		double dx = static_cast<double>(pt.x - center.x);
		double dy = static_cast<double>(pt.y - center.y);
		pt.x = center.x + static_cast<int>(std::round(dx * factor));
		pt.y = center.y + static_cast<int>(std::round(dy * factor));
		};
	scalePoint(startPoint);
	scalePoint(endPoint);
}

CircleShap::CircleShap(CPoint center, CPoint r)
{
	centerPoint = center;
	rPoint = r;

	centerX = static_cast<double>(center.x);
	centerY = static_cast<double>(center.y);
	rdx = static_cast<double>(r.x - center.x);
	rdy = static_cast<double>(r.y - center.y);
}

void CircleShap::Draw(CPaintDC* pdc)
{
	// 以高精度计算半径并在绘制时四舍五入为像素
	double radius_d = std::sqrt(rdx * rdx + rdy * rdy);
	int r = static_cast<int>(std::round(radius_d));

	// 更新整数副本（兼容旧代码）
	centerPoint.x = static_cast<int>(std::round(centerX));
	centerPoint.y = static_cast<int>(std::round(centerY));
	rPoint.x = centerPoint.x + static_cast<int>(std::round(rdx));
	rPoint.y = centerPoint.y + static_cast<int>(std::round(rdy));

	CBrush* pOld = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
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
	centerX += delta.cx;
	centerY += delta.cy;
	centerPoint.x = static_cast<int>(std::round(centerX));
	centerPoint.y = static_cast<int>(std::round(centerY));
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

void CircleShap::Scale(double factor, CPoint center)
{// 以传入的 center (整数) 为缩放中心，按 double 进行计算
	double cx = static_cast<double>(center.x);
	double cy = static_cast<double>(center.y);

	// 缩放圆心位置相对于缩放中心
	centerX = cx + (centerX - cx) * factor;
	centerY = cy + (centerY - cy) * factor;

	// 缩放半径向量
	rdx *= factor;
	rdy *= factor;

	// 同步整数副本
	centerPoint.x = static_cast<int>(std::round(centerX));
	centerPoint.y = static_cast<int>(std::round(centerY));
	rPoint.x = centerPoint.x + static_cast<int>(std::round(rdx));
	rPoint.y = centerPoint.y + static_cast<int>(std::round(rdy));
}



void CShap::ChangeSelected(CPoint point)
{
	Selected = IsSelected(point);
}

void CShap::Destroy()
{
	this->~CShap();
}

static inline double DegToRad(double deg) {
	return deg * (acos(-1.0) / 180.0);
}

void RectShap::UpdateIntCorners()
{
	double fx[4], fy[4];
	ComputeFloatCorners(fx, fy);
	for (int i = 0; i < 4; ++i) {
		cornersInt[i].x = static_cast<int>(std::round(fx[i]));
		cornersInt[i].y = static_cast<int>(std::round(fy[i]));
	}
}


void RectShap::ComputeFloatCorners(double outX[4], double outY[4])
{
	// 未旋转时顶点相对于中心的坐标（顺序：LT, RT, RB, LB）
	double rx[4] = { -halfW, halfW, halfW, -halfW };
	double ry[4] = { -halfH, -halfH, halfH, halfH };

	double rad = DegToRad(angleDeg);
	double cosv = std::cos(rad);
	double sinv = std::sin(rad);

	for (int i = 0; i < 4; ++i) {
		double tx = rx[i] * cosv - ry[i] * sinv;
		double ty = rx[i] * sinv + ry[i] * cosv;
		outX[i] = centerX + tx;
		outY[i] = centerY + ty;
	}
}

RectShap::RectShap(CPoint topLeft, CPoint bottomRight)
{
	// 规范化输入：计算左上与右下
	int left = min(topLeft.x, bottomRight.x);
	int right = max(topLeft.x, bottomRight.x);
	int top = min(topLeft.y, bottomRight.y);
	int bottom = max(topLeft.y, bottomRight.y);

	centerX = (left + right) / 2.0;
	centerY = (top + bottom) / 2.0;
	halfW = (right - left) / 2.0;
	halfH = (bottom - top) / 2.0;
	angleDeg = 0.0;

	UpdateIntCorners();
}

void RectShap::Draw(CPaintDC* pdc)
{
	if (!pdc) return;

	// 使用 NULL_BRUSH 只绘制轮廓
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&pen);

	// 绘制多边形（闭合）
	pdc->MoveTo(cornersInt[0]);
	for (int i = 1; i < 4; ++i) pdc->LineTo(cornersInt[i]);
	pdc->LineTo(cornersInt[0]);

	pdc->SelectObject(pOldPen);
	pdc->SelectObject(pOldBrush);
}

bool RectShap::IsSelected(CPoint point)
{
	const double tolerance = 5.0;
	double px = static_cast<double>(point.x);
	double py = static_cast<double>(point.y);

	// 浮点顶点
	double vx[4], vy[4];
	ComputeFloatCorners(vx, vy);

	// 检查点到每条边的最短距离
	for (int i = 0; i < 4; ++i) {
		int j = (i + 1) % 4;
		double ax = vx[i], ay = vy[i];
		double bx = vx[j], by = vy[j];
		double abx = bx - ax, aby = by - ay;
		double apx = px - ax, apy = py - ay;
		double ab2 = abx * abx + aby * aby;
		double t = 0.0;
		if (ab2 > 1e-9) t = (apx * abx + apy * aby) / ab2;
		if (t < 0.0) t = 0.0;
		if (t > 1.0) t = 1.0;
		double cx = ax + t * abx;
		double cy = ay + t * aby;
		double dx = px - cx;
		double dy = py - cy;
		double dist = std::sqrt(dx * dx + dy * dy);
		if (dist <= tolerance) return true;
	}
	return false;
}

void RectShap::DrawSelection(CPaintDC* pdc)
{
	if (!Selected || !pdc) return;

	CPen dashPen(PS_DASH, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&dashPen);
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));

	pdc->MoveTo(cornersInt[0]);
	for (int i = 1; i < 4; ++i) pdc->LineTo(cornersInt[i]);
	pdc->LineTo(cornersInt[0]);

	pdc->SelectObject(pOldBrush);
	pdc->SelectObject(pOldPen);
}

void RectShap::Move(CSize delta)
{
	centerX += delta.cx;
	centerY += delta.cy;
	UpdateIntCorners();
}

void RectShap::Rotate(double degrees)
{
	angleDeg += degrees;
	// 规范化角度（可选）
	if (angleDeg > 360.0 || angleDeg < -360.0) {
		angleDeg = std::fmod(angleDeg, 360.0);
	}
	UpdateIntCorners();
}

CPoint RectShap::GetCenter() const
{
	return CPoint(static_cast<int>(std::round(centerX)), static_cast<int>(std::round(centerY)));
}

void RectShap::Scale(double factor, CPoint center)
{
	double cx = static_cast<double>(center.x);
	double cy = static_cast<double>(center.y);

	// 缩放中心位置
	centerX = cx + (centerX - cx) * factor;
	centerY = cy + (centerY - cy) * factor;

	// 缩放尺寸
	halfW *= factor;
	halfH *= factor;

	UpdateIntCorners();
}
