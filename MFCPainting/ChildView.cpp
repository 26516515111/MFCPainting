
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
	// 保险：显式置空（与头文件内联初始化双保险）
	m_pD2DFactory = nullptr;
	m_pRenderTarget = nullptr;
	m_pBlackBrush = nullptr;
	m_pRedBrush = nullptr;
	m_pSelectionBrush = nullptr;
}

CChildView::~CChildView()
{
	// 若加载过位图，释放
	m_bitmapImage.Destroy();
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
ON_WM_MOUSEWHEEL()
ON_COMMAND(ID_32773, &CChildView::OnRect)
ON_COMMAND(ID_32774, &CChildView::OnTriangle)
ON_COMMAND(ID_32775, &CChildView::OnDiamond)
ON_COMMAND(ID_32776, &CChildView::OnPara)
ON_COMMAND(ID_32777, &CChildView::OnCur)
ON_COMMAND(ID_32778, &CChildView::OnPolyline)
ON_COMMAND(ID_32779, &CChildView::OnIntersection)
ON_COMMAND(ID_32780, &CChildView::OnVertical)
ON_COMMAND(ID_32781, &CChildView::OnCircleCenter)
ON_COMMAND(ID_32782, &CChildView::OnCenterTangent)
ON_COMMAND(ID_32787, &CChildView::OnSave)
ON_COMMAND(ID_32788, &CChildView::OnLine_1)
ON_COMMAND(ID_32789, &CChildView::OnBresenham_Line)
ON_COMMAND(ID_32790, &CChildView::OnMiddleLine)
ON_COMMAND(ID_BRESENHAM32792, &CChildView::OnBresenhamLine)
ON_COMMAND(ID_32791, &CChildView::OnMiddleCircle)
ON_COMMAND(ID_BRESENHAM32793, &CChildView::OnBresenhamCircle)
ON_COMMAND(ID_32794, &CChildView::OnLineWidth1)
ON_COMMAND(ID_32795, &CChildView::OnOnLineWidth2)
ON_COMMAND(ID_32799, &CChildView::OnOnLineWidth4)
ON_COMMAND(ID_32800, &CChildView::OnLineShape0)
ON_COMMAND(ID_32801, &CChildView::OnOnLineShape1)
ON_COMMAND(ID_32802, &CChildView::OnOnLineShape2)
ON_COMMAND(ID_32803, &CChildView::OnOnLineShape3)
ON_COMMAND(ID_32798, &CChildView::OnLineWidth8)
ON_WM_ERASEBKGND()
ON_COMMAND(ID_32804, &CChildView::OnSeedFillMode)
ON_COMMAND(ID_32805, &CChildView::OnBarrierFillMode)
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
	//CPaintDC dc(this); // 用于验证区域

	//// 若当前处于“位图查看模式”，直接绘制 BMP
	//if (m_showBitmap && !m_bitmapImage.IsNull()) {
	//	// 将 BMP 贴到(0,0)。如需居中/缩放，可自行计算目标矩形
	//	m_bitmapImage.Draw(dc.m_hDC, 0, 0);
	//	return;
	//}

	//	// 备用：使用原来的GDI绘制
	//for (const auto& ln : Shaps)
	//	{
	//		ln->Draw(&dc);
	//	}
	//
	//if (StateCheck()) {
	//		for (const auto& shp : Shaps)
	//		{
	//			/*shp->ChangeSelected(points[0]);*/
	//			if (shp->Selected) {
	//				shp->DrawSelection(&dc);
	//			}
	//		}
	//	}
	//else if(!IsSelected){
	//	//绘制交点
	//	if (!AbleShapes.empty()) {
	//		// 计算交点
	//		ShapController* SC = new ShapController(AbleShapes);
	//		SC->DrawIntersections(&dc);
	//		AbleShapes.clear();
	//		delete SC;
	//	}
	//	if (targetShape) {
	//		targetShape->ShowCenter(&dc);
	//		targetShape = NULL;
	//	}
	//	DrawDot(dc);
	//}
	//CPaintDC dc(this);

	CPaintDC dc(this); // 用于最终绘制到屏幕的DC

	// 1. 准备一个临时的内存DC用于双缓冲
	CRect rc;
	GetClientRect(&rc);
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap bufferBitmap;
	bufferBitmap.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&bufferBitmap);

	// 2. 绘制背景：将持久画布绘制到内存DC上
	if (!m_canvasImage.IsNull()) {
		m_canvasImage.Draw(memDC.GetSafeHdc(), 0, 0);
	}
	else {
		memDC.FillSolidRect(&rc, RGB(255, 255, 255));
	}

	// 3. 如果有填充预览，则绘制预览并直接显示
	if (m_hasFillImage && !m_fillImage.IsNull()) {
		m_fillImage.Draw(memDC.GetSafeHdc(), 0, 0);
		dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
		memDC.SelectObject(pOldBitmap);
		return;
	}

	// 4. 在内存DC上绘制所有活动的矢量图形
	// GDI 图形
	for (auto* shp : Shaps) {
		if (!dynamic_cast<LineShap*>(shp) && !dynamic_cast<CircleShap*>(shp)) {
			shp->Draw(&memDC);
		}
	}
	for (auto* shp : Shaps) {
		if (shp->Selected &&
			!dynamic_cast<LineShap*>(shp) &&
			!dynamic_cast<CircleShap*>(shp)) {
			shp->DrawSelection(&memDC);
		}
	}

	// D2D 图形 (也需要绘制到内存DC)
	// 为了简化，我们继续使用GDI的Draw方法来绘制D2D图形到内存DC
	for (auto* shp : Shaps) {
		if (auto ln = dynamic_cast<LineShap*>(shp)) {
			ln->Draw(&memDC);
			ln->DrawSelection(&memDC);
		}
		else if (auto cir = dynamic_cast<CircleShap*>(shp)) {
			cir->Draw(&memDC);
			cir->DrawSelection(&memDC);
		}
	}

	// 5. 在内存DC上绘制辅助点等
	if (!IsSelected) {
		if (!AbleShapes.empty()&&Isfinish) {
			ShapController* SC = new ShapController(AbleShapes);
			SC->DrawIntersections(&memDC);
			AbleShapes.clear();
			delete SC;
		}
		if (targetShape) {
			targetShape->ShowCenter(&memDC);
			targetShape = nullptr;
		}
		DrawDot(memDC);
	}

	// 6. 将内存DC的内容一次性绘制到屏幕上
	dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBitmap);
}

