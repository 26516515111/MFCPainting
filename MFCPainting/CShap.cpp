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

void DiamondShap::UpdateIntCorners()
{
	double fx[4], fy[4];
	ComputeFloatCorners(fx, fy);
	for (int i = 0; i < 4; ++i) {
		cornersInt[i].x = static_cast<int>(std::round(fx[i]));
		cornersInt[i].y = static_cast<int>(std::round(fy[i]));
	}
}

static inline double Deg2Rad(double deg) { return deg * (acos(-1.0) / 180.0); }


void DiamondShap::ComputeFloatCorners(double outX[4], double outY[4])
{
	// 未旋转时四个顶点相对于中心为： ( +halfDx, 0 ), (0, +halfDy), ( -halfDx, 0 ), (0, -halfDy)
	double rx[4] = { halfDx, 0.0, -halfDx, 0.0 };
	double ry[4] = { 0.0, halfDy, 0.0, -halfDy };

	double rad = Deg2Rad(angleDeg);
	double cosv = std::cos(rad);
	double sinv = std::sin(rad);

	for (int i = 0; i < 4; ++i) {
		double tx = rx[i] * cosv - ry[i] * sinv;
		double ty = rx[i] * sinv + ry[i] * cosv;
		outX[i] = centerX + tx;
		outY[i] = centerY + ty;
	}
}

DiamondShap::DiamondShap(CPoint topLeft, CPoint bottomRight)
{
	// 规范化矩形
	int left = min(topLeft.x, bottomRight.x);
	int right = max(topLeft.x, bottomRight.x);
	int top = min(topLeft.y, bottomRight.y);
	int bottom = max(topLeft.y, bottomRight.y);

	// 中心与半对角（即菱形的半宽/半高）
	centerX = (left + right) / 2.0;
	centerY = (top + bottom) / 2.0;
	halfDx = (right - left) / 2.0;  // 横向半对角（右顶相对于中心的 x 偏移）
	halfDy = (bottom - top) / 2.0;  // 纵向半对角（下顶相对于中心的 y 偏移）
	angleDeg = 0.0;

	UpdateIntCorners();
}

void DiamondShap::Draw(CPaintDC* pdc)
{
	if (!pdc) return;
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&pen);

	pdc->MoveTo(cornersInt[0]); // 右
	for (int i = 1; i < 4; ++i) pdc->LineTo(cornersInt[i]);
	pdc->LineTo(cornersInt[0]);

	pdc->SelectObject(pOldPen);
	pdc->SelectObject(pOldBrush);
}

static double PointSegmentDistanceDouble(double px, double py, double ax, double ay, double bx, double by)
{
	double abx = bx - ax;
	double aby = by - ay;
	double apx = px - ax;
	double apy = py - ay;
	double ab2 = abx * abx + aby * aby;
	double t = 0.0;
	if (ab2 > 1e-12) t = (apx * abx + apy * aby) / ab2;
	if (t < 0.0) t = 0.0;
	if (t > 1.0) t = 1.0;
	double cx = ax + t * abx;
	double cy = ay + t * aby;
	double dx = px - cx;
	double dy = py - cy;
	return std::sqrt(dx * dx + dy * dy);
}

static bool PointInPolygon(const double vx[], const double vy[], int n, double px, double py)
{
	bool inside = false;
	for (int i = 0, j = n - 1; i < n; j = i++) {
		bool intersect = ((vy[i] > py) != (vy[j] > py)) &&
			(px < (vx[j] - vx[i]) * (py - vy[i]) / (vy[j] - vy[i] + 1e-12) + vx[i]);
		if (intersect) inside = !inside;
	}
	return inside;
}

bool DiamondShap::IsSelected(CPoint point)
{
	const double tolerance = 5.0;
	double px = static_cast<double>(point.x);
	double py = static_cast<double>(point.y);

	// 计算浮点顶点
	double vx[4], vy[4];
	ComputeFloatCorners(vx, vy);

	// 检查点到任意边距离
	for (int i = 0; i < 4; ++i) {
		int j = (i + 1) % 4;
		double dist = PointSegmentDistanceDouble(px, py, vx[i], vy[i], vx[j], vy[j]);
		if (dist <= tolerance) return true;
	}

	return false;
}

