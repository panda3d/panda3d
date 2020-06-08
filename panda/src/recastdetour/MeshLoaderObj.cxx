
#include "MeshLoaderObj.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#define _USE_MATH_DEFINES
#include <math.h>
#include "geom.h"
#include "nodePath.h"
#include "geomVertexReader.h"
#include "geomNode.h"
#include "pmap.h"
#include "pvector.h"


std::map<LVector3,int> mp;
int indexTemp = 0;

rcMeshLoaderObj::rcMeshLoaderObj() :
	m_scale(1.0f),
	m_verts(0),
	m_tris(0),
	m_normals(0),
	m_vertCount(0),
	m_triCount(0),
	temp_vcount(0)
{
}

rcMeshLoaderObj::~rcMeshLoaderObj()
{
	delete[] m_verts;
	delete[] m_normals;
	delete[] m_tris;
}

void rcMeshLoaderObj::addVertex(float x, float y, float z, int& cap)
{
	if (m_vertCount + 1 > cap)
	{
		cap = !cap ? 8 : cap * 2;
		float* nv = new float[cap * 3];
		if (m_vertCount)
			memcpy(nv, m_verts, m_vertCount * 3 * sizeof(float));
		delete[] m_verts;
		m_verts = nv;
	}
	float* dst = &m_verts[m_vertCount * 3];
	*dst++ = x * m_scale;
	*dst++ = y * m_scale;
	*dst++ = z * m_scale;
	m_vertCount++;
}

void rcMeshLoaderObj::addTriangle(int a, int b, int c, int& cap)
{
	if (m_triCount + 1 > cap)
	{
		cap = !cap ? 8 : cap * 2;
		int* nv = new int[cap * 3];
		if (m_triCount)
			memcpy(nv, m_tris, m_triCount * 3 * sizeof(int));
		delete[] m_tris;
		m_tris = nv;
	}
	int* dst = &m_tris[m_triCount * 3];
	*dst++ = a;
	*dst++ = b;
	*dst++ = c;
	m_triCount++;
}


static char* parseRow(char* buf, char* bufEnd, char* row, int len)
{
	bool start = true;
	bool done = false;
	int n = 0;
	while (!done && buf < bufEnd)
	{
		char c = *buf;
		buf++;
		// multirow
		switch (c)
		{
		case '\\':
			break;
		case '\n':
			if (start) break;
			done = true;
			break;
		case '\r':
			break;
		case '\t':
		case ' ':
			if (start) break;
			// else falls through
		default:
			start = false;
			row[n++] = c;
			if (n >= len - 1)
				done = true;
			break;
		}
	}
	row[n] = '\0';
	return buf;
}

static int parseFace(char* row, int* data, int n, int vcnt)
{
	int j = 0;
	while (*row != '\0')
	{
		// Skip initial white space
		while (*row != '\0' && (*row == ' ' || *row == '\t'))
			row++;
		char* s = row;
		// Find vertex delimiter and terminated the string there for conversion.
		while (*row != '\0' && *row != ' ' && *row != '\t')
		{
			if (*row == '/') *row = '\0';
			row++;
		}
		if (*s == '\0')
			continue;
		int vi = atoi(s);
		data[j++] = vi < 0 ? vi + vcnt : vi - 1;
		if (j >= n) return j;
	}
	return j;
}

