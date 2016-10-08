/* ***************************************************************************
 *
 *  Copyright (C) 2013-2016 University of Dundee
 *  All rights reserved. 
 *
 *  This file is part of SAMoS (Soft Active Matter on Surfaces) program.
 *
 *  SAMoS is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  SAMoS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ****************************************************************************/

/*!
 * \file constraint_sphere.hpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 24-Oct-2013
 * \brief Declaration of ConstraintSphere class.
 */ 

#ifndef __CONSTRAINT_SPHERE_HPP__
#define __CONSTRAINT_SPHERE_HPP__

#include <cmath>

#include "system.hpp"
#include "parse_parameters.hpp"
#include "constraint.hpp"


using std::sqrt;
using std::sin;
using std::cos;

/*! Enforces all particles to be on the surface of a sphere of radius 
 *  R centred at the origin. All velocities will point in the tangent
 *  direction.
*/
class ConstraintSphere : public Constraint
{
public:
  
  //! Constructor
  //! \param id unique constraint id
  //! \param sys pointer to the system object
  //! \param msg Pointer to the internal state messenger
  //! \param param parameters that define the manifolds (e.g., sphere radius)
  ConstraintSphere(SystemPtr sys, MessengerPtr msg, pairs_type& param) : Constraint(sys,msg,param)
  { 
    if (param.find("r") == param.end())
    {
      m_msg->msg(Messenger::WARNING,"Spherical constraint. No radius set. Assuming 1.");
      m_r = 1.0;
    }
    else
    {
      m_msg->msg(Messenger::INFO,"Spherical constraint. Radius set to "+param["r"]+".");
      m_r = lexical_cast<double>(param["r"]);
    }
    m_msg->write_config("constraint.sphere.r",lexical_cast<string>(m_r));
  }
  
  //! Enforce constraint
  void enforce(Particle&);
  
  //! Computes normal to the surface
  void compute_normal(Particle& p, double& Nx, double& Ny, double& Nz) { Nx = p.x/m_r; Ny = p.y/m_r; Nz = p.z/m_r; p.Nx = Nx; p.Ny = Ny; p.Nz = Nz; }
  
  // Computer gradient at a point
  void compute_gradient(Particle& p, double& gx, double& gy, double& gz) { }
  
  // Value of the constraint
  double constraint_value(Particle& p) { return 0.0; }
  
  // Rescale constraint
  bool rescale();
      
private:
  
  double m_r;     //!< Radius of the confining sphere
  
};

typedef shared_ptr<ConstraintSphere> ConstraintSpherePtr;  //!< Shared pointer to the Constraint object

#endif
