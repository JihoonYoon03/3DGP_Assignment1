#include "framework.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"
#include "Camera.h"

//=============================
void CTriangle::CalculateNormal(const CVertex& vertex0, const CVertex& vertex1, const CVertex& vertex2)
//=============================
{
	XMVECTOR p0 = XMLoadFloat3(&vertex0.m_xmf3Position);
	XMVECTOR p1 = XMLoadFloat3(&vertex1.m_xmf3Position);
	XMVECTOR p2 = XMLoadFloat3(&vertex2.m_xmf3Position);

	XMVECTOR v0 = p1 - p0;
	XMVECTOR v1 = p2 - p0;

	XMStoreFloat3(&m_Normal, XMVector3Normalize(XMVector3Cross(v0, v1)));
}

//void CTriangle::SetVertex(int nIndex, const CVertex& vertex)
//{
//	if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices) {
//		m_pVertices[nIndex] = vertex;
//	}
//}

//=============================
CMesh::~CMesh()
//=============================
{

}

void CMesh::Release()
{
	m_nReferences--;
	if (m_nReferences <= 0)
		delete this;
}

//void CMesh::SetMesh(int nIndex, CTriangle* pPolygon)
//{
//	if ((0 <= nIndex) && (nIndex < m_nPolygons)) {
//		m_ppPolygons[nIndex] = pPolygon;
//		m_nDrawingPoints += pPolygon->m_nVertices;
//	}
//}

