
#ifndef DB_PARTS_HPP
#define DB_PARTS_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// forward declarations
class Part;
class FEMFile;

class DB_Parts {

private:
  FEMFile* femfile;
  std::vector< std::unique_ptr<Part> > parts;
  std::unordered_map<int,size_t> id2index;

public:
  DB_Parts(FEMFile* _femfile);
  ~DB_Parts();

  size_t size() const;
  void print_parts() const;
  std::vector<Part*> get_parts();
  Part* get_part_byName(const std::string&);
  template<typename T>
  Part* get_part_byID(T _id);
  template<typename T>
  Part* get_part_byIndex(T _index);
  Part* add_part_byID(int _partID, const std::string& name="");

};


template<typename T>
Part* DB_Parts::get_part_byID(T _id){

  static_assert(std::is_integral<T>::value, "Integer number required.");

  const auto& it = this->id2index.find(_id);
  if(it != id2index.end()){
    return parts[it->second].get();
  } else {
    // :(
    return nullptr;
  }

}


template<typename T>
Part* DB_Parts::get_part_byIndex(T _index){

  static_assert(std::is_integral<T>::value, "Integer number required.");

  // Part existing
  if( _index < parts.size() ){
    return parts[_index].get();
  } else {
    // :(
    return nullptr;
  }

}
#endif
