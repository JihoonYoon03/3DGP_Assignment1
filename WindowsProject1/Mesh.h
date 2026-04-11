#pragma once

class CPoint3D {
public:
	CPoint3D() {}
	CPoint3D(float x, float y, float z) : x{ x }, y{ y }, z{ z } {}
	virtual ~CPoint3D() {}

	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

class CVertex {
public:
	CVertex() {}
	CVertex(float x, float y, float z) : m_f3Position{ CPoint3D(x, y, z) } {}
	virtual ~CVertex() {}

	CPoint3D	m_f3Position;
};

class CPolygon {
public:
	CPolygon() {}
	CPolygon(int nVertices);
	virtual ~CPolygon();

	// 폴리곤을 구성하는 정점들의 리스트
	int			m_nVertices	= 0;
	CVertex*	m_pVertices	= nullptr;

	void SetVertex(int nIndex, CVertex vertex);
};

class CMesh {
public:
	CMesh() {}
	CMesh(int nPolygons);
	virtual ~CMesh();

private:
	int m_nReferences = 1;

public:
	void AddRef() { m_nReferences++; }
	void Release();

private:
	int			m_nPolygons = 0;
	CPolygon**	m_ppPolygons = nullptr;

public:
	void SetPolygon(int nIndex, CPolygon* pPolygon);

	virtual void Render(HDC hDCFrameBuffer);

};

class CCubeMesh : public CMesh {
public:
	CCubeMesh(float fWidth = 4.f, float fHeight = 4.f, float fDepth = 4.f);
	virtual ~CCubeMesh();
};