# Changelog

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [0.6] - 09/20/2021

### New

- Add "useSections" key to input file to allow user to run uncore test flow
  using "Sections/Uncore*" or use old "uncore" section from the input file without
  recompiling the code.
- Add support for PCH crashlog extraction flow.
- Add optional BAFI summary output.
- Use optane-memory repo for NVD feature.

## [0.5] - 06/30/2021

### Bug Fix
- Input file reading errors were not reported and crashdump output file did not
have any data suggesting the read error happened.

### New

- Performance enhancements for Uncore section were added. This controls via
`_max_time_sec` flag for PCI (30 seconds), MMIO (30 seconds) and RDIAMSR (30
seconds) records. If ACD takes more than the time mentioned by `_max_time_sec`
for a particular record, it will abort further collection of those record and
move ahead to other records. If such case is hit, `_<section_name>_aborted` key
will be inserted in the output file.
- Performance enhancement for big core section were added. This controls via
`_max_collection_sec` flag (default 15 seconds). If ACD takes more time than
mentioned by `_max_collection_sec`, it will abort further collection of big core
crash data and move ahead to other records. If such a case is hit,
`_big_core_aborted` key will be inserted in output file.
- Enable optional NVD feature to collect CSRs.
- Enable optional triage feature.
- Extended and added new Unit tests for various sections of crashdump.
- SPR: Added acd_verify for processing the output file into a report.

### Changed
- Changes to uncore MCA hierarchy  in  the input file.
- In uncore MCA add skip bank when there is an error on that bank, will print `N/A`.

## [0.4] - 03/31/2021

### Bug Fix

- Fix BIOS_ID Dbus path issue.

### New

- Update Version label to 0.4
- Add register counts to output file.
- Add additional registers to MetaData.CPU section(s). ierrloggingreg, firstierrtsc, firstmcerrtsc and mcerrloggingreg.
- Implement PECI Driver Selection option in Input file.
- Many Unit Test enhancements.
- SPR: Implement first revision of PM_Info section.
- SPR: Support in Big_core input file for SPR B0 specific change.


### Changed

- Modify big_core wait algorithm to wait for the first core to be available for big_core data for the IERR case only.
- Change Ordering on how Records are collected, new method is to collect by each record type accross all CPU’s then move to the next Record Type.
- Remove "vcode_ver" from MetaData section due to being N/A.
- Updates to input file for Uncore Register section.
- ICX: Modify PM_Info Register names in input files. Non-Functional change.
- SPR: Remove Address_Map record from SPR. Only, Registers are now in Uncore section.
- SPR: Changes to MCA registers in Input file.

## [0.3] - 12/15/2020

### Bug Fix

### New

- Support for crashdump records MetaData, MCA, TOR, Uncore, BigCore. (for SPR, the address_map record will now be included in the Uncore register Record.) PM_Info section will be supported in the Beta release.

### Changed

