import os
import sys

sys.path += [ 'src/python', 'tools' ]

import numpy as np

from numpy.distutils.core import setup, Extension

from meta import scanallmeta
from listclasses import get_module_list
from gen_factory import write_factory_f90, write_factory_c

###

cwd = os.getcwd()
srcdir = '{0}/src'.format(cwd)

###

#
# Module configuration
#

inc_dirs = [ ]
lib_dirs = [ ]
# mdcorelib is a dependency, build below by the setup command
libs = [ 'mdcorelib' ]
extra_link_args = [ ]

###

#
# Get MKL configuration
#

mklroot = os.getenv('MKLROOT')
if mklroot is None:
    mklroot = os.getenv('MKL_ROOT')
if mklroot is not None:
    for lib_dir in [ '{0}/lib/em64t'.format(mklroot), 
                     '{0}/lib/intel64'.format(mklroot) ]:
        if os.path.exists(lib_dir):
            lib_dirs += [ lib_dir ]
    #libs += [ 'mkl_intel_lp64', 'mkl_intel_thread', 'mkl_lapack',
    #          'mkl_core', 'mkl_def', 'irc_s', 'iomp5', 'ifcore', 'ifport',
    #          'stdc++' ]
    libs += [ 'ifcore', 'ifport' ]
    extra_link_args += [ '-mkl=sequential' ]
else:
    libs += [ 'blas', 'lapack' ]

###

#
# Build list of source files
#

lib_srcs = [ ]
mod_srcs = [ ]

# MDCORE support modules
lib_srcs += [ 'build/versioninfo.f90',
            ]

lib_srcs += [ '{0}/support/'.format(srcdir)+i for i in
              [ 'c_f.f90',
                'error.f90',
                'System.f90',
                'MPI_context.f90',
                'Units.f90',
                'PeriodicTable.f90',
                'f_linearalgebra.f90',
                'f_ptrdict.f90',
                'c_ptrdict.c',
                'io.f90',
                'f_logging.f90',
                'c_logging.c',
                'timer.f90',
                'tls.f90',
                'misc.f90',
                'data.f90',
                'simple_spline.f90',
                'supplib.f90',
                'mdcore.f90',
                ]
              ]

lib_srcs += [ '{0}/special/'.format(srcdir)+i for i in
              [ 'table2d.f90',
                'table3d.f90',
                'table4d.f90',
                ]
              ]

lib_srcs += [ '{0}/python/f90/'.format(srcdir)+i for i in
              [ 'python_particles.f90',
                'python_filter.f90',
                'python_neighbors.f90',
                'particles_wrap.f90',
                'neighbors_wrap.f90',
                'python_helper.f90',
                ]
              ]

# Generate versioninfo
os.system('sh src/gen_versioninfo.sh src build Python')

# Other stuff
mod_srcs += [ '{0}/python/c/py_f.c'.format(srcdir),
              '{0}/python/c/particles.c'.format(srcdir),
              '{0}/python/c/neighbors.c'.format(srcdir),
              '{0}/python/c/potential.c'.format(srcdir),
              '{0}/python/c/analysis.c'.format(srcdir),
              '{0}/python/c/mdcoremodule.c'.format(srcdir),
              ]

###

#
# Scan all metadata and get list of potentials
#

#print 'Scanning f90 metadata in directory {0}...'.format(srcdir)
metadata = scanallmeta(srcdir)

#print 'Writing factories...'

mods2, fns2 = get_module_list(metadata, 'potentials',
                              finterface_list = [ 'register_data' ],
                              exclude_list = [ 'MolecularTightBinding',
                                               'VariableCharge',
                                               'ConstantForce',
                                               'HarmonicHook',
                                               'Harmonic',
                                               'LennardJones',
                                               'Confinement',
                                               'SlidingP',
                                               'FFMTip' ]
                              )
lib_srcs += fns2

write_factory_f90(mods2, 'potential', 'build/potentials_factory_f90.f90')
write_factory_c(mods2, 'potential', 
                '{0}/python/c/factory.template.c'.format(srcdir),
                'build/potentials_factory_c.c',
                '{0}/python/c/factory.template.h'.format(srcdir),
                'build/potentials_factory_c.h')
lib_srcs += [ 'build/potentials_factory_f90.f90',
              'build/potentials_factory_c.c',
              ]

f = open('build/have.inc', 'w')
print >> f, '#ifndef __HAVE_INC'
print >> f, '#define __HAVE_INC'
for classabbrev, classname, classtype, s in mods2:
    print >> f, '#define HAVE_{0}'.format(classabbrev.upper())
print >> f, '#endif'
f.close()

###

#
# C and Fortran compile flags
#

inc_dirs += [ np.get_include(),
              'build',
              'src',
              'src/libAtoms',
              'src/support',
              'src/potentials',
              ]

lib_macros = [ ( 'NO_BIND_C_OPTIONAL', None ),
               ( 'MDCORE_PYTHON', None ),
               ]

###

setup(
    name = 'mdcore',
    version = '0.1',
    packages = [
        'mdcore'
        ],
    package_dir = {
        'mdcore': 'src/python/mdcore'
        },
    libraries = [
        ( 'mdcorelib', dict(
                sources = lib_srcs,
                include_dirs = inc_dirs,
                macros = lib_macros,
                extra_compiler_args = [ '-fPIC' ],
                )
          )
        ],
    ext_modules = [
        Extension(
            '_mdcore',
            mod_srcs,
            include_dirs = inc_dirs,
            library_dirs = lib_dirs,
            libraries = libs,
            extra_compile_args = [ '-fPIC' ],
            extra_link_args = extra_link_args,
            )
        ]
    )
