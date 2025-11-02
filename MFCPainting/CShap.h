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
