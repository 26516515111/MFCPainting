
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
	bool StateCheck();
	void DrawDot(CPaintDC& dc);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLineDraw();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	void DrawPoly(CPoint& point);
	void DrawCur(CPoint& point);
	void DrawPara(CPoint& point);
	void DrawDiamond(CPoint& point);
	void DrawTriangle(CPoint& point);
	void DrawRect(CPoint& point);
	void CheckSelectedPoint(CPoint& point);
	void DrawCircle(CPoint& point);
	void DrawLine(CPoint& point);
private:
	std::vector<CPoint> points;
	std::vector<CPoint> dots;
	std::vector<CShap*> Shaps;


	// D2D资源
	ID2D1Factory* m_pD2DFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	ID2D1SolidColorBrush* m_pBlackBrush;
	ID2D1SolidColorBrush* m_pRedBrush;
	ID2D1SolidColorBrush* m_pSelectionBrush;

	BOOL InitializeD2D();
	void CleanupD2D();
	void ResizeD2D();


#pragma region Line
	bool IsDrawLine = false;
	int LineNum = 2;
#pragma endregion

#pragma region Circle
	bool IsCircle = false;
	int CircleNum = 2;
#pragma endregion

#pragma region Rect
	bool IsRect = false;
	int RectNum = 2;
#pragma endregion

#pragma region Triangle
	bool IsTriangle = false;
	int TriangleNum = 3;
#pragma endregion

#pragma region DiamondShap
	bool IsDiamond = false;
	int DiamondNum = 2;
#pragma endregion

#pragma region ParaShap
	bool IsPara = false;
	int ParaNum = 3;
#pragma endregion

#pragma region CurShap
	bool IsCur = false;
	int CurNum = 4;
#pragma endregion

#pragma region Polyline
	bool IsPoly = false;
#pragma endregion

#pragma region Select
	bool IsSelected = false;
	bool IsSelectedSave = false;
#pragma endregion

#pragma region Move
	// 拖拽相关
	CShap* m_pDragged = nullptr;   // 正在拖拽的图元指针（nullptr 表示无）
	bool m_bDragging = false;      // 是否处于拖拽中
	CPoint m_lastMouse;            // 上一次鼠标位置
#pragma endregion

#pragma region Ratota
	// 拖拽相关
	bool m_bRotating = false;
	CShap* m_pRotated = nullptr; // 当前正在旋转的图元
	double m_lastRotateAngle = 0.0; // 上一次鼠标角度（弧度）
	CPoint m_rotateCenter; // 旋转中心（缓存）
#pragma endregion

//求交点
#pragma region Intersection
	bool IsInter = false;
	std::vector<CShap*> AbleShapes;
#pragma endregion

#pragma region Vertical
	bool IsVertical = false;
#pragma endregion
#pragma region CircleCenter
	bool IsCircleCenter = false;
	CShap* targetShape = NULL;
#pragma endregion


public:


	afx_msg void OnCircle();
	afx_msg void OnSelect();
//	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDelete();


	// 提供D2D资源访问
	ID2D1RenderTarget* GetD2DRenderTarget() { return m_pRenderTarget; }
	ID2D1SolidColorBrush* GetBlackBrush() { return m_pBlackBrush; }
	ID2D1SolidColorBrush* GetRedBrush() { return m_pRedBrush; }
	
	
	ID2D1SolidColorBrush* GetSelectionBrush() { return m_pSelectionBrush; }
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRect();
	afx_msg void OnTriangle();
	afx_msg void OnDiamond();
	afx_msg void OnPara();
	afx_msg void OnCur();
	afx_msg void OnPolyline();
	afx_msg void OnIntersection();
	afx_msg void OnVertical();
	afx_msg void OnCircleCenter();
};

