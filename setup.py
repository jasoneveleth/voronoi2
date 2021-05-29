from distutils.core import setup, Extension
import numpy
from Cython.Distutils import build_ext
from Cython.Build import cythonize

extensions = [Extension("voronoi",
                 sources=["voronoi.pyx", "bintree.c", "fortunes.c", "heap.c", "main.c"],
                 include_dirs=[numpy.get_include()],
                 define_macros=[('NPY_NO_DEPRECATED_API', 'NPY_1_7_API_VERSION'),('FLOAT',),('CYTHON',)]
                 )]

setup(
    cmdclass={'build_ext': build_ext},
    ext_modules=cythonize(extensions, language_level="3")
    )

