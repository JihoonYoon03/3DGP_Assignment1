#pragma once

class CVertex {
public:
	CVertex() : m_xmf3Position{ XMFLOAT3(0.f, 0.f, 0.f) } {}
	CVertex(float x, float y, float z) : m_xmf3Position{ XMFLOAT3(x, y, z) } {}
	virtual ~CVertex() {}

	XMFLOAT3	m_xmf3Position;
};

class CTriangle {
public:
	CTriangle() {}
	CTriangle(const UINT& index) : m_StartIndex(index) {}
	virtual ~CTriangle() {}

	UINT				m_StartIndex = 0;
	XMFLOAT3			m_Normal{};
	float				m_averageZ = 0;

	void CalculateNormal(const CVertex& vertex0, const CVertex& vertex1, const CVertex& vertex2);
};

class CMesh {
public:
	CMesh() {}
	CMesh(const WCHAR* fileName, float fScale = 1.0f);
	virtual ~CMesh();

	void LoadMeshFromObj(const WCHAR* pstrFileName, float fScale);
	void SetMesh(std::vector<CVertex>& vertices, std::vector<UINT>& indices);
	
	virtual void Render(HDC hDCFrameBuffer, class CCamera* camera, const XMVECTOR& LocalCameraPos, const XMVECTOR& dirLightPos, std::vector<HPEN>& hPens, std::vector<HBRUSH>& hBrushes);

	bool RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float& fNearHitDistance);
	bool CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float& fNearHitDistance);

	void AddRef() { m_nReferences++; }
	void Release();
	
	BoundingOrientedBox			m_xmOOBB = BoundingOrientedBox();

private:
	int m_nReferences = 1;
	
	// raw 데이터 (정점, 인덱스)
	std::vector<CVertex>	m_Vertices;
	std::vector<UINT>		m_Indices;

	// 평면 데이터 (인덱스 위치, 노멀)
	std::vector<CTriangle>	m_Triangles;

	// 최종적으로 그려낼 정점 모임
	size_t					m_nDrawingPoints = 0;
	std::vector<POINT>		m_vDrawingPoints;
};

class CCubeMesh : public CMesh {
public:
	CCubeMesh(float fWidth = 4.f, float fHeight = 4.f, float fDepth = 4.f);
	virtual ~CCubeMesh();
};

class CWallMesh : public CMesh
{
public:
	CWallMesh(float fWidth = 4.0f, float fHeight = 4.0f, float fDepth = 4.0f, int nSubRects = 20);
	virtual ~CWallMesh() {}
};

class CAirplaneMesh : public CMesh {
public:
	CAirplaneMesh(float fWidth = 20.f, float fHeight = 20.f, float fDepth = 4.f);
	virtual ~CAirplaneMesh();
};