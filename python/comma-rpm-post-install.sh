#!/bin/sh

cd /tmp/comma/python_modules_for_rpm_install && python setup.py install
rm -rf /tmp/comma/python_modules_for_rpm_install
