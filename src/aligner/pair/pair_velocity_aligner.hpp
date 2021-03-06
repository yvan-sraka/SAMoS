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
 * \file pair_velocity_aligner.hpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 05-Mar-2014
 * \brief Declaration of PairVelocityAlign class
 */ 

#ifndef __PAIR_VELOCITY_ALIGN_HPP__
#define __PAIR_VELOCITY_ALIGN_HPP__

#include <cmath>
#include <vector>

#include "pair_aligner.hpp"

using std::make_pair;
using std::sqrt;
using std::vector;

//! Structure that handles parameters for the velocity alignment 
struct VelocityAlignParameters
{
  double J;
  double rcut;
};

/*! PairVelocityAlign implements the "velocity" type alignment between neighbouring particles.
 *  For all particles within the cutoff distance \f$ r_{cut} \left(\leq r_{nl}\right) \f$
 *  (\f$ r_{nl} \f$ being neighbour list cutoff distance) we compute torque on the particle as
 *  \f$ \vec \tau_i = 2 J \sum_j \left(\vec v_i\cdot\vec v_j\right)\vec v_i\times\vec v_j \f$ 
 *  (for nematic coupling) and as
 *  \f$ \vec \tau_i = 2 J \sum_j \vec v_i\times\vec v_j \f$ (for polar case),
 *  where \f$ J \f$ is the coupling constant and \f$ \vec v_j \f$ is velocity of j-th neighbour. 
 */
class PairVelocityAlign : public PairAlign
{
public:
  
  //! Constructor
  //! \param sys Pointer to the System object
  //! \param msg Pointer to the internal state messenger
  //! \param nlist Pointer to the global neighbour list
  //! \param param Contains information about all parameters (J and cutoff distance)
  PairVelocityAlign(SystemPtr sys, MessengerPtr msg, NeighbourListPtr nlist, pairs_type& param) : PairAlign(sys, msg, nlist, param)
  {
    int ntypes = m_system->get_ntypes();
    if (param.find("J") == param.end())
    {
      m_msg->msg(Messenger::WARNING,"No coupling constant (J) specified for velocity alignment. Setting it to 1.");
      m_J = 1.0;
    }
    else
    {
      m_msg->msg(Messenger::INFO,"Global coupling constant (J) for velocity alignment is set to "+param["J"]+".");
      m_J = lexical_cast<double>(param["J"]);
    }
    m_msg->write_config("aligner.pair.velocity.J",lexical_cast<string>(m_J));

    if (param.find("rcut") == param.end())
    {
      m_msg->msg(Messenger::WARNING,"No cutoff distance (rcut) specified for velocity alignment. Setting it to the global neighbour list cutoff.");
      m_rcut = m_nlist->get_cutoff();
    }
    else
    {
      m_msg->msg(Messenger::INFO,"Global cutoff distance (rcut) for velocity alignment is set to "+param["rcut"]+".");
      m_rcut = lexical_cast<double>(param["rcut"]);
    }
    m_msg->write_config("aligner.pair.velocity.rcut",lexical_cast<string>(m_rcut));

    if (param.find("nematic") == param.end())
    {
      m_msg->msg(Messenger::WARNING,"Velocity alignment. Assuming polar alignment.");
      m_msg->write_config("aligner.pair.velocity.nematic","false");
      m_nematic = false;
    }
    else
    {
      m_msg->msg(Messenger::INFO,"Velocity alignment. Assuming nematic alignment.");
      m_nematic = true;
      m_msg->write_config("aligner.pair.velocity.nematic","true");
    }
    
    m_pair_params = new VelocityAlignParameters*[ntypes];
    for (int i = 0; i < ntypes; i++)
    {
      m_pair_params[i] = new VelocityAlignParameters[ntypes];
      for (int j = 0; j < ntypes; j++)
      {
        m_pair_params[i][j].J = m_J;
        m_pair_params[i][j].rcut = m_rcut;
      }
    }
  }
  
