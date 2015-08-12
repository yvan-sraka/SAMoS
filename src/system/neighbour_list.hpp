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
 * \file neighbour_list.hpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 17-Oct-2013
 * \brief Declaration of NeighbourList class.
 */ 

#ifndef __NEIGHBOUR_LIST_HPP__
#define __NEIGHBOUR_LIST_HPP__

#include <vector>
#include <stdexcept>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/ref.hpp>

#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>



#include "messenger.hpp"
#include "system.hpp"
#include "cell_list.hpp"
#include "parse_parameters.hpp"

using std::vector;


/*! Auxiliary structure to keep coordinates of the first state 
 *  after the build so we can check if it is necessary to rebuild.
*/
struct PartPos
{
  //! Constructor
  //! \param x x coordinate of the stored position 
  //! \param y y coordinate of the stored position 
  //! \param z z coordinate of the stored position 
  PartPos(double x, double y, double z) : x(x), y(y), z(z) { }
  //@{
  double x, y, z;  //!< Stores actual position 
  //@}
};

/*! Auxiliary data structure used by the Boost Graph Library
 *  to find all faces
 */ 
struct vertex_visitor : public boost::planar_face_traversal_visitor
{
  void begin_face()  { face.clear();  }
  void next_vertex(int v) {  face.push_back(v);  }
  void end_face() { faces.push_back(face); }
  vector<int>  face;
  vector<vector<int> > faces;
};



/*! This class handles neighbour lists for fast potential and force 
 *  calculations. It is implemented as a vector of vectors (for performance). 
 *  Since the system is on a curved surface
 *  it would be hard to make a general cell list. Therefore, we relay on
 *  the slower but generic N^2 list generation 
*/
class NeighbourList
{
public:
  
  //! Construct NeighbourList object
  //! \param sys Reference to the System object
  //! \param msg Constant reference to the Messenger object
  //! \param cutoff Cutoff distance (should be set to potential cutoff distance + padding distance)
  //! \param pad Padding distance
  NeighbourList(SystemPtr sys, MessengerPtr msg, double cutoff, double pad, pairs_type& param) : m_system(sys), 
                                                                                                 m_msg(msg),
                                                                                                 m_cut(cutoff), 
                                                                                                 m_pad(pad), 
                                                                                                 m_build_contacts(false),
                                                                                                 m_build_faces(false),
                                                                                                 m_contact_dist(0.0)
  {
    m_msg->write_config("nlist.cut",lexical_cast<string>(m_cut));
    m_msg->write_config("nlist.pad",lexical_cast<string>(m_pad));
    // Check if box is large enough for cell list
    if (m_system->get_box()->Lx > 2.0*(cutoff+pad) && m_system->get_box()->Ly > 2.0*(cutoff+pad) && m_system->get_box()->Lz > 2.0*(cutoff+pad))
    {
      m_use_cell_list = true;
      m_cell_list = boost::shared_ptr<CellList>(new CellList(m_system,m_msg,cutoff+pad));
      m_msg->msg(Messenger::INFO,"Using cell lists for neighbour list builds.");
      m_msg->write_config("nlist.build_type","cell");
    }
    else
    {
      m_use_cell_list = false;
      m_msg->msg(Messenger::INFO,"Box dimensions are too small to be able to use cell lists. Neighbour list will be built using N^2 algorithm.");
      m_msg->write_config("nlist.build_type","n_square");
    }
    if (param.find("build_contacts") != param.end())
    {
      m_build_contacts = true;
      m_msg->msg(Messenger::INFO,"Neighbour list will also build contact network.");
      m_msg->write_config("nlist.contact_network","true"); 
    }
    if (param.find("build_faces") != param.end())
    {
      m_build_contacts = true;
      m_build_faces = true;
      m_msg->msg(Messenger::INFO,"Neighbour list will also build faces.");
      m_msg->write_config("nlist.faces","true"); 
    }
    if (param.find("contact_distance") == param.end())
    {
      m_msg->msg(Messenger::WARNING,"Neighbour list. No contact distance set. Assuming default sum or particle radii.");
      m_msg->write_config("nlist.contact_distance","0.0"); 
    }
    else
    {
      m_msg->msg(Messenger::INFO,"Neighbour list. No contact distance set. Setting contact distance to "+param["contact_distance"]+".");
      m_msg->write_config("nlist.contact_distance",param["contact_distance"]); 
      m_contact_dist = lexical_cast<double>(param["contact_distance"]);
    }
    
    this->build();
  }
  
  //! Destructor
  ~NeighbourList()
  {
    for(unsigned int i = 0; i < m_list.size(); i++)
      m_list[i].clear();
    m_list.clear();
    m_old_state.clear();
  }
  
  //! Check is neighbour list of the given particle needs update
  bool need_update(Particle&);
  
  //! Returns true is faces list exists
  bool has_faces() { return m_build_faces; }
  
  //! Get neighbour list for a give particle
  //! \param id Particle id
  //! \return Reference to the particle's neighbour list
  vector<int>& get_neighbours(int id) { return m_list[id]; }
  
  //! Get faces
  vector<vector<int> >& get_faces() { return m_faces.faces; }
  
  //! Get neighbour list cutoff distance
  double get_cutoff() { return m_cut;  }  //!< \return neighbour list cutoff distance
  
  //! Rescales neigbour list cutoff
  //! \param scale scale factor
  void rescale_cutoff(double scale)
  {
    m_cut *= scale;
    if (m_use_cell_list && m_system->get_box()->Lx > 2.0*(m_cut+m_pad) && m_system->get_box()->Ly > 2.0*(m_cut+m_pad) && m_system->get_box()->Lz > 2.0*(m_cut+m_pad))
    {
      m_cell_list = boost::shared_ptr<CellList>(new CellList(m_system,m_msg,m_cut+m_pad));
      m_msg->msg(Messenger::INFO,"Rescaling neighbour list cutoff.");
      m_msg->msg(Messenger::INFO,"Still using cell lists for neighbour list builds.");
    }
    else
    {
      m_use_cell_list = false;
      m_msg->msg(Messenger::INFO,"Rescaling neighbour list cutoff.");
      m_msg->msg(Messenger::INFO,"No longer possible to use cell lists for neighbour list builds.");
    }
    this->build();
  }
  
  
  //! Build neighbour list
  void build();
  
  //! Build faces
  bool build_faces();
  
private:
  
  SystemPtr m_system;              //!< Pointer to the System object
  MessengerPtr m_msg;              //!< Handles messages sent to output
  CellListPtr m_cell_list;         //!< Pointer to the cell list
  vector<vector<int> >  m_list;    //!< Holds the list for each particle 
  vector<PartPos> m_old_state;     //!< Coordinates of particles right after the build
  double m_cut;                    //!< List build cutoff distance 
  double m_pad;                    //!< Padding distance (m_cut should be set to potential cutoff + m_pad)
  bool m_use_cell_list;            //!< If true, use cell list to speed up neighbour list builds
  bool m_build_contacts;           //!< If true, build list of contacts
  bool m_build_faces;              //!< It true, build list of faces based on contact network
  double m_contact_dist;           //!< Distance over which to assume particles to be in contact 
  vector<vector<int> >  m_contact_list;    //!< Holds the contact list for each particle
  vertex_visitor  m_faces;         //!< List of all faces computed from the contact network
  
  // Actual neighbour list builds
  void build_nsq();    //!< Build with N^2 algorithm
  void build_cell();   //!< Build using cells list
  
  // Does contact list build
  void build_contacts();
    
};

typedef shared_ptr<NeighbourList> NeighbourListPtr;

#endif