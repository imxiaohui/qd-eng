
extern "C"
{
#include "dyna_cpp/dyna/femzip.h"
#include <stdio.h>
}
#include "dyna_cpp/dyna/FemzipBuffer.hpp"
#include "dyna_cpp/utility/FileUtility.hpp"
#include <iostream>
#include <bitset>
#include <sstream>

using namespace std;

/*
 * Constructor
 */
FemzipBuffer::FemzipBuffer(string _filepath)
   : AbstractBuffer(4) {

  // Init vars
  this->filepath = _filepath;
  if(!FileUtility::check_ExistanceAndAccess(this->filepath)){
    throw("File \"" + this->filepath + "\" does not exist or is locked.");
  }
  this->init_vars();

  // version check
  float unzipversion = 0.;
	float fileunzipversion = 0.;

  femunziplib_version(&unzipversion);
  femunziplib_version_file((char*) this->filepath.c_str(), &this->filetype, &fileunzipversion,&this->ier);
  this->check_ier("Femzip Error during femunziplib_version_file.");

  if(unzipversion<fileunzipversion){
    throw("Femzip version older than file version.");
  }

	/* SIZES */
	get_Size((char*) this->filepath.c_str(), this->filetype, this->adjust, &this->size_geo,
		&this->size_state, &this->size_disp, &this->size_activity,
		&this->size_post, &this->ier);
	this->check_ier("Femzip Error during reading of sizes.");

}


/*
 * Destructor
 */
FemzipBuffer::~FemzipBuffer(){

}

/*
 * Initialize the variables for the class.
 */
void FemzipBuffer::init_vars(){

  //this->wordSize = 4; // byte

  // general
  this->filetype = 1;
  this->ier = 0;
  this->pos = 0;
  // Sizing
  this->size_geo = 0;
  this->size_state = 0;
  this->size_disp = 0;
  this->size_activity = 0;
  this->size_post = 0;
  this->size_titles = 0;
  // States
  this->iTimeStep = 1; // ... why
  this->nTimeStep = 0;
  this->size_times = 0;
  // config
  this->adjust = 5;

}

/*
 * Read the geometry buffer.
 */
void FemzipBuffer::read_geometryBuffer(){

  string nonsense_s = "nonsense";
  char* nonsense = (char*) nonsense_s.c_str();
  char* argv[] = { nonsense, (char*) this->filepath.c_str() };
  int p1[1000];
  int p2[1000];
  int l1 = 0;
  int l2 = 0;
  wrapinput(2, argv, p1, p2, &l1, &l2);

  this->current_buffer.reserve(sizeof(int)*this->size_geo);
  geometry_read(p1, p2, &l1, &l2, &this->ier, &this->pos, (int*) &this->current_buffer[0], &this->size_geo);
  this->check_ier("Femzip Error while reading geometry.");

}


/*
 * Free the geometry buffer.
 */
void FemzipBuffer::free_geometryBuffer(){

}


/*
 * Read the part buffer.
 */
void FemzipBuffer::read_partBuffer(){

  this->pos = 0;
  part_titles_read(&this->ier, &this->pos, (int*) &this->current_buffer[0], &this->size_titles);
  this->current_buffer.reserve(sizeof(int)*this->size_geo);
  this->pos = 0;
  part_titles_read(&this->ier, &this->pos, (int*) &this->current_buffer[0], &this->size_titles);
	check_ier("Femzip Error during part_titles_read.");

}


/*
 * Free the part buffer.
 */
void FemzipBuffer::free_partBuffer(){

}


/*
 * Init the state reading.
 */
void FemzipBuffer::init_nextState(){

  this->iTimeStep = 1;
  int retry = 0;
  int size_times = 2000;
  retry: this->timese = new float[size_times];
	this->pos = 0;
	ctimes_read(&this->ier, &this->pos, &this->nTimeStep, this->timese, &size_times);
	if (this->ier == 9)
	{
		if (retry < 1) {
			retry++;
			goto retry;
		}
    if(this->timese != NULL){
      delete[] this->timese;
      this->timese = NULL;
    }
    throw(string("Femzip Buffer Error: size for the states buffer is to small."));
	}

  if(this->timese != NULL){
    delete[] this->timese;
    this->timese = NULL;
  }
	check_ier("Femzip Error during ctimes_read.");

  this->timese = NULL;

  // preload timestep
  this->next_state_buffer = std::async(
    [](int _iTimestep, int _size_state){
      int _ier = 0;
      int _pos = 0;
      vector<char> state_buffer(sizeof(int)*_size_state);
      states_read(&_ier, &_pos, &_iTimestep,(int*) &state_buffer[0], &_size_state);
      if(_ier != 0){
        if(state_buffer.size() != 0){
          state_buffer = vector<char>();
        }
      }

      return std::move(state_buffer);
    },this->iTimeStep, this->size_state);
}


/*
 * Init the state reading.
 */
void FemzipBuffer::read_nextState(){

  #ifdef QD_DEBUG
  cout << "Loading state: " << this->iTimeStep << "/" << this->nTimeStep << endl;
  #endif

  this->current_buffer = this->next_state_buffer.get();
  if(this->current_buffer.size() == 0){
    throw(string("FEMZIP Error during state reading."));
  }

  this->iTimeStep++;

  // preload timestep
  this->next_state_buffer = std::async(
    [](int _iTimestep, int _size_state){
      int _ier = 0;
      int _pos = 0;
      vector<char> state_buffer(sizeof(int)*_size_state);
      states_read(&_ier, &_pos, &_iTimestep,(int*) &state_buffer[0], &_size_state);
      if(_ier != 0){
        if(state_buffer.size() != 0){
          state_buffer = vector<char>();
        }
      }

      return std::move(state_buffer);
    },this->iTimeStep, this->size_state);

}


/*
 * Is there another state?
 */
bool FemzipBuffer::has_nextState(){
  return this->iTimeStep <= this->nTimeStep;
}


/*
 * Is there another state?
 */
void FemzipBuffer::rewind_nextState(){

  this->iTimeStep = 1;

  // Size
  this->pos =0;
	get_Size((char*) this->filepath.c_str(), this->filetype, this->adjust, &this->size_geo,
		&this->size_state, &this->size_disp, &this->size_activity,
		&this->size_post, &this->ier);
	this->check_ier("Femzip Error during reading of sizes.");
  // geom must be read again ...
  this->read_geometryBuffer();
  this->free_geometryBuffer();
  // parts not ... at least
  // ...
  // states
  this->init_nextState();

}


/*
 * End the state reading.
 */
void FemzipBuffer::end_nextState(){

  states_close(&this->ier, &this->pos, (int*) &this->current_buffer[0], &this->size_geo);
  current_buffer.clear();

  close_read(&this->ier);
  this->check_ier("Femzip Error during state of file.");

}

/*
 * Close the file.
 */
void FemzipBuffer::finish_reading(){

   close_read(&this->ier);
   this->check_ier("Femzip Error during closing of file.");

}

/*
 * Check for error
 */
void FemzipBuffer::check_ier(string message){
  if(this->ier != 0){
    throw(message);
  }
}


