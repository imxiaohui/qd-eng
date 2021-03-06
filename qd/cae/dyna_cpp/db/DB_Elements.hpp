
#ifndef DB_ELEMENTS_HPP
#define DB_ELEMENTS_HPP

// forward declarations
//class Element;
class DB_Nodes;
class DB_Parts;
class FEMFile;

// includes
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Element.hpp"
#include "dyna_cpp/utility/TextUtility.hpp"

class DB_Elements {

private:
  FEMFile* femfile;
  DB_Nodes* db_nodes;
  DB_Parts* db_parts;

  std::unordered_map<int,size_t> id2index_elements2;
  std::unordered_map<int,size_t> id2index_elements4;
  std::unordered_map<int,size_t> id2index_elements8;
  std::vector< std::unique_ptr<Element> > elements2;
  std::vector< std::unique_ptr<Element> > elements4;
  std::vector< std::unique_ptr<Element> > elements8;

public:
  DB_Elements(FEMFile* _femfile);
  ~DB_Elements();
  FEMFile* get_femfile();
  DB_Nodes* get_db_nodes();
  Element* add_element_byD3plot(const ElementType _eType, 
                                const int _id, 
                                const std::vector<int>& _elem_data);
  Element* add_element_byKeyFile(ElementType _eType, 
                                 int _id, 
                                 int _partid, 
                                 std::vector<int> _node_ids);

  size_t size(ElementType _type = NONE);
  void reserve(const ElementType _type, const size_t _size);
  template <typename T>
  Element* get_elementByID(ElementType _eType, T _id);
  template <typename T>
  Element* get_elementByIndex(ElementType _eType, T _index);

};


/** Get the element by it's internal id and it's type.
 *
 * @param int _elementType : see description
 * @param int _elementID : id of the element (like in the solver)
 * @return Element* _element
 *
 * Type may be: Element.ElementType
 * NONE = 0 -> error
 * BEAM = 1
 * SHELL = 2
 * SOLID = 3
 */
template <typename T>
Element* DB_Elements::get_elementByID(ElementType _elementType,T _elementID){

  static_assert(std::is_integral<T>::value, "Integer number required.");

  if(_elementType == BEAM){
    const auto& it = this->id2index_elements2.find(_elementID);
    if(it == id2index_elements2.end())
      return nullptr;
    return elements2[it->second].get();

  } else if(_elementType == SHELL){
    const auto& it = this->id2index_elements4.find(_elementID);
    if(it == id2index_elements4.end())
      return nullptr;
    return elements4[it->second].get();

  } else if(_elementType == SOLID){
    const auto& it = this->id2index_elements8.find(_elementID);
    if(it == id2index_elements8.end())
      return nullptr;
    return elements8[it->second].get();

  }

  throw(std::string("Can not get element with elementType:")+to_string(_elementID));

}

/** Get the element by it's internal index and it's type.
 *
 * @param int _elementType : see description
 * @param int _elementIndex : index of the element (not id!)
 * @return Element* _element
 *
 * Type may be: Element.ElementType
 * NONE = 0 -> error
 * BEAM = 1
 * SHELL = 2
 * SOLID = 3
 */
template <typename T>
//typename std::enable_if<std::is_integral<T>::value>::type
Element* DB_Elements::get_elementByIndex(ElementType _elementType,T _elementIndex){

  static_assert(std::is_integral<T>::value, "Integer number required.");

   if(_elementType == BEAM){
     if(_elementIndex < elements2.size())
       return elements2[_elementIndex].get();
     return nullptr;

   } else if(_elementType == SHELL){
     if(_elementIndex < elements4.size())
       return elements4[_elementIndex].get();
     return nullptr;

   } else if(_elementType == SOLID){
     if(_elementIndex < elements8.size())
       return elements8[_elementIndex].get();
     return nullptr;

   }

  throw(std::string("Can not get element with elementType:")+to_string(_elementIndex));

}



#endif
