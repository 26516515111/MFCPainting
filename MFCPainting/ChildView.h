// ChildView.h: CChildView 类的接口
//


#pragma once

#include<vector>
#include<cmath>
#include "CShap.h"
#include <atlimage.h> 
#include "Dx2D.h"
#include <memory>
#include<stack>

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
	/*void DrawDot(CPaintDC& dc);*/
	void DrawDot(CDC& dc);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLineDraw();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	void DrawPolyGon(CPoint& point);
	void DrawPoly(CPoint& point);
	void DrawCur(CPoint& point);
	void DrawPara(CPoint& point);
	void DrawDiamond(CPoint& point);
	void DrawTriangle(CPoint& point);
	void DrawRect(CPoint& point);
	void CheckSelectedPoint(CPoint& point);
	void DrawCircle(CPoint& point);
	void DrawLine(CPoint& point);

	// 添加：保存/加载 BMP 的接口
	bool SaveAsBitmap(const CString& path);
	bool LoadBitmapFile(const CString& path);
	void CloseBitmapView() { m_showBitmap = false; Invalidate(); } // 退出位图查看模式（回到矢量绘制）


private:
	int m_currentLineWidth = 1;
	int m_currentLineStyle = 0; // 0-solid 1-dash 2-dot 3-dashdot
	std::vector<CPoint> points;//存储绘制图形的点
	std::vector<CPoint> dots;//存储绘制的辅助点
	std::vector<CShap*> Shaps;//存储绘制的图形

	// 添加：位图查看状态与图像
	CImage m_bitmapImage{};
	bool   m_showBitmap = false;

	// D2D资源
	ID2D1Factory* m_pD2DFactory = nullptr;
	ID2D1HwndRenderTarget* m_pRenderTarget = nullptr;
	ID2D1SolidColorBrush* m_pBlackBrush = nullptr;
	ID2D1SolidColorBrush* m_pRedBrush = nullptr;
	ID2D1SolidColorBrush* m_pSelectionBrush = nullptr;


#pragma region Line
	bool IsDrawLine = false;
	int LineNum = 2;
	int DrawLineMode = 0;
#pragma endregion

#pragma region Circle
	bool IsCircle = false;
	int CircleNum = 2;
	int DrawCirclesMode = 0;
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

#pragma region Polygon
	bool IsPolygon = false;
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
	CPoint FixedRotatePoint = NULL; // 固定旋转点
#pragma endregion

//求交点
#pragma region Intersection
	bool IsInter = false;
	bool Isfinish = false;
	std::vector<CShap*> AbleShapes;
#pragma endregion

#pragma region Vertical
	bool IsVertical = false;
#pragma endregion
#pragma region CircleCenter
	bool IsCircleCenter = false;
	CircleShap* targetShape = NULL;
#pragma endregion
#pragma region CircleTangent
	bool IsCircleTangent = false;
#pragma endregion

#pragma region Clip
	bool IsClipMode = false;
	int m_clipAlgo = 0; // 0: Cohen-Sutherland, 1: Midpoint
	bool m_isSelectingClipRect = false;
	CRect m_clipRect;
	CPoint m_clipStartPoint;
#pragma endregion


public:
	void ApplyWidthToSelection(int w);
	void ApplyStyleToSelection(int s);

	afx_msg void OnCircle();
	afx_msg void OnSelect();
//	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDelete();


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
	afx_msg void OnCenterTangent();
	afx_msg void OnSave();
	afx_msg void OnLine_1();
	afx_msg void OnBresenham_Line();
	afx_msg void OnMiddleLine();
	afx_msg void OnBresenhamLine();
	afx_msg void OnMiddleCircle();
	afx_msg void OnBresenhamCircle();
	afx_msg void OnLineWidth1();
	afx_msg void OnOnLineWidth2();
	afx_msg void OnOnLineWidth4();
	afx_msg void OnLineShape0();
	afx_msg void OnOnLineShape1();
	afx_msg void OnOnLineShape2();
	afx_msg void OnOnLineShape3();
	afx_msg void OnLineWidth8();

private:
    CDx2D m_dx2d;
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	// --- 填充模式与像素函数声明（与图元解耦） ---
private:
	bool   IsSeedFill = false;
	bool   IsBarrierFill = false;
	CImage m_fillImage;       // 填充后的整幅位图（叠加显示）
	bool   m_hasFillImage = false;
	COLORREF m_fillColor = RGB(255, 0, 0); // 填充颜色，默认为红色
	CImage m_canvasImage;
	// 填充算法
	void ResetAllModes(); // 重置所有模式的辅助函数
	void FlattenFillToCanvas(); // 新增：将填充结果固化到画布上
	void PerformFill(CPoint seedPoint);
	void SeedFill(CImage& img, CPoint seed, COLORREF fillColor, COLORREF boundaryColor);
	void ScanlineFill(CImage& img, CPoint seed, COLORREF fillColor, COLORREF boundaryColor);
public:
	afx_msg void OnSeedFillMode();
	afx_msg void OnBarrierFillMode();
	afx_msg void OnCLineCut();
	afx_msg void OnMLineCut();
	afx_msg void OnSRectCut();
	afx_msg void OnWRectCut();
private:
	void ClipLinesWithRect(const CRect& rect);
	void ClipPolygonsWithRect(const CRect& rect);
	void ClipPolygonsWithRectWA(const CRect& rect);
public:
	afx_msg void OnPolyGon();
};