bool rcMeshLoaderObj::load(const std::string& filename)
{
	char* buf = 0;
	FILE* fp = fopen(filename.c_str(), "rb");
	if (!fp)
		return false;
	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return false;
	}
	long bufSize = ftell(fp);
	if (bufSize < 0)
	{
		fclose(fp);
		return false;
	}
	if (fseek(fp, 0, SEEK_SET) != 0)
	{
		fclose(fp);
		return false;
	}
	buf = new char[bufSize];
	if (!buf)
	{
		fclose(fp);
		return false;
	}
	size_t readLen = fread(buf, bufSize, 1, fp);
	fclose(fp);

	if (readLen != 1)
	{
		delete[] buf;
		return false;
	}

	char* src = buf;
	char* srcEnd = buf + bufSize;
	char row[512];
	int face[32];
	float x, y, z;
	int nv;
	int vcap = 0;
	int tcap = 0;

	while (src < srcEnd)
	{
		// Parse one row
		row[0] = '\0';
		src = parseRow(src, srcEnd, row, sizeof(row) / sizeof(char));
		// Skip comments
		if (row[0] == '#') continue;
		if (row[0] == 'v' && row[1] != 'n' && row[1] != 't')
		{
			// Vertex pos
			sscanf(row + 1, "%f %f %f", &x, &y, &z);
			addVertex(x, z, -y, vcap);		//exchanged y and z to match with panda3d's coordinate system
		}
		if (row[0] == 'f')
		{
			// Faces
			nv = parseFace(row + 1, face, 32, m_vertCount);
			for (int i = 2; i < nv; ++i)
			{
				const int a = face[0];
				const int b = face[i - 1];
				const int c = face[i];
				if (a < 0 || a >= m_vertCount || b < 0 || b >= m_vertCount || c < 0 || c >= m_vertCount)
					continue;
				addTriangle(a, b, c, tcap);
			}
		}
	}

	delete[] buf;

	// Calculate normals.
	m_normals = new float[m_triCount * 3];
	for (int i = 0; i < m_triCount * 3; i += 3)
	{
		const float* v0 = &m_verts[m_tris[i] * 3];
		const float* v1 = &m_verts[m_tris[i + 1] * 3];
		const float* v2 = &m_verts[m_tris[i + 2] * 3];
		float e0[3], e1[3];
		for (int j = 0; j < 3; ++j)
		{
			e0[j] = v1[j] - v0[j];
			e1[j] = v2[j] - v0[j];
		}
		float* n = &m_normals[i];
		n[0] = e0[1] * e1[2] - e0[2] * e1[1];
		n[1] = e0[2] * e1[0] - e0[0] * e1[2];
		n[2] = e0[0] * e1[1] - e0[1] * e1[0];
		float d = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		if (d > 0)
		{
			d = 1.0f / d;
			n[0] *= d;
			n[1] *= d;
			n[2] *= d;
		}
	}

	m_filename = filename;
	return true;
}
std::vector<LVector3> vc,fc;
bool rcMeshLoaderObj::load(NodePath node)
{
	NodePathCollection geomNodeCollection = node.find_all_matches("**/+GeomNode");
	//std::cout << "Number of geomNode_paths: " << geomNodeCollection.get_num_paths() << std::endl;
	int vcap = 0;
	int tcap = 0;


	for (size_t i = 0; i < geomNodeCollection.get_num_paths(); ++i)
	{
		temp_vcount = m_vertCount;
		//std::cout << "Vertices count: " << m_vertCount << std::endl;
		PT(GeomNode) g = DCAST(GeomNode, geomNodeCollection.get_path(i).node());
		//std::cout << "GeomPath i = " << i << std::endl;
		processGeomNode(g, vcap, tcap);
	}
	for (int i = 0;i < vc.size();i++) {
		//std::cout << "v " << vc[i][0] << " " << vc[i][1] << " " << vc[i][2] << "\n";
	}
	for (int i = 0;i < fc.size();i++) {
		//std::cout << "f " << fc[i][0] << " " << fc[i][1] << " " << fc[i][2] << "\n";
	}
	return true;
}

