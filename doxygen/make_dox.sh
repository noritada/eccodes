#!/bin/sh

# No need to do tools. We use another mechanism. See top-level
# confluence directory
#cd ../tools
#./make_dox.ksh
#cd ../doxygen

set -e

./process_C_header.pl ../src/grib_api.h ../src/eccodes.h > eccodes.h

# Python doxygen module will be "ecCodes" to distinguish it from "eccodes" for F90
./process_python.pl ../python/gribapi.py > ecCodes.py

rm -fr ../html/*
touch ../html/Makefile.am
doxygen grib_api_wiz.cfg

# Do not copy this. Use default generated doxygen.css
# cp doxygen.css ../html/

# Remove temp files
rm -f eccodes.h
rm -f ecCodes.py

# Change the links from any example code back to confluence
F90_file=../html/classeccodes.html
Py_file=../html/namespaceec_codes.html

fnames="
bufr_get_keys
grib_index
grib_get_keys
grib_clone
grib_copy_message
count_messages
grib_samples
grib_set_missing
"

for fn in $fnames; do
  perl -p -i -e "s|$fn\.f90|<a href=\"https://software.ecmwf.int/wiki/display/ECC/$fn\" target=\"_blank\">$fn.f90</a>|" $F90_file
done

pnames="
grib_get_keys
bufr_get_keys
grib_nearest
grib_index
grib_multi_write
grib_iterator
grib_clone
"
for fn in $pnames; do
  perl -p -i -e "s|$fn\.py|<a href=\"https://software.ecmwf.int/wiki/display/ECC/$fn\" target=\"_blank\">$fn.py</a>|" $Py_file
done


echo DONE
