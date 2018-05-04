#include "apfMDS.h"
#include <apfMesh2.h>
#include <apfShape.h>
#include <apfNumbering.h>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <gmi.h>
#include <pcu_util.h>
#include <cstdlib>

#define MAX_ELEM_NODES 20

namespace apf {

typedef std::map<int, apf::Vector3> Nodes;

static void readNodes(std::istream& f, Nodes& nodes)
{
  char c;
  std::string line;
  std::pair<int, apf::Vector3> entry;
  while (f.peek() != '!' && f.peek() != EOF)
  {
      std::getline(f, line);
	  line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
	  std::stringstream ss(line);
	  ss >> entry.first >>c >> entry.second[0] >>c  >> entry.second[1] >> c >> entry.second[2];
	//  std::cout << entry.first <<" "<< entry.second[0] <<" "  << entry.second[1] 
	//  << " " << entry.second[2] << std::endl;
	  nodes.insert(entry);
  }    
}

typedef std::map<int, MeshEntity*> Vertices;

static Mesh2* parseElems(std::string upper,std::istream& f, Nodes& nodes)
{
  Mesh2* m = 0;
  apf::FieldShape* shape = 0;
  apf::FieldShape* prevShape = 0;
  apf::Numbering* enumbers = 0;

  unsigned n_nodes_per_elem = 0;
  int apfType=0;
  if (upper.find("T2") != std::string::npos ||
      upper.find("B2") != std::string::npos)
  {
    apfType = apf::Mesh::EDGE;
	n_nodes_per_elem = 2;
  }
  else if (upper.find("S4") != std::string::npos ||
           upper.find("CQUAD4") != std::string::npos)
  {   
    apfType = apf::Mesh::QUAD;
	n_nodes_per_elem = 4;
  }
  else if (upper.find("CTETR4") != std::string::npos ||
           upper.find("341") != std::string::npos)
  {   
    apfType = apf::Mesh::TET;
	n_nodes_per_elem = 4;
  }
  else if (upper.find("CTETR10") != std::string::npos ||
           upper.find("342") != std::string::npos)
  {   
    apfType = apf::Mesh::TET;
	n_nodes_per_elem = 10;
  }
  else if (upper.find("CHEXA8") != std::string::npos ||
           upper.find("361") != std::string::npos)
  {   
    apfType = apf::Mesh::HEX;
	n_nodes_per_elem = 8;
	shape = apf::getLagrange(1);
  }
  else if (upper.find("CHEXA20") != std::string::npos ||
           upper.find("362") != std::string::npos)
  {   
    apfType = apf::Mesh::HEX;
	n_nodes_per_elem = 20;
	shape = apf::getLagrange(2);
  }
  else
  {
	  std::cerr << "Unrecognized element type: " << upper << std::endl;
  }
  
  if (!m) {
      m = makeEmptyMdsMesh(gmi_load(".null"), Mesh::typeDimension[apfType], false);
      if (shape != m->getShape())
        changeMeshShape(m, shape, false);
      enumbers = createNumbering(m, "fstr_element", getConstant(m->getDimension()), 1);
  }
  if (prevShape) PCU_ALWAYS_ASSERT(prevShape == shape);

  while (f.peek() != '!' && f.peek() != EOF)
  {
	Downward ev;
	Vertices verts;
	
	long eid = 0;
    char c;
    f >> eid >> c;
	unsigned id_count=0;
	while (id_count < n_nodes_per_elem)
    {
		// Read entire line (up to carriage return) of comma-separated values
        std::string csv_line;
        std::getline(f, csv_line);

        // Create a stream object out of the current line
        std::stringstream line_stream(csv_line);

        // Process the comma-separated values
        std::string cell;
        while (std::getline(line_stream, cell, ','))
        {
			char * endptr;
            long nid = std::strtol(cell.c_str(), &endptr, /*base=*/10);
			if (nid!=0 || cell.c_str() != endptr)
            {
				if (!verts.count(nid)) {
					MeshEntity* v = m->createVert(0);
					m->setPoint(v, 0, nodes[nid]);
					verts[nid] = v;
				}
				ev[id_count] = verts[nid];
			    id_count++;
				
			}
		}
	}
	MeshEntity* e = buildElement(m, 0, apfType, ev);
	number(enumbers, e, 0, 0, eid);
    prevShape = shape;
  }

  return m;
}

Mesh2* loadMdsFromFSTR(const char* filename)
{
	std::ifstream f;
	f.open(filename, std::ios::in);
    if (!f) {
      fprintf(stderr,"couldn't open FSTR file \"%s\"\n",filename);
      abort();
    }

    Nodes nodes;
	Mesh2* m= NULL;
	std::string s;
    while (true)
    {
      std::getline(f, s);
	  if (f)
      {
	    std::string upper(s);
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
	    if (upper.find("!NODE") == static_cast<std::string::size_type>(0))
        {
		  readNodes(f, nodes);
	    }
	    else if (upper.find("!ELEMENT") == static_cast<std::string::size_type>(0))
        {
		   m = parseElems(upper, f, nodes);
	    }
	  } 
	
	  if (f.eof()) break;
    }
  
  m->acceptChanges();
  deriveMdsModel(m);
  return m;
}

}
