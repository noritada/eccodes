#!/bin/sh
# Copyright 2005-2018 ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities granted to it by
# virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.
#

. ./include.sh

label="grib_lam_gp"
temp=temp.$label.txt

dump_and_check()
{
    input=$1
    ${tools_dir}/grib_dump -O $input >$temp 2>&1
    set +e
    # Look for the word ERROR in output. We should not find any
    grep -q 'ERROR ' $temp
    if [ $? -eq 0 ]; then
        echo "File $input: found string ERROR in grib_dump output!"
        exit 1
    fi
    set -e
}


${test_dir}/lam_gp

dump_and_check lam_gp_lambert_lam.grib
dump_and_check lam_gp_mercator_lam.grib
dump_and_check lam_gp_polar_stereographic_lam.grib

rm -f $temp
