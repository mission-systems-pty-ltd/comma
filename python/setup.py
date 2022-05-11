#!/usr/bin/env python

import setuptools, pathlib #from distutils.core import setup
import comma.version

# https://realpython.com/pypi-publish-python-package/

setuptools.setup(
        name                = 'comma-py', # quick and dirty to make python packaging work
        version             = comma.version.__version__,
        classifiers = [
          'Environment :: Console',
          'Intended Audience :: End Users/Desktop',
          'Intended Audience :: Developers',
          'License :: OSI Approved :: BSD License',
          'Operating System :: MacOS :: MacOS X',
          'Operating System :: Microsoft :: Windows',
          'Operating System :: POSIX',
          'Programming Language :: Python :: 3',
          'Topic :: Communications :: Email'
        ],
        description         = 'csv and fixed-width binary python utilities',
        url                 = 'https://gitlab.com/orthographic/comma',
        license             = 'BSD 3-Clause',
        long_description    = ( pathlib.Path(__file__).parent / "README.md" ).read_text(),
        long_description_content_type = "text/markdown",
        author              = "vsevolod vlaskine",
        author_email        = "vsevolod.vlaskine@gmail.com",
        maintainer          = 'vsevolod vlaskine',
        maintainer_email    = 'vsevolod.vlaskine@gmail.com',
        python_requires     = '>=3.6',
        install_requires    = [ 'numpy' ], # todo?
        packages            = [ 'comma', 'comma.csv', 'comma.csv.applications', 'comma.io', 'comma.numpy', 'comma.signal', 'comma.util', 'comma.cpp_bindings', 'comma.application' ],
        package_dir         = { 'comma': 'comma', 'comma.cpp_bindings': 'comma/cpp_bindings' },
        package_data        = { 'comma.cpp_bindings': [ '*.so', '*.dll' ] },
        entry_points        = { 'console_scripts': ['csv-eval=comma.csv.applications.csv_eval:main'] } #scripts             = [ "comma/csv/applications/csv-eval" ]
     )
