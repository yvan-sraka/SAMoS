/* *************************************************************
 *  
 *   Active Particles on Curved Spaces (APCS)
 *   
 *   Author: Rastko Sknepnek
 *  
 *   Division of Physics
 *   School of Engineering, Physics and Mathematics
 *   University of Dundee
 *   
 *   (c) 2013, 2014
 *   
 *   This program cannot be used, copied, or modified without
 *   explicit permission of the author.
 * 
 * ************************************************************* */

/*!
 * \file constraint_cylinder.cpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 10-Oct-2014
 * \brief Implementation of the cylindrical constraint
 */ 

#include "constraint_cylinder.hpp"

/*! Force particle to be confined to the surface of a cylinder and
 *  its velocity to be tangent to it. We assume that during the integration 
 *  step particles do not move to much away from the surface and we project 
 *  them down to the surface of the cylinder
 *  \param p Particle which is to be projected onto the cylinder
 */
void ConstraintCylinder::enforce(Particle& p)
{
  bool periodic = m_system->get_periodic();
  BoxPtr box = m_system->get_box();
  double x = p.x, y = p.y;
  double R = sqrt(x*x + y*y);
  double s = m_r/R;
  // Scale back to the surface
  p.x *= s; p.y *= s; 
  // Compute unit normal
  double Nx = p.x/m_r, Ny = p.y/m_r;
  // compute v.N
  double v_dot_N = p.vx*Nx + p.vy*Ny;
  // compute n.N
  double n_dot_N = p.nx*Nx + p.ny*Ny;
  // Project velocity onto tangent plane
  p.vx -= v_dot_N*Nx; p.vy -= v_dot_N*Ny;
  // Project director onto tangent plane
  p.nx -= n_dot_N*Nx; p.ny -= n_dot_N*Ny;
  // normalize director
  double inv_len = 1.0/sqrt(p.nx*p.nx + p.ny*p.ny + p.nz*p.nz);
  p.nx *= inv_len;  p.ny *= inv_len;  p.nz *= inv_len;
  if (periodic)
  {
    if (p.z > box->zhi) p.z -= box->Lz;
    else if (p.z < box->zlo) p.z += box->Lz;
  }
}
