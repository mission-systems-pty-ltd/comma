#!/usr/bin/env python

from distutils.core import setup

setup(
        name                = 'comma',
        version             = '1.0',
        description         = 'comma python utilties',
        url                 = 'https://github.com/acfr/comma',
        license             = 'BSD 3-Clause',
        packages            = [ 'comma', 'comma.csv', 'comma.csv.applications', 'comma.io' ],
     )
