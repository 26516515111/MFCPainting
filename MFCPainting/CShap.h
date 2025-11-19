#pragma once
#include <cmath>
#include <algorithm>
#include <afxwin.h>
//调用dx2d库
#include <d2d1.h>
#include <d2d1helper.h>
#include <vector>
#pragma comment(lib, "d2d1.lib")
class CShap
{
public:
	
	bool Selected = false;
	virtual void Draw(CPaintDC* pdc) = 0;
	virtual bool IsSelected(CPoint point) = 0;
	virtual void ChangeSelected(CPoint point);
	virtual void DrawSelection(CPaintDC* pdc) = 0;
	virtual void Move(CSize delta) = 0;
	virtual void Rotate(double degrees) = 0;
	virtual CPoint GetCenter() const = 0;
	virtual ~CShap() = default;

	// 新增：缩放图元
	virtual void Scale(double factor, CPoint center) = 0;

	virtual void Destroy();
	CShap() = default;
};

class LineShap :public CShap {
private:
	CPoint startPoint;
	CPoint endPoint;
	int DrawMethod; // 0-中点画线法，1-Bresenham画线法
	int lineWidth;
	int lineStyle;
public:
	LineShap(CPoint start, CPoint end, int drawmethod = 0, int w = 1, int style = 0);
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
	void GetEndpoints(CPoint& a, CPoint& b); // 非 const，内部可能需要更新缓存
	//绘制垂线
	LineShap* CreatePerpendicularAt(CPoint pointOnOrNearLine, double halfLength = 50.0) const;
	//中点画线法
	void Draw_b(CPaintDC* pdc);
	//Bresenham画线法
	void Draw_B(CPaintDC* pdc);

	void SetLineWidth(int w) { lineWidth = max(1, w); }
	void SetLineStyle(int s) { lineStyle = std::clamp(s, 0, 3); }
	int  GetLineWidth() const { return lineWidth; }
	int  GetLineStyle() const { return lineStyle; }
	
};

class CircleShap :public CShap {

private:
	CPoint centerPoint;
	CPoint rPoint;
	int DrawMethod;
	int lineWidth;
	int lineStyle;// 0-solid 1-dash 2-dot 3-dashdot

	// 高精度内部表示，避免每次缩放累积精度丢失
	double centerX = 0.0;
	double centerY = 0.0;
	double rdx = 0.0; // rPoint.x - center.x
	double rdy = 0.0;
public:

	CircleShap(CPoint center, CPoint r,int Drawmethod = 0, int w = 1, int style = 0);

	// 通过 CShap 继承
	void Draw(CPaintDC* pdc) override;
	// 中点画园
	void CDraw_b(CPaintDC* pdc);
	//bersenham画圆
	void CDraw_B(CPaintDC* pdc);

	bool IsSelected(CPoint point) override;
	void DrawSelection(CPaintDC* pdc) override;

	// 通过 CShap 继承
	void Move(CSize delta) override;

	// 通过 CShap 继承
	void Rotate(double degrees) override;
	CPoint GetCenter() const override;

	// 通过 CShap 继承
	void Scale(double factor, CPoint center) override;
	void GetCenterAndRadius(CPoint& centerOut, double& radiusOut);
	void ShowCenter(CPaintDC* pdc);

	LineShap* CreateTangentAt(CPoint pointOnOrNearCircle, double halfLength = 100.0);
	void SetLineWidth(int w) { lineWidth = max(1, w); }
	void SetLineStyle(int s) { lineStyle = std::clamp(s, 0, 3); }
	int  GetLineWidth() const { return lineWidth; }
	int  GetLineStyle() const { return lineStyle; }
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
	void GetCornersInt(CPoint outCorners[4]);

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
	void GetIntPoints(CPoint& a, CPoint& b, CPoint& c);

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
	void GetCornersInt(CPoint outCorners[4]);

};

class ParallelogramShap : public CShap {
private:
	// 双精度顶点（按顺序 A,B,C,D）
	double x1 = 0.0, y1 = 0.0;
	double x2 = 0.0, y2 = 0.0;
	double x3 = 0.0, y3 = 0.0;
	double x4 = 0.0, y4 = 0.0;

	// 缓存整数顶点用于绘制
	CPoint p1Int, p2Int, p3Int, p4Int;

	// 同步整数顶点
	void UpdateIntPoints();

public:
	// 构造：输入三个点 A,B,C（第四点 D 自动计算）
	ParallelogramShap(CPoint a, CPoint b, CPoint c);

	// 绘制/交互实现
	void Draw(CPaintDC* pdc) override;
	bool IsSelected(CPoint point) override;
	void DrawSelection(CPaintDC* pdc) override;

	// 变换
	void Move(CSize delta) override;
	void Rotate(double degrees) override;
	void Scale(double factor, CPoint center) override;
	CPoint GetCenter() const override;

	// 默认销毁
	void Destroy() override {}
	void GetIntPoints(CPoint& a, CPoint& b, CPoint& c, CPoint& d);

};
// -------- 新增：三次贝塞尔曲线 CurveShap --------
class CurveShap : public CShap {
private:
	// 控制点（双精度以避免缩放精度丢失）
	double x0, y0; // 起点
	double x1, y1; // 控制点1
	double x2, y2; // 控制点2
	double x3, y3; // 终点

	// 采样点（用于绘制与命中检测），整数缓存
	std::vector<CPoint> samplesInt;

	// 采样精度（段数）
	int sampleSegments = 40;

	// 计算 t 处贝塞尔点（double）
	void GetPointOnBezier(double t, double& outx, double& outy) const;

	// 重新采样（在变换后调用）
	void UpdateSamples();

public:
	// 构造：四个控制点
	CurveShap(CPoint p0, CPoint p1, CPoint p2, CPoint p3);

	// 通过 CShap 继承
	void Draw(CPaintDC* pdc) override;
	bool IsSelected(CPoint point) override;
	void DrawSelection(CPaintDC* pdc) override;

	// 变换
	void Move(CSize delta) override;
	void Rotate(double degrees) override;
	void Scale(double factor, CPoint center) override;
	CPoint GetCenter() const override;

	// 可选销毁
	void Destroy() override {}
	const std::vector<CPoint>& GetSamplesInt();

};

class PolylineShap : public CShap {
private:
	// 双精度顶点列表
	std::vector<double> xs;
	std::vector<double> ys;

	// 缓存整数顶点用于绘制/交互
	std::vector<CPoint> ptsInt;

	// 重建整数缓存
	void UpdateIntPoints();

public:
	// 构造：传入顶点列表（至少 2 个点）
	PolylineShap(const std::vector<CPoint>& points);

	// 绘制与交互
	void Draw(CPaintDC* pdc) override;
	bool IsSelected(CPoint point) override;
	void DrawSelection(CPaintDC* pdc) override;

	// 变换
	void Move(CSize delta) override;
	void Rotate(double degrees) override;
	void Scale(double factor, CPoint center) override;
	CPoint GetCenter() const override;

	// 清理
	void Destroy() override {}
	const std::vector<CPoint>& GetIntPoints();
};

class ShapController {
private: 
	std::vector<CShap*> shapes;
	
public:
	ShapController(std::vector<CShap*> shape);

	// 新增：计算所有图形间的交点并返回（整数点）
	std::vector<CPoint> ComputeAllIntersections() const;

	// 新增：在给定设备上下文的左上角绘制交点列表（从 (2,2) 向下）
	void DrawIntersections(CPaintDC* pdc) const;
};