bool CChildView::StateCheck()
{
	return IsSelected && !IsSelectedSave||IsInter;
}

#pragma region DrawShapes
void CChildView::DrawLine(CPoint& point)
{
	if (IsDrawLine && points.size() < LineNum) {
		points.push_back(point);
	}
	// 达到两点，保存线段，清空临时点集
	if (IsDrawLine && points.size() == LineNum)
	{
		
		LineShap* newadd = new LineShap(points[0], points[1], DrawLineMode, m_currentLineWidth, m_currentLineStyle);
		Shaps.push_back(newadd);
		IsDrawLine = false;
		points.clear();
		DrawLineMode = 0;
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
		Shaps.push_back(new CircleShap(points[0], points[1], DrawCirclesMode, m_currentLineWidth, m_currentLineStyle));
		IsCircle = false;
		points.clear();
		DrawCirclesMode = 0;
	}
}
//void CChildView::DrawDot(CPaintDC& dc)
//{
//	// 绘制已保存的点（实心小圆）
//	CBrush brush(RGB(0, 0, 0));
//	CBrush* pOldBrush = dc.SelectObject(&brush);
//	const int r = 3; // 半径
//	for (const auto& pt : points)
//	{
//		dc.Ellipse(pt.x - r, pt.y - r, pt.x + r + 1, pt.y + r + 1);
//	}
//	dc.SelectObject(pOldBrush);
//}
void CChildView::DrawDot(CDC& dc)
{
	CBrush brush(RGB(0, 0, 0));
	CBrush* pOldBrush = dc.SelectObject(&brush);
	const int r = 3;
	for (const auto& pt : points) {
		dc.Ellipse(pt.x - r, pt.y - r, pt.x + r + 1, pt.y + r + 1);
	}
	dc.SelectObject(pOldBrush);
}
void CChildView::DrawRect(CPoint& point)
{
	if (IsRect && points.size() < RectNum) {
		points.push_back(point);
	}
	// 达到两点，保存线段，清空临时点集
	if (IsRect && points.size() == RectNum)
	{

		RectShap* newadd = new RectShap(points[0], points[1]);
		Shaps.push_back(newadd);
		IsRect = false;
		points.clear();
	}
}

