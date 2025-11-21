#include "pch.h"
#include "Dx2D.h"
#include <vector>
#include <cmath>

bool CDx2D::Initialize(HWND hWnd) {
    Cleanup();
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory)))
        return false;
    RECT rc{}; ::GetClientRect(hWnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
    if (FAILED(m_pFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hWnd, size),
        &m_pRT)))
        return false;
    m_pRT->SetDpi(96.f, 96.f);
    if (FAILED(m_pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBrush)))
        return false;
    return true;
}

void CDx2D::Cleanup() {
    if (m_pBrush) { m_pBrush->Release(); m_pBrush = nullptr; }
    if (m_pRT)    { m_pRT->Release();    m_pRT = nullptr; }
    if (m_pFactory) { m_pFactory->Release(); m_pFactory = nullptr; }
}

bool CDx2D::Resize(UINT w, UINT h) {
    if (!m_pRT) return false;
    return SUCCEEDED(m_pRT->Resize(D2D1::SizeU(w, h)));
}

bool CDx2D::BeginDraw() {
    if (!m_pRT) return false;
    m_pRT->BeginDraw();
    // 保持无残留变换
    m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
    return true;
}

void CDx2D::EndDraw() {
    if (!m_pRT) return;
    HRESULT hr = m_pRT->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) Cleanup();
}

void CDx2D::Clear(const D2D1::ColorF& color) {
    if (m_pRT) m_pRT->Clear(color);
}

void CDx2D::EnsureBrushColor(const D2D1::ColorF& color) {
    if (m_pBrush) m_pBrush->SetColor(color);
}

ID2D1StrokeStyle* CDx2D::CreateStroke(int style) {
    if (!m_pFactory) return nullptr;

    D2D1_STROKE_STYLE_PROPERTIES props = D2D1::StrokeStyleProperties();
    // 根据样式决定是否使用自定义虚线
    if (style == 0) {
        props.dashStyle = D2D1_DASH_STYLE_SOLID;
    }
    else {
        props.dashStyle = D2D1_DASH_STYLE_CUSTOM;
    }
    props.startCap = D2D1_CAP_STYLE_FLAT;
    props.endCap = D2D1_CAP_STYLE_FLAT;
    props.dashCap = D2D1_CAP_STYLE_FLAT;
    props.lineJoin = D2D1_LINE_JOIN_MITER;
    props.miterLimit = 1.0f;

    static const float dashDash[] = { 5.f, 3.f };                 // style=1
    static const float dashDot[] = { 1.f, 3.f };                 // style=2 (dot)
    static const float dashDashDot[] = { 5.f, 3.f, 1.f, 3.f };       // style=3

    const float* arr = nullptr;
    UINT count = 0;
    switch (style) {
    case 1: arr = dashDash;     count = _countof(dashDash);     break;
    case 2: arr = dashDot;      count = _countof(dashDot);      break;
    case 3: arr = dashDashDot;  count = _countof(dashDashDot);  break;
    default: break; // 0 -> SOLID
    }

    ID2D1StrokeStyle* stroke = nullptr;
    HRESULT hr = m_pFactory->CreateStrokeStyle(props, arr, count, &stroke);
    if (FAILED(hr)) return nullptr;
    return stroke;
}

bool CDx2D::PatternShouldDraw(int idx, int style) const {
    switch(style) {
        case 0: return true;
        case 1: return (idx % 8) < 5;
        case 2: return (idx % 4) == 0;
        case 3: { int m = idx % 12; return (m < 5) || (m == 8); }
        default: return true;
    }
}

void CDx2D::PlotThickPixel(int x, int y, int lineWidth, const D2D1::ColorF& color) {
    if (!m_pRT || !m_pBrush) return;
    EnsureBrushColor(color);
    int half = (lineWidth - 1) / 2;
    D2D1_RECT_F rf;
    for (int oy = -half; oy <= half; ++oy) {
        for (int ox = -half; ox <= half; ++ox) {
            rf = D2D1::RectF((FLOAT)(x + ox), (FLOAT)(y + oy), (FLOAT)(x + ox + 1), (FLOAT)(y + oy + 1));
            m_pRT->FillRectangle(rf, m_pBrush);
        }
    }
}

