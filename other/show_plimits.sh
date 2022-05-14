#!/bin/sh

# Copyright (c) 2022 by J.W https://github.com/jakwings/GoodbyeBigSlow.kext
#
#   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
#
#  0. You just DO WHAT THE FUCK YOU WANT TO.

set -euf; unset -v IFS; export LC_ALL=C

echo() {
  printf '%s\n' "$*"
}

# TODO: query using GoodbyeBigSlowClient instead
sysctl -A -e \
  | sed -e '/^[^=]*[Pp][Ll][Ii][Mm][Ii][Tt]/ b p' \
        -e '/^hw.busfrequency=/ b p' \
        -e d -e ':p' -e 's/=/ = /'

script_dir="$(dirname -- "$0")"

xslt() {
  xsltproc --nonet --novalid --path "${script_dir}" ${1+"$@"}
}

if data="$(ioreg -ad1 -rn IOPMrootDomain)" && [ ok = "${data:+ok}" ]; then
  ncpu="$(sysctl -n hw.logicalcpu_max 2>/dev/null || true)"
  echo "${data}" | xslt --stringparam nCPU.max "${ncpu}" \
                   "${script_dir}/show_plimits-1.xsl" - \
  || true
else
  echo '[INFO] IORegistryEntry:IOService:IOPMrootDomain not found'
fi

if data="$(ioreg -ad1 -rn X86PlatformPlugin)" && [ ok = "${data:+ok}" ]; then
  #factor="$(sysctl -n hw.busfrequency 2>/dev/null || true)"
  echo "${data}" | xslt "${script_dir}/show_plimits-2.xsl" - || true
else
  echo '[INFO] X86PlatformPlugin.kext not loaded into the kernel'
fi
