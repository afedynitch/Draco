//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   mesh/test/tstX3D_Draco_Mesh_Reader.cc
 * \author Ryan Wollaeger <wollaeger@lanl.gov>
 * \date   Tuesday, Jul 10, 2018, 10:23 am
 * \brief  X3D_Draco_Mesh_Reader class unit test.
 * \note   Copyright (C) 2018 Los Alamos National Security, LLC.
 *         All rights reserved. */
//---------------------------------------------------------------------------//

#include "Test_Mesh_Interface.hh"
#include "c4/ParallelUnitTest.hh"
#include "ds++/Release.hh"
#include "ds++/Soft_Equivalence.hh"
#include "mesh/Draco_Mesh_Builder.hh"
#include "mesh/X3D_Draco_Mesh_Reader.hh"

using rtt_mesh::Draco_Mesh;
using rtt_mesh::Draco_Mesh_Builder;
using rtt_mesh::X3D_Draco_Mesh_Reader;

//---------------------------------------------------------------------------//
// TESTS
//---------------------------------------------------------------------------//

// Parse an X3D file format and compare to reference
void read_x3d_mesh_2d(rtt_c4::ParallelUnitTest &ut) {

  // >>> PARSE MESH

  const std::string inputpath = ut.getTestSourcePath();
  const std::string filename = inputpath + "x3d.mesh.in";
  const std::vector<std::string> bdy_filenames = {
      inputpath + "x3d.mesh.bdy1.in", inputpath + "x3d.mesh.bdy2.in",
      inputpath + "x3d.mesh.bdy3.in", inputpath + "x3d.mesh.bdy4.in"};

  // construct reader
  std::shared_ptr<X3D_Draco_Mesh_Reader> x3d_reader(
      new X3D_Draco_Mesh_Reader(filename, bdy_filenames));

  // read mesh
  x3d_reader->read_mesh();

  // >>> CHECK HEADER DATA

  if (x3d_reader->get_process() != 0)
    ITFAILS;

  if (x3d_reader->get_numdim() != 2)
    ITFAILS;

  if (x3d_reader->get_numcells() != 1)
    ITFAILS;

  if (x3d_reader->get_numnodes() != 4)
    ITFAILS;

  // >>> CHECK CELL-NODE DATA

  if (x3d_reader->get_celltype(0) != 4)
    ITFAILS;

  std::vector<int> test_cellnodes = {0, 1, 3, 2};
  if (x3d_reader->get_cellnodes(0) != test_cellnodes)
    ITFAILS;

  // >>> CHECK NODE-COORD DATA

  std::vector<std::vector<double>> test_coords = {
      {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 1.0, 0.0}};
  for (int node = 0; node < 4; ++node) {
    if (x3d_reader->get_nodecoord(node) != test_coords[node])
      ITFAILS;
  }

  // >>> CHECK SIDE DATA

  if (x3d_reader->get_numsides() != 4)
    ITFAILS;

  std::vector<std::vector<int>> test_sidenodes = {
      {0, 1}, {1, 3}, {2, 3}, {0, 2}};

  // check each side's data
  for (int side = 0; side < 4; ++side) {

    // sides must always give 2 nodes per face in X3D
    if (x3d_reader->get_sidetype(side) != 2)
      ITFAILS;

    // boundary conditions are not supplied in X3D
    if (x3d_reader->get_sideflag(side) != 0)
      ITFAILS;

    // check node indices
    if (x3d_reader->get_sidenodes(side) != test_sidenodes[side])
      ITFAILS;
  }

  // successful test output
  if (ut.numFails == 0)
    PASSMSG("2D X3D_Draco_Mesh_Reader parsing tests ok.");
  return;
}

