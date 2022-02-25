#!/bin/sh

# Copyright (C) 2022 by J.W https://github.com/jakwings/GoodbyeBigSlow.kext
#
#   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
#
#  0. You just DO WHAT THE FUCK YOU WANT TO.

set -euf; unset -v IFS; export LC_ALL=C

exec >&2

script_dir="$(dirname -- "$0")"

vendor="$(sysctl -n machdep.cpu.vendor)"
processor="$(sysctl -n machdep.cpu.brand_string)"

recognized=no

case "${vendor}" in
  (*[Ii][Nn][Tt][Ee][Ll]*)
    signature="$(sysctl -n machdep.cpu.family machdep.cpu.model)"
    signature="$(printf '%02X_%02X' $signature)"

    # CPU specifications
    # https://ark.intel.com/content/www/us/en/ark/search/featurefilter.html
    # Intel® 64 and IA-32 architectures software developer's manual
    # https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
    # Volume 4: Model-specific registers
    # https://cdrdv2.intel.com/v1/dl/getContent/671098
    # 335592-sdm-vol-4.pdf (Table 2-12, 2-15, 2-20, 2-39) MSR_POWER_CTL: 1FCH
    case "${signature}" in
      #2.5# Intel® Atom processors @ Goldmont
      (06_5C) recognized=yes ;;
      #2.6# Intel® Atom processors @ Goldmont Plus
      (06_7A) recognized=yes ;;
      #2.7# Intel® processors @ Tremont
      (06_86|06_96|06_9C) recognized=yes ;;
      #2.8# Intel® Core™ i7 and i5 processors @ Nehalem
      (06_1A|06_1E|06_1F|06_2E) recognized=yes ;;
      #2.9# Intel® Xeon® Processor 5600 Series @ Westmere
      #2.9# Intel® Core™ i7, i5 and i3 processors @ Westmere
      (06_25|06_2C) recognized=yes ;;
      #2.10# Intel® Xeon® Processor E7 Family @ Westmere
      (06_2F) recognized=yes ;;
      #2.11# Intel® processors @ Sandy Bridge
      (06_2A|06_2D) recognized=yes ;;
      #2.12.1# Intel® Xeon® Processor E5 v2 Family @ Ivy Bridge-E
      (06_3E) recognized=yes ;;
      #2.13# 4th-Gen Intel® Core™ processors @ Haswell
      #2.13# Intel® Xeon® Processor E3-1200 v3 Family @ Haswell
      (06_3C|06_45|06_46) recognized=yes ;;
      #2.14# Intel® Xeon® Processor E5 v3 and E7 v3 Family @ Haswell-E
      (06_3F) recognized=yes ;;
      #2.15# Intel® Core™ M-5xxx processors @ Broadwell
      #2.15# Intel® Xeon® Processor E3-1200 v4 Family @ Broadwell
      #2.15# 5th-Gen Intel® Core™ processors @ Broadwell
      (06_3D|06_47) recognized=yes ;;
      #2.16.1# Intel® Xeon® Processor D Family @ Broadwell
      (06_56) recognized=yes ;;
      #2.16.2# Intel® Xeon® processor E5 v4 Family @ Broadwell
      (06_4F) case "${processor}" in (*Xeon*E5*v4*) recognized=yes; esac ;;
      #2.17# 6th-Gen Intel® Core™ processors @ Skylake
      (06_4E|06_5E) recognized=yes ;;
      #2.17# Intel® Xeon® Processor Scalable Family @ Skylake
      #2.17# 2nd-Gen Intel® Xeon® Processor Scalable Family @ Cascade Lake
      #2.17# 3rd-Gen Intel® Xeon® Processor Scalable Family @ Cooper Lake
      (06_55) recognized=yes ;;
      #2.17# 7th-Gen Intel® Core™ processors @ Kaby Lake
      #2.17# 8th and 9th-Gen Intel® Core™ processors @ Coffee Lake
      #2.17# Intel® Xeon® E processors @ Coffee Lake
      (06_8E|06_9E) recognized=yes ;;
      #2.17# 8th-Gen Intel® Core™ i3 processors @ Cannon Lake
      (06_66) recognized=yes ;;
      #2.17# 10th-Gen Intel® Core™ processors @ Comet Lake
      (06_A5|06_A6) recognized=yes ;;
      #2.17# 10th-Gen Intel® Core™ processors @ Ice Lake
      (06_7D|06_7E) recognized=yes ;;
      #2.17# 11th-Gen Intel® Core™ processors @ Tiger Lake
      (06_8C|06_8D) recognized=yes ;;
      #2.17# 3rd-Gen Intel® Xeon® Processor Scalable Family @ Ice Lake
      (06_6A|06_6C) recognized=yes ;;
      #2.17# 12th-Gen Intel® Core™ processors @ Alder Lake
      (06_97|06_9A|06_BF) recognized=yes ;;
      (*)
        printf '[ERROR] Unsupported processor: %s\n' "${processor}"
        printf '[ERROR] Unsupported signature: %s\n' "${signature}"
    esac
    ;;
  (*) printf '[ERROR] Unsupported CPU vendor: %s\n' "${processor}"
esac

if [ -t 0 ]; then
  src_kext="${script_dir}/GoodbyeBigSlow.kext"
  if [ -e "${src_kext}" ]; then
    printf '\nContinue to install this kext ? [yes/NO] '
    read -r answer
    case "${answer}" in
      ([yY]*)
        dst_dir=/Library/Extensions
        dst_kext="${dst_dir}/GoodbyeBigSlow.kext"
        if [ -e "${dst_kext}" ]; then
          printf '[INFO] Found existing GoodbyeBigSlow.kext in /Library/Extensions\n'
          backup="${dst_kext}-backup-$$"
          printf '[INFO] Moving existing GoodbyeBigSlow.kext to %s ...\n' "${backup}"
          sudo cp -R "${dst_kext}" "${backup}"
          sudo kextunload -quiet "${dst_kext}"
          sudo rm -R "${dst_kext}"
        else
          sudo mkdir -p "${dst_dir}"
        fi
        printf '[INFO] Installing GoodbyeBigSlow.kext to /Library/Extensions ...\n'
        sudo cp -R -- "${src_kext}" "${dst_dir}"
        sudo kextload -quiet "${dst_kext}" || true
        sudo touch "${dst_dir}"
        printf 'Done.\n'
        exit 0
        ;;
      (*)
        printf 'Canceled.\n'
        exit 1
    esac
  fi
fi

if [ yes = "${recognized}" ]; then
  exit 0
else
  exit 1
fi
