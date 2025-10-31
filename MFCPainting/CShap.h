#pragma once
#include <cmath>
class CShap
{
public:
	
	bool Selected;
	virtual void Draw(CPaintDC* pdc) = 0;
	virtual bool IsSelected(CPoint point) = 0;
	virtual void ChangeSelected(CPoint point);
	virtual void DrawSelection(CPaintDC* pdc) = 0;
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

};

class CircleShap :public CShap {

private:
	CPoint centerPoint;
	CPoint rPoint;
public:

	CircleShap(CPoint center, CPoint r);

	// 通过 CShap 继承
	void Draw(CPaintDC* pdc) override;
	bool IsSelected(CPoint point) override;
	void DrawSelection(CPaintDC* pdc) override;
};

