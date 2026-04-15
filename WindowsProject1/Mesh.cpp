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

void CPolygon::SetVertex(int nIndex, const CVertex& vertex)
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

void Draw2DLine(HDC hDCFrameBuffer, XMFLOAT3& f3PrevProject, XMFLOAT3& f3CurProject)
{
	// 투영 좌표계 2점을 화면 좌표계로 변환, 그 점을 선분으로 그림
	XMFLOAT3 f3Prev = CGraphicsPipeline::ScreenTransform(f3PrevProject);
	XMFLOAT3 f3Cur = CGraphicsPipeline::ScreenTransform(f3CurProject);

	::MoveToEx(hDCFrameBuffer, (long)f3Prev.x, (long)f3Prev.y, nullptr);
	::LineTo(hDCFrameBuffer, (long)f3Cur.x, (long)f3Cur.y);
}

void CMesh::Render(HDC hDCFrameBuffer)
{
	XMFLOAT3 f3InitProject, f3PrevProject, f3Intersect;
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
			XMFLOAT3 f3CurProject = CGraphicsPipeline::Project(pVertices[i].m_xmf3Position);

			f3CurProject = CGraphicsPipeline::ScreenTransform(f3CurProject);

			// 여기서 화면 잘림 여부 검사하기

			// 렌더링 할 정점 추가
			m_vDrawingPoints.push_back(POINT{ (long)f3CurProject.x, (long)f3CurProject.y });
		}
		Polygon(hDCFrameBuffer, m_vDrawingPoints.data() + startIndex, nVertices);
	}

#else
	// 모든 다각형 렌더링
	for (int j = 0; j < m_nPolygons; j++)
	{
		int nVertices = m_ppPolygons[j]->m_nVertices;
		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;

		f3PrevProject = f3InitProject = CGraphicsPipeline::Project(pVertices[0].m_xmf3Position);
		bPrevInside = bInitInside = (-1.0f <= f3InitProject.x) && (f3InitProject.x <= 1.0f) &&
			(-1.0f <= f3InitProject.y) && (f3InitProject.y <= 1.0f);

		for (int i = 1; i < nVertices; i++)
		{
			XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(pVertices[i].m_xmf3Position);
			bCurInside = (-1.0f <= f3CurrentProject.x) && (f3CurrentProject.x <= 1.0f) && (-1.0f <= f3CurrentProject.y) && (f3CurrentProject.y <= 1.0f);
			if (((0.0f <= f3CurrentProject.z) && (f3CurrentProject.z <= 1.0f)) && ((bCurInside || bPrevInside))) ::Draw2DLine(hDCFrameBuffer, f3PrevProject, f3CurrentProject);
			f3PrevProject = f3CurrentProject;
			bPrevInside = bCurInside;
		}
		if (((0.0f <= f3InitProject.z) && (f3InitProject.z <= 1.0f)) && ((bInitInside || bPrevInside)))
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


// ========================================================
CAirplaneMesh::CAirplaneMesh(float fWidth, float fHeight, float fDepth) : CMesh(24)
// ========================================================
{
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	float x1 = fx * 0.2f, y1 = fy * 0.2f, x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy - y3)) / x1) * x2 + (fy - y3);
	int i = 0;

	//Upper Plane
	CPolygon* pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);

	//Lower Plane
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	SetPolygon(i++, pFace);

	//Right Plane
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(2, CVertex(+x2, +y2, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(2, CVertex(+x2, +y2, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	//Back/Right Plane
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	SetPolygon(i++, pFace);

	//Left Plane
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(2, CVertex(-x2, +y2, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(2, CVertex(-x2, +y2, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	SetPolygon(i++, pFace);

	//Back/Left Plane
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);
}

CAirplaneMesh::~CAirplaneMesh()
{

}