void CChildView::DrawTriangle(CPoint& point)
{
	if (IsTriangle && points.size() < TriangleNum) {
		points.push_back(point);
	}
	// 达到两点，保存线段，清空临时点集
	if (IsTriangle && points.size() == TriangleNum)
	{

		TriangleShap* newadd = new TriangleShap(points[0], points[1], points[2]);
		Shaps.push_back(newadd);
		IsTriangle = false;
		points.clear();
	}
}
void CChildView::DrawDiamond(CPoint& point)
{
	if (IsDiamond && points.size() < DiamondNum) {
		points.push_back(point);
	}
	// 达到两点，保存线段，清空临时点集
	if (IsDiamond && points.size() == DiamondNum)
	{

		DiamondShap* newadd = new DiamondShap(points[0], points[1]);
		Shaps.push_back(newadd);
		IsDiamond = false;
		points.clear();

	}
}
void CChildView::DrawPara(CPoint& point)
{
	if (IsPara && points.size() < ParaNum) {
		points.push_back(point);
	}
	// 达到两点，保存线段，清空临时点集
	if (IsPara && points.size() == ParaNum)
	{

		ParallelogramShap* newadd = new ParallelogramShap(points[0], points[1], points[2]);
		Shaps.push_back(newadd);
		IsPara = false;
		points.clear();


	}
}
void CChildView::DrawCur(CPoint& point)
{
	if (IsCur) {
		points.push_back(point);
	}
}
void CChildView::DrawPoly(CPoint& point)
{
	if (IsPoly) {
		points.push_back(point);
	}
}

#pragma endregion
#pragma region Save
bool CChildView::SaveAsBitmap(const CString& path)
{
	CRect rc;
	GetClientRect(&rc);
	const int w = rc.Width();
	const int h = rc.Height();
	if (w <= 0 || h <= 0) return false;

	// 创建用于保存的 32bit 位图
	CImage img;
	HRESULT hrCreate = img.Create(w, h, 32);
	if (FAILED(hrCreate)) return false;

	// 抓取当前窗口客户区像素
	CClientDC wndDC(this);
	HDC hImgDC = img.GetDC();
	BOOL ok = ::BitBlt(hImgDC, 0, 0, w, h, wndDC.m_hDC, 0, 0, SRCCOPY);
	img.ReleaseDC();

	if (!ok) { img.Destroy(); return false; }

	// 保存为 BMP（扩展名决定格式，.bmp 即 BMP）
	HRESULT hrSave = img.Save(path);
	img.Destroy();
	return SUCCEEDED(hrSave);
}

bool CChildView::LoadBitmapFile(const CString& path)
{
	m_bitmapImage.Destroy();
	HRESULT hr = m_bitmapImage.Load(path);
	if (FAILED(hr)) {
		m_showBitmap = false;
		return false;
	}
	m_showBitmap = true;
	Invalidate(); // 触发重绘以显示位图
	return true;
}


#pragma endregion

void CChildView::CheckSelectedPoint(CPoint& point)
{
	if (IsSelected) {
		points.clear();
		points.push_back(point);
	}
}


#pragma region onMenu

void CChildView::OnSeedFillMode()
{
	// TODO: 在此添加命令处理程序代码
	IsSeedFill = true;
	IsBarrierFill = false;
	IsSelected = false;
	IsSelectedSave = false;
	m_hasFillImage = false; // 清除旧的填充结果
	Invalidate();
}

void CChildView::OnBarrierFillMode()
{
	// TODO: 在此添加命令处理程序代码
	IsBarrierFill = true;
	IsSeedFill = false;
	IsSelected = false;
	IsSelectedSave = false;
	m_hasFillImage = false; // 清除旧的填充结果
	Invalidate();
}
//绘制直线
void CChildView::OnLineDraw()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	points.clear();
	IsDrawLine = true;
	IsSelected = false;
	IsSelectedSave = false;
}
void CChildView::OnLine_1()
{
}
void CChildView::OnBresenham_Line()
{
}
void CChildView::OnMiddleLine()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	points.clear();
	IsDrawLine = true;
	DrawLineMode = 1;
	IsSelected = false;
	IsSelectedSave = false;
}
void CChildView::OnBresenhamLine()
{
	// TODO: 在此添加命令处理程序代码
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	points.clear();
	IsDrawLine = true;
	DrawLineMode = 2;
	IsSelected = false;
	IsSelectedSave = false;
}


//绘制圆
void CChildView::OnCircle()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	points.clear();
	IsCircle = true;
	IsSelected = false;
	IsSelectedSave = false;
}
void CChildView::OnMiddleCircle()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	points.clear();
	IsCircle = true;
	DrawCirclesMode = 1;
	IsSelected = false;
	IsSelectedSave = false;
}
void CChildView::OnBresenhamCircle()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	points.clear();
	IsCircle = true;
	DrawCirclesMode = 2;
	IsSelected = false;
	IsSelectedSave = false;
}


//选择模式
void CChildView::OnSelect()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	points.clear();
	IsSelected = true;

}

void CChildView::OnTriangle()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	points.clear();
	IsTriangle = true;
	IsSelected = false;
	IsSelectedSave = false;
}

void CChildView::OnDelete()
{
	// TODO: 在此添加命令处理程序代码
	if (IsSelected) {
		
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

void CChildView::OnRect()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	IsSelected = false;
	IsSelectedSave = false;
	IsRect = true;
}

void CChildView::OnDiamond()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	points.clear();
	IsDiamond = true;
	IsSelected = false;
	IsSelectedSave = false;
}

void CChildView::OnPara()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	points.clear();
	IsPara = true;
	IsSelected = false;
	IsSelectedSave = false;
}

void CChildView::OnCur()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	IsSelected = false;
	IsSelectedSave = false;
	IsCur = true;
}
void CChildView::OnPolyline()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	IsSelected = false;
	IsSelectedSave = false;
	IsPoly = true;
}
//求交点
void CChildView::OnIntersection()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	IsInter = true;
	IsSelected = false;
	IsSelectedSave = false;
	AbleShapes.clear();
}
//求垂线
void CChildView::OnVertical()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	IsVertical = true;
	IsSelected = false;
	IsSelectedSave = false;
}
//绘制圆心
void CChildView::OnCircleCenter()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	IsCircleCenter = true;
	IsSelected = false;
	IsSelectedSave = false;
}
void CChildView::OnCenterTangent()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	IsCircleTangent = true;
	IsSelected = false;
	IsSelectedSave = false;
}
void CChildView::OnSave()
{
	// TODO: 在此添加命令处理程序代码
	ResetAllModes();
	IsSelected = false;
	IsSelectedSave = false;

	// 选择保存路径（默认扩展名 .bmp）
	CFileDialog dlg(FALSE, _T("bmp"), _T("drawing.bmp"),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
		_T("位图文件 (*.bmp)|*.bmp||"));
	if (dlg.DoModal() != IDOK) return;

	// 确保当前画面已完成重绘，避免抓屏得到旧帧/空白
	RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);

	const CString path = dlg.GetPathName();
	if (!SaveAsBitmap(path)) {
		AfxMessageBox(_T("保存失败，请确认路径或权限。"));
		return;
	}
}

