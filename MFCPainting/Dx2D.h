#pragma once
#include <d2d1.h>
#include <d2d1helper.h>
#pragma comment(lib,"d2d1.lib")

class CDx2D {
public:
    CDx2D() = default;
    ~CDx2D() { Cleanup(); }

    bool Initialize(HWND hWnd);
    void Cleanup();
    bool Resize(UINT width, UINT height);

    bool BeginDraw();
    void EndDraw();
    bool IsReady() const { return m_pRT != nullptr; }
    void Clear(const D2D1::ColorF& color = D2D1::ColorF(D2D1::ColorF::White));

    // 直线
    void DrawLineDirect(POINT a, POINT b, float width, int style, const D2D1::ColorF& color);
    void DrawLineMidpoint(POINT a, POINT b, float width, int style, const D2D1::ColorF& color);
    void DrawLineBresenham(POINT a, POINT b, float width, int style, const D2D1::ColorF& color);

    // 圆（原始）
    void DrawCircleDirect(POINT center, float radius, float width, int style, const D2D1::ColorF& color);
    void DrawCircleMidpoint(POINT c, int r, float width, int style, const D2D1::ColorF& color);
    void DrawCircleBresenham(POINT c, int r, float width, int style, const D2D1::ColorF& color);

    // 新增：智能选择（根据算法 / 线型自动优化）
    void DrawCircleSmart(POINT center, int radius, float width, int style, int algorithm, const D2D1::ColorF& color);

private:
    ID2D1Factory* m_pFactory = nullptr;
    ID2D1HwndRenderTarget* m_pRT = nullptr;
    ID2D1SolidColorBrush* m_pBrush = nullptr;

    void EnsureBrushColor(const D2D1::ColorF& color);
    ID2D1StrokeStyle* CreateStroke(int style);
    bool PatternShouldDraw(int idx, int style) const;
    void PlotThickPixel(int x, int y, int lineWidth, const D2D1::ColorF& color);

    // 角度采样虚线圆（均匀线型）
    void DrawCircleAngleDashed(POINT center, int radius, float width, int style, const D2D1::ColorF& color);
};