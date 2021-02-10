#!/usr/bin/env python

from distutils.core import setup # from setuptools import setup
import comma.version

setup(
        name                = 'comma',
        version             = comma.version.__version__,
        classifiers = [
          'Development Status :: 4 - Beta',
          'Environment :: Console',
          'Intended Audience :: End Users/Desktop',
          'Intended Audience :: Developers',
          'License :: OSI Approved :: BSD 3-Clause',
          'Operating System :: MacOS :: MacOS X',
          'Operating System :: Microsoft :: Windows',
          'Operating System :: POSIX',
          'Programming Language :: Python',
          'Topic :: Communications :: Email'
        ],
        description         = 'comma python utilties',
        install_requires    = [ 'numpy' ],
        url                 = 'https://gitlab.com/orthographic/comma',
        license             = 'BSD 3-Clause',
        maintainer          = 'vsevolod vlaskine',
        maintainer_email    = 'vsevolod.vlaskine@gmail.com',
        packages            = [ 'comma', 'comma.csv', 'comma.csv.applications', 'comma.io', 'comma.numpy', 'comma.signal', 'comma.util', 'comma.cpp_bindings', 'comma.application' ],
        package_dir         = { 'comma.cpp_bindings': 'comma/cpp_bindings' },
        package_data        = { 'comma.cpp_bindings': [ '*.so', '*.dll' ] },
        scripts             = [ "comma/csv/applications/csv-eval" ]
     )
