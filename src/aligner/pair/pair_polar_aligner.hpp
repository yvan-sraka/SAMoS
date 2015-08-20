/* *************************************************************
 *  
 *   Soft Active Mater on Surfaces (SAMoS)
 *   
 *   Author: Rastko Sknepnek
 *  
 *   Division of Physics
 *   School of Engineering, Physics and Mathematics
 *   University of Dundee
 *   
 *   (c) 2013, 2014
 * 
 *   School of Science and Engineering
 *   School of Life Sciences 
 *   University of Dundee
 * 
 *   (c) 2015
 * 
 *   Author: Silke Henkes
 * 
 *   Department of Physics 
 *   Institute for Complex Systems and Mathematical Biology
 *   University of Aberdeen  
 * 
 *   (c) 2014, 2015
 *  
 *   This program cannot be used, copied, or modified without
 *   explicit written permission of the authors.
 * 
 * ************************************************************* */

/*!
 * \file pair_polar_aligner.hpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 17-Jan-2014
 * \brief Declaration of PairPolarAlign class
 */ 

#ifndef __PAIR_POLAR_ALIGN_HPP__
#define __PAIR_POLAR_ALIGN_HPP__

#include <cmath>
#include <vector>

#include "pair_aligner.hpp"

using std::make_pair;
using std::sqrt;
using std::vector;

//! Structure that handles parameters for the polar alignment 
struct PolarAlignParameters
{
  double J;
  double rcut;
};


/*! PairPolarAlign implements the "polar" type alignment between neighbouring particles.
 *  For all particles within the cutoff distance \f$ r_{cut} \left(\leq r_{nl}\right) \f$
 *  (\f$ r_{nl} \f$ being neighbour list cutoff distance) we compute torque on the particle as
 *  \f$ \vec \tau_i = 2 J \sum_j \vec n_i\times\vec n_j \f$,
 *  where \f$ J \f$ is the coupling constant and \f$ \vec n_j \f$ is director of j-th neighbour. 
 */
class PairPolarAlign : public PairAlign
{
public:
  
  //! Constructor
  //! \param sys Pointer to the System object
  //! \param msg Pointer to the internal state messenger
  //! \param nlist Pointer to the global neighbour list
  //! \param param Contains information about all parameters (J and cutoff distance)
  PairPolarAlign(SystemPtr sys, MessengerPtr msg, NeighbourListPtr nlist, pairs_type& param) : PairAlign(sys, msg, nlist, param)
  {
    int ntypes = m_system->get_ntypes();
    if (param.find("J") == param.end())
    {
      m_msg->msg(Messenger::WARNING,"No coupling constant (J) specified for polar alignment. Setting it to 1.");
      m_J = 1.0;
    }
    else
    {
      m_msg->msg(Messenger::INFO,"Global coupling constant (J) for polar alignment is set to "+param["J"]+".");
      m_J = lexical_cast<double>(param["J"]);
    }
    m_msg->write_config("aligner.pair.polar.J",lexical_cast<string>(m_J));
    if (param.find("rcut") == param.end())
    {
      m_msg->msg(Messenger::WARNING,"No cutoff distance (rcut) specified for polar alignment. Setting it to the global neighbour list cutoff.");
      m_rcut = m_nlist->get_cutoff();
    }
    else
    {
      m_msg->msg(Messenger::INFO,"Global cutoff distance (rcut) for polar alignment is set to "+param["rcut"]+".");
      m_rcut = lexical_cast<double>(param["rcut"]);
    }
    m_msg->write_config("aligner.pair.polar.rcut",lexical_cast<string>(m_rcut));
    m_pair_params = new PolarAlignParameters*[ntypes];
    for (int i = 0; i < ntypes; i++)
    {
      m_pair_params[i] = new PolarAlignParameters[ntypes];
      for (int j = 0; j < ntypes; j++)
      {
        m_pair_params[i][j].J = m_J;
        m_pair_params[i][j].rcut = m_rcut;
      }
    }
  }
  
  virtual ~PairPolarAlign()
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
      m_msg->msg(Messenger::ERROR,"type_1 has not been defined for pairwise alignment in polar aligner.");
      throw runtime_error("Missing key for pair alignment parameters.");
    }
    if (pair_param.find("type_2") == pair_param.end())
    {
      m_msg->msg(Messenger::ERROR,"type_2 has not been defined for pairwise alignment in polar aligner.");
      throw runtime_error("Missing key for pair potential parameters.");
    }
    type_1 = lexical_cast<int>(pair_param["type_1"]);
    type_2 = lexical_cast<int>(pair_param["type_2"]);
    
    if (pair_param.find("J") != pair_param.end())
    {
      m_msg->msg(Messenger::INFO,"Polar pairwise alignment. Setting coupling constant to "+pair_param["J"]+" for particle pair of types ("+lexical_cast<string>(type_1)+" and "+lexical_cast<string>(type_2)+").");
      param["J"] = lexical_cast<double>(pair_param["J"]);
    }
    else
    {
      m_msg->msg(Messenger::INFO,"Polar pairwise alignment. Using default strength ("+lexical_cast<string>(m_J)+") for particle pair of types ("+lexical_cast<string>(type_1)+" and "+lexical_cast<string>(type_2)+").");
      param["J"] = m_J;
    }
    m_msg->write_config("aligner.pair.polar.types."+pair_param["type_1"]+"_and_"+pair_param["type_2"]+".J",lexical_cast<string>(param["J"]));
    if (pair_param.find("rcut") != pair_param.end())
    {
      m_msg->msg(Messenger::INFO,"Polar pairwise alignment. Setting cutoff distance to "+pair_param["rcut"]+" for particle pair of types ("+lexical_cast<string>(type_1)+" and "+lexical_cast<string>(type_2)+").");
      param["rcut"] = lexical_cast<double>(pair_param["rcut"]);
    }
    else
    {
      m_msg->msg(Messenger::INFO,"Polar pairwise alignment. Using default cutoff distance ("+lexical_cast<string>(m_rcut)+") for particle pair of types ("+lexical_cast<string>(type_1)+" and "+lexical_cast<string>(type_2)+").");
      param["rcut"] = m_rcut;
    } 
    m_msg->write_config("aligner.pair.polar.types."+pair_param["type_1"]+"_and_"+pair_param["type_2"]+".rcut",lexical_cast<string>(param["rcut"]));
    
    m_pair_params[type_1-1][type_2-1].J = param["J"];
    if (type_1 != type_2)
      m_pair_params[type_2-1][type_1-1].J = param["J"];
    m_pair_params[type_1-1][type_2-1].rcut = param["rcut"];
    if (type_1 != type_2)
      m_pair_params[type_2-1][type_1-1].rcut = param["rcut"];
    
    m_has_pair_params = true;
  }
  
  //! Returns true since polar alignment needs neighbour list
  bool need_nlist() { return true; }
  
  //! Computes "torques"
  void compute();
  
  
private:
       
  double m_J;                             //!< Coupling constant
  double m_rcut;                          //!< Cutoff distance (has to be less than neighbour list cutoff)
  PolarAlignParameters** m_pair_params;   //!< type specific pair parameters 
     
};

typedef shared_ptr<PairPolarAlign> PairPolarAlignPtr;

#endif
