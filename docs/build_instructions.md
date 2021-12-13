# Building CrashDump with Intel-BMC/openbmc

## Building OpenBMC image with the crashdump source code

1. Clone Intel OpenBMC code from [GitHub](https://github.com/Intel-BMC/openbmc).
    ```shell
    git clone https://github.com/Intel-BMC/openbmc
    cd openbmc
    ```

2. Clone the `meta-egs` repo inside meta-openbmc-mods.
    ```shell
    cd meta-openbmc-mods
    git clone git@github.com:Intel-BMC/meta-egs.git
    ```

3. Create a `crashdump` folder and include the following bitbake file in it.
    ```shell
    cd meta-openbmc-mods/meta-common/recipes-core
    mkdir crashdump
    cd crashdump
    touch crashdump_git.bb
    ```

4. Follow this step if you have got `crashdump` source in a tar.gz or zip file.
Else go to step 5.
    ```shell
    cd meta-openbmc-mods/meta-common/recipes-core/crashdump
    mkdir -p files/crashdump
    tar -xvf crashdump.tar.gz -C ./files/crashdump
    ```

5. Open `crashdump_git.bb` in your favorite editor and include the following in
it. Please make sure bb file format follows bitbake rules. The bitbake file has
dependencies as shown below with the `DEPENDS` command. Choose the right
`SRC_URI` method in the bitbake file depending on how you have received your
`crashdump` source code. If you have access, you can also find the bitbake file
at `https://github.com/Intel-BMC/meta-restricted/recipes-core/crashdump/crashdump_git.bb`.

    ```bitbake
    inherit obmc-phosphor-dbus-service
    inherit obmc-phosphor-systemd

    SUMMARY = "CPU Crashdump"
    DESCRIPTION = "CPU utilities for dumping CPU Crashdump and registers over PECI"

    DEPENDS = "boost cjson sdbusplus safec gtest libpeci"
    inherit cmake systemd

    EXTRA_OECMAKE = "-DYOCTO_DEPENDENCIES=ON -DCRASHDUMP_BUILD_UT=OFF"

    LICENSE = "Proprietary"
    LIC_FILES_CHKSUM = "file://LICENSE;md5=43c09494f6b77f344027eea0a1c22830"

    # If you have access to the repo where crashdump is released
    SRC_URI = "git://git@github.com/Intel-BMC/crashdump.git;protocol=ssh;branch=egs"
    SRCREV = "8d1e8934e48a96e65981dd62b061b14f57e6f72b"
    S = "${WORKDIR}/git"

    # or if you have been provided with crashdump source code
    #SRC_URI = "file://crashdump"
    #S = "${WORKDIR}/crashdump"

    SYSTEMD_SERVICE_${PN} += "com.intel.crashdump.service"

    # linux-libc-headers guides this way to include custom uapi headers
    CFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
    CFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include"
    CXXFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
    CXXFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include"
    do_configure[depends] += "virtual/kernel:do_shared_workdir"
    ```

6. When the `crashdump` module compiles successfully, OpenBMC needs to install
`crashdump` to the image file. Add the `IMAGE_INSTALL` command to the `local.
conf.sample` file in the `conf` folder.

    ```shell
    cd meta-openbmc-mods/meta-egs
    vi ./conf/local.conf.sample
    ```

    ```vi
    IMAGE_INSTALL += "crashdump"
    ```

7. After the bitbake file is created, use `devtool` as follows to allow OpenBMC
to use the `crashdump` from the source code.
    ```shell
    cd <root of openbmc source code>
    source oe-init-build-env
    devtool modify crashdump
    devtool build crashdump
    ```

8. Build `openbmc` image.

    ```shell
    bitbake intel-platforms
    ```

## New Input File Format (NIFF)

Follow below steps to enable NIFF test flow:

1. Copy input file to /tmp/crashdump/input directory

```example
cp /usr/share/crashdump/input/crashdump_input_spr.json /tmp/crashdump/input
```

2. Set input file "UseSections" value to "true"

## Enabling optional features

### NVD

1. Ask Intel representative for optane-memory repo access and follow repo
   instruction to setup .bb file.

   [Github](https://github.com/Intel-BMC/optane-memory)

2. Change `NVD_SECTION` flag to ON in CMakeList.txt

   ```
   option (NVD_SECTION "Add NVD section to the crashdump contents." ON)
   ```

   Add fis to DEPENDS variable if it does not already exist.
   ```
   DEPENDS = "boost cjson sdbusplus safec gtest libpeci fis"
   ```

3. For non-Yocto build only

   ```shell
   git clone git@github.com:Intel-BMC/optane-memory.git
   cd optane-memory\fis
   mkdir build && cd build
   cmake ..
   sudo make install
   ```

### BAFI (BMC assisted FRU Isolation)

1. Ask Intel representative for `Intel System Crashdump BMC assisted FRU Isolation`
   access and follow repo instruction to setup .bb file.

   [Intel Developer Zone](https://www.intel.com/content/www/us/en/secure/design/confidential/software-kits/kit-details.html?kitId=686344)

2. Select one of below BAFI options:

   a) Change `TRIAGE_SECTION` flag to ON in CMakeList.txt to enable BAFI triage output (no NDA required):

   ```
   option (TRIAGE_SECTION "Add triage section to the crashdump contents." ON)
   ```

   b) Change `BAFI_NDA_OUTPUT` flag to ON in CMakeList.txt to enable BAFI full output:

   Notes: NDA (Non Disclosure Agreement) is required to see output. Keep this flag OFF when users of crashdump do not have an Intel NDA.

   ```
   option (BAFI_NDA_OUTPUT "Add summary section to the crashdump contents." ON)
   ```

3. For Non-Yocto build only

   Notes: Current BAFI source code is not fully compatible with -Werror flag.

   ```shell
   # unzip BAFI source code inside crashdump source directory and named bafi
   cd bafi
   mv include bafi

   # Add the following line to CMakeList.txt
   include_directories (${BAFI_SOURCE_DIR})

   Remove "-Werror" flag in CMakeList.txt
   ```

## Building and running Crashdump Unit tests

1. Inside the crashdump source code folder, run the following steps

    ```shell
    mkdir build
    cd build
    cmake .. -DCRASHDUMP_BUILD_UT=ON -DCODE_COVERAGE=ON
    make
    ```

2. To run the unit tests execute the binary file in
    ```shell
    cd build
    ./crashdump_ut
    ```

3. To generate the code coverage report, run the following `ccov` target. The
code coverage report can be launched by opening in `build\ccov\index.html`.
    ```
    cd build
    cmake .. -DCRASHDUMP_BUILD_UT=ON -DCODE_COVERAGE=ON
    make ccov
    ```

## Troubleshooting
The following are possible issues and the corresponding solutions.

### PECI lib failure
- EagleStream/Whitley supports PECI 4.0, please make sure the PECI driver
supports 4.0
- Telemetry Watcher Config commands may not yet be released and can be found in
the repository containing the peci patches. The repository location is
[meta-restricted](https://github.com/Intel-BMC/meta-restricted/tree/master/recipes-core/libpeci/libpeci).
This may require contacting Intel for access to the repository.

### safec fail
- This is the lib which provides bounds checking to make existing standard C
library safer.
- Use devtool to compile safec, make sure source in bb file.
- Confirm source version of safec
- Remove the perl dependency

### SECURITY FLAGS
- If your compiler supports it we recommend to add to the cmake file.
    ```cmake
    set(CMAKE_CXX_FLAGS "-fstack-protector-strong")
    set(CMAKE_C_FLAGS "-fstack-protector-strong")
    ```