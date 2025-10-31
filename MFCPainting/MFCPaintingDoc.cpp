#include "pch.h"
#include "framework.h"
#include "MFCPaintingDoc.h"
#include "Shapes/CShape.h"

IMPLEMENT_DYNCREATE(CMFCPaintingDoc, CDocument)

CMFCPaintingDoc::CMFCPaintingDoc()
{
}

CMFCPaintingDoc::~CMFCPaintingDoc()
{
    for (int i = 0; i < m_shapes.GetSize(); ++i) delete m_shapes[i];
    m_shapes.RemoveAll();
}

void CMFCPaintingDoc::Serialize(CArchive& ar)
{
    CDocument::Serialize(ar);
    if (ar.IsStoring()) {
        int n = m_shapes.GetSize();
        ar << n;
        for (int i = 0; i < n; ++i) {
            CObject* p = m_shapes[i];
            ar << p;
        }
    } else {
        int n = 0;
        ar >> n;
        for (int i = 0; i < n; ++i) {
            CObject* p = nullptr;
            ar >> p;
            if (p) m_shapes.Add(static_cast<CShape*>(p));
        }
    }
}

#ifdef _DEBUG
void CMFCPaintingDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CMFCPaintingDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif
