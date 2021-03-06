
#include <math.h> // sqrt
#include <algorithm> // std::max
#include <cmath>        // std::abs
#include <string>
#include <utility> // std::move
#include "dyna_cpp/dyna/D3plot.hpp"
#include "dyna_cpp/db/Element.hpp"
#include "dyna_cpp/db/Node.hpp"
#include "dyna_cpp/db/DB_Nodes.hpp"
#include "dyna_cpp/db/DB_Elements.hpp"
#include "dyna_cpp/utility/TextUtility.hpp"
#include "dyna_cpp/utility/MathUtility.hpp"

using namespace std;

/*
 * Constructor.
 */
Element::Element(const int _elementID, const ElementType _elementType, const vector<size_t>& _node_indexes, DB_Elements* _db_elements) 
              : is_rigid( false ),
                elementID( _elementID ),
                elemType( _elementType ),
                db_elements( _db_elements ),
                nodes( _node_indexes ){

  // Checks
  if (_db_elements == nullptr)
    throw(string("DB_Elements of an element may not be nullptr in constructor."));

  this->check();

}


/*
 * Destructor.
 */
Element::~Element(){

}


/*
 * Comparator.
 */
bool Element::operator<(const Element &other) const
{
  return(this->elementID < other.elementID);
}


/** Set whether the element is rigid
 * 
 * @param _is_rigid rigid status of the element
 */
void Element::set_is_rigid(bool _is_rigid){
  this->is_rigid = _is_rigid;
}

/** Get whether the element is rigid or not
 *
 * @return is_rigid rigid status of the element
 */
bool Element::get_is_rigid() const {
  return this->is_rigid;
}


/** Get the element type of the element.
 *
 * @return ElementType elementType : NONE, BEAM, SHELL or SOLID
 *
 * NONE = 0
 * BEAM = 1
 * SHELL = 2
 * SOLID = 3
 */
ElementType Element::get_elementType() const {
  return this->elemType;
}


/** Get the elementID.
 * @return int elementID
 */
int Element::get_elementID() const {
  return this->elementID;
}


/** Get the nodes of the element in a set.
 *
 */
vector<Node*> Element::get_nodes() const {

  DB_Nodes* db_nodes = this->db_elements->get_db_nodes();
  vector<Node*> node_vec;

  for(vector<size_t>::const_iterator it=this->nodes.begin(); it != this->nodes.end(); it++){

    Node* _node = db_nodes->get_nodeByIndex(*it);
    if(_node != nullptr){
      node_vec.push_back(_node);
    } else{
      throw(string("Node with index:")+to_string(*it)+string(" in Element:")+to_string(this->elementID)+string(" was not found in DB."));
    }

  }

  return node_vec;
}


/** Return the ids of the elements nodes
 *
 * @return vector<int> node_ids
 */
vector<int> Element::get_node_ids() const {
   
  vector<int> node_ids;
  DB_Nodes* db_nodes = db_elements->get_db_nodes();
  for( size_t iNode=0; iNode<this->nodes.size(); ++iNode ){
    node_ids.push_back( db_nodes->get_id_from_index<int>(nodes[iNode]) );
  }
  return node_ids;

}


/** Return the ids of the elements nodes
 *
 * @return vector<size_t> node_indexes
 */
vector<size_t> Element::get_node_indexes() const {
   return this->nodes;
}


/** Append a value to the series of plastic strain
 *
 */
void Element::add_plastic_strain(float _platic_strain){

  /*
  if(_platic_strain < 0)
    throw("Element:"+to_string(this->elementID)+" tries to add a negative plastic strain:"+to_string(_platic_strain));
  */

  this->plastic_strain.push_back(_platic_strain);
}


/** Append a value to the series of internal energy.
 *
 */
void Element::add_energy(float _energy){

//  if(_energy < 0)
//    throw("Element:"+to_string(this->elementID)+" tries to add a negative energy:"+to_string(_energy));

  this->energy.push_back(_energy);
}


/*
 * Append a value to the series of strain.
 */
void Element::add_strain(vector<float> _strain){

  if(_strain.size() < 1)
    throw("Element:"+to_string(this->elementID)+" tries to add strain vector of length:"+to_string(_strain.size())+"!=6");

  this->strain.push_back(_strain);
}