  virtual ~PairVelocityAlign()
  {
    for (int i = 0; i < m_system->get_ntypes(); i++)
      delete [] m_pair_params[i];
    delete [] m_pair_params;
  }
                                                                                                                
  //! Set pair parameters data for pairwise alignment    
  void set_pair_parameters(pairs_type& pair_param)
  {
    map<string,double> param;
    int type_1, type_2;
    
    if (pair_param.find("type_1") == pair_param.end())
    {
      m_msg->msg(Messenger::ERROR,"type_1 has not been defined for pairwise alignment in velocity aligner.");
      throw runtime_error("Missing key for pair alignment parameters.");
    }
    if (pair_param.find("type_2") == pair_param.end())
    {
      m_msg->msg(Messenger::ERROR,"type_2 has not been defined for pairwise alignment in velocity aligner.");
      throw runtime_error("Missing key for pair potential parameters.");
    }
    type_1 = lexical_cast<int>(pair_param["type_1"]);
    type_2 = lexical_cast<int>(pair_param["type_2"]);
    
    if (pair_param.find("J") != pair_param.end())
    {
      m_msg->msg(Messenger::INFO,"velocity pairwise alignment. Setting coupling constant to "+pair_param["J"]+" for particle pair of types ("+lexical_cast<string>(type_1)+" and "+lexical_cast<string>(type_2)+").");
      param["J"] = lexical_cast<double>(pair_param["J"]);
    }
    else
    {
      m_msg->msg(Messenger::INFO,"velocity pairwise alignment. Using default strength ("+lexical_cast<string>(m_J)+") for particle pair of types ("+lexical_cast<string>(type_1)+" and "+lexical_cast<string>(type_2)+").");
      param["J"] = m_J;
    }
    m_msg->write_config("aligner.pair.velocity.types."+pair_param["type_1"]+"_and_"+pair_param["type_2"]+".J",lexical_cast<string>(param["J"]));
    
    if (pair_param.find("rcut") != pair_param.end())
    {
      m_msg->msg(Messenger::INFO,"velocity pairwise alignment. Setting cutoff distance to "+pair_param["rcut"]+" for particle pair of types ("+lexical_cast<string>(type_1)+" and "+lexical_cast<string>(type_2)+").");
      param["rcut"] = lexical_cast<double>(pair_param["rcut"]);
    }
    else
    {
      m_msg->msg(Messenger::INFO,"velocity pairwise alignment. Using default cutoff distance ("+lexical_cast<string>(m_rcut)+") for particle pair of types ("+lexical_cast<string>(type_1)+" and "+lexical_cast<string>(type_2)+").");
      param["rcut"] = m_rcut;
    }    
    m_msg->write_config("aligner.pair.velocity.types."+pair_param["type_1"]+"_and_"+pair_param["type_2"]+".rcut",lexical_cast<string>(param["rcut"]));
        
    m_pair_params[type_1-1][type_2-1].J = param["J"];
    if (type_1 != type_2)
      m_pair_params[type_2-1][type_1-1].J = param["J"];
    m_pair_params[type_1-1][type_2-1].rcut = param["rcut"];
    if (type_1 != type_2)
      m_pair_params[type_2-1][type_1-1].rcut = param["rcut"];
    
    m_has_pair_params = true;
  }
  
  //! Returns true since velocity alignment needs neighbour list
  bool need_nlist() { return true; }
  
  //! Computes "torques"
  void compute();
  
  
private:
       
  double m_J;       //!< Coupling constant
  double m_rcut;    //!< Cutoff distance (has to be less than neighbour list cutoff)
  bool m_nematic;   //!< If true apply nematic ordering, otherwise, use polar alignment
  VelocityAlignParameters** m_pair_params;   //!< type specific pair parameters 
     
};

typedef shared_ptr<PairVelocityAlign> PairVelocityAlignPtr;

#endif
