
#ifndef D3PLOT_HPP
#define D3PLOT_HPP

// forward declarations
class KeyFile;
class DB_Nodes;
class DB_Parts;
class DB_Elements;
class AbstractBuffer;

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <algorithm> // trim
#include "dyna_cpp/db/FEMFile.hpp"


class D3plot : public FEMFile {

private:
  std::string dyna_title;
  std::string dyna_datetime; // BUGGY

  int dyna_ndim; // dimension parameter
  int dyna_numnp; // number of nodes
  int dyna_mdlopt; // describes element deletion
  int dyna_mattyp; // material types section is read

  int dyna_nglbv; // global vars per timestep

  int dyna_nel2; // #elements with 2 nodes (beams)
  int dyna_nel4; // #elements with 4 nodes (shells)
  int dyna_nel48; // # 8 node shell elements?!?!?!
  int dyna_nel8; // #elements with 8 nodes (solids)
  int dyna_nel20; // # 20 node solid elements
  int dyna_nelth; // #thshells

  int dyna_nmmat;   // #mats
  int dyna_nummat2; // #mats for 1d/2d/3d/th elems
  int dyna_nummat4;
  int dyna_nummat8;
  int dyna_nummatth;

  int dyna_nv1d; // #vars for 1d/2d/3d/th elems
  int dyna_nv2d;
  int dyna_nv3d;
  int dyna_nv3dt;

  int dyna_maxint; // #layers of integration points
  int dyna_istrn; // indicates whether strain was written
  int dyna_neiph; // extra variables for solids
  int dyna_neips; // extra variables for shells

  int dyna_iu; // Indicators for: disp/vel/accel/temp
  int dyna_iv;
  int dyna_ia;
  int dyna_it;
  int dyna_idtdt; // temp change rate, numnp vals after temps

  int dyna_narbs; // dunno ... seems important

  int dyna_ioshl1; // 6 shell stresses
  int dyna_ioshl2; // shell plastic strain
  int dyna_ioshl3; // shell forces
  int dyna_ioshl4; // thick,energy,2 extra

  int dyna_extra; // double header length indicator
  int dyna_numprop; // number of properties dude!!!

  int dyna_numrbe; // number of rigid body shell elems
  std::vector<int> dyna_irbtyp; // rigid body material type numbers (internal)

  // just for checks ... can not be handled.
  int dyna_nmsph; // #nodes of sph
  int dyna_ngpsph; // #mats of sph
  int dyna_ialemat; // # some ale stuff .. it's late ...

  // Own Variables
  int nStates;
  std::vector<float> timesteps;

  bool own_nel10;
  bool own_external_numbers_I8 ;

  int wordPosition;
  int wordsToRead;
  int wordPositionStates;

  bool useFemzip;
  int femzip_state_offset;

  // Checks for already read variables
  bool plastic_strain_is_read;
  unsigned int plastic_strain_read;
  bool energy_is_read;
  unsigned int energy_read;
  bool strain_is_read;
  unsigned int strain_read;
  bool stress_is_read;
  unsigned int stress_read;
  bool stress_mises_is_read;
  unsigned int stress_mises_read;
  bool disp_is_read;
  unsigned int disp_read;
  bool acc_is_read;
  unsigned int acc_read;
  bool vel_is_read;
  unsigned int vel_read;
  std::vector<unsigned int> history_is_read;
  std::vector<unsigned int> history_shell_is_read;
  std::vector<unsigned int> history_solid_is_read;

  std::vector<unsigned int> history_var_read;
  std::vector<unsigned int> history_shell_read;
  std::vector<unsigned int> history_solid_read;

  std::vector<unsigned int> history_var_mode;
  std::vector<unsigned int> history_shell_mode;
  std::vector<unsigned int> history_solid_mode;

  AbstractBuffer* buffer;

  // Functions
  void init_vars();
  void read_header();
  void read_matsection();
  void read_geometry();
  std::vector< std::vector<float> > read_geometry_nodes();
  std::vector< std::vector<int> >   read_geometry_elem8();
  std::vector< std::vector<int> >   read_geometry_elem4();
  std::vector< std::vector<int> >   read_geometry_elem2();
  std::vector< std::vector<int> >   read_geometry_numbering();
  void                  read_geometry_parts();
  void read_states_init();
  void read_states_parse(std::vector<std::string>);
  unsigned int read_states_parse_readMode(const std::string& _variable) const;
  void read_states_displacement();
  void read_states_velocity();
  void read_states_acceleration();
  void read_states_elem8(size_t iState);
  void read_states_elem4(size_t iState);
  bool isFileEnding(int);

  // === P U B L I C === //
public:
  D3plot(std::string filepath, 
         std::vector<std::string> _variables = std::vector<std::string>(),
         bool _use_femzip = false);
  ~D3plot();
  void info();
  void read_states(std::vector<std::string> _variables);
  void clear( const std::vector<std::string>& _variables = std::vector<std::string>() );
  std::vector<float> get_timesteps();
  bool displacement_is_read();
  bool is_d3plot(){return true;};
  bool is_keyFile(){return false;};
  D3plot* get_d3plot(){return this;};
  KeyFile* get_keyFile(){throw(std::string("You can not get a keyfile handle from a d3plot ... for now."));};

};

#endif
