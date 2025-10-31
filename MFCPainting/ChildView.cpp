
// ChildView.cpp: CChildView 类的实现
//

#include "pch.h"
#include "framework.h"
#include "MFCPainting.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_32771, &CChildView::OnLineDraw)
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_32772, &CChildView::OnCircle)
	ON_COMMAND(ID_32783, &CChildView::OnSelect)
//	ON_WM_MOUSEHOVER()
ON_WM_MOUSEMOVE()
ON_COMMAND(ID_32784, &CChildView::OnDelete)
END_MESSAGE_MAP()



// CChildView 消息处理程序

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	

	//绘制图像
	for (const auto& ln : Shaps)
	{
		ln->Draw(&dc);
	}


	if (IsSelected) {
		for (const auto& shp : Shaps)
		{
			shp->ChangeSelected(points[0]);
			if (shp->Selected) {
				shp->DrawSelection(&dc);
			}
		}
	}
	else {
	      DrawDot(dc);
	}
	
	// 不要为绘制消息而调用 CWnd::OnPaint()
}

void CChildView::DrawDot(CPaintDC& dc)
{
	// 绘制已保存的点（实心小圆）
	CBrush brush(RGB(0, 0, 0));
	CBrush* pOldBrush = dc.SelectObject(&brush);
	const int r = 3; // 半径
	for (const auto& pt : points)
	{
		dc.Ellipse(pt.x - r, pt.y - r, pt.x + r + 1, pt.y + r + 1);
	}
	dc.SelectObject(pOldBrush);
}


//按下左键
void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CWnd::OnLButtonDown(nFlags, point);

	CheckSelectedPoint(point);
	DrawLine(point);

	DrawCircle(point);


	Invalidate(); // 触发重绘
}

void CChildView::CheckSelectedPoint(CPoint& point)
{
	if (IsSelected) {
		points.clear();
		points.push_back(point);
	}
}

void CChildView::DrawCircle(CPoint& point)
{
	if (IsCircle && points.size() < CircleNum) {
		points.push_back(point);
	}
	// 达到两点，保存线段，清空临时点集
	if (IsCircle && points.size() == CircleNum)
	{
		Shaps.push_back(new CircleShap(points[0], points[1]));
		IsCircle = false;
		points.clear();
	}
}

void CChildView::DrawLine(CPoint& point)
{
	if (IsDrawLine && points.size() < LineNum) {
		points.push_back(point);
	}
	// 达到两点，保存线段，清空临时点集
	if (IsDrawLine && points.size() == LineNum)
	{
		
		LineShap *newadd = new LineShap(points[0],points[1]);
		Shaps.push_back(newadd);
		IsDrawLine = false;
		points.clear();
	}
}

//绘制直线
void CChildView::OnLineDraw()
{
	// TODO: 在此添加命令处理程序代码
	points.clear();
	IsDrawLine = true;
	IsSelected = false;
}

void CChildView::OnCircle()
{
	// TODO: 在此添加命令处理程序代码
	points.clear();
	IsCircle = true;
	IsSelected = false;
}

void CChildView::OnSelect()
{
	// TODO: 在此添加命令处理程序代码
	points.clear();
	IsSelected = true;

}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CWnd::OnMouseMove(nFlags, point);
	if (IsSelected) {
		for (const auto& shp : Shaps)
		{
			if (shp->IsSelected(point)) {
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
			}
		}
	}
}

void CChildView::OnDelete()
{
	// TODO: 在此添加命令处理程序代码
	if (IsSelected && !points.empty()) {
		
		for (auto it = Shaps.begin(); it!=Shaps.end();it++) {
			CShap* shp = *it;
			if (shp->Selected) {
				Shaps.erase(it);
				shp->Destroy();
				delete shp;
				break;
				
			}
			
		}
	}
	Invalidate(); // 触发重绘
}