/*================= 直线：Direct2D 原生 =================*/
void CDx2D::DrawLineDirect(POINT a, POINT b, float width, int style, const D2D1::ColorF& color) {
    if (!m_pRT || !m_pBrush) return;
    EnsureBrushColor(color);
    ID2D1StrokeStyle* stroke = CreateStroke(style);
    m_pRT->DrawLine(
        D2D1::Point2F((FLOAT)a.x, (FLOAT)a.y),
        D2D1::Point2F((FLOAT)b.x, (FLOAT)b.y),
        m_pBrush,
        width < 1.f ? 1.f : width,
        stroke
    );
    if (stroke) stroke->Release();
}

/*================= 直线：中点法 =================*/
void CDx2D::DrawLineMidpoint(POINT a, POINT b, float width, int style, const D2D1::ColorF& color) {
    if (!m_pRT || !m_pBrush) return;
    int x0 = a.x, y0 = a.y;
    int x1 = b.x, y1 = b.y;
    int lineW = (int)max(1.f, width);

    auto Plot = [&](int px, int py, int stepIndex) {
        if (!PatternShouldDraw(stepIndex, style)) return;
        PlotThickPixel(px, py, lineW, color);
    };

    if (x0 == x1) {
        if (y0 > y1) std::swap(y0, y1);
        int step = 0;
        for (int y = y0; y <= y1; ++y) Plot(x0, y, step++);
        return;
    }
    if (y0 == y1) {
        if (x0 > x1) std::swap(x0, x1);
        int step = 0;
        for (int x = x0; x <= x1; ++x) Plot(x, y0, step++);
        return;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    int sx = (dx > 0) ? 1 : -1;
    int sy = (dy > 0) ? 1 : -1;
    int adx = abs(dx);
    int ady = abs(dy);

    int stepIndex = 0;

    if (ady <= adx) {
        int err = 2 * ady - adx;
        int x = x0, y = y0;
        for (int i = 0; i <= adx; ++i) {
            Plot(x, y, stepIndex++);
            if (err >= 0) { y += sy; err -= 2 * adx; }
            err += 2 * ady;
            x += sx;
        }
    } else {
        int err = 2 * adx - ady;
        int x = x0, y = y0;
        for (int i = 0; i <= ady; ++i) {
            Plot(x, y, stepIndex++);
            if (err >= 0) { x += sx; err -= 2 * ady; }
            err += 2 * adx;
            y += sy;
        }
    }
}

/*================= 直线：Bresenham =================*/
void CDx2D::DrawLineBresenham(POINT a, POINT b, float width, int style, const D2D1::ColorF& color) {
    int x0 = a.x, y0 = a.y, x1 = b.x, y1 = b.y;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    int stepIndex = 0;
    while (true) {
        if (PatternShouldDraw(stepIndex++, style))
            PlotThickPixel(x0, y0, (int)max(1.f, width), color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

/*================= 圆：Direct2D 原生 =================*/
void CDx2D::DrawCircleDirect(POINT center, float radius, float width, int style, const D2D1::ColorF& color) {
    if (!m_pRT || !m_pBrush || radius <= 0.f) return;
    EnsureBrushColor(color);
    ID2D1StrokeStyle* stroke = CreateStroke(style);
    D2D1_ELLIPSE e = D2D1::Ellipse(D2D1::Point2F((FLOAT)center.x, (FLOAT)center.y), radius, radius);
    m_pRT->DrawEllipse(e, m_pBrush, width < 1.f ? 1.f : width, stroke);
    if (stroke) stroke->Release();
}

/*================= 圆：中点法（保留仅用于实线演示） =================*/
void CDx2D::DrawCircleMidpoint(POINT c, int r, float width, int style, const D2D1::ColorF& color) {
    if (r <= 0) return;
    int x = 0, y = r;
    int d = 1 - r;
    int lineW = (int)max(1.f, width);
    // 仅在 style==0 时使用点绘制（否则不均匀）
    if (style != 0) { DrawCircleAngleDashed(c, r, width, style, color); return; }

    auto Plot8 = [&](int px, int py) {
        PlotThickPixel(c.x + px, c.y + py, lineW, color);
        PlotThickPixel(c.x - px, c.y + py, lineW, color);
        PlotThickPixel(c.x + px, c.y - py, lineW, color);
        PlotThickPixel(c.x - px, c.y - py, lineW, color);
        PlotThickPixel(c.x + py, c.y + px, lineW, color);
        PlotThickPixel(c.x - py, c.y + px, lineW, color);
        PlotThickPixel(c.x + py, c.y - px, lineW, color);
        PlotThickPixel(c.x - py, c.y - px, lineW, color);
    };

    Plot8(x, y);
    while (x < y) {
        ++x;
        if (d < 0) d += 2 * x + 1;
        else { --y; d += 2 * (x - y) + 1; }
        Plot8(x, y);
    }
}

/*================= 圆：Bresenham（同上，仅实线） =================*/
void CDx2D::DrawCircleBresenham(POINT c, int r, float width, int style, const D2D1::ColorF& color) {
    if (r <= 0) return;
    if (style != 0) { DrawCircleAngleDashed(c, r, width, style, color); return; }
    int x = 0, y = r;
    int d = 3 - 2 * r;
    int lineW = (int)max(1.f, width);

    auto Plot8 = [&](int px, int py) {
        PlotThickPixel(c.x + px, c.y + py, lineW, color);
        PlotThickPixel(c.x - px, c.y + py, lineW, color);
        PlotThickPixel(c.x + px, c.y - py, lineW, color);
        PlotThickPixel(c.x - px, c.y - py, lineW, color);
        PlotThickPixel(c.x + py, c.y + px, lineW, color);
        PlotThickPixel(c.x - py, c.y + px, lineW, color);
        PlotThickPixel(c.x + py, c.y - px, lineW, color);
        PlotThickPixel(c.x - py, c.y - px, lineW, color);
    };

    Plot8(x, y);
    while (x <= y) {
        ++x;
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; --y; }
        Plot8(x, y);
    }
}

/*================= 圆：角度采样虚线 =================*/
void CDx2D::DrawCircleAngleDashed(POINT c, int r, float width, int style, const D2D1::ColorF& color) {
    if (r <= 0) return;
    int lineW = (int)max(1.f, width);
    // 周长约 2πr，每步角度约 1 像素长度
    double dTheta = 1.0 / r;
    int stepIndex = 0;
    for (double theta = 0.0; theta < 2 * 3.141592653589793; theta += dTheta) {
        int x = (int)std::lround(c.x + r * std::cos(theta));
        int y = (int)std::lround(c.y + r * std::sin(theta));
        if (PatternShouldDraw(stepIndex++, style))
            PlotThickPixel(x, y, lineW, color);
    }
}

/*================= 圆：统一入口 =================*/
void CDx2D::DrawCircleSmart(POINT center, int radius, float width, int style, int algorithm, const D2D1::ColorF& color) {
    // 0: Direct2D 原生 (推荐)；1: 中点法；2: Bresenham
    if (algorithm == 0) {
        DrawCircleDirect(center, (float)radius, width, style, color);
        return;
    }
    if (algorithm == 1) {
        DrawCircleMidpoint(center, radius, width, style, color);
        return;
    }
    if (algorithm == 2) {
        DrawCircleBresenham(center, radius, width, style, color);
        return;
    }
    // 兜底：用 Direct
    DrawCircleDirect(center, (float)radius, width, style, color);
}                                                                                                                           