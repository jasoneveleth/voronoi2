from distutils.core import setup, Extension
import numpy
from Cython.Distutils import build_ext

extensions = [Extension("voronoi",
                 sources=["voronoi.pyx", "bintree.c", "fortunes.c", "heap.c", "cython_stuff.c"],
                 include_dirs=[numpy.get_include()]
                 )]

setup(
    cmdclass={'build_ext': build_ext},
    ext_modules=extensions
)

