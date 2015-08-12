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
 * \file log_pair_align.hpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 03-Mar-2014
 * \brief Declaration of LogPairAlign class
 */ 

#ifndef __LOG_PAIR_ALIGN_H__
#define __LOG_PAIR_ALIGN_H__


#include "log.hpp"

//! LogPairAlign class
/*! Logs different types of pair energies
 */
class LogPairAlign : public Log
{
public:
  
  //! Construct Log object
  //! \param sys pointer to a system object
  //! \param compute pointer to a compute object
  //! \param pot Interaction handler object
  //! \param align Alignment handler object
  //! \param type Type of the pair alignment to log (should match the pair_align type)
  LogPairAlign(SystemPtr sys, MessengerPtr msg, PotentialPtr pot, AlignerPtr align, const string& type) : Log(sys, msg, pot, align), m_type(type)  { }
  
  virtual ~LogPairAlign() { }
  
  //! \return current time step
  string operator()()
  {
    return str(format("%12.6e ") % m_aligner->compute_pair_alignment_energy_of_type(m_type));
  }
  
private:
  
  string m_type;    //!< Pair energy type to log
  
};

typedef shared_ptr<LogPairAlign> LogPairAlignPtr;

#endif