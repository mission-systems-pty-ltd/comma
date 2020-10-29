#!/usr/bin/env python

from distutils.core import setup # from setuptools import setup
import comma.version

setup(
        name                = 'comma',
        version             = comma.version.__version__,
        description         = 'comma python utilties',
        install_requires    = [ 'numpy' ],
        url                 = 'https://gitlab.com/orthographic/comma',
        license             = 'BSD 3-Clause',
        scripts             = ["comma/csv/applications/csv-eval"],
        packages            = [ 'comma', 'comma.csv', 'comma.csv.applications', 'comma.io', 'comma.numpy', 'comma.signal', 'comma.util', 'comma.cpp_bindings', 'comma.application' ],
        package_dir         = { 'comma.cpp_bindings': 'comma/cpp_bindings' },
        package_data        = { 'comma.cpp_bindings': [ '*.so', '*.dll' ] }
     )