void DiamondShap::DrawSelection(CPaintDC* pdc)
{
	if (!Selected || !pdc) return;
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
	CPen dashPen(PS_DASH, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&dashPen);

	pdc->MoveTo(cornersInt[0]);
	for (int i = 1; i < 4; ++i) pdc->LineTo(cornersInt[i]);
	pdc->LineTo(cornersInt[0]);

	pdc->SelectObject(pOldPen);
	pdc->SelectObject(pOldBrush);
}

void DiamondShap::Move(CSize delta)
{
	centerX += delta.cx;
	centerY += delta.cy;
	UpdateIntCorners();
}

void DiamondShap::Rotate(double degrees)
{
	angleDeg += degrees;
	if (angleDeg > 360.0 || angleDeg < -360.0) angleDeg = std::fmod(angleDeg, 360.0);
	UpdateIntCorners();
}

CPoint DiamondShap::GetCenter() const
{
	return CPoint(static_cast<int>(std::round(centerX)), static_cast<int>(std::round(centerY)));
}

void DiamondShap::Scale(double factor, CPoint center)
{
	// 以传入 center 为缩放中心（整数）
	double cx = static_cast<double>(center.x);
	double cy = static_cast<double>(center.y);

	centerX = cx + (centerX - cx) * factor;
	centerY = cy + (centerY - cy) * factor;

	halfDx *= factor;
	halfDy *= factor;

	UpdateIntCorners();
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

static inline void RotateDoublePoint(double& x, double& y, double cx, double cy, double rad)
{
	double dx = x - cx;
	double dy = y - cy;
	double cosv = std::cos(rad);
	double sinv = std::sin(rad);
	double nx = dx * cosv - dy * sinv;
	double ny = dx * sinv + dy * cosv;
	x = cx + nx;
	y = cy + ny;
}

void TriangleShap::UpdateIntPoints()
{
	p1Int.x = static_cast<int>(std::round(x1));
	p1Int.y = static_cast<int>(std::round(y1));
	p2Int.x = static_cast<int>(std::round(x2));
	p2Int.y = static_cast<int>(std::round(y2));
	p3Int.x = static_cast<int>(std::round(x3));
	p3Int.y = static_cast<int>(std::round(y3));
}

TriangleShap::TriangleShap(CPoint a, CPoint b, CPoint c)
{
	x1 = static_cast<double>(a.x); y1 = static_cast<double>(a.y);
	x2 = static_cast<double>(b.x); y2 = static_cast<double>(b.y);
	x3 = static_cast<double>(c.x); y3 = static_cast<double>(c.y);
	UpdateIntPoints();
}

void TriangleShap::Draw(CPaintDC* pdc)
{
	if (!pdc) return;
	// 使用空画刷只绘制轮廓
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&pen);

	pdc->MoveTo(p1Int);
	pdc->LineTo(p2Int);
	pdc->LineTo(p3Int);
	pdc->LineTo(p1Int);

	pdc->SelectObject(pOldPen);
	pdc->SelectObject(pOldBrush);
}


static double PointSegmentDistance(double px, double py, double ax, double ay, double bx, double by)
{
	// 计算点到线段 AB 的最短距离（double）
	double abx = bx - ax;
	double aby = by - ay;
	double apx = px - ax;
	double apy = py - ay;
	double ab2 = abx * abx + aby * aby;
	double t = 0.0;
	if (ab2 > 1e-12) t = (apx * abx + apy * aby) / ab2;
	if (t < 0.0) t = 0.0;
	if (t > 1.0) t = 1.0;
	double cx = ax + t * abx;
	double cy = ay + t * aby;
	double dx = px - cx;
	double dy = py - cy;
	return std::sqrt(dx * dx + dy * dy);
}

static bool PointInTriangle(double px, double py, double ax, double ay, double bx, double by, double cx, double cy)
{
	// 使用重心法 / 矢量叉积判断点是否在三角形内部
	double v0x = cx - ax, v0y = cy - ay;
	double v1x = bx - ax, v1y = by - ay;
	double v2x = px - ax, v2y = py - ay;

	double dot00 = v0x * v0x + v0y * v0y;
	double dot01 = v0x * v1x + v0y * v1y;
	double dot02 = v0x * v2x + v0y * v2y;
	double dot11 = v1x * v1x + v1y * v1y;
	double dot12 = v1x * v2x + v1y * v2y;

	double denom = dot00 * dot11 - dot01 * dot01;
	if (std::abs(denom) < 1e-12) return false; // 退化三角形

	double invDenom = 1.0 / denom;
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
	return (u >= 0) && (v >= 0) && (u + v <= 1);
}


bool TriangleShap::IsSelected(CPoint point)
{
	const double tolerance = 5.0;
	double px = static_cast<double>(point.x);
	double py = static_cast<double>(point.y);

	// 边缘距离检测
	double d1 = PointSegmentDistance(px, py, x1, y1, x2, y2);
	if (d1 <= tolerance) return true;
	double d2 = PointSegmentDistance(px, py, x2, y2, x3, y3);
	if (d2 <= tolerance) return true;
	double d3 = PointSegmentDistance(px, py, x3, y3, x1, y1);
	if (d3 <= tolerance) return true;

	return false;
}

void TriangleShap::DrawSelection(CPaintDC* pdc)
{
	if (!Selected || !pdc) return;
	// 虚线笔绘制三角形轮廓
	CPen pen(PS_DASH, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&pen);
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));

	pdc->MoveTo(p1Int);
	pdc->LineTo(p2Int);
	pdc->LineTo(p3Int);
	pdc->LineTo(p1Int);

	pdc->SelectObject(pOldBrush);
	pdc->SelectObject(pOldPen);
}

void TriangleShap::Move(CSize delta)
{
	x1 += delta.cx; y1 += delta.cy;
	x2 += delta.cx; y2 += delta.cy;
	x3 += delta.cx; y3 += delta.cy;
	UpdateIntPoints();
}

void TriangleShap::Rotate(double degrees)
{
	double rad = degrees * (acos(-1.0) / 180.0);
	CPoint c = GetCenter();
	double cx = static_cast<double>(c.x);
	double cy = static_cast<double>(c.y);
	RotateDoublePoint(x1, y1, cx, cy, rad);
	RotateDoublePoint(x2, y2, cx, cy, rad);
	RotateDoublePoint(x3, y3, cx, cy, rad);
	UpdateIntPoints();
}

CPoint TriangleShap::GetCenter() const
{
	// 使用重心（三个顶点平均）作为中心
	double cx = (x1 + x2 + x3) / 3.0;
	double cy = (y1 + y2 + y3) / 3.0;
	return CPoint(static_cast<int>(std::round(cx)), static_cast<int>(std::round(cy)));
}

void TriangleShap::Scale(double factor, CPoint center)
{
	double cx = static_cast<double>(center.x);
	double cy = static_cast<double>(center.y);
	auto scalePt = [&](double& x, double& y) {
		x = cx + (x - cx) * factor;
		y = cy + (y - cy) * factor;
		};
	scalePt(x1, y1);
	scalePt(x2, y2);
	scalePt(x3, y3);
	UpdateIntPoints();
}


// -------- ParallelogramShap 实现（追加到 CShap.cpp 末尾） --------

void ParallelogramShap::UpdateIntPoints()
{
	p1Int.x = static_cast<int>(std::round(x1));
	p1Int.y = static_cast<int>(std::round(y1));
	p2Int.x = static_cast<int>(std::round(x2));
	p2Int.y = static_cast<int>(std::round(y2));
	p3Int.x = static_cast<int>(std::round(x3));
	p3Int.y = static_cast<int>(std::round(y3));
	p4Int.x = static_cast<int>(std::round(x4));
	p4Int.y = static_cast<int>(std::round(y4));
}

ParallelogramShap::ParallelogramShap(CPoint a, CPoint b, CPoint c)
{
	x1 = static_cast<double>(a.x); y1 = static_cast<double>(a.y);
	x2 = static_cast<double>(b.x); y2 = static_cast<double>(b.y);
	x3 = static_cast<double>(c.x); y3 = static_cast<double>(c.y);

	// 计算第四点 D = A + (C - B)
	x4 = x1 + (x3 - x2);
	y4 = y1 + (y3 - y2);

	UpdateIntPoints();
}

