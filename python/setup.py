#!/usr/bin/env python

from distutils.core import setup

setup(
        name                = 'comma',
        version             = open('comma/version.py').readlines()[-1].strip().split()[-1].strip('\"'),
        description         = 'comma python utilties',
        url                 = 'https://github.com/acfr/comma',
        license             = 'BSD 3-Clause',
        packages            = [ 'comma', 'comma.csv', 'comma.csv.applications', 'comma.io', 'comma.numpy', 'comma.signal', 'comma.util' ],
        package_dir         = { 'comma.io': 'comma/io' },
        package_data        = { 'comma.io': ['stream.so' ] }
     )
