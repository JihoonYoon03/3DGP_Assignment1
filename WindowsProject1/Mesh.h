#pragma once

/*// == XMFLOAT3
class CPoint3D {
public:
	CPoint3D() {}
	CPoint3D(float x, float y, float z) : x{ x }, y{ y }, z{ z } {}
	virtual ~CPoint3D() {}

	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};
*/

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
	CTriangle(const uint32_t& index) : m_StartIndex(index) {}
	virtual ~CTriangle() {}

	uint32_t			m_StartIndex;
	XMFLOAT3			m_Normal;
	float				m_averageZ = 0;

	void CalculateNormal(const CVertex& vertex0, const CVertex& vertex1, const CVertex& vertex2);
};

class CMesh {
public:
	CMesh() {}
	virtual ~CMesh();

	void SetMesh(std::vector<CVertex>&& vertices, std::vector<uint32_t>&& indices);

	//virtual void Render(HDC hDCFrameBuffer, class CCamera* camera);
	virtual void Render(HDC hDCFrameBuffer, class CCamera* camera, const XMVECTOR& LocalCameraPos);

	void AddRef() { m_nReferences++; }
	void Release();

private:
	int m_nReferences = 1;
	
	// raw 데이터 (정점, 인덱스)
	std::vector<CVertex>	m_Vertices;
	std::vector<uint32_t>	m_Indices;

	// 평면 데이터 (인덱스 위치, 노멀)
	std::vector<CTriangle>	m_Triangles;

	// 최종적으로 그려낼 정점 모임
	size_t m_nDrawingPoints = 0;
	std::vector<POINT> m_vDrawingPoints;
};

class CCubeMesh : public CMesh {
public:
	CCubeMesh(float fWidth = 4.f, float fHeight = 4.f, float fDepth = 4.f);
	virtual ~CCubeMesh();
};

class CAirplaneMesh : public CMesh {
public:
	CAirplaneMesh(float fWidth = 20.f, float fHeight = 20.f, float fDepth = 4.f);
	virtual ~CAirplaneMesh();
};