Crashdump

## Installation Steps:
Dependencies:
-Install sdbusplus  (https://github.com/openbmc/sdbusplus)
    Tested with: 1befc49be9b32b1c04cc9485c132827c86aa891c
-Install boost (https://www.boost.org/) Tested with: version 1.66
-Install CJSON (https://github.com/DaveGamble/cJSON) Tested with verison v1.7.12
-Install safeclib (https://sourceforge.net/projects/safeclib/files/)
    Tested with: f1708afcbba69f75e80920b75aa3d7baee3f611d
-(optional)Install gtest (https://github.com/google/googletest)
    Tested with: 2134e3fd857d952e03ce76064fad5ac6e9036104

## BUILD
run cmake . to generate make files
(Optional to run unit testing) run cmake --DCRASHDUMP_BUILD_UT=ON
run make on same directory

## SECURITY FLAGS
if your compiler supports we recommend to add to the cmake file
set(CMAKE_CXX_FLAGS "-fstack-protector-strong")
set(CMAKE_C_FLAGS "-fstack-protector-strong")

## AutoStart
copy the file com.intel.crashdump into the /lib/systemd/system directory.
Once it is copied you can manually start the process typing systemctl start
    com.intel.crashdump
