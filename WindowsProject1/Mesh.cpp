#include "framework.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"

//=============================
CPolygon::CPolygon(int nVertices)
//=============================
{
	m_nVertices = nVertices;
	m_pVertices = new CVertex[nVertices];
}

CPolygon::~CPolygon()
{
	if (m_pVertices) delete[] m_pVertices;
}

void CPolygon::SetVertex(int nIndex, CVertex vertex)
{
	if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices) {
		m_pVertices[nIndex] = vertex;
	}
}

//=============================
CMesh::CMesh(int nPolygons)
//=============================
{
	m_nPolygons = nPolygons;
	m_ppPolygons = new CPolygon * [nPolygons];
}

CMesh::~CMesh()
{
	if (m_ppPolygons) {
		for (int i = 0; i < m_nPolygons; i++) {
			if (m_ppPolygons[i]) {
				delete m_ppPolygons[i];
			}
		}
		delete[] m_ppPolygons;
	}
}

void CMesh::Release()
{
	m_nReferences--;
	if (m_nReferences <= 0)
		delete this;
}

void CMesh::SetPolygon(int nIndex, CPolygon* pPolygon)
{
	if ((0 <= nIndex) && (nIndex < m_nPolygons)) {
		m_ppPolygons[nIndex] = pPolygon;
		m_nDrawingPoints += pPolygon->m_nVertices;
	}
}

void Draw2DLine(HDC hDCFrameBuffer, CPoint3D& f3PrevProject, CPoint3D& f3CurProject)
{
	// 투영 좌표계 2점을 화면 좌표계로 변환, 그 점을 선분으로 그림
	CPoint3D f3Prev = CGraphicsPipeline::ScreenTransform(f3PrevProject);
	CPoint3D f3Cur = CGraphicsPipeline::ScreenTransform(f3CurProject);

	::MoveToEx(hDCFrameBuffer, (long)f3Prev.x, (long)f3Prev.y, nullptr);
	::LineTo(hDCFrameBuffer, (long)f3Cur.x, (long)f3Cur.y);
}

void CMesh::Render(HDC hDCFrameBuffer)
{
	CPoint3D f3InitProject, f3PrevProject, f3Intersect;
	bool bPrevInside = false, bInitInside = false, bCurInside = false, bIntersectInside = false;

	// 벡터 메모리 할당 없는경우 크기 초기화
	if (m_vDrawingPoints.capacity() < m_nDrawingPoints) {
		m_vDrawingPoints.reserve(m_nDrawingPoints);
	}

	m_vDrawingPoints.clear();

#ifndef WIREFRAME_MODE

	// 모든 다각형 렌더링
	for (int j = 0; j < m_nPolygons; j++) {
		int nVertices = m_ppPolygons[j]->m_nVertices;
		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;

		// 이전에 데이터 넣은 만큼의 위치 기억, 오프셋 활용
		size_t startIndex = m_vDrawingPoints.size();

		// 모든 정점 원근 투영 변환 및 렌더링
		for (int i = 0; i < nVertices; i++) {
			CPoint3D f3CurProject = CGraphicsPipeline::Project(pVertices[i].m_f3Position);

			f3CurProject = CGraphicsPipeline::ScreenTransform(f3CurProject);

			// 여기서 화면 잘림 여부 검사하기

			// 렌더링 할 정점 추가
			m_vDrawingPoints.push_back(POINT{ (long)f3CurProject.x, (long)f3CurProject.y });
		}
		Polygon(hDCFrameBuffer, m_vDrawingPoints.data() + startIndex, nVertices);
	}

#else

	// 모든 다각형 렌더링
	for (int j = 0; j < m_nPolygons; j++) {
		int nVertices = m_ppPolygons[j]->m_nVertices;
		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;

		// 첫 정점 원근 투영 변환
		f3PrevProject = f3InitProject = CGraphicsPipeline::Project(pVertices[0].m_f3Position);

		// 투영 사각형 포함 여부 체크
		bPrevInside = bInitInside = (-1.f <= f3InitProject.x) && (f3InitProject.x <= 1.f) &&
			(-1.f <= f3InitProject.y) && (f3InitProject.y <= 1.f);

		// 모든 정점 원근 투영 변환 및 렌더링
		for (int i = 0; i < nVertices; i++) {
			CPoint3D f3CurProject = CGraphicsPipeline::Project(pVertices[i].m_f3Position);

			bCurInside = (-1.f <= f3CurProject.x) && (f3CurProject.x <= 1.f) &&
				(-1.f <= f3CurProject.y) && (f3CurProject.y <= 1.f);

			// 변환된 점이 투영 사각형 포함 시 이전과 현재 점을 선분으로 그림
			if (((f3PrevProject.z >= 0.f) || (f3CurProject.z >= 0.f))
				&& ((bCurInside || bPrevInside))) {
				::Draw2DLine(hDCFrameBuffer, f3PrevProject, f3CurProject);
				f3PrevProject = f3CurProject;
				bPrevInside = bCurInside;
			}
		}
	}

	// 마지막 정점과 시작점 잇기
	if (((f3PrevProject.z >= 0.f) || (f3InitProject.z >= 0.f)) &&
		((bInitInside || bPrevInside))) {
		::Draw2DLine(hDCFrameBuffer, f3PrevProject, f3InitProject);
	}

#endif
}

CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth) :
	CMesh(6)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	CPolygon* pFrontFace = new CPolygon(4);
	pFrontFace->SetVertex(0, { -fHalfWidth, +fHalfHeight, -fHalfDepth });
	pFrontFace->SetVertex(1, { +fHalfWidth, +fHalfHeight, -fHalfDepth });
	pFrontFace->SetVertex(2, { +fHalfWidth, -fHalfHeight, -fHalfDepth });
	pFrontFace->SetVertex(3, { -fHalfWidth, -fHalfHeight, -fHalfDepth });
	SetPolygon(0, pFrontFace);

	CPolygon* pTopFace = new CPolygon(4);
	pTopFace->SetVertex(0, { -fHalfWidth, +fHalfHeight, +fHalfDepth });
	pTopFace->SetVertex(1, { +fHalfWidth, +fHalfHeight, +fHalfDepth });
	pTopFace->SetVertex(2, { +fHalfWidth, +fHalfHeight, -fHalfDepth });
	pTopFace->SetVertex(3, { -fHalfWidth, +fHalfHeight, -fHalfDepth });
	SetPolygon(1, pTopFace);

	CPolygon* pBackFace = new CPolygon(4);
	pBackFace->SetVertex(0, { -fHalfWidth, -fHalfHeight, +fHalfDepth });
	pBackFace->SetVertex(1, { +fHalfWidth, -fHalfHeight, +fHalfDepth });
	pBackFace->SetVertex(2, { +fHalfWidth, +fHalfHeight, +fHalfDepth });
	pBackFace->SetVertex(3, { -fHalfWidth, +fHalfHeight, +fHalfDepth });
	SetPolygon(2, pBackFace);

	CPolygon* pBottomFace = new CPolygon(4);
	pBottomFace->SetVertex(0, { -fHalfWidth, -fHalfHeight, -fHalfDepth });
	pBottomFace->SetVertex(1, { +fHalfWidth, -fHalfHeight, -fHalfDepth });
	pBottomFace->SetVertex(2, { +fHalfWidth, -fHalfHeight, +fHalfDepth });
	pBottomFace->SetVertex(3, { -fHalfWidth, -fHalfHeight, +fHalfDepth });
	SetPolygon(3, pBottomFace);

	CPolygon* pLeftFace = new CPolygon(4);
	pLeftFace->SetVertex(0, { -fHalfWidth, +fHalfHeight, +fHalfDepth });
	pLeftFace->SetVertex(1, { -fHalfWidth, +fHalfHeight, -fHalfDepth });
	pLeftFace->SetVertex(2, { -fHalfWidth, -fHalfHeight, -fHalfDepth });
	pLeftFace->SetVertex(3, { -fHalfWidth, -fHalfHeight, +fHalfDepth });
	SetPolygon(4, pLeftFace);

	CPolygon* pRightFace = new CPolygon(4);
	pRightFace->SetVertex(0, { +fHalfWidth, +fHalfHeight, -fHalfDepth });
	pRightFace->SetVertex(1, { +fHalfWidth, +fHalfHeight, +fHalfDepth });
	pRightFace->SetVertex(2, { +fHalfWidth, -fHalfHeight, +fHalfDepth });
	pRightFace->SetVertex(3, { +fHalfWidth, -fHalfHeight, -fHalfDepth });
	SetPolygon(5, pRightFace);
}

CCubeMesh::~CCubeMesh()
{

}