void ParallelogramShap::Draw(CPaintDC* pdc)
{
	if (!pdc) return;
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&pen);

	pdc->MoveTo(p1Int);
	pdc->LineTo(p2Int);
	pdc->LineTo(p3Int);
	pdc->LineTo(p4Int);
	pdc->LineTo(p1Int);

	pdc->SelectObject(pOldPen);
	pdc->SelectObject(pOldBrush);
}

bool ParallelogramShap::IsSelected(CPoint point)
{
	const double tolerance = 5.0;
	double px = static_cast<double>(point.x);
	double py = static_cast<double>(point.y);

	// 计算浮点顶点数组
	double vx[4] = { x1, x2, x3, x4 };
	double vy[4] = { y1, y2, y3, y4 };

	// 检查到任意边的最短距离
	for (int i = 0; i < 4; ++i) {
		int j = (i + 1) % 4;
		double dist = PointSegmentDistanceDouble(px, py, vx[i], vy[i], vx[j], vy[j]);
		if (dist <= tolerance) return true;
	}

	// 内部点判断（射线法）
	if (PointInPolygon(vx, vy, 4, px, py)) return true;

	return false;
}

void ParallelogramShap::DrawSelection(CPaintDC* pdc)
{
	if (!Selected || !pdc) return;
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
	CPen dashPen(PS_DASH, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&dashPen);

	pdc->MoveTo(p1Int);
	pdc->LineTo(p2Int);
	pdc->LineTo(p3Int);
	pdc->LineTo(p4Int);
	pdc->LineTo(p1Int);

	pdc->SelectObject(pOldBrush);
	pdc->SelectObject(pOldPen);
}

void ParallelogramShap::Move(CSize delta)
{
	x1 += delta.cx; y1 += delta.cy;
	x2 += delta.cx; y2 += delta.cy;
	x3 += delta.cx; y3 += delta.cy;
	x4 += delta.cx; y4 += delta.cy;
	UpdateIntPoints();
}

void ParallelogramShap::Rotate(double degrees)
{
	double rad = degrees * (acos(-1.0) / 180.0);
	CPoint c = GetCenter();
	double cx = static_cast<double>(c.x);
	double cy = static_cast<double>(c.y);
	RotateDoublePoint(x1, y1, cx, cy, rad);
	RotateDoublePoint(x2, y2, cx, cy, rad);
	RotateDoublePoint(x3, y3, cx, cy, rad);
	RotateDoublePoint(x4, y4, cx, cy, rad);
	UpdateIntPoints();
}

CPoint ParallelogramShap::GetCenter() const
{
	double cx = (x1 + x2 + x3 + x4) / 4.0;
	double cy = (y1 + y2 + y3 + y4) / 4.0;
	return CPoint(static_cast<int>(std::round(cx)), static_cast<int>(std::round(cy)));
}

void ParallelogramShap::Scale(double factor, CPoint center)
{
	double cx = static_cast<double>(center.x);
	double cy = static_cast<double>(center.y);
	auto scalePt = [&](double& x, double& y) {
		x = cx + (x - cx) * factor;
		y = cy + (y - cy) * factor;
		};
	scalePt(x1, y1);
	scalePt(x2, y2);
	scalePt(x3, y3);
	scalePt(x4, y4);
	UpdateIntPoints();
}




// 计算贝塞尔曲线 t 点
void CurveShap::GetPointOnBezier(double t, double& outx, double& outy) const
{
	// 三次贝塞尔公式
	double u = 1.0 - t;
	double b0 = u * u * u;
	double b1 = 3.0 * u * u * t;
	double b2 = 3.0 * u * t * t;
	double b3 = t * t * t;
	outx = b0 * x0 + b1 * x1 + b2 * x2 + b3 * x3;
	outy = b0 * y0 + b1 * y1 + b2 * y2 + b3 * y3;
}

void CurveShap::UpdateSamples()
{
	samplesInt.clear();
	if (sampleSegments < 1) sampleSegments = 1;
	samplesInt.reserve(sampleSegments + 1);
	for (int i = 0; i <= sampleSegments; ++i) {
		double t = static_cast<double>(i) / static_cast<double>(sampleSegments);
		double sx, sy;
		GetPointOnBezier(t, sx, sy);
		samplesInt.emplace_back(static_cast<int>(std::round(sx)), static_cast<int>(std::round(sy)));
	}
}

