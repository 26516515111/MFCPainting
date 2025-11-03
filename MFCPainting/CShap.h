#pragma once
#include <cmath>
#include <afxwin.h>
//调用dx2d库
#include <d2d1.h>
#include <d2d1helper.h>
#pragma comment(lib, "d2d1.lib")
class CShap
{
public:
	
	bool Selected;
	virtual void Draw(CPaintDC* pdc) = 0;
	virtual bool IsSelected(CPoint point) = 0;
	virtual void ChangeSelected(CPoint point);
	virtual void DrawSelection(CPaintDC* pdc) = 0;
	virtual void Move(CSize delta) = 0;
	virtual void Rotate(double degrees) = 0;
	virtual CPoint GetCenter() const = 0;

	// 新增：缩放图元
	virtual void Scale(double factor, CPoint center) = 0;

	virtual void Destroy();
	CShap() = default;
};

class LineShap :public CShap {
private:
	CPoint startPoint;
	CPoint endPoint;
public:
	LineShap(CPoint start, CPoint end);
	

	// 通过 CShap 继承
	void Draw(CPaintDC* pdc) override;

	bool IsSelected(CPoint point) override;

	void DrawSelection(CPaintDC* pdc) override;


	// 通过 CShap 继承
	void Move(CSize delta) override;


	// 通过 CShap 继承
	void Rotate(double degrees) override;

	CPoint GetCenter() const override;


	// 通过 CShap 继承
	void Scale(double factor, CPoint center) override;

};

class CircleShap :public CShap {

private:
	CPoint centerPoint;
	CPoint rPoint;

	// 高精度内部表示，避免每次缩放累积精度丢失
	double centerX = 0.0;
	double centerY = 0.0;
	double rdx = 0.0; // rPoint.x - center.x
	double rdy = 0.0;
public:

	CircleShap(CPoint center, CPoint r);

	// 通过 CShap 继承
	void Draw(CPaintDC* pdc) override;
	bool IsSelected(CPoint point) override;
	void DrawSelection(CPaintDC* pdc) override;

	// 通过 CShap 继承
	void Move(CSize delta) override;

	// 通过 CShap 继承
	void Rotate(double degrees) override;
	CPoint GetCenter() const override;

	// 通过 CShap 继承
	void Scale(double factor, CPoint center) override;
};

class RectShap :public CShap {
private:
	// 浮点中心与半宽半高，角度（度）
	double centerX = 0.0;
	double centerY = 0.0;
	double halfW = 0.0;
	double halfH = 0.0;
	double angleDeg = 0.0; // 旋转角度，度

	// 缓存整数顶点（按顺序：左上、右上、右下、左下）
	CPoint cornersInt[4];

	// 计算并更新整数顶点（在变换后调用）
	void UpdateIntCorners();

	// 计算四个浮点顶点（outX/outY 长度为4）
	void ComputeFloatCorners(double outX[4], double outY[4]);
public:
	RectShap(CPoint topLeft, CPoint bottomRight);




	// 通过 CShap 继承
	void Draw(CPaintDC* pdc) override;

	bool IsSelected(CPoint point) override;

	void DrawSelection(CPaintDC* pdc) override;

	void Move(CSize delta) override;

	void Rotate(double degrees) override;

	CPoint GetCenter() const override;

	void Scale(double factor, CPoint center) override;

};

class TriangleShap :public CShap {
private:
	// 精确坐标（double）以避免缩放/旋转精度丢失
	double x1 = 0.0, y1 = 0.0;
	double x2 = 0.0, y2 = 0.0;
	double x3 = 0.0, y3 = 0.0;

	// 缓存整数顶点用于绘制（每次变换后同步）
	CPoint p1Int;
	CPoint p2Int;
	CPoint p3Int;

	// 将 double 顶点同步为整数
	void UpdateIntPoints();

public:
	// 构造：传入三个顶点
	TriangleShap(CPoint a, CPoint b, CPoint c);

	// 通过 CShap 继承
	void Draw(CPaintDC* pdc) override;
	bool IsSelected(CPoint point) override;
	void DrawSelection(CPaintDC* pdc) override;
	void Move(CSize delta) override;
	void Rotate(double degrees) override;
	CPoint GetCenter() const override;
	void Scale(double factor, CPoint center) override;
};

class DiamondShap :public CShap {
private:
	// 浮点中心与半对角向量（横向、纵向），以及旋转角（度）
	double centerX = 0.0;
	double centerY = 0.0;
	double halfDx = 0.0; // 横向半对角（右顶相对于中心的 x 偏移）
	double halfDy = 0.0; // 纵向半对角（下顶相对于中心的 y 偏移）
	double angleDeg = 0.0;

	// 缓存整数顶点（顺序：右、下、左、上）
	CPoint cornersInt[4];

	// 计算并更新整数顶点
	void UpdateIntCorners();

	// 计算四个浮点顶点到 outX/outY（长度 4）
	void ComputeFloatCorners(double outX[4], double outY[4]);

public:
	// 构造：以中心与横向对角端点（rightPoint）与纵向对角端点（bottomPoint）构造
	// rightPoint：位于右顶（或任意位于中心水平线上靠右的点），bottomPoint：位于下顶（或位于中心垂直线下方的点）
	DiamondShap(CPoint topLeft, CPoint bottomRight);

	// 通过 CShap 继承
	void Draw(CPaintDC* pdc) override;
	bool IsSelected(CPoint point) override;
	void DrawSelection(CPaintDC* pdc) override;
	void Move(CSize delta) override;
	void Rotate(double degrees) override;
	CPoint GetCenter() const override;
	void Scale(double factor, CPoint center) override;
};