void CChildView::OnLineWidth1()
{
	// TODO: 在此添加命令处理程序代码
	m_currentLineWidth = 1; ApplyWidthToSelection(1); Invalidate();
}
void CChildView::OnOnLineWidth2()
{
	// TODO: 在此添加命令处理程序代码
	m_currentLineWidth = 2; ApplyWidthToSelection(2); Invalidate();
}

void CChildView::OnOnLineWidth4()
{
	// TODO: 在此添加命令处理程序代码
	m_currentLineWidth = 4; ApplyWidthToSelection(4); Invalidate();
}
void CChildView::OnLineWidth8()
{
	// TODO: 在此添加命令处理程序代码
	m_currentLineWidth = 8; ApplyWidthToSelection(8); Invalidate();
}
void CChildView::OnLineShape0()
{
	// TODO: 在此添加命令处理程序代码
	m_currentLineStyle = 0; ApplyStyleToSelection(0); Invalidate();
}

void CChildView::OnOnLineShape1()
{
	// TODO: 在此添加命令处理程序代码
	m_currentLineStyle = 1; ApplyStyleToSelection(1); Invalidate();
}

void CChildView::OnOnLineShape2()
{
	// TODO: 在此添加命令处理程序代码
	m_currentLineStyle = 2; ApplyStyleToSelection(2); Invalidate();
}

void CChildView::OnOnLineShape3()
{
	// TODO: 在此添加命令处理程序代码
	m_currentLineStyle = 3; ApplyStyleToSelection(3); Invalidate();
}

#pragma endregion

#pragma region Event
void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if (cx > 0 && cy > 0) {
		m_dx2d.Resize((UINT)cx, (UINT)cy);

		// 新增：窗口大小改变时，重新创建画布
		if (!m_canvasImage.IsNull()) {
			m_canvasImage.Destroy();
		}
		m_canvasImage.Create(cx, cy, 32);
		CDC* pDC = CDC::FromHandle(m_canvasImage.GetDC());
		pDC->FillSolidRect(0, 0, cx, cy, RGB(255, 255, 255));
		m_canvasImage.ReleaseDC();
	}

}