void CMesh::SetMesh(std::vector<CVertex>&& vertices, std::vector<uint32_t>&& indices)
{
	m_Vertices = std::move(vertices);
	m_Indices = std::move(indices);

	// 삼각형 계산
	// 3개 단위의 인덱스가 아닌 경우 예외처리
	if (m_Indices.size() % 3 != 0) {
		std::wstring buf = std::format(L"Mesh Data Error :: Vertices {} / Indices {}\n", m_Vertices.size(), m_Indices.size());
		OutputDebugString(buf.c_str());
		return;
	}

	else {

		m_Triangles.reserve(m_Indices.size() / 3);

		for (size_t i = 0; i < m_Indices.size(); i += 3) {
			CTriangle triangle{ static_cast<uint32_t>(i) };

			triangle.CalculateNormal(
				m_Vertices[m_Indices[i]],
				m_Vertices[m_Indices[i + 1]],
				m_Vertices[m_Indices[i + 2]]
			);

			m_Triangles.push_back(triangle);
		}
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

void CMesh::Render(HDC hDCFrameBuffer, CCamera* camera, const XMVECTOR& LocalCameraPos)
{
	XMFLOAT3 f3InitProject, f3PrevProject, f3Intersect;
	bool bPrevInside = false, bInitInside = false, bCurInside = false, bIntersectInside = false;

#ifndef WIREFRAME_MODE

	// 모든 다각형 렌더링
	for (const auto& triangle : m_Triangles) {

		// 은면제거
		// 매쉬 로컬 좌표계에서 평면 노멀과 카메라 look의 내적 수행
		XMVECTOR normal = XMLoadFloat3(&triangle.m_Normal);
		XMVECTOR look = XMVectorSubtract(XMLoadFloat3(&m_Vertices[m_Indices[triangle.m_StartIndex]].m_xmf3Position), LocalCameraPos);

		if (XMVectorGetX(XMVector3Dot(normal, look)) > 0.f) continue;

		// 모든 정점 원근 투영 변환 및 렌더링
		XMFLOAT3 f3CurProject1 = CGraphicsPipeline::Project(m_Vertices[m_Indices[triangle.m_StartIndex]].m_xmf3Position);
		XMFLOAT3 f3CurProject2 = CGraphicsPipeline::Project(m_Vertices[m_Indices[triangle.m_StartIndex + 1]].m_xmf3Position);
		XMFLOAT3 f3CurProject3 = CGraphicsPipeline::Project(m_Vertices[m_Indices[triangle.m_StartIndex + 2]].m_xmf3Position);

		f3CurProject1 = CGraphicsPipeline::ScreenTransform(f3CurProject1);
		f3CurProject2 = CGraphicsPipeline::ScreenTransform(f3CurProject2);
		f3CurProject3 = CGraphicsPipeline::ScreenTransform(f3CurProject3);

		// 여기서 화면 잘림 여부 검사하기
		POINT a[3] = {
			{f3CurProject1.x, f3CurProject1.y},
			{f3CurProject2.x, f3CurProject2.y},
			{f3CurProject3.x, f3CurProject3.y},
		};

		Polygon(hDCFrameBuffer, a, 3);
	}

#else
	// 모든 다각형 렌더링
	for (const auto& triangle : m_Triangles) {
		
		// 은면제거
		// 매쉬 로컬 좌표계에서 평면 노멀과 카메라 look의 내적 수행
		XMVECTOR normal = XMLoadFloat3(&triangle.m_Normal);
		XMVECTOR look = XMVectorSubtract(XMLoadFloat3(&m_Vertices[m_Indices[triangle.m_StartIndex]].m_xmf3Position), LocalCameraPos);

		if (XMVectorGetX(XMVector3Dot(normal, look)) > 0.f) continue;

		f3PrevProject = f3InitProject = CGraphicsPipeline::Project(m_Vertices[m_Indices[triangle.m_StartIndex]].m_xmf3Position);
		bPrevInside = bInitInside = (-1.0f <= f3InitProject.x) && (f3InitProject.x <= 1.0f) &&
									(-1.0f <= f3InitProject.y) && (f3InitProject.y <= 1.0f);

		for (int i = 1; i < 3; ++i)	{
			XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(m_Vertices[m_Indices[triangle.m_StartIndex + i]].m_xmf3Position);
			bCurInside =	(-1.0f <= f3CurrentProject.x) && (f3CurrentProject.x <= 1.0f) &&
							(-1.0f <= f3CurrentProject.y) && (f3CurrentProject.y <= 1.0f);

			if (((0.0f <= f3CurrentProject.z) && (f3CurrentProject.z <= 1.0f)) && ((bCurInside || bPrevInside)))
				::Draw2DLine(hDCFrameBuffer, f3PrevProject, f3CurrentProject);

			f3PrevProject = f3CurrentProject;
			bPrevInside = bCurInside;
		}
		if (((0.0f <= f3InitProject.z) && (f3InitProject.z <= 1.0f)) && ((bInitInside || bPrevInside)))
			::Draw2DLine(hDCFrameBuffer, f3PrevProject, f3InitProject);
	}
#endif
}

CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	std::vector<CVertex> vertices = {
		// 앞
		{ -fHalfWidth, +fHalfHeight, -fHalfDepth },
		{ +fHalfWidth, +fHalfHeight, -fHalfDepth },
		{ +fHalfWidth, -fHalfHeight, -fHalfDepth },
		{ -fHalfWidth, -fHalfHeight, -fHalfDepth },

		// 위
		{ -fHalfWidth, +fHalfHeight, +fHalfDepth },
		{ +fHalfWidth, +fHalfHeight, +fHalfDepth },
		{ +fHalfWidth, +fHalfHeight, -fHalfDepth },
		{ -fHalfWidth, +fHalfHeight, -fHalfDepth },

		// 뒤
		{ -fHalfWidth, -fHalfHeight, +fHalfDepth },
		{ +fHalfWidth, -fHalfHeight, +fHalfDepth },
		{ +fHalfWidth, +fHalfHeight, +fHalfDepth },
		{ -fHalfWidth, +fHalfHeight, +fHalfDepth },

		// 바닥
		{ -fHalfWidth, -fHalfHeight, -fHalfDepth },
		{ +fHalfWidth, -fHalfHeight, -fHalfDepth },
		{ +fHalfWidth, -fHalfHeight, +fHalfDepth },
		{ -fHalfWidth, -fHalfHeight, +fHalfDepth },

		// 좌
		{ -fHalfWidth, +fHalfHeight, +fHalfDepth },
		{ -fHalfWidth, +fHalfHeight, -fHalfDepth },
		{ -fHalfWidth, -fHalfHeight, -fHalfDepth },
		{ -fHalfWidth, -fHalfHeight, +fHalfDepth },

		// 우
		{ +fHalfWidth, +fHalfHeight, -fHalfDepth },
		{ +fHalfWidth, +fHalfHeight, +fHalfDepth },
		{ +fHalfWidth, -fHalfHeight, +fHalfDepth },
		{ +fHalfWidth, -fHalfHeight, -fHalfDepth }
	};

	std::vector<uint32_t> indices = {
		// 앞
		0, 1, 2, 0, 2, 3,

		// 위
		4, 5, 6, 4, 6, 7,

		// 뒤
		8, 9, 10, 8, 10, 11,

		// 바닥
		12, 13, 14, 12, 14, 15,

		// 좌
		16, 17, 18, 16, 18, 19,

		// 우
		20, 21, 22, 20, 22, 23
	};

	SetMesh(std::move(vertices), std::move(indices));
}

CCubeMesh::~CCubeMesh()
{

}


// ========================================================
CAirplaneMesh::CAirplaneMesh(float fWidth, float fHeight, float fDepth)
// ========================================================
{
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	float x1 = fx * 0.2f, y1 = fy * 0.2f, x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy - y3)) / x1) * x2 + (fy - y3);
	int i = 0;

	// 총 16개의 고유 정점 배열 정의
	std::vector<CVertex> vertices = {
		// 앞면 (-fz) : 인덱스 0 ~ 7
		{  0.0f, +(fy + y3), -fz }, // 0
		{  0.0f, 0.0f,       -fz }, // 1
		{   +x1, -y1,        -fz }, // 2
		{   -x1, -y1,        -fz }, // 3
		{   +x2, +y2,        -fz }, // 4
		{   +fx, -y3,        -fz }, // 5
		{   -x2, +y2,        -fz }, // 6
		{   -fx, -y3,        -fz }, // 7

		// 뒷면 (+fz) : 인덱스 8 ~ 15
		{  0.0f, +(fy + y3), +fz }, // 8
		{  0.0f, 0.0f,       +fz }, // 9
		{   +x1, -y1,        +fz }, // 10
		{   -x1, -y1,        +fz }, // 11
		{   +x2, +y2,        +fz }, // 12
		{   +fx, -y3,        +fz }, // 13
		{   -x2, +y2,        +fz }, // 14
		{   -fx, -y3,        +fz }  // 15
	};

	std::vector<uint32_t> indices = {
		// Upper Plane
		0, 2, 1,
		0, 1, 3,
		4, 5, 2,
		6, 3, 7,

		// Lower Plane
		8, 9, 10,
		8, 11, 9,
		12, 10, 13,
		14, 15, 11,

		// Right Plane
		0, 8, 4,
		4, 8, 12,
		4, 12, 5,
		5, 12, 13,

		// Back/Right Plane
		2, 5, 13,
		2, 13, 10,
		1, 2, 10,
		1, 10, 9,

		// Left Plane
		8, 0, 6,
		8, 6, 14,
		14, 6, 7,
		14, 7, 15,

		// Back/Left Plane
		1, 9, 11,
		1, 11, 3,
		3, 11, 15,
		3, 15, 7
	};

	SetMesh(std::move(vertices), std::move(indices));

	//// Upper Plane
	//CTriangle* pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	//pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	//pFace->SetVertex(2, CVertex(0.0f, 0.0f, -fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	//pFace->SetVertex(1, CVertex(0.0f, 0.0f, -fz));
	//pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	//pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	//pFace->SetVertex(2, CVertex(+x1, -y1, -fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(-x2, +y2, -fz));
	//pFace->SetVertex(1, CVertex(-x1, -y1, -fz));
	//pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	//SetMesh(i++, pFace);

	////Lower Plane
	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	//pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	//pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	//pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	//pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(+x2, +y2, +fz));
	//pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	//pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	//pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	//pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	//SetMesh(i++, pFace);

	////Right Plane
	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	//pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), +fz));
	//pFace->SetVertex(2, CVertex(+x2, +y2, -fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	//pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), +fz));
	//pFace->SetVertex(2, CVertex(+x2, +y2, +fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	//pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	//pFace->SetVertex(2, CVertex(+fx, -y3, -fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(+fx, -y3, -fz));
	//pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	//pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	//SetMesh(i++, pFace);

	////Back/Right Plane
	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	//pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	//pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	//pFace->SetVertex(1, CVertex(+fx, -y3, +fz));
	//pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	//pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	//pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	//pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	//pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	//SetMesh(i++, pFace);

	////Left Plane
	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	//pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), -fz));
	//pFace->SetVertex(2, CVertex(-x2, +y2, -fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	//pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	//pFace->SetVertex(2, CVertex(-x2, +y2, +fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	//pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	//pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	//pFace->SetVertex(1, CVertex(-fx, -y3, -fz));
	//pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	//SetMesh(i++, pFace);

	////Back/Left Plane
	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	//pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	//pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	//pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	//pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	//pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	//pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	//SetMesh(i++, pFace);

	//pFace = new CTriangle(3);
	//pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	//pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	//pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	//SetMesh(i++, pFace);
}

CAirplaneMesh::~CAirplaneMesh()
{

}