/** Append a value to the series of stress.
 * 
 */
void Element::add_stress(vector<float> _stress){

  if(_stress.size() != 6)
    throw("Element:"+to_string(this->elementID)+" tries to add stress vector of length:"+to_string(_stress.size())+"!=6");

  this->stress.push_back(_stress);
}


/** Append a value to the series of mises stress
 *
 */
void Element::add_stress_mises(float _stress_mises){
  this->stress_mises.push_back(_stress_mises);
}


/*
 * Append history vars to the time seris.
 */
void Element::add_history_vars(vector<float> vars,size_t iTimestep){

   if(iTimestep < this->history_vars.size()){
      for(size_t ii=0; ii<vars.size(); ++ii){
         this->history_vars[iTimestep].push_back(vars[ii]);
      }
   } else {
      this->history_vars.push_back(vars);
   }

}


/**
 * Get the series of plastic strain. The
 * plastic strain here is accurately the
 * efficient plastic strain and thus a
 * single value, not a tensor.
 */
vector<float> Element::get_plastic_strain() const {

  return this->plastic_strain;
}


/*
 * Get the series of the internal energy.
 */
vector<float> Element::get_energy() const {

  return this->energy;
}


/*
 * Get the coordinates of the element, which is
 * the average of all nodes.
 */
vector<float> Element::get_coords(int iTimestep) const {

   if(this->nodes.size() < 1)
      throw(string("Element with id ")+to_string(this->elementID)+string(" has no nodes and thus no coords."));

   if(this->db_elements->get_femfile()->is_d3plot()){
      if( (iTimestep != 0) && (!this->db_elements->get_femfile()->get_d3plot()->displacement_is_read()) )
         throw(string("Displacements were not read yet. Please use read_states=\"disp\"."));
   } else if(this->db_elements->get_femfile()->is_keyFile()) {
      if( iTimestep != 0 )
         throw(string("Since a KeyFile has no states, you can not use the iTimeStep argument in element.get_coords."));
   } else {
      throw(string("FEMFile is neither a d3plot, nor a keyfile in element.get_coords"));
   }

   if( iTimestep < 0 )
      iTimestep = static_cast<int>(this->db_elements->get_femfile()->get_d3plot()->get_timesteps().size()) + iTimestep; // Python array style

   if( (iTimestep < 0) )
      throw(string("Specified timestep exceeds real time step size."));

   DB_Nodes* db_nodes = this->db_elements->get_db_nodes();

   Node* current_node = nullptr;
   vector<float> coords_elem(3, 0.);
   vector<float> coords_node;
   vector< vector<float> > disp_node;

   for( const auto& node_index : this->nodes ){

      current_node = db_nodes->get_nodeByIndex(node_index);
      coords_node = current_node->get_coords();

      coords_elem[0] += coords_node[0];
      coords_elem[1] += coords_node[1];
      coords_elem[2] += coords_node[2];

      // Coords at different timestep
      if(iTimestep != 0){

         disp_node = current_node->get_disp();

         // Check correctness
         if( iTimestep >= static_cast<long>(disp_node.size()) )
            throw(string("Specified timestep exceeds real time step size."));

         coords_elem[0] += disp_node[iTimestep][0];
         coords_elem[1] += disp_node[iTimestep][1];
         coords_elem[2] += disp_node[iTimestep][2];
      }
  }

  float _nodes_size = (float) this->nodes.size(); 
  coords_elem[0] /= _nodes_size;
  coords_elem[1] /= _nodes_size;
  coords_elem[2] /= _nodes_size;

  return std::move(coords_elem);
}

/*
 * Get an estimate for the average element length. This takes the
 * maximum distance (diagonal) from the first node and multiplies
 * it with a volume factor (beam=1,shell=sqrt(2),solid=sqrt(3))
 */
