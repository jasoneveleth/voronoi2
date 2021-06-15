from distutils.core import setup, Extension
import numpy
from Cython.Distutils import build_ext
from Cython.Build import cythonize
import os

macros = [('NPY_NO_DEPRECATED_API', 'NPY_1_7_API_VERSION'),('NMAIN',1)]

if os.getenv('REPEL'):
    macros.append(('REPEL',1))

files = ["voronoi.pyx", "src/bintree.c", "src/edgelist.c", "src/fortunes.c", "src/geometry.c", "src/gradient.c", "src/heap.c", "src/main.c"]
extensions = [Extension("voronoi",
                 files,
		 extra_compile_args=['-std=c11'],
                 include_dirs=[numpy.get_include()],
                 define_macros=macros
                 )]

setup(
    cmdclass={'build_ext': build_ext},
    ext_modules=cythonize(extensions, language_level="3", build_dir="build")
    )

