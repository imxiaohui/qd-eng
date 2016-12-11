#!/usr/bin/env python

import os
import sys
import platform
import compileall
import numpy as np
from setuptools import find_packages,setup,Extension

# ======= S E T T I N G S ======= #
boost_path = "libs/boost_1_61_0"
femzip_path = "libs/femzip" # optional
# ====== D E V E L O P E R ====== #
debugging_mode = False
_version = "0.2.0"
# =============================== #

# py -> pyc
compileall.compile_dir('src/')

# (0) Compiler Stuff
# Check for MinGW usage
use_mingw = False
for ii in range(len(sys.argv)):
	if (sys.argv[ii] == "-c") and (sys.argv[ii+1] == "mingw32"):
		use_mingw=True
	if (sys.argv[ii] == "--compiler=mingw32"):
		use_mingw=True

# (1) Native Code Stuff
# (1.1) DYNA-POST toolbox
if not os.path.isdir(boost_path):
    raise Exception("Invalid boost library path: %s." % boost_path)
    #b2 --toolset=msvc-10.0 --build-type=complete architecture=x86 address-model=64 stage
compiler_args_dyna = []
include_dirs_dyna = [boost_path,np.get_include()]
lib_dirs_dyna = [] # ["libs/boost_1_61_0/lib64-msvc-9.0"]
libs_dyna  = [] # ["boost_python"]
srcs_dyna = ["src/qd/cae/dyna_cpp/python_api/wrapper.cpp",
    "src/qd/cae/dyna_cpp/db/FEMFile.cpp",
    "src/qd/cae/dyna_cpp/db/DB_Elements.cpp",
    "src/qd/cae/dyna_cpp/db/DB_Nodes.cpp",
    "src/qd/cae/dyna_cpp/db/DB_Parts.cpp",
    "src/qd/cae/dyna_cpp/db/Element.cpp",
    "src/qd/cae/dyna_cpp/db/Node.cpp",
    "src/qd/cae/dyna_cpp/db/Part.cpp",
    "src/qd/cae/dyna_cpp/dyna/D3plotBuffer.cpp",
    "src/qd/cae/dyna_cpp/dyna/D3plot.cpp",
    "src/qd/cae/dyna_cpp/dyna/KeyFile.cpp",
    "src/qd/cae/dyna_cpp/dyna/DynaKeyword.cpp",
    "src/qd/cae/dyna_cpp/utility/FileUtility.cpp",
    "src/qd/cae/dyna_cpp/utility/TextUtility.cpp",
    "src/qd/cae/dyna_cpp/utility/MathUtility.cpp"]
# FEMZIP usage? Libraries present?
# You need to download the femzip libraries yourself from SIDACT GmbH
# If you have questions, write a mail.
if os.path.isdir(femzip_path):
    if (platform.system() == "Windows") and os.path.isdir(os.path.join(femzip_path,"Windows_VS2010_MT","x64")):
        srcs_dyna.append("src/qd/cae/dyna_cpp/dyna/FemzipBuffer.cpp")
        lib_dirs_dyna.append(os.path.join(femzip_path,"Windows_VS2010_MT","x64"))
        libs_dyna = ['femunziplib_standard_dyna','ipp_zlib','ippcoremt',
            'ippdcmt','ippsmt','ifwin','ifconsol','ippvmmt','libmmt',
            'libirc','svml_dispmt','msvcrt']
        compiler_args_dyna.append("/DQD_USE_FEMZIP")
    elif (platform.system() == "Linux") and os.path.isdir(os.path.join(femzip_path,"Linux","64Bit")):
        srcs_dyna.append("src/qd/cae/dyna_cpp/dyna/FemzipBuffer.cpp")
        lib_dirs_dyna.append(os.path.join(femzip_path,"Linux","64Bit"))
        libs_dyna = ['femunzip_dyna_standard','ipp_z','ippcore',
            'ippdc','ipps','ifcore_pic','ifcoremt','imf',
            'ipgo','irc','svml','ippcore_l','stdc++','dl']
        compiler_args_dyna.append("-DQD_USE_FEMZIP")
else:
	print("FEMZIP library %s not found. Compiling without femzip support." % femzip_path)

# CFLAGS linux
if (platform.system().lower() == "linux") or (platform.system().lower() == "linux2") or use_mingw:
	#compiler_args_dyna.append("-std=c++11") # I really wish so ...
	compiler_args_dyna.append("-O3")
	if not use_mingw:
		compiler_args_dyna.append("-fPIC")
	if debugging_mode:
		compiler_args_dyna.append("-DQD_DEBUG")
# CFLAGS Windows
else:
	compiler_args_dyna.append("/EHa")
	if debugging_mode:
		compiler_args_dyna.append("/DQD_DEBUG")
dyna_extension = Extension("dyna_cpp", srcs_dyna,
                                      extra_compile_args = compiler_args_dyna,
									  library_dirs=lib_dirs_dyna,
									  libraries=libs_dyna,
									  include_dirs=include_dirs_dyna,)


# (2) setup
setup(name = 'qd',
		version = _version,
		license = 'GNU GPL v3',
		description = 'QD-Engineering python library',
		author = 'C. Diez',
        url    = 'www.qd-eng.de',
        author_email = 'constantin.diez@gmail.com',
		packages=(['qd',
                 'qd.cae',
                 'qd.numerics']),
		#packages=find_packages(),
		package_dir={'qd'    : 'src/qd',
                     'qd.cae' : 'src/qd/cae',
                     'qd.numerics' : 'src/qd/numerics'},
        ext_package='qd.cae', # where to place c extensions
        ext_modules=[dyna_extension],
		install_requires=['numpy'],
		keywords=['cae','engineering','ls-dyna','postprocessing','preprocessing'],
        classifiers=['Development Status :: 3 - Alpha',
                     'Programming Language :: C++',
                     'Topic :: Scientific/Engineering',
                     'Intended Audience :: Science/Research',
                     'Topic :: Utilities',
                     'Topic :: Engineering',
                     'Topic :: CAE',
                     'Topic :: FEM',
                     'Programming Language :: Python :: 2.7'],)
