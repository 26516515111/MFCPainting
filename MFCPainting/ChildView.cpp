
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
ON_WM_SIZE()
ON_WM_DESTROY()
ON_WM_CREATE()
ON_WM_LBUTTONUP()
ON_WM_RBUTTONUP()
ON_WM_RBUTTONDOWN()
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
	CPaintDC dc(this); // 用于验证区域

		// 备用：使用原来的GDI绘制
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

	if (IsSelected) {
		points.clear();
		points.push_back(point);

		bool found = false;
		m_pDragged = nullptr;
		for (int i = static_cast<int>(Shaps.size()) - 1; i >= 0; --i) {
			if (!found && Shaps[i]->IsSelected(point)) {
				// 将该图元设为选中并开始拖拽
				Shaps[i]->ChangeSelected(point);
				m_pDragged = Shaps[i];
				found = true;
			}
			else {
				Shaps[i]->Selected = false;
			}
		}

		if (m_pDragged) {
			m_bDragging = true;
			m_lastMouse = point;
			SetCapture(); // 捕获鼠标，保证拖拽期间接收鼠标消息
		}
		Invalidate();
		return; // 选择模式下不创建新图形
	}
	else
	{
		CheckSelectedPoint(point);
		DrawLine(point);
		DrawCircle(point);
	}


	Invalidate(); // 触发重绘
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CWnd::OnMouseMove(nFlags, point);
	// 旋转优先处理
	if (m_bRotating && m_pRotated) {
		double curAngle = std::atan2(static_cast<double>(point.y - m_rotateCenter.y),
			static_cast<double>(point.x - m_rotateCenter.x));
		double deltaRad = curAngle - m_lastRotateAngle;
		// 将弧度差转换为度
		double deltaDeg = deltaRad * (180.0 / acos(-1.0));
		if (std::abs(deltaDeg) > 1e-6) {
			m_pRotated->Rotate(deltaDeg);
			m_lastRotateAngle = curAngle;
			Invalidate();
		}
		return;
	}
	if (m_bDragging && m_pDragged) {
		// 计算偏移并移动选中图元
		CSize delta(point.x - m_lastMouse.x, point.y - m_lastMouse.y);
		if (delta.cx != 0 || delta.cy != 0) {
			m_pDragged->Move(delta);
			m_lastMouse = point;
			Invalidate(); // 重绘以显示移动结果
		}
		return;
	}
	// 非拖拽状态下保持原有的悬停/光标逻辑
	if (IsSelected) {
		bool hit = false;
		for (auto it = Shaps.rbegin(); it != Shaps.rend(); ++it) {
			if ((*it)->IsSelected(point)) {
				hit = true;
				break;
			}
		}
		::SetCursor(::LoadCursor(nullptr, hit ? IDC_HAND : IDC_ARROW));
	}
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

BOOL CChildView::InitializeD2D()
{
	// 创建D2D工厂
	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&m_pD2DFactory
	);

	if (FAILED(hr)) return FALSE;

	RECT rc;
	GetClientRect(&rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

	// 创建渲染目标
	hr = m_pD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(GetSafeHwnd(), size),
		&m_pRenderTarget
	);

	if (FAILED(hr)) return FALSE;

	// 创建画刷
	hr = m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&m_pBlackBrush
	);

	if (FAILED(hr)) return FALSE;

	hr = m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Red),
		&m_pRedBrush
	);

	if (FAILED(hr)) return FALSE;

	hr = m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Blue, 0.5f),
		&m_pSelectionBrush
	);

	return SUCCEEDED(hr);
}

void CChildView::CleanupD2D()
{
	if (m_pSelectionBrush) m_pSelectionBrush->Release();
	if (m_pRedBrush) m_pRedBrush->Release();
	if (m_pBlackBrush) m_pBlackBrush->Release();
	if (m_pRenderTarget) m_pRenderTarget->Release();
	if (m_pD2DFactory) m_pD2DFactory->Release();

	m_pSelectionBrush = nullptr;
	m_pRedBrush = nullptr;
	m_pBlackBrush = nullptr;
	m_pRenderTarget = nullptr;
	m_pD2DFactory = nullptr;
}

void CChildView::ResizeD2D()
{
	if (m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(&rc);
		m_pRenderTarget->Resize(D2D1::SizeU(rc.right, rc.bottom));
	}
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

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	ResizeD2D();
	// TODO: 在此处添加消息处理程序代码
}

void CChildView::OnDestroy()
{
	CleanupD2D();
	CWnd::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	InitializeD2D();
	return 0;
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CWnd::OnLButtonUp(nFlags, point);
	if (m_bDragging) {
		m_bDragging = false;
		m_pDragged = nullptr;
		ReleaseCapture();
		Invalidate(); // 最终重绘
	}
}

void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CWnd::OnRButtonUp(nFlags, point);
	if (m_bRotating) {
		m_bRotating = false;
		m_pRotated = nullptr;
		ReleaseCapture();
		Invalidate();
	}
}

void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CWnd::OnRButtonDown(nFlags, point);
	if (!IsSelected) return;

	// 找当前被选中的图元优先，否则使用命中检测
	CShap* target = nullptr;
	for (int i = static_cast<int>(Shaps.size()) - 1; i >= 0; --i) {
		if (Shaps[i]->Selected) { target = Shaps[i]; break; }
	}
	if (!target) {
		for (int i = static_cast<int>(Shaps.size()) - 1; i >= 0; --i) {
			if (Shaps[i]->IsSelected(point)) { target = Shaps[i]; break; }
		}
	}
	if (!target) return;

	m_pRotated = target;
	m_bRotating = true;
	m_rotateCenter = m_pRotated->GetCenter();
	// 初始角度（弧度）
	m_lastRotateAngle = std::atan2(static_cast<double>(point.y - m_rotateCenter.y),
		static_cast<double>(point.x - m_rotateCenter.x));
	SetCapture();
}
