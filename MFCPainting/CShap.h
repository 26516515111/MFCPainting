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
