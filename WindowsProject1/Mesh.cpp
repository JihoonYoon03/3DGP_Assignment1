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
CMesh::CMesh(const WCHAR* fileName)
//=============================
{
	LoadMeshFromObj(fileName);
}

CMesh::~CMesh()
{

}

void CMesh::Release()
{
	m_nReferences--;
	if (m_nReferences <= 0)
		delete this;
}

void CMesh::LoadMeshFromObj(const WCHAR* fileName)
{
	std::ifstream file(fileName);
	if (not file) {
		OutputDebugStringW(L".Obj file Load Error\n");
		return;
	}

	std::string line;
	std::vector<CVertex> vertexBuf;
	std::vector<UINT> indiceBuf;

	while (std::getline(file, line))
	{
		if (line.empty()) continue;
		std::stringstream ss(line);
		std::string prefix;
		ss >> prefix;

		if (prefix == "v") {
			XMFLOAT3 pos;
			ss >> pos.x >> pos.y >> pos.z;
			vertexBuf.push_back(CVertex(pos.x * 0.05f, pos.y * 0.05f, pos.z * 0.05f));
		}
		else if (prefix == "f") {
			UINT i[3];
			ss >> i[0] >> i[1] >> i[2];

			// 1-based index를 0-based로 변환하여 인덱스 벡터에 저장
			UINT idx1 = i[0] - 1;
			UINT idx2 = i[1] - 1;
			UINT idx3 = i[2] - 1;

			indiceBuf.push_back(idx1);
			indiceBuf.push_back(idx2);
			indiceBuf.push_back(idx3);
		}
	}

	SetMesh(vertexBuf, indiceBuf);
}