// Parse and build an X3D file format and compare to reference mesh
void build_x3d_mesh_2d(rtt_c4::ParallelUnitTest &ut) {

  // use Cartesian geometry
  const Draco_Mesh::Geometry geometry = Draco_Mesh::Geometry::CARTESIAN;

  // >>> CREATE REFERENCE MESH

  // set the number of cells and nodes
  const size_t num_xdir = 1;
  const size_t num_ydir = 1;

  // generate a constainer for data needed in mesh construction
  rtt_mesh_test::Test_Mesh_Interface mesh_iface(num_xdir, num_ydir);

  // short-cut to some arrays
  const std::vector<unsigned> &cell_type = mesh_iface.cell_type;
  const std::vector<unsigned> &cell_to_node_linkage =
      mesh_iface.cell_to_node_linkage;
  const std::vector<unsigned> &side_node_count = mesh_iface.side_node_count;
  const std::vector<unsigned> &side_to_node_linkage =
      mesh_iface.side_to_node_linkage;

  // instantiate the mesh
  std::shared_ptr<Draco_Mesh> ref_mesh(new Draco_Mesh(
      mesh_iface.dim, geometry, cell_type, cell_to_node_linkage,
      mesh_iface.side_set_flag, side_node_count, side_to_node_linkage,
      mesh_iface.coordinates, mesh_iface.global_node_number));

  // >>> PARSE AND BUILD MESH

  const std::string inputpath = ut.getTestSourcePath();
  const std::string filename = inputpath + "x3d.mesh.in";
  const std::vector<std::string> bdy_filenames = {
      inputpath + "x3d.mesh.bdy1.in", inputpath + "x3d.mesh.bdy2.in",
      inputpath + "x3d.mesh.bdy3.in", inputpath + "x3d.mesh.bdy4.in"};

  // construct reader
  std::shared_ptr<X3D_Draco_Mesh_Reader> x3d_reader(
      new X3D_Draco_Mesh_Reader(filename, bdy_filenames));

  // read mesh
  x3d_reader->read_mesh();

  // instantiate a mesh builder and build the mesh
  Draco_Mesh_Builder<X3D_Draco_Mesh_Reader> mesh_builder(x3d_reader);
  std::shared_ptr<Draco_Mesh> mesh = mesh_builder.build_mesh(geometry);

  // check that the scalar data is correct
  if (mesh->get_dimension() != ref_mesh->get_dimension())
    ITFAILS;
  if (mesh->get_geometry() != ref_mesh->get_geometry())
    ITFAILS;
  if (mesh->get_num_cells() != ref_mesh->get_num_cells())
    ITFAILS;
  if (mesh->get_num_nodes() != ref_mesh->get_num_nodes())
    ITFAILS;

  // check that layout is correct (empty for one cell, no side or ghost data)
  if ((mesh->get_cc_linkage()).size() > 0)
    ITFAILS;
  if ((mesh->get_cs_linkage()).size() != 1)
    ITFAILS;
  if ((mesh->get_cg_linkage()).size() > 0)
    ITFAILS;

  // check side flag indices (should be different)
  if (mesh->get_side_set_flag() == ref_mesh->get_side_set_flag())
    ITFAILS;

  // check ghost cell data (should be empty defaults)
  if (mesh->get_ghost_cell_numbers() != ref_mesh->get_ghost_cell_numbers())
    ITFAILS;
  if (mesh->get_ghost_cell_ranks() != ref_mesh->get_ghost_cell_ranks())
    ITFAILS;

  // check that the vector of coordinates match the reference mesh
  if (!rtt_dsxx::soft_equiv(mesh->get_node_coord_vec(),
                            ref_mesh->get_node_coord_vec()))
    ITFAILS;

  // check that each cell has the correct sides
  {
    std::vector<unsigned> test_sn_linkage =
        mesh_iface.flatten_sn_linkage(mesh->get_cs_linkage());

    // check that sn_linkage is a permutation of the original side-node linkage
    std::vector<unsigned>::const_iterator sn_first =
        side_to_node_linkage.begin();
    std::vector<unsigned>::const_iterator test_sn_first =
        test_sn_linkage.begin();
    for (unsigned side = 0; side < mesh_iface.num_sides; ++side) {

      // check that sn_linkage is a permutation of the original side-node linkage
      if (!std::is_permutation(test_sn_first,
                               test_sn_first + side_node_count[side], sn_first,
                               sn_first + side_node_count[side]))
        ITFAILS;

      // update the iterators
      sn_first += side_node_count[side];
      test_sn_first += side_node_count[side];
    }
  }

  // successful test output
  if (ut.numFails == 0)
    PASSMSG("2D X3D_Draco_Mesh_Reader mesh build tests ok.");
  return;
}

//---------------------------------------------------------------------------//

int main(int argc, char *argv[]) {
  rtt_c4::ParallelUnitTest ut(argc, argv, rtt_dsxx::release);
  try {
    Insist(rtt_c4::nodes() == 1, "This test only uses 1 PE.");
    read_x3d_mesh_2d(ut);
    build_x3d_mesh_2d(ut);
  }
  UT_EPILOG(ut);
}

//---------------------------------------------------------------------------//
// end of mesh/test/tstX3D_Draco_Mesh_Reader.cc
//---------------------------------------------------------------------------//
