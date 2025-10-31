
// ChildView.h: CChildView 类的接口
//


#pragma once

#include<vector>
#include<cmath>
#include "CShap.h"

// CChildView 窗口

class CChildView : public CWnd
{
// 构造
public:
	CChildView();

// 特性
public:

// 操作
public:

// 重写
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CChildView();

	// 生成的消息映射函数
protected:
	afx_msg void OnPaint();
	void DrawDot(CPaintDC& dc);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLineDraw();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	void CheckSelectedPoint(CPoint& point);
	void DrawCircle(CPoint& point);
	void DrawLine(CPoint& point);
private:
	std::vector<CPoint> points;
	std::vector<CPoint> dots;
	std::vector<CShap*> Shaps;
#pragma region Line
	bool IsDrawLine = false;
	int LineNum = 2;
#pragma endregion

#pragma region Circle
	bool IsCircle = false;
	int CircleNum = 2;
#pragma endregion

#pragma region Select
	bool IsSelected = false;
#pragma endregion


public:
	afx_msg void OnCircle();
	afx_msg void OnSelect();
//	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDelete();
};