float Element::get_estimated_element_size() const {

   if(this->nodes.size() < 1)
      throw("Element with id "+to_string(this->elementID)+" has no nodes and thus no size.");

   DB_Nodes* db_nodes = this->db_elements->get_db_nodes();

   float maxdist = -1.;
   vector<float> ncoords;
   vector<float> basis_coords;
   for(vector<size_t>::const_iterator it=this->nodes.begin(); it != this->nodes.end(); ++it){
      ncoords = db_nodes->get_nodeByID(*it)->get_coords();
      if(it != this->nodes.begin()){
         ncoords = MathUtility::v_subtr(ncoords,basis_coords);
         ncoords[0] *= ncoords[0];
         ncoords[1] *= ncoords[1];
         ncoords[2] *= ncoords[2];

         maxdist = max(maxdist,ncoords[0]+ncoords[1]+ncoords[2]);
      } else {
         basis_coords=ncoords;
      }
   }

   if(this->elemType == SHELL){
      if(this->nodes.size() == 3){
         return sqrt(maxdist); // tria
      } else if(this->nodes.size() ==  4){
         return sqrt(maxdist)/1.41421356237f; // quad
      } else {
         throw("Unknown node number:"+to_string(this->nodes.size())+" of element +"+to_string(this->elementID)+"+ for shells.");
      }
   }
   if(this->elemType == SOLID){
      if(this->nodes.size() == 4){
         return sqrt(maxdist); // tria
      } else if(this->nodes.size() == 8){
         return sqrt(maxdist)/1.73205080757f; // hexa
      } else if(this->nodes.size() ==  5){
         return sqrt(maxdist);  // pyramid ... difficult to handle
      } else if(this->nodes.size() ==  6){
         return sqrt(maxdist)/1.41421356237f; // penta
      } else {
         throw("Unknown node number:"+to_string(this->nodes.size())+" of element +"+to_string(this->elementID)+"+ for solids.");
      }
   }
   if(this->elemType == BEAM){
      if(this->nodes.size() != 2)
         throw("Unknown node number:"+to_string(this->nodes.size())+" of element +"+to_string(this->elementID)+"+ for beams.");
      return sqrt(maxdist); // beam
   }
   throw(string("Unknown element type, expected BEAM/SHELL/SOLID."));

}

/*
 * Get the series of strain. Strain is a vector
 * of 6 entries which represent the strain
 * tensor data.
 * e = [e_xx,e_yy,e_zz,e_xy,e_yz,e_xz]
 */
vector< vector<float> > Element::get_strain() const {

  return this->strain;
}


/*
 * Get the series of stress. Stress is a vector
 * of 6 entries which represent the stress
 * tensor data.
 * s = [s_xx,s_yy,s_zz,s_xy,s_yz,s_xz]
 */
vector< vector<float> > Element::get_stress() const {

  return this->stress;
}

/** Get the mises stress over time
 *
 */
vector<float> Element::get_stress_mises() const {
  return this->stress_mises;
}

/*
 * Get the series of history variables.
 */
vector< vector<float> > Element::get_history_vars() const{

  return this->history_vars;
}

/*
 * Check of the element type is correct regarding the node size.
 */
void Element::check() const{

   if(this->elemType == SHELL){
      if( (this->nodes.size() < 3) | (this->nodes.size() > 4))
         throw("A shell element must have 3 or 4 nodes. You have "+to_string(this->nodes.size()));
      return;
   }

   if(this->elemType == SOLID){
      if( (this->nodes.size() < 4) | (this->nodes.size() > 8) | (this->nodes.size() == 7))
         throw("A solid element must have 4,5,6 or 8 nodes. You have "+to_string(this->nodes.size()));
      return;
   }
   if(this->elemType == BEAM){
      if(this->nodes.size() != 2)
         throw("A beam element must have exactly 2 nodes. You have "+to_string(this->nodes.size()));
   }

}


/** Clear the elements energy data
 */
void Element::clear_energy(){
  this->energy.clear();
}


/** Clear the elements plastic strain
 */
void Element::clear_plastic_strain(){
  this->plastic_strain.clear();
}


/** Clear the elements stress
 */
void Element::clear_stress(){
  this->stress.clear();
}


/** Clear the elements mises stress
 */
void Element::clear_stress_mises(){
  this->stress_mises.clear();
}


/** Clear the elements strain
 */
void Element::clear_strain(){
  this->strain.clear();
}


/** Clear the elements history data
 */
void Element::clear_history_vars(){
  this->history_vars.clear();
}