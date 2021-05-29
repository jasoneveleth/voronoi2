from distutils.core import setup, Extension
import numpy
from Cython.Distutils import build_ext
from Cython.Build import cythonize

extensions = [Extension("voronoi",
                 sources=["voronoi.pyx", "src/bintree.c", "src/fortunes.c", "src/heap.c", "src/main.c"],
                 include_dirs=[numpy.get_include()],
                 define_macros=[('NPY_NO_DEPRECATED_API', 'NPY_1_7_API_VERSION'),('NMAIN',)]
                 )]

setup(
    cmdclass={'build_ext': build_ext},
    ext_modules=cythonize(extensions, language_level="3", build_dir="build")
    )