void CMesh::SetMesh(std::vector<CVertex>& vertices, std::vector<UINT>& indices)
{
	m_Vertices = std::move(vertices);
	m_Indices = std::move(indices);

	// 삼각형 계산
	// 3개 단위의 인덱스가 아닌 경우 예외처리
	if (m_Indices.size() % 3 != 0) {
		std::wstring buf = std::format(L"Mesh Data Error :: Vertices {} / Indices {}\n", m_Vertices.size(), m_Indices.size());
		OutputDebugStringW(buf.c_str());
		return;
	}
	else {
		m_Triangles.reserve(m_Indices.size() / 3);

		for (size_t i = 0; i < m_Indices.size(); i += 3) {
			CTriangle triangle{ static_cast<UINT>(i) };

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

	// 출력할 삼각형들만 저장하는 리스트
	std::vector<CTriangle*> renderList;

#ifndef WIREFRAME_MODE

	// 모든 다각형 렌더링
	for (auto& triangle : m_Triangles) {

		// 은면제거
		// 매쉬 로컬 좌표계에서 평면 노멀과 카메라 look의 내적 수행
		XMVECTOR normal = XMLoadFloat3(&triangle.m_Normal);
		XMVECTOR look = XMVectorSubtract(XMLoadFloat3(&m_Vertices[m_Indices[triangle.m_StartIndex]].m_xmf3Position), LocalCameraPos);

		if (XMVectorGetX(XMVector3Dot(normal, look)) > 0.f) continue;

		// 카메라 좌표계로 먼저 변환
		XMFLOAT4X4 viewMatrix = camera->GetViewMatrix();

		XMFLOAT3 f3CurProject1 = CGraphicsPipeline::WorldViewTransform(m_Vertices[m_Indices[triangle.m_StartIndex]].m_xmf3Position, viewMatrix);
		XMFLOAT3 f3CurProject2 = CGraphicsPipeline::WorldViewTransform(m_Vertices[m_Indices[triangle.m_StartIndex + 1]].m_xmf3Position, viewMatrix);
		XMFLOAT3 f3CurProject3 = CGraphicsPipeline::WorldViewTransform(m_Vertices[m_Indices[triangle.m_StartIndex + 2]].m_xmf3Position, viewMatrix);

		triangle.m_averageZ = (f3CurProject1.z + f3CurProject2.z + f3CurProject3.z) / 3;

		// 카메라 뒤에 있다면 컬링
		/*if (f3CurProject1.z < 0.f || f3CurProject2.z < 0.f || f3CurProject3.z < 0.f) {
			OutputDebugStringW(std::wstring{ L"카메라 뒤로 넘어감 체킹\n" }.c_str());
			continue;
		}*/

		// 렌더 대상 리스트에 해당 삼각형 저장
		renderList.push_back(&triangle);
	}

	// z좌표 기준 내림차순 정렬, 먼 것부터 렌더링
	std::sort(renderList.begin(), renderList.end(), [](const CTriangle* a, const CTriangle* b) {
		return a->m_averageZ > b->m_averageZ;
		});

	for (const auto* triangle : renderList) {

		// 모든 정점 원근 투영 변환 및 렌더링
		XMFLOAT4X4 PerspectiveProject = camera->GetPerspectiveProjectMatrix();

		XMFLOAT3 f3CurProject1 = CGraphicsPipeline::Project(m_Vertices[m_Indices[triangle->m_StartIndex]].m_xmf3Position);
		XMFLOAT3 f3CurProject2 = CGraphicsPipeline::Project(m_Vertices[m_Indices[triangle->m_StartIndex + 1]].m_xmf3Position);
		XMFLOAT3 f3CurProject3 = CGraphicsPipeline::Project(m_Vertices[m_Indices[triangle->m_StartIndex + 2]].m_xmf3Position);

		f3CurProject1 = CGraphicsPipeline::ScreenTransform(f3CurProject1);
		f3CurProject2 = CGraphicsPipeline::ScreenTransform(f3CurProject2);
		f3CurProject3 = CGraphicsPipeline::ScreenTransform(f3CurProject3);

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

		for (int i = 1; i < 3; ++i) {
			XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(m_Vertices[m_Indices[triangle.m_StartIndex + i]].m_xmf3Position);
			bCurInside = (-1.0f <= f3CurrentProject.x) && (f3CurrentProject.x <= 1.0f) &&
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

bool CMesh::RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float& fNearHitDistance)
{
	float fHitDistance;
	bool bIntersected = TriangleTests::Intersects(xmRayOrigin, xmRayDirection, v0, v1, v2, fHitDistance);
	if (bIntersected && (fHitDistance < fNearHitDistance)) 
		fNearHitDistance = fHitDistance;

	return bIntersected;
}

bool CMesh::CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float& fNearHitDistance)
{
	bool nIntersections = false;
	// 1차 검사
	bool bIntersected = m_xmOOBB.Intersects(xmvPickRayOrigin, xmvPickRayDirection, fNearHitDistance);

	// 정밀 검사
	if (bIntersected) {
		for (const auto& triangle : m_Triangles) {
			XMVECTOR v0 = XMLoadFloat3(&(m_Vertices[m_Indices[triangle.m_StartIndex]].m_xmf3Position));
			XMVECTOR v1 = XMLoadFloat3(&(m_Vertices[m_Indices[triangle.m_StartIndex + 1]].m_xmf3Position));
			XMVECTOR v2 = XMLoadFloat3(&(m_Vertices[m_Indices[triangle.m_StartIndex + 2]].m_xmf3Position));
			BOOL bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, fNearHitDistance);
			if (bIntersected) {
				nIntersections = true;
				break;
			}
		}
	}

	return nIntersections;
}

// ========================================================
CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth)
// ========================================================
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

	std::vector<UINT> indices = {
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

	SetMesh(vertices, indices);
	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CCubeMesh::~CCubeMesh()
{

}

// ========================================================
CWallMesh::CWallMesh(float fWidth, float fHeight, float fDepth, int nSubRects)
// ========================================================
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;
	float fCellWidth = fWidth * (1.0f / nSubRects);
	float fCellHeight = fHeight * (1.0f / nSubRects);
	float fCellDepth = fDepth * (1.0f / nSubRects);

	std::vector<CVertex> vertices;
	std::vector<UINT> indices;

	// 총 사각형 개수 = (왼쪽, 오른쪽, 위, 아래) + (앞, 뒤)
	int nTotalQuads = (4 * nSubRects * nSubRects) + 2;
	vertices.reserve(nTotalQuads * 4);
	indices.reserve(nTotalQuads * 6);

	// 사각형을 2개의 삼각형 단위로 나누어 인덱스와 정점을 추가
	auto AddQuad = [&](const CVertex& v0, const CVertex& v1, const CVertex& v2, const CVertex& v3) {
		UINT startIndex = static_cast<UINT>(vertices.size());
		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);

		// 삼각형 1 (0, 1, 2)
		indices.push_back(startIndex + 0);
		indices.push_back(startIndex + 1);
		indices.push_back(startIndex + 2);

		// 삼각형 2 (0, 2, 3)
		indices.push_back(startIndex + 0);
		indices.push_back(startIndex + 2);
		indices.push_back(startIndex + 3);
		};

	for (int i = 0; i < nSubRects; i++)
	{
		for (int j = 0; j < nSubRects; j++)
		{
			// Left Face
			AddQuad(
				CVertex(-fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + (j * fCellDepth)),
				CVertex(-fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + (j * fCellDepth)),
				CVertex(-fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)),
				CVertex(-fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth))
			);

			// Right Face
			AddQuad(
				CVertex(+fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + (j * fCellDepth)),
				CVertex(+fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + (j * fCellDepth)),
				CVertex(+fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)),
				CVertex(+fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth))
			);

			// Top Face
			AddQuad(
				CVertex(-fHalfWidth + (i * fCellWidth), +fHalfHeight, -fHalfDepth + (j * fCellDepth)),
				CVertex(-fHalfWidth + ((i + 1) * fCellWidth), +fHalfHeight, -fHalfDepth + (j * fCellDepth)),
				CVertex(-fHalfWidth + ((i + 1) * fCellWidth), +fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)),
				CVertex(-fHalfWidth + (i * fCellWidth), +fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth))
			);

			// Bottom Face
			AddQuad(
				CVertex(-fHalfWidth + (i * fCellWidth), -fHalfHeight, -fHalfDepth + (j * fCellDepth)),
				CVertex(-fHalfWidth + ((i + 1) * fCellWidth), -fHalfHeight, -fHalfDepth + (j * fCellDepth)),
				CVertex(-fHalfWidth + ((i + 1) * fCellWidth), -fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)),
				CVertex(-fHalfWidth + (i * fCellWidth), -fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth))
			);
		}
	}

	// Front Face
	AddQuad(
		CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth),
		CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth),
		CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth),
		CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth)
	);

	// Back Face
	AddQuad(
		CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth),
		CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth),
		CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth),
		CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth)
	);

	SetMesh(vertices, indices);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
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

	std::vector<UINT> indices = {
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

	SetMesh(vertices, indices);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fx, fy, fz), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CAirplaneMesh::~CAirplaneMesh()
{

}