// 构造
CurveShap::CurveShap(CPoint p0, CPoint p1, CPoint p2, CPoint p3)
{
	x0 = static_cast<double>(p0.x); y0 = static_cast<double>(p0.y);
	x1 = static_cast<double>(p1.x); y1 = static_cast<double>(p1.y);
	x2 = static_cast<double>(p2.x); y2 = static_cast<double>(p2.y);
	x3 = static_cast<double>(p3.x); y3 = static_cast<double>(p3.y);
	UpdateSamples();
}

// 绘制：用采样点绘制折线近似曲线
void CurveShap::Draw(CPaintDC* pdc)
{
	if (!pdc) return;
	if (samplesInt.empty()) UpdateSamples();
	// 绘制为折线
	auto it = samplesInt.begin();
	if (it == samplesInt.end()) return;
	pdc->MoveTo(*it);
	for (++it; it != samplesInt.end(); ++it) {
		pdc->LineTo(*it);
	}
}

// 命中检测：点到采样折线段距离
static double PointSegmentDistDoubleLocal(double px, double py, double ax, double ay, double bx, double by)
{
	double abx = bx - ax;
	double aby = by - ay;
	double apx = px - ax;
	double apy = py - ay;
	double ab2 = abx * abx + aby * aby;
	double t = 0.0;
	if (ab2 > 1e-12) t = (apx * abx + apy * aby) / ab2;
	if (t < 0.0) t = 0.0;
	if (t > 1.0) t = 1.0;
	double cx = ax + t * abx;
	double cy = ay + t * aby;
	double dx = px - cx;
	double dy = py - cy;
	return std::sqrt(dx * dx + dy * dy);
}

bool CurveShap::IsSelected(CPoint point)
{
	const double tolerance = 5.0;
	if (samplesInt.empty()) UpdateSamples();
	// 逐段检测
	for (size_t i = 0; i + 1 < samplesInt.size(); ++i) {
		double ax = static_cast<double>(samplesInt[i].x);
		double ay = static_cast<double>(samplesInt[i].y);
		double bx = static_cast<double>(samplesInt[i + 1].x);
		double by = static_cast<double>(samplesInt[i + 1].y);
		double d = PointSegmentDistDoubleLocal(static_cast<double>(point.x), static_cast<double>(point.y), ax, ay, bx, by);
		if (d <= tolerance) return true;
	}
	return false;
}

void CurveShap::DrawSelection(CPaintDC* pdc)
{
	if (!Selected || !pdc) return;
	if (samplesInt.empty()) UpdateSamples();
	CPen dashPen(PS_DASH, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&dashPen);
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));

	pdc->MoveTo(samplesInt[0]);
	for (size_t i = 1; i < samplesInt.size(); ++i) pdc->LineTo(samplesInt[i]);
	// 结束恢复
	pdc->SelectObject(pOldBrush);
	pdc->SelectObject(pOldPen);
}

// 变换：移动
void CurveShap::Move(CSize delta)
{
	x0 += delta.cx; y0 += delta.cy;
	x1 += delta.cx; y1 += delta.cy;
	x2 += delta.cx; y2 += delta.cy;
	x3 += delta.cx; y3 += delta.cy;
	UpdateSamples();
}

// 变换：旋转（绕中心）
void CurveShap::Rotate(double degrees)
{
	double rad = degrees * (acos(-1.0) / 180.0);
	CPoint c = GetCenter();
	double cx = static_cast<double>(c.x);
	double cy = static_cast<double>(c.y);
	RotateDoublePoint(x0, y0, cx, cy, rad);
	RotateDoublePoint(x1, y1, cx, cy, rad);
	RotateDoublePoint(x2, y2, cx, cy, rad);
	RotateDoublePoint(x3, y3, cx, cy, rad);
	UpdateSamples();
}

// 缩放（以 center 为中心）
void CurveShap::Scale(double factor, CPoint center)
{
	double cx = static_cast<double>(center.x);
	double cy = static_cast<double>(center.y);
	auto scalePt = [&](double& x, double& y) {
		x = cx + (x - cx) * factor;
		y = cy + (y - cy) * factor;
		};
	scalePt(x0, y0); scalePt(x1, y1); scalePt(x2, y2); scalePt(x3, y3);
	UpdateSamples();
}

