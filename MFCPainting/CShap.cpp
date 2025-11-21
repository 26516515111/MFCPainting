#include "pch.h"
#include "CShap.h"
#include <sstream>
#include <iomanip>
#include <set>
#include <string>
#include <algorithm>



// 通用像素样式判断辅助（内部静态）
static bool PixelPatternShouldDraw(int idx, int style)
{
	// idx 为序号（沿主迭代的步数），style 参考定义
	switch (style)
	{
	case 0: return true; // solid
	case 1: { // dash: 画5跳3
		int m = idx % 8;
		return m < 5;
	}
	case 2: { // dot: 画1跳3
		int m = idx % 4;
		return m == 0;
	}
	case 3: { // dashdot: 画5跳3画1跳3循环 (总长: 5+3+1+3=12)
		int m = idx % 12;
		if (m < 5) return true;          // dash
		if (m >= 8 && m == 8) return true; // single dot at position 8
		return false;
	}
	default: return true;
	}
}

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

LineShap::LineShap(CPoint start, CPoint end,int drawmethod, int w, int style)
{
	startPoint = start;
	endPoint = end;
	DrawMethod = drawmethod;
	lineWidth = max(1, w);
	lineStyle = std::clamp(style, 0, 3);
}

void LineShap::Draw(CDC* pdc)
{
	if (DrawMethod == 1) { // 中点法加粗
		Draw_b(pdc);
		return;
	}
	else if (DrawMethod == 2) { // Bresenham 加粗
		Draw_B(pdc);
		return;
	}
	// 普通 GDI
	CPen pen(lineStyle == 0 ? PS_SOLID :
		lineStyle == 1 ? PS_DASH :
		lineStyle == 2 ? PS_DOT : PS_DASHDOT,
		lineWidth, RGB(0, 0, 0));
	CPen* old = pdc->SelectObject(&pen);
	pdc->MoveTo(startPoint);
	pdc->LineTo(endPoint);
	pdc->SelectObject(old);
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

void LineShap::DrawSelection(CDC* pdc)
{
	if (!Selected) return;

	// 使用虚线笔绘制
	CPen pen(PS_DASH, 1, RGB(0, 0, 0));
	CPen* pOldPen = pdc->SelectObject(&pen);
	int originalDrawMethod = DrawMethod;
	DrawMethod = 0;
	pdc->MoveTo(startPoint);
	pdc->LineTo(endPoint);

	pdc->SelectObject(pOldPen);
	DrawMethod = originalDrawMethod;
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

CircleShap::CircleShap(CPoint center, CPoint r,int drawmethod,int w,int style)
{
	centerPoint = center;
	rPoint = r;
	centerX = (double)center.x;
	centerY = (double)center.y;
	rdx = (double)(r.x - center.x);
	rdy = (double)(r.y - center.y);
	DrawMethod = drawmethod;
	lineWidth = max(1, w);
	lineStyle = std::clamp(style, 0, 3);
}

void CircleShap::Draw(CDC* pdc)
{
	if (DrawMethod == 1) { CDraw_b(pdc); return; }
	else if (DrawMethod == 2) { CDraw_B(pdc); return; }

	double radius_d = std::sqrt(rdx * rdx + rdy * rdy);
	int r = (int)std::round(radius_d);

	// 使用 GDI 画粗圆（只画边，不填充内部）
	CPen pen(lineStyle == 0 ? PS_SOLID :
		lineStyle == 1 ? PS_DASH :
		lineStyle == 2 ? PS_DOT : PS_DASHDOT,
		lineWidth, RGB(0, 0, 0));
	CPen* oldPen = pdc->SelectObject(&pen);
	CBrush* oldBrush = pdc->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
	pdc->Ellipse(centerPoint.x - r, centerPoint.y - r, centerPoint.x + r + 1, centerPoint.y + r + 1);
	pdc->SelectObject(oldPen);
	pdc->SelectObject(oldBrush);
}

// 替换/新增 CircleShap::CDraw_b —— 中点圆算法 + 线宽(lineWidth) + 线型(lineStyle)
static bool CirclePatternShouldDrawMid(int idx, int style)
{
	switch (style)
	{
	case 0: return true;                    // solid
	case 1: { int m = idx % 8;  return m < 5; }               // dash 5画3空
	case 2: { return (idx % 4) == 0; }                        // dot 1画3空
	case 3: { int m = idx % 12; return (m < 5) || (m == 8); } // dashdot: 5画3空1画3空
	default: return true;
	}
}

void CircleShap::CDraw_b(CDC* pdc)
{
	if (!pdc) return;

	// 基础半径
	int dx = rPoint.x - centerPoint.x;
	int dy = rPoint.y - centerPoint.y;
	int r = (int)(std::sqrt((double)dx * dx + (double)dy * dy) + 0.5);
	if (r <= 0) { // 退化
		pdc->SetPixel(centerPoint.x, centerPoint.y, RGB(0, 0, 0));
		return;
	}

	const int thick = max(1, lineWidth);
	const int half = (thick - 1) / 2;

	// 绘制一个“粗点”——简易方块加粗（可换成环或圆形点，当前足够满足线宽要求）
	auto PutThick = [&](int px, int py, int idx)
		{
			if (!CirclePatternShouldDrawMid(idx, lineStyle)) return;
			if (thick == 1)
			{
				pdc->SetPixel(px, py, RGB(0, 0, 0));
				return;
			}
			for (int oy = -half; oy <= half; ++oy)
				for (int ox = -half; ox <= half; ++ox)
					pdc->SetPixel(px + ox, py + oy, RGB(0, 0, 0));
		};

	// 八向对称
	auto Plot8 = [&](int x, int y, int& idx)
		{
			int cx = centerPoint.x;
			int cy = centerPoint.y;
			PutThick(cx + x, cy + y, idx++);
			PutThick(cx - x, cy + y, idx++);
			PutThick(cx + x, cy - y, idx++);
			PutThick(cx - x, cy - y, idx++);
			PutThick(cx + y, cy + x, idx++);
			PutThick(cx - y, cy + x, idx++);
			PutThick(cx + y, cy - x, idx++);
			PutThick(cx - y, cy - x, idx++);
		};

	// 中点圆算法初始化
	int x = 0;
	int y = r;
	int d = 1 - r;
	int patternIndex = 0;

	Plot8(x, y, patternIndex);
	while (x < y)
	{
		++x;
		if (d < 0)
		{
			d += 2 * x + 1;
		}
		else
		{
			--y;
			d += 2 * (x - y) + 1;
		}
		Plot8(x, y, patternIndex);
	}
}

void CircleShap::CDraw_B(CDC* pdc)
{
	if (!pdc) return;

	// 计算半径（使用当前 rPoint 与 centerPoint）
	int dx = rPoint.x - centerPoint.x;
	int dy = rPoint.y - centerPoint.y;
	int r = (int)(std::sqrt(dx * dx + dy * dy) + 0.5);
	if (r <= 0) return;

	// 线宽参数
	int thick = max(1, lineWidth);
	int half = (thick - 1) / 2;

	// 线型模式判定（同直线逻辑）
	auto PatternDraw = [&](int stepIndex)->bool {
		switch (lineStyle)
		{
		case 0: return true; // solid
		case 1: { // dash: 画5跳3
			int m = stepIndex % 8;
			return m < 5;
		}
		case 2: { // dot: 画1跳3
			int m = stepIndex % 4;
			return m == 0;
		}
		case 3: { // dashdot: 画5跳3画1跳3 (总周期12)
			int m = stepIndex % 12;
			if (m < 5) return true;      // dash
			if (m == 8) return true;     // dot
			return false;
		}
		default: return true;
		}
		};

	// 绘制一个点（考虑线宽：使用局部小块，简单近似）
	auto PutPixelThick = [&](int px, int py, int stepIndex) {
		if (!PatternDraw(stepIndex)) return;
		if (thick == 1) {
			pdc->SetPixel(px, py, RGB(0, 0, 0));
		}
		else {
			for (int oy = -half; oy <= half; ++oy)
				for (int ox = -half; ox <= half; ++ox)
					pdc->SetPixel(px + ox, py + oy, RGB(0, 0, 0));
		}
		};

	// 八向对称绘制
	auto Plot8 = [&](int x, int y, int stepIndex) {
		int cx = centerPoint.x;
		int cy = centerPoint.y;
		PutPixelThick(cx + x, cy + y, stepIndex);
		PutPixelThick(cx - x, cy + y, stepIndex);
		PutPixelThick(cx + x, cy - y, stepIndex);
		PutPixelThick(cx - x, cy - y, stepIndex);
		PutPixelThick(cx + y, cy + x, stepIndex);
		PutPixelThick(cx - y, cy + x, stepIndex);
		PutPixelThick(cx + y, cy - x, stepIndex);
		PutPixelThick(cx - y, cy - x, stepIndex);
		};

	// Bresenham 初始化
	int x = 0;
	int y = r;
	int d = 3 - 2 * r;
	int stepIndex = 0;

	Plot8(x, y, stepIndex++);

	while (x <= y) {
		++x;
		if (d < 0) {
			d += 4 * x + 6;
		}
		else {
			d += 4 * (x - y) + 10;
			--y;
		}
		Plot8(x, y, stepIndex++);
	}

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

void CircleShap::DrawSelection(CDC* pdc)
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
	centerPoint.x = (int)std::round(centerX);
	centerPoint.y = (int)std::round(centerY);

	// 关键：同时平移 rPoint，保持 rPoint - centerPoint 的向量不变
	rPoint.Offset(delta);

	// 若使用 rdx/rdy 的分支，也可同步更新，避免以后混用造成差异
	rdx = (double)(rPoint.x - centerPoint.x);
	rdy = (double)(rPoint.y - centerPoint.y);
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

void DiamondShap::Draw(CDC* pdc)
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

void DiamondShap::DrawSelection(CDC* pdc)
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

void RectShap::Draw(CDC* pdc)
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

void RectShap::DrawSelection(CDC* pdc)
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

void TriangleShap::Draw(CDC* pdc)
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

void TriangleShap::DrawSelection(CDC* pdc)
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

void ParallelogramShap::Draw(CDC* pdc)
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

void ParallelogramShap::DrawSelection(CDC* pdc)
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
void CurveShap::Draw(CDC* pdc)
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

void CurveShap::DrawSelection(CDC* pdc)
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

void PolylineShap::Draw(CDC* pdc)
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

void PolylineShap::DrawSelection(CDC* pdc)
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


// LineShap
void LineShap::GetEndpoints(CPoint& a, CPoint& b) { a = startPoint; b = endPoint; }

LineShap* LineShap::CreatePerpendicularAt(CPoint pointOnOrNearLine, double halfLength) const
{
	// 线段端点
	double x1 = static_cast<double>(startPoint.x);
	double y1 = static_cast<double>(startPoint.y);
	double x2 = static_cast<double>(endPoint.x);
	double y2 = static_cast<double>(endPoint.y);

	// 方向向量
	double dx = x2 - x1;
	double dy = y2 - y1;
	double len2 = dx * dx + dy * dy;
	const double EPS = 1e-9;
	if (len2 < EPS) {
		// 退化为点，无法生成垂线
		return nullptr;
	}

	// 将传入点投影到由 (startPoint->endPoint) 定义的直线（保证在直线上）
	double px = static_cast<double>(pointOnOrNearLine.x);
	double py = static_cast<double>(pointOnOrNearLine.y);
	double t = ((px - x1) * dx + (py - y1) * dy) / len2;
	double projx = x1 + t * dx;
	double projy = y1 + t * dy;

	// 垂直方向向量 = (-dy, dx)，归一化
	double vx = -dy;
	double vy = dx;
	double vlen = std::hypot(vx, vy);
	if (vlen < EPS) return nullptr;
	vx /= vlen;
	vy /= vlen;

	// 两端点
	double ax = projx + vx * halfLength;
	double ay = projy + vy * halfLength;
	double bx = projx - vx * halfLength;
	double by = projy - vy * halfLength;

	CPoint a(static_cast<int>(std::lround(ax)), static_cast<int>(std::lround(ay)));
	CPoint b(static_cast<int>(std::lround(bx)), static_cast<int>(std::lround(by)));

	return new LineShap(a, b);
}

void LineShap::Draw_b(CDC* pdc)
{
	// 为简洁复用原算法结构，添加粗线绘制辅助 lambda
	if (!pdc) return;

	// 用于样式判断与粗线绘制
	auto PlotThickPixel = [&](int cx, int cy, int stepIndex)
		{
			if (!PixelPatternShouldDraw(stepIndex, lineStyle)) return;
			int half = (lineWidth - 1) / 2;
			if (lineWidth == 1) {
				pdc->SetPixel(cx, cy, RGB(0, 0, 0));
				return;
			}
			// 简易法向扩展：垂直方向
			for (int dy = -half; dy <= half; ++dy)
				pdc->SetPixel(cx, cy + dy, RGB(0, 0, 0));
		};

	// 保证左到右
	if (startPoint.x > endPoint.x) std::swap(startPoint, endPoint);

	// 垂直线
	if (startPoint.x == endPoint.x) {
		if (startPoint.y > endPoint.y) std::swap(startPoint, endPoint);
		int stepIndex = 0;
		for (int y = startPoint.y; y <= endPoint.y; ++y)
			PlotThickPixel(startPoint.x, y, stepIndex++);
		return;
	}

	float m = (float)(endPoint.y - startPoint.y) / (endPoint.x - startPoint.x);
	int stepIndex = 0;

	if (m >= 0 && m <= 1) {
		int dx = endPoint.x - startPoint.x;
		int dy = endPoint.y - startPoint.y;
		int d = 2 * dy - dx;
		int incrE = 2 * dy;
		int incrNE = 2 * (dy - dx);
		int x = startPoint.x;
		int y = startPoint.y;
		PlotThickPixel(x, y, stepIndex++);
		while (x < endPoint.x) {
			if (d <= 0) { d += incrE; ++x; }
			else { d += incrNE; ++x; ++y; }
			PlotThickPixel(x, y, stepIndex++);
		}
	}
	else if (m > 1) {
		int dx = endPoint.x - startPoint.x;
		int dy = endPoint.y - startPoint.y;
		int d = dy - 2 * dx;
		int incrN = -2 * dx;
		int incrNE = 2 * (dy - dx);
		int x = startPoint.x;
		int y = startPoint.y;
		PlotThickPixel(x, y, stepIndex++);
		while (y < endPoint.y) {
			if (d <= 0) { d += incrNE; ++x; ++y; }
			else { d += incrN; ++y; }
			PlotThickPixel(x, y, stepIndex++);
		}
	}
	else if (m < 0 && m >= -1) {
		int dx = endPoint.x - startPoint.x;
		int dy = endPoint.y - startPoint.y;
		int d = 2 * dy + dx;
		int incrE = 2 * dy;
		int incrSE = 2 * (dy + dx);
		int x = startPoint.x;
		int y = startPoint.y;
		PlotThickPixel(x, y, stepIndex++);
		while (x < endPoint.x) {
			if (d <= 0) { d += incrSE; ++x; --y; }
			else { d += incrE; ++x; }
			PlotThickPixel(x, y, stepIndex++);
		}
	}
	else { // m < -1
		int dx = endPoint.x - startPoint.x;
		int dy = endPoint.y - startPoint.y;
		int d = dy + 2 * dx;
		int incrS = 2 * dx;
		int incrSE = 2 * (dy + dx);
		int x = startPoint.x;
		int y = startPoint.y;
		PlotThickPixel(x, y, stepIndex++);
		while (y > endPoint.y) {
			if (d <= 0) { d += incrS; --y; }
			else { d += incrSE; ++x; --y; }
			PlotThickPixel(x, y, stepIndex++);
		}
	}
}

void LineShap::Draw_B(CDC* pdc)
{
	if (!pdc) return;
	auto PlotThickPixel = [&](int cx, int cy, int stepIndex)
		{
			if (!PixelPatternShouldDraw(stepIndex, lineStyle)) return;
			int half = (lineWidth - 1) / 2;
			if (lineWidth == 1) {
				pdc->SetPixel(cx, cy, RGB(0, 0, 0));
				return;
			}
			for (int dy = -half; dy <= half; ++dy)
				pdc->SetPixel(cx, cy + dy, RGB(0, 0, 0));
		};

	int x0 = startPoint.x;
	int y0 = startPoint.y;
	int x1 = endPoint.x;
	int y1 = endPoint.y;

	if (x0 > x1) { std::swap(startPoint, endPoint); x0 = startPoint.x; y0 = startPoint.y; x1 = endPoint.x; y1 = endPoint.y; }

	// 垂直线
	if (x0 == x1) {
		if (y0 > y1) std::swap(y0, y1);
		int stepIndex = 0;
		for (int y = y0; y <= y1; ++y)
			PlotThickPixel(x0, y, stepIndex++);
		return;
	}

	int dx = x1 - x0;
	int dy = y1 - y0;
	float m = (float)dy / dx;
	int stepIndex = 0;

	if (std::fabs(m) <= 1.0f) {
		int y = y0;
		int S = (dy > 0) ? 1 : -1;
		int absDy = std::abs(dy);
		int error = 2 * absDy - dx;
		for (int x = x0; x <= x1; ++x) {
			PlotThickPixel(x, y, stepIndex++);
			if (error > 0) { y += S; error -= 2 * dx; }
			error += 2 * absDy;
		}
	}
	else {
		if (y0 > y1) { std::swap(x0, x1); std::swap(y0, y1); dx = x1 - x0; dy = y1 - y0; }
		int x = x0;
		int S = (dx > 0) ? 1 : -1;
		int absDx = std::abs(dx);
		int error = 2 * absDx - (y1 - y0);
		for (int y = y0; y <= y1; ++y) {
			PlotThickPixel(x, y, stepIndex++);
			if (error > 0) { x += S; error -= 2 * (y1 - y0); }
			error += 2 * absDx;
		}
	}

}

void LineShap::DrawD2D(CDx2D& dx)
{
	CPoint a = startPoint, b = endPoint;
	switch (DrawMethod) {
	case 1: // 中点法
		dx.DrawLineMidpoint(a, b,
			(float)lineWidth,
			lineStyle,
			D2D1::ColorF(D2D1::ColorF::Black));
		break;
	case 2: // Bresenham
		dx.DrawLineBresenham(a, b,
			(float)lineWidth,
			lineStyle,
			D2D1::ColorF(D2D1::ColorF::Black));
		break;
	default: // 直接
		dx.DrawLineDirect(a, b,
			(float)lineWidth,
			lineStyle,
			D2D1::ColorF(D2D1::ColorF::Black));
		break;
	}
}

void LineShap::DrawSelectionD2D(CDx2D& dx)
{
	if (!Selected) return;
	// 使用虚线样式 (style=1)，线宽略加 1 方便区分
	dx.DrawLineDirect(startPoint, endPoint,
		(float)max(1, lineWidth + 1),
		1,
		D2D1::ColorF(D2D1::ColorF::Red)); // 改为红色高亮，
}

// CircleShap
void CircleShap::GetCenterAndRadius(CPoint& centerOut, double& radiusOut)
{
	// 如果内部使用 double centerX/centerY，优先使用；否则回退到 centerPoint
	centerOut.x = static_cast<int>(std::round(centerX != 0.0 || centerY != 0.0 ? centerX : centerPoint.x));
	centerOut.y = static_cast<int>(std::round(centerY != 0.0 || centerX != 0.0 ? centerY : centerPoint.y));
	// 半径由 rdx/rdy 或 rPoint 计算
	if (rdx != 0.0 || rdy != 0.0) {
		radiusOut = std::sqrt(rdx * rdx + rdy * rdy);
	}
	else {
		double dx = rPoint.x - centerOut.x;
		double dy = rPoint.y - centerOut.y;
		radiusOut = std::sqrt(dx * dx + dy * dy);
	}
}

void CircleShap::ShowCenter(CDC* pdc)
{
	if (!pdc) return;

	// 获取中心与半径
	CPoint center;
	double radius = 0.0;
	GetCenterAndRadius(center, radius);

	// 绘制圆心点（实心小圆）
	CBrush brush(RGB(255, 0, 0)); // 红色点
	CBrush* pOldBrush = pdc->SelectObject(&brush);
	const int dotR = 4; // 点半径像素
	pdc->Ellipse(center.x - dotR, center.y - dotR, center.x + dotR + 1, center.y + dotR + 1);
	pdc->SelectObject(pOldBrush);

	// 在左上角显示坐标文本
	COLORREF oldColor = pdc->SetTextColor(RGB(0, 0, 0));
	int oldBkMode = pdc->SetBkMode(TRANSPARENT);

	CString msg;
	msg.Format(_T("圆心: (%d, %d)  坐标: x=%.2f y=%.2f  半径=%.2f"),
		center.x, center.y,
		static_cast<double>(center.x), static_cast<double>(center.y),
		radius);

	// 留一点边距
	const int textX = 2;
	const int textY = 2;
	pdc->TextOut(textX, textY, msg);

	// 恢复原有画笔/文本属性
	pdc->SetTextColor(oldColor);
	pdc->SetBkMode(oldBkMode);
}

LineShap* CircleShap::CreateTangentAt(CPoint pointOnOrNearCircle, double halfLength)
{
	// 获取圆心与半径
	CPoint center;
	double radius = 0.0;
	GetCenterAndRadius(center, radius);
	

	// 向量从圆心指向输入点
	double vx = static_cast<double>(pointOnOrNearCircle.x - center.x);
	double vy = static_cast<double>(pointOnOrNearCircle.y - center.y);
	double dist = std::sqrt(vx * vx + vy * vy);

	// 距离太小（几乎在圆心），无法定义切线
	if (dist < 1e-6) return nullptr;

	// 若输入点不在圆上，则将其投影到圆周（保证切线正确）
	if (std::abs(dist - radius) > 1.0) { // 容差 1 像素
		vx = vx / dist * radius;
		vy = vy / dist * radius;
		pointOnOrNearCircle.x = static_cast<int>(std::round(center.x + vx));
		pointOnOrNearCircle.y = static_cast<int>(std::round(center.y + vy));
		// 更新向量与距离为圆周点
		dist = radius;
	}

	// 切线方向垂直于半径向量 (vx, vy)，取方向 (-vy, vx)
	double tx = -vy;
	double ty = vx;
	double tlen = std::sqrt(tx * tx + ty * ty);
	if (tlen < 1e-6) return nullptr;
	tx /= tlen;
	ty /= tlen;

	// 计算切线两端点
	CPoint p1(
		static_cast<int>(std::round(pointOnOrNearCircle.x + tx * halfLength)),
		static_cast<int>(std::round(pointOnOrNearCircle.y + ty * halfLength))
	);
	CPoint p2(
		static_cast<int>(std::round(pointOnOrNearCircle.x - tx * halfLength)),
		static_cast<int>(std::round(pointOnOrNearCircle.y - ty * halfLength))
	);

	// 返回新创建的 LineShap（调用者负责释放或将其加入管理容器）
	return new LineShap(p1, p2);
}

void CircleShap::DrawD2D(CDx2D& dx)
{
	CPoint c; double rr = 0.0;
	GetCenterAndRadius(c, rr);
	int r = (int)std::round(rr);
	if (r <= 0) return;
	// DrawMethod: 0=Direct2D,1=中点,2=Bresenham
	dx.DrawCircleSmart(c, r,
		(float)lineWidth,
		lineStyle,
		DrawMethod,
		D2D1::ColorF(D2D1::ColorF::Black));
}

void CircleShap::DrawSelectionD2D(CDx2D& dx)
{
	if (!Selected) return;
	CPoint c; double rr = 0.0;
	GetCenterAndRadius(c, rr);
	dx.DrawCircleDirect(c, (float)rr,
		(float)max(1, lineWidth + 1),
		1,
		D2D1::ColorF(D2D1::ColorF::Red));
}

// RectShap
void RectShap::GetCornersInt(CPoint outCorners[4])
{
	UpdateIntCorners();
	for (int i = 0; i < 4; ++i) outCorners[i] = cornersInt[i];
}

// TriangleShap
void TriangleShap::GetIntPoints(CPoint& a, CPoint& b, CPoint& c)
{
	UpdateIntPoints();
	a = p1Int; b = p2Int; c = p3Int;
}

// DiamondShap
void DiamondShap::GetCornersInt(CPoint outCorners[4])
{
	UpdateIntCorners();
	for (int i = 0; i < 4; ++i) outCorners[i] = cornersInt[i];
}

// ParallelogramShap
void ParallelogramShap::GetIntPoints(CPoint& a, CPoint& b, CPoint& c, CPoint& d)
{
	UpdateIntPoints();
	a = p1Int; b = p2Int; c = p3Int; d = p4Int;
}

// CurveShap
const std::vector<CPoint>& CurveShap::GetSamplesInt()
{
	UpdateSamples();
	return samplesInt;
}

// PolylineShap
const std::vector<CPoint>& PolylineShap::GetIntPoints()
{
	UpdateIntPoints();
	return ptsInt;
}

// ---------------- - 几何工具函数（本文件作用域）---------------- -

namespace {
	const double EPS = 1e-8;

	struct DPoint { double x, y; };
	inline DPoint toD(const CPoint& p) { return { (double)p.x, (double)p.y }; }
	inline CPoint toI(const DPoint& p) { return CPoint((int)std::lround(p.x), (int)std::lround(p.y)); }

	// 线段 - 线段交点（若有交点且在段内） -> 返回 {true, point}
	bool SegSegIntersect(const DPoint& p1, const DPoint& p2, const DPoint& p3, const DPoint& p4, DPoint& out)
	{
		// 参数化求解
		double x1 = p1.x, y1 = p1.y;
		double x2 = p2.x, y2 = p2.y;
		double x3 = p3.x, y3 = p3.y;
		double x4 = p4.x, y4 = p4.y;
		double den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
		if (std::abs(den) < EPS) return false; // 平行或共线（不处理重叠段）
		double px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / den;
		double py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / den;
		// 检查是否在两个线段上
		auto inSeg = [&](double x, double a, double b)->bool {
			double lo = min(a, b) - EPS, hi = max(a, b) + EPS;
			return x >= lo && x <= hi;
			};
		if (inSeg(px, x1, x2) && inSeg(py, y1, y2) && inSeg(px, x3, x4) && inSeg(py, y3, y4)) {
			out = { px, py };
			return true;
		}
		return false;
	}

	// 线段 - 圆 交点（可能 0/1/2 个），返回所有位于段内的交点
	std::vector<DPoint> SegCircleIntersect(const DPoint& p1, const DPoint& p2, const DPoint& c, double r)
	{
		std::vector<DPoint> res;
		// 把线段作为 p = p1 + t*(d), t in [0,1]
		DPoint d{ p2.x - p1.x, p2.y - p1.y };
		DPoint f{ p1.x - c.x, p1.y - c.y };
		double a = d.x * d.x + d.y * d.y;
		double b = 2 * (f.x * d.x + f.y * d.y);
		double cc = f.x * f.x + f.y * f.y - r * r;
		double disc = b * b - 4 * a * cc;
		if (disc < -EPS) return res;
		disc = max(0.0, disc);
		double sqrtD = std::sqrt(disc);
		double t1 = (-b - sqrtD) / (2 * a);
		double t2 = (-b + sqrtD) / (2 * a);
		auto checkT = [&](double t) {
			if (t + EPS >= 0.0 && t <= 1.0 + EPS) {
				return true;
			}
			return false;
			};
		if (checkT(t1)) res.push_back({ p1.x + d.x * t1, p1.y + d.y * t1 });
		if (disc > EPS && checkT(t2)) res.push_back({ p1.x + d.x * t2, p1.y + d.y * t2 });
		return res;
	}

	// 圆 - 圆 交点（0/1/2）
	std::vector<DPoint> CircleCircleIntersect(const DPoint& c1, double r1, const DPoint& c2, double r2)
	{
		std::vector<DPoint> res;
		double dx = c2.x - c1.x;
		double dy = c2.y - c1.y;
		double d = std::hypot(dx, dy);
		if (d < EPS) return res; // 同心或几乎同心，忽略
		if (d > r1 + r2 + EPS) return res; // 相离
		if (d < std::abs(r1 - r2) - EPS) return res; // 内含
		// 求交点
		double a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
		double h2 = r1 * r1 - a * a;
		if (h2 < -EPS) return res;
		h2 = max(0.0, h2);
		double xm = c1.x + a * (dx) / d;
		double ym = c1.y + a * (dy) / d;
		double rx = -dy * (std::sqrt(h2) / d);
		double ry = dx * (std::sqrt(h2) / d);
		if (h2 < EPS) {
			res.push_back({ xm, ym });
		}
		else {
			res.push_back({ xm + rx, ym + ry });
			res.push_back({ xm - rx, ym - ry });
		}
		return res;
	}

	// 合并近似重复点（基于 tol）
	std::vector<CPoint> UniqueRoundPoints(const std::vector<DPoint>& in, double tol = 1e-3)
	{
		std::vector<DPoint> tmp = in;
		std::vector<DPoint> out;
		for (const auto& p : tmp) {
			bool found = false;
			for (const auto& q : out) {
				if (std::hypot(p.x - q.x, p.y - q.y) <= tol) { found = true; break; }
			}
			if (!found) out.push_back(p);
		}
		std::vector<CPoint> res;
		res.reserve(out.size());
		for (auto& p : out) res.push_back(toI(p));
		return res;
	}
}

// ----------------- ShapController 实现 -----------------

ShapController::ShapController(std::vector<CShap*> shape)
	: shapes(std::move(shape))
{
}

// 将任意 shape 转换为线段集合（以 double 表示）
// 对于圆类型返回空线段集合（圆单独处理）
static void ExtractSegmentsOrCircle(CShap* s,
	std::vector<std::pair<DPoint, DPoint>>& outSegments,
	bool& isCircle, DPoint& circleCenter, double& circleRadius)
{
	isCircle = false;
	// LineShap
	if (auto ls = dynamic_cast<LineShap*>(s)) {
		CPoint a, b; ls->GetEndpoints(a, b);
		outSegments.push_back({ toD(a), toD(b) });
		return;
	}
	// CircleShap
	if (auto cs = dynamic_cast<CircleShap*>(s)) {
		CPoint c; double r;
		cs->GetCenterAndRadius(c, r);
		isCircle = true; circleCenter = toD(c); circleRadius = r;
		return;
	}
	// Rect/Triangle/Diamond/Parallelogram -> 4-or-3-point polygon
	if (auto rs = dynamic_cast<RectShap*>(s)) {
		CPoint corners[4]; rs->GetCornersInt(corners);
		for (int i = 0; i < 4; i++) {
			outSegments.push_back({ toD(corners[i]), toD(corners[(i + 1) % 4]) });
		}
		return;
	}
	if (auto ts = dynamic_cast<TriangleShap*>(s)) {
		CPoint a, b, c; ts->GetIntPoints(a, b, c);
		outSegments.push_back({ toD(a), toD(b) });
		outSegments.push_back({ toD(b), toD(c) });
		outSegments.push_back({ toD(c), toD(a) });
		return;
	}
	if (auto ds = dynamic_cast<DiamondShap*>(s)) {
		CPoint corners[4]; ds->GetCornersInt(corners);
		for (int i = 0; i < 4; i++) outSegments.push_back({ toD(corners[i]), toD(corners[(i + 1) % 4]) });
		return;
	}
	if (auto ps = dynamic_cast<ParallelogramShap*>(s)) {
		CPoint a, b, c, d; ps->GetIntPoints(a, b, c, d);
		outSegments.push_back({ toD(a), toD(b) });
		outSegments.push_back({ toD(b), toD(c) });
		outSegments.push_back({ toD(c), toD(d) });
		outSegments.push_back({ toD(d), toD(a) });
		return;
	}
	// PolylineShap
	if (auto pls = dynamic_cast<PolylineShap*>(s)) {
		auto pts = pls->GetIntPoints();
		if (pts.size() >= 2) {
			for (size_t i = 0; i + 1 < pts.size(); ++i) outSegments.push_back({ toD(pts[i]), toD(pts[i + 1]) });
		}
		return;
	}
	// CurveShap -> 使用采样点近似
	if (auto cs = dynamic_cast<CurveShap*>(s)) {
		auto samp = cs->GetSamplesInt();
		if (samp.size() >= 2) {
			for (size_t i = 0; i + 1 < samp.size(); ++i) outSegments.push_back({ toD(samp[i]), toD(samp[i + 1]) });
		}
		return;
	}
	// 其它类型未处理
}

// 计算所有图元间交点
std::vector<CPoint> ShapController::ComputeAllIntersections() const
{
	std::vector<DPoint> found;
	// 对每一对 shapes
	for (size_t i = 0; i < shapes.size(); ++i) {
		for (size_t j = i + 1; j < shapes.size(); ++j) {
			CShap* a = shapes[i];
			CShap* b = shapes[j];
			// 先分别提取线段集合或圆信息
			std::vector<std::pair<DPoint, DPoint>> segA, segB;
			bool aIsCircle = false, bIsCircle = false;
			DPoint aC; double aR = 0.0; DPoint bC; double bR = 0.0;
			// NOTE: ExtractSegmentsOrCircle 接受 非const CShap*；但我们在 const 成员里有 shaps 为 mutable? shaps 成员是 non-const pointers,
			// 我们需要去掉 constness：const_cast 用于调用提取
			ExtractSegmentsOrCircle(const_cast<CShap*>(a), segA, aIsCircle, aC, aR);
			ExtractSegmentsOrCircle(const_cast<CShap*>(b), segB, bIsCircle, bC, bR);

			// circle - circle
			if (aIsCircle && bIsCircle) {
				auto pts = CircleCircleIntersect(aC, aR, bC, bR);
				for (auto& p : pts) found.push_back(p);
				continue;
			}

			// segment - circle (either order)
			if (aIsCircle && !bIsCircle) {
				for (auto& seg : segB) {
					auto pts = SegCircleIntersect(seg.first, seg.second, aC, aR);
					for (auto& p : pts) found.push_back(p);
				}
				continue;
			}
			if (!aIsCircle && bIsCircle) {
				for (auto& seg : segA) {
					auto pts = SegCircleIntersect(seg.first, seg.second, bC, bR);
					for (auto& p : pts) found.push_back(p);
				}
				continue;
			}

			// segment - segment 全部组合
			for (auto& sa : segA) {
				for (auto& sb : segB) {
					DPoint ip;
					if (SegSegIntersect(sa.first, sa.second, sb.first, sb.second, ip)) {
						found.push_back(ip);
					}
				}
			}
		}
	}
	// 去重并四舍五入为整数点
	auto uniqueInts = UniqueRoundPoints(found, 1e-3);
	return uniqueInts;
}

// 在屏幕左上角绘制交点列表
void ShapController::DrawIntersections(CDC* pdc) const
{
	if (!pdc) return;
	auto pts = ComputeAllIntersections();
	int x = 2, y = 2;
	std::wstring line;
	for (size_t i = 0; i < pts.size(); ++i) {
		std::wostringstream wss;
		wss << L"(" << pts[i].x << L"," << pts[i].y << L")";
		line = wss.str();
		pdc->TextOutW(x, y, line.c_str());
		y += 16; // 行间距
		// 避免太长列表溢出：若有太多可以只显示前 N（此处显示全部）
	}
}
