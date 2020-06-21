#include "navMesh.h"
#include "geom.h"
#include "geomTrifans.h"

NavMesh::NavMesh() {

}

NavMesh::~NavMesh() {
  
}

NavMesh::NavMesh(dtNavMesh *nav_mesh, rcPolyMesh *pmesh, rcPolyMeshDetail *dmesh) {
  _nav_mesh = nav_mesh;
  _pmesh = pmesh;
  _dmesh = dmesh;
}

PT(GeomNode) NavMesh::draw_poly_mesh_geom() {

  PT(GeomVertexData) vdata;
  vdata = new GeomVertexData("vertexInfo", GeomVertexFormat::get_v3c4(), Geom::UH_static);
  vdata->set_num_rows(_pmesh->nverts);

  GeomVertexWriter vertex(vdata, "vertex");
  GeomVertexWriter colour(vdata, "color");

  const int nvp = _pmesh->nvp;
  std::cout << "nvp: " << nvp << std::endl;
  const float cs = _pmesh->cs;
  std::cout << "cs: " << cs << std::endl;
  const float ch = _pmesh->ch;
  std::cout << "ch: " << ch << std::endl;
  const float* orig = _pmesh->bmin;
  std::cout << "orig: " << orig[0] << "\t" << orig[1] << "\t" << orig[2] << std::endl;

  std::cout << "_pmesh->npolys: " << _pmesh->npolys << std::endl;
  std::cout << "_pmesh->nverts: " << _pmesh->nverts << std::endl;

  for (int i = 0;i < _pmesh->nverts * 3;i += 3) {

    const unsigned short* v = &_pmesh->verts[i];

    //convert to world space
    const float x = orig[0] + v[0] * cs;
    const float y = orig[1] + v[1] * ch;
    const float z = orig[2] + v[2] * cs;

    //vertex.add_data3(x, -z, y); //if origingally model is z-up
    vertex.add_data3(x, y, z); //if originally model is y-up
    colour.add_data4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1);
    std::cout << "index: " << i / 3 << "\t" << x << "\t" << y << "\t" << z << "\n";

  }

  PT(GeomNode) node;
  node = new GeomNode("gnode");

  PT(GeomTrifans) prim;
  prim = new GeomTrifans(Geom::UH_static);

  for (int i = 0; i < _pmesh->npolys; ++i) {


    const unsigned short* p = &_pmesh->polys[i*nvp * 2];

    // Iterate the vertices.
    //unsigned short vi[3];  // The vertex indices.
    for (int j = 0; j < nvp; ++j) {
      if (p[j] == RC_MESH_NULL_IDX) {
        break;// End of vertices.
      }
      if (p[j + nvp] == RC_MESH_NULL_IDX) {
        prim->add_vertex(p[j]);
        // The edge beginning with this vertex is a solid border.
      }
      else {
        prim->add_vertex(p[j]);
        // The edge beginning with this vertex connects to 
        // polygon p[j + nvp].
      }
      std::cout << "p[j]: " << p[j] << std::endl;

    }
    prim->close_primitive();

  }
  PT(Geom) polymeshgeom;
  polymeshgeom = new Geom(vdata);
  polymeshgeom->add_primitive(prim);

  node->add_geom(polymeshgeom);
  std::cout << "Number of Polygons: " << _pmesh->npolys << std::endl;


  return node;
}