// 中心：控制点平均（可根据需要改为曲线几何中心）
CPoint CurveShap::GetCenter() const
{
	double cx = (x0 + x1 + x2 + x3) / 4.0;
	double cy = (y0 + y1 + y2 + y3) / 4.0;
	return CPoint(static_cast<int>(std::round(cx)), static_cast<int>(std::round(cy)));
}


void PolylineShap::UpdateIntPoints()
{
	ptsInt.clear();
	size_t n = xs.size();
	ptsInt.reserve(n);
	for (size_t i = 0; i < n; ++i) {
		ptsInt.emplace_back(static_cast<int>(std::round(xs[i])), static_cast<int>(std::round(ys[i])));
	}
}

PolylineShap::PolylineShap(const std::vector<CPoint>& points)
{
	xs.clear(); ys.clear();
	for (const auto& p : points) {
		xs.push_back(static_cast<double>(p.x));
		ys.push_back(static_cast<double>(p.y));
	}
	if (xs.size() < 2) {
		// 保证至少两个点（退化为微小线段）
		if (xs.empty()) { xs.push_back(0); ys.push_back(0); }
		xs.push_back(xs.back() + 1.0);
		ys.push_back(ys.back() + 1.0);
	}
	UpdateIntPoints();
}

void PolylineShap::Draw(CPaintDC* pdc)
{
	if (!pdc || ptsInt.empty()) return;
	auto it = ptsInt.begin();
	pdc->MoveTo(*it);
	for (++it; it != ptsInt.end(); ++it) pdc->LineTo(*it);
}

bool PolylineShap::IsSelected(CPoint point)
{
	const double tolerance = 5.0;
	if (ptsInt.size() < 2) return false;

	// 逐段检测点到线段距离
	for (size_t i = 0; i + 1 < ptsInt.size(); ++i) {
		double ax = static_cast<double>(ptsInt[i].x);
		double ay = static_cast<double>(ptsInt[i].y);
		double bx = static_cast<double>(ptsInt[i + 1].x);
		double by = static_cast<double>(ptsInt[i + 1].y);
		double d = PointSegmentDistanceDouble(static_cast<double>(point.x), static_cast<double>(point.y), ax, ay, bx, by);
		if (d <= tolerance) return true;
	}
	// 多义线一般没有“内部”，故仅靠边缘命中
	return false;
}

void PolylineShap::DrawSelection(CPaintDC* pdc)
{
	if (!Selected || !pdc || ptsInt.empty()) return;
	CPen dashPen(PS_DASH, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&dashPen);
	CBrush* pOldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));

	pdc->MoveTo(ptsInt[0]);
	for (size_t i = 1; i < ptsInt.size(); ++i) pdc->LineTo(ptsInt[i]);

	pdc->SelectObject(pOldBrush);
	pdc->SelectObject(pOldPen);
}

void PolylineShap::Move(CSize delta)
{
	for (size_t i = 0; i < xs.size(); ++i) {
		xs[i] += delta.cx;
		ys[i] += delta.cy;
	}
	UpdateIntPoints();
}

void PolylineShap::Rotate(double degrees)
{
	double rad = degrees * (acos(-1.0) / 180.0);
	CPoint c = GetCenter();
	double cx = static_cast<double>(c.x);
	double cy = static_cast<double>(c.y);
	for (size_t i = 0; i < xs.size(); ++i) {
		RotateDoublePoint(xs[i], ys[i], cx, cy, rad);
	}
	UpdateIntPoints();
}

CPoint PolylineShap::GetCenter() const
{
	double sx = 0.0, sy = 0.0;
	size_t n = xs.size();
	for (size_t i = 0; i < n; ++i) { sx += xs[i]; sy += ys[i]; }
	sx /= static_cast<double>(n);
	sy /= static_cast<double>(n);
	return CPoint(static_cast<int>(std::round(sx)), static_cast<int>(std::round(sy)));
}

void PolylineShap::Scale(double factor, CPoint center)
{
	double cx = static_cast<double>(center.x);
	double cy = static_cast<double>(center.y);
	for (size_t i = 0; i < xs.size(); ++i) {
		xs[i] = cx + (xs[i] - cx) * factor;
		ys[i] = cy + (ys[i] - cy) * factor;
	}
	UpdateIntPoints();
}