void rcMeshLoaderObj::processPrimitive(const GeomPrimitive *orig_prim, const GeomVertexData *vdata, int& tcap) {

	GeomVertexReader vertex(vdata, "vertex");

	CPT(GeomPrimitive) prim = orig_prim->decompose();
	//std::cout << "prim->get_num_primitives(): " << prim->get_num_primitives() << std::endl;
	for (size_t k = 0; k < prim->get_num_primitives(); ++k) {
		int s = prim->get_primitive_start(k);
		int e = prim->get_primitive_end(k);
		//std::cout << "(s, e): " << s << "\t" << e << "\n";
		LVector3 v;
		if (e - s == 3)
		{
			int a = prim->get_vertex(s);
			vertex.set_row(a);
			v = vertex.get_data3();
			a = mp[v];

			int b = prim->get_vertex(s + 1);
			vertex.set_row(b);
			v = vertex.get_data3();
			b = mp[v];
			
			int c = prim->get_vertex(s + 2);
			vertex.set_row(c);
			v = vertex.get_data3();
			c = mp[v];

			//if (a < 0 || a >= m_vertCount || b < 0 || b >= m_vertCount || c < 0 || c >= m_vertCount)
				//continue;
			//std::cout <<"f: " << a << "\t" << b << "\t" << c << "\ntcap: "<<tcap<<"\n";
			LVector3 xvx = { float(a + 1), float(b + 1), float(c + 1) };
			fc.push_back(xvx);
			addTriangle(a, b, c, tcap);
		}
		else if(e - s > 3 ) {
			//std::cout << "More than 3 vertices\n";
			for (int i = s+2; i < e; ++i)
			{
				int a = prim->get_vertex(s);
				vertex.set_row(a);
				v = vertex.get_data3();
				a = mp[v];

				int b = prim->get_vertex(i-1);
				vertex.set_row(b);
				v = vertex.get_data3();
				b = mp[v];

				int c = prim->get_vertex(i);
				vertex.set_row(c);
				v = vertex.get_data3();
				c = mp[v];

				//if (a < 0 || a >= m_vertCount || b < 0 || b >= m_vertCount || c < 0 || c >= m_vertCount)
					//continue;
				//std::cout << "f: " << a << "\t" << b << "\t" << c << "\ntcap: " << tcap << "\n";
				LVector3 xvx = { float(a + 1), float(b + 1), float(c + 1) };
				fc.push_back(xvx);
				addTriangle(a, b, c, tcap);
			}
		}
		else continue;
		
	}
	return;
}

void rcMeshLoaderObj::processVertexData(const GeomVertexData *vdata, int& vcap) {
	GeomVertexReader vertex(vdata, "vertex");
	float x, y, z;
	
	while (!vertex.is_at_end()) {
		//std::cout << vcap++ << std::endl;
		LVector3 v = vertex.get_data3();
		x = v[0];
		y = v[1];
		z = v[2];
		if (mp.find(v) == mp.end())
		{
			addVertex(x, z, -y, vcap);
			mp[v] = indexTemp++;
			LVector3 xvx = { v[0],v[2],-v[1] };
			vc.push_back(xvx);
		}
		
		//nout << "vcount: " << m_vertCount << "\tvcap: "<< vcap <<"\n";
		//nout << "V = " << v << std::endl;
	}
	return;
}
void rcMeshLoaderObj::processGeom(CPT(Geom) geom, int& vcap, int& tcap) {
	
	CPT(GeomVertexData) vdata = geom->get_vertex_data();
	
	processVertexData(vdata, vcap);
	//std::cout << "geom->get_num_primitives(): " << geom->get_num_primitives() << std::endl;
	for (size_t i = 0; i < geom->get_num_primitives(); ++i) {
		
		CPT(GeomPrimitive) prim = geom->get_primitive(i);
		processPrimitive(prim, vdata, tcap);
	}
	return;
}
void rcMeshLoaderObj::processGeomNode(GeomNode *geomnode, int& vcap, int& tcap) {

	//std::cout << "Number of geoms: " << geomnode->get_num_geoms() << std::endl;
	if (geomnode->get_num_geoms() > 1)
	{
		std::cout << "Cannot proceed: Make sure number of geoms = 1\n";
		return;
	}
	for (size_t j = 0; j < geomnode->get_num_geoms(); ++j) {
		
		CPT(Geom) geom = geomnode->get_geom(j);
		processGeom(geom, vcap, tcap);
	}
	return;
}