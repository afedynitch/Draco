//----------------------------------*-C++-*-----------------------------------//
/*!
 * \file   mesh_element/Geometry.hh
 * \author Kent Budge
 * \date   Tue Dec 21 14:28:56 2004
 * \brief
 * \note   Copyright (C) 2016-2020 Triad National Security, LLC.
 *         All rights reserved. */
//----------------------------------------------------------------------------//

#ifndef mesh_element_Geometry_hh
#define mesh_element_Geometry_hh

namespace rtt_mesh_element {

/*! Enumerates supported geometries.
 *
 * The order of enumerated values is not arbiterary. The corresponding integral
 * value is the number of suppressed dimensions in the geometry, e.g., axisymmetric
 * geometry looks 2-D but is actually 3-D (one suppressed dimension) while spherical
 * geometry looks 1-D but is actually 3-D (two suppressed dimensions.) The number
 * of suppressed dimensions is used in some formulas in a number of hydrodynamics
 * codes, so it seems like a good idea for us to adopt this convention as well.
 *
 * We specify the base as int to guarantee better interoperability with FORTRAN codes.
 */
enum Geometry : int {
  CARTESIAN,    //!< 1D (slab) or 2D (XY) Cartesian geometry
  AXISYMMETRIC, //!< 2D (cylindrical) R-Z
  SPHERICAL,    //!< 1D SPHERICAL
  END_GEOMETRY  //!< Sentinel value
};

} // end namespace rtt_mesh_element

#endif // mesh_element_Geometry_hh

//----------------------------------------------------------------------------//
// end of mesh_elementGeometry.hh
//----------------------------------------------------------------------------//
