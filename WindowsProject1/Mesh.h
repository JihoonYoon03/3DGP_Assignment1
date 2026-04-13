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

class CPolygon {
public:
	CPolygon() {}
	CPolygon(int nVertices);
	virtual ~CPolygon();

	// 폴리곤을 구성하는 정점들의 리스트
	int			m_nVertices	= 0;
	CVertex*	m_pVertices	= nullptr;

	void SetVertex(int nIndex, CVertex& vertex);
	void SetVertex(int nIndex, const CVertex& vertex);
};

class CMesh {
public:
	CMesh() {}
	CMesh(int nPolygons);
	virtual ~CMesh();

	void SetPolygon(int nIndex, CPolygon* pPolygon);

	virtual void Render(HDC hDCFrameBuffer);

	void AddRef() { m_nReferences++; }
	void Release();

private:
	int m_nReferences = 1;
	
	int			m_nPolygons = 0;
	CPolygon**	m_ppPolygons = nullptr;

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