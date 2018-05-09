#include <apf.h>
#include <pumi.h>
#include <gmi_null.h>
#include <apfMDS.h>
#include <apfMesh2.h>
#include <PCU.h>
#include <cstdlib>

int main(int argc, char** argv)
{
  MPI_Init(&argc,&argv);
  PCU_Comm_Init();
  if ( argc != 4 ) {
    if ( !PCU_Comm_Self() )
      printf("Usage: %s <in .msh> <out .dmg> <out .smb>\n", argv[0]);
    MPI_Finalize();
    exit(EXIT_FAILURE);
  }
  gmi_register_null();
  apf::Mesh2* m = apf::loadMdsFromFSTR(argv[1]);
  m->verify();
  gmi_write_dmg(m->getModel(), argv[2]);
  
  pumi::instance()->mesh = m;
  pumi_mesh_print(m, true);
  pumi_mesh_write( m, "mesh", "vtk" );
  m->writeNative(argv[3]);
  m->destroyNative();
  apf::destroyMesh(m);
  PCU_Comm_Free();
  MPI_Finalize();
}