void CChildView::OnDestroy()
{
	m_dx2d.Cleanup();
	CWnd::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	m_dx2d.Initialize(this->m_hWnd);
	// 新增：初始化画布
	CRect rc;
	GetClientRect(&rc);
	if (rc.Width() > 0 && rc.Height() > 0) {
		m_canvasImage.Create(rc.Width(), rc.Height(), 32);
		CDC* pDC = CDC::FromHandle(m_canvasImage.GetDC());
		pDC->FillSolidRect(&rc, RGB(255, 255, 255));
		m_canvasImage.ReleaseDC();
	}

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

	if (IsSeedFill || IsBarrierFill) {
		FlattenFillToCanvas();
		ResetAllModes();
		IsSelected = true; // 返回到选择模式
		return;
	}

	if (!IsSelected) {
		if (IsPoly) {
			PolylineShap* newadd = new PolylineShap(points);
			Shaps.push_back(newadd);
			IsPoly = false;
			points.clear();
		}
		if (IsInter) {
			IsInter = false;
			for (int i = static_cast<int>(Shaps.size()) - 1; i >= 0; --i) {
				Shaps[i]->Selected = false;
			}
			Isfinish = true;
			//AbleShapes.clear(); // 在此处清除
		}
		// 达到两点，保存线段，清空临时点集
		if (IsCur&&points.size()>=2)
		{
			CurveShap* newadd = new CurveShap(points);
			Shaps.push_back(newadd);
			IsCur = false;
			points.clear();
		}
	    Invalidate(); // 触发重绘
		return;
	}

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

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (!IsSelected)
		return CWnd::OnMouseWheel(nFlags, zDelta, pt);
	// 把屏幕坐标转换为客户区坐标（MFC 传入的 pt 是屏幕坐标）
	ScreenToClient(&pt);

	// 以每格滚轮缩放因子 1.1 为基准；正滚放大，负滚缩小
	const double step = 1.1;
	double factor = std::pow(step, static_cast<double>(zDelta) / 120.0); // zDelta 每 120 为一格

	// 确定缩放目标：优先被选中图元
	CShap* target = nullptr;
	for (int i = static_cast<int>(Shaps.size()) - 1; i >= 0; --i) {
		if (Shaps[i]->Selected)
		{
			target = Shaps[i]; break;
		}
	}

	if (target) {
		// 以图元自身中心缩放（也可以改为以鼠标位置缩放：传 pt）
		IsSelectedSave = true;
		CPoint center = target->GetCenter();
		target->Scale(factor, center);
		Invalidate();
	}

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CWnd::OnLButtonDown(nFlags, point);

	// 如果是填充模式，执行填充并返回
	if (IsSeedFill || IsBarrierFill) {
		PerformFill(point);
		return;
	}

	if (IsSelected) {
		//缩放结束
		IsSelectedSave = false;

		/*points.clear();
		points.push_back(point);*/

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
	else if (IsInter) {
		for (int i = static_cast<int>(Shaps.size()) - 1; i >= 0; --i) {
			if (Shaps[i]->IsSelected(point)) {
				Shaps[i]->ChangeSelected(point);
				AbleShapes.push_back(Shaps[i]);
			}
		}
		Invalidate();
		return; // 选择模式下不创建新图形
	}
	else if (IsVertical) {
		for (int i = static_cast<int>(Shaps.size()) - 1; i >= 0; --i) {
			if (Shaps[i]->IsSelected(point)) {
				auto pLine = dynamic_cast<LineShap*>(Shaps[i]);
				if (pLine)
				{
				   Shaps.push_back(pLine->CreatePerpendicularAt(point));
				   IsVertical = false;
				   break;
				}
			}
		}
		Invalidate();
		return; // 选择模式下不创建新图形
	}
	else if (IsCircleCenter) {
		for (int i = static_cast<int>(Shaps.size()) - 1; i >= 0; --i) {
			if (Shaps[i]->IsSelected(point)) {
				auto pCircle = dynamic_cast<CircleShap*>(Shaps[i]);
				if (pCircle)
				{
					targetShape = pCircle;
					IsCircleCenter = false;
					break;
				}
			}
		}
		Invalidate();
		return; // 选择模式下不创建新图形
	}
	else if (IsCircleTangent) {
		for (int i = static_cast<int>(Shaps.size()) - 1; i >= 0; --i) {
			if (!Shaps[i]) continue;
			if (Shaps[i]->IsSelected(point)) {
				auto pCircle = dynamic_cast<CircleShap*>(Shaps[i]);
				if (pCircle)
				{
					Shaps.push_back(pCircle->CreateTangentAt(point));
					IsCircleTangent = false;
					break;
				}
			}
		}
		Invalidate();
		return; // 选择模式下不创建新图形
	}
	else
	{
		CheckSelectedPoint(point);
		DrawLine(point);
		DrawCircle(point);
		DrawRect(point);
		DrawTriangle(point);
		DrawDiamond(point);
		DrawPara(point);
		DrawCur(point);
		DrawPoly(point);
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
	if (IsSelected||IsInter||IsVertical||IsCircleCenter||IsCircleTangent) {
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
#pragma endregion

// 添加两个辅助（放在 cpp 末尾或合适位置）
void CChildView::ApplyWidthToSelection(int w)
{
	if (!IsSelected) return;
	for (auto* shp : Shaps) {
		if (shp->Selected) {
			if (auto ln = dynamic_cast<LineShap*>(shp)) ln->SetLineWidth(w);
			if (auto ci = dynamic_cast<CircleShap*>(shp)) ci->SetLineWidth(w);
		}
	}
}
void CChildView::ApplyStyleToSelection(int s)
{
	if (!IsSelected) return;
	for (auto* shp : Shaps) {
		if (shp->Selected) {
			if (auto ln = dynamic_cast<LineShap*>(shp)) ln->SetLineStyle(s);
			if (auto ci = dynamic_cast<CircleShap*>(shp)) ci->SetLineStyle(s);
		}
	}
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return TRUE;
	/*return CWnd::OnEraseBkgnd(pDC);*/
}

#pragma region Filed
void CChildView::ResetAllModes()
{
	Isfinish = false;
	IsDrawLine = false;
	IsCircle = false;
	IsSelected = false;
	IsRect = false;
	IsTriangle = false;
	IsDiamond = false;
	IsPara = false;
	IsCur = false;
	IsPoly = false;
	IsInter = false;
	IsVertical = false;
	IsCircleCenter = false;
	IsCircleTangent = false;
	IsSeedFill = false;
	IsBarrierFill = false;

	// 清理相关状态
	points.clear();
	AbleShapes.clear();
	if (m_hasFillImage) {
		m_hasFillImage = false;
		Invalidate(); // 如果之前有填充，需要重绘以清除
	}
}
void CChildView::FlattenFillToCanvas()
{
	if (!m_hasFillImage || m_fillImage.IsNull()) return;

	// 将填充结果位图绘制到持久画布上
	CDC* pCanvasDC = CDC::FromHandle(m_canvasImage.GetDC());
	m_fillImage.Draw(pCanvasDC->GetSafeHdc(), 0, 0);
	m_canvasImage.ReleaseDC();

	// 清空矢量图形列表，因为它们已经被“固化”到画布里了
	for (auto* shp : Shaps) {
		delete shp;
	}
	Shaps.clear();

	// 重置填充状态并重绘
	m_hasFillImage = false;
	m_fillImage.Destroy();
	Invalidate();
}
void CChildView::PerformFill(CPoint seedPoint)
{
	// 1. 准备一个与窗口同样大小的 CImage 用于绘制和填充
	CRect rc;
	GetClientRect(&rc);
	if (m_fillImage.IsNull()) {
		m_fillImage.Create(rc.Width(), rc.Height(), 32);
	}

	// 2. 在 CImage 的 DC 上绘制所有图形
	CDC* pImageDC = CDC::FromHandle(m_fillImage.GetDC());

	// 先画上当前的画布
	if (!m_canvasImage.IsNull()) {
		m_canvasImage.Draw(pImageDC->GetSafeHdc(), 0, 0);
	}
	else {
		pImageDC->FillSolidRect(&rc, RGB(255, 255, 255)); // 如果画布为空，则清空为白色
	}

	// 再画上所有矢量图形
	for (auto* shp : Shaps) {
		if (auto ln = dynamic_cast<LineShap*>(shp)) ln->Draw(pImageDC);
		else if (auto cir = dynamic_cast<CircleShap*>(shp)) cir->Draw(pImageDC);
		else shp->Draw(pImageDC);
	}
	//pImageDC->FillSolidRect(&rc, RGB(255, 255, 255)); // 清空为白色背景

	//// 绘制 GDI 图形
	//for (auto* shp : Shaps) {
	//	if (!dynamic_cast<LineShap*>(shp) && !dynamic_cast<CircleShap*>(shp)) {
	//		shp->Draw(pImageDC);
	//	}
	//}
	//// 绘制 D2D 图形（需要一个适配器）
	//// 注意：这里简化处理，直接在内存DC上用GDI重绘D2D图形，以获取像素数据
	//for (auto* shp : Shaps) {
	//	if (auto ln = dynamic_cast<LineShap*>(shp)) ln->Draw(pImageDC);
	//	else if (auto cir = dynamic_cast<CircleShap*>(shp)) cir->Draw(pImageDC);
	//}

	// 3. 确定边界颜色和种子点颜色
	COLORREF boundaryColor = RGB(0, 0, 0); // 假设边界总是黑色
	COLORREF seedColor = m_fillImage.GetPixel(seedPoint.x, seedPoint.y);

	// 如果点击在边界上或已填充区域，则不操作
	if (seedColor == boundaryColor || seedColor == m_fillColor) {
		m_fillImage.ReleaseDC();
		return;
	}

	// 4. 根据模式选择填充算法
	if (IsSeedFill) {
		SeedFill(m_fillImage, seedPoint, m_fillColor, boundaryColor);
	}
	else if (IsBarrierFill) {
		ScanlineFill(m_fillImage, seedPoint, m_fillColor, boundaryColor);
	}

	m_fillImage.ReleaseDC();

	// 5. 设置标志并重绘窗口
	m_hasFillImage = true;
	Invalidate();
}

void CChildView::SeedFill(CImage& img, CPoint seed, COLORREF fillColor, COLORREF boundaryColor)
{
	if (img.IsNull()) return;

	int width = img.GetWidth();
	int height = img.GetHeight();
	COLORREF oldColor = img.GetPixel(seed.x, seed.y);

	if (oldColor == fillColor || oldColor == boundaryColor) {
		return;
	}

	std::stack<CPoint> points;
	points.push(seed);

	while (!points.empty()) {
		CPoint pt = points.top();
		points.pop();

		if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height) {
			continue;
		}

		if (img.GetPixel(pt.x, pt.y) == oldColor) {
			img.SetPixel(pt.x, pt.y, fillColor);

			points.push(CPoint(pt.x + 1, pt.y));
			points.push(CPoint(pt.x - 1, pt.y));
			points.push(CPoint(pt.x, pt.y + 1));
			points.push(CPoint(pt.x, pt.y - 1));
		}
	}

}

void CChildView::ScanlineFill(CImage& img, CPoint seed, COLORREF fillColor, COLORREF boundaryColor)
{
	if (img.IsNull()) return;

	int width = img.GetWidth();
	int height = img.GetHeight();
	COLORREF oldColor = img.GetPixel(seed.x, seed.y);

	if (oldColor == fillColor || oldColor == boundaryColor) {
		return;
	}

	std::stack<CPoint> seeds;
	seeds.push(seed);

	while (!seeds.empty()) {
		CPoint currentSeed = seeds.top();
		seeds.pop();

		int x = currentSeed.x;
		int y = currentSeed.y;

		// 找到当前扫描线的左边界
		int xLeft = x;
		while (xLeft >= 0 && img.GetPixel(xLeft, y) != boundaryColor && img.GetPixel(xLeft, y) != fillColor) {
			xLeft--;
		}
		xLeft++;

		// 找到当前扫描线的右边界
		int xRight = x;
		while (xRight < width && img.GetPixel(xRight, y) != boundaryColor && img.GetPixel(xRight, y) != fillColor) {
			xRight++;
		}
		xRight--;

		// 填充当前扫描线段
		for (int i = xLeft; i <= xRight; ++i) {
			img.SetPixel(i, y, fillColor);
		}

		// 在上方和下方扫描线寻找新的种子点
		for (int i = xLeft; i <= xRight; ++i) {
			// 上方
			if (y > 0) {
				COLORREF colorAbove = img.GetPixel(i, y - 1);
				if (colorAbove != boundaryColor && colorAbove != fillColor) {
					seeds.push(CPoint(i, y - 1));
				}
			}
			// 下方
			if (y < height - 1) {
				COLORREF colorBelow = img.GetPixel(i, y + 1);
				if (colorBelow != boundaryColor && colorBelow != fillColor) {
					seeds.push(CPoint(i, y + 1));
				}
			}
		}
	}
}
#pragma endregion


