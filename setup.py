from distutils.core import setup, Extension
import numpy
from Cython.Distutils import build_ext
from Cython.Build import cythonize

files = ["voronoi.pyx", "src/bintree.c", "src/edgelist.c", "src/fortunes.c", "src/geometry.c", "src/gradient.c", "src/heap.c", "src/main.c"]
extensions = [Extension("voronoi",
                 files,
                 include_dirs=[numpy.get_include()],
                 define_macros=[('NPY_NO_DEPRECATED_API', 'NPY_1_7_API_VERSION'),('NMAIN',)]
                 )]

setup(
    cmdclass={'build_ext': build_ext},
    ext_modules=cythonize(extensions, language_level="3", build_dir="build")
    )

