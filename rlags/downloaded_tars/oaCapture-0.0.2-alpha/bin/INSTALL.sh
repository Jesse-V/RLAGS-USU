#!/bin/bash
#
# INSTALL.sh -- install script for the binary distribution
#
# Copyright 2013 James Fidell (james@openastroproject.org)
#
# License:
#
# This file is part of the Open Astro Project.
#
# The Open Astro Project is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Open Astro Project is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the Open Astro Project.  If not, see
# <http://www.gnu.org/licenses/>.
#

uid=`id -u`
if [ $uid -ne 0 ]; then
  echo "This script must be run as root"
  exit 1
fi

mkdir -p /usr/local/openastro/bin
mkdir -p /usr/local/openastro/lib

cp oacapture/oacapture /usr/local/openastro/bin

mkdir -p /usr/local/bin
ln -s /usr/local/openastro/bin/oacapture /usr/local/bin

UDEVDIR=/etc/udev/rules.d
if [ -d $UDEVDIR ]
then
  cp udev/70-asi-cameras.rules $UDEVDIR
else
  echo "$UDEVDIR not found."
  echo "If you use an ASI camera you will need to install"
  echo "udev/70-asi-cameras.rules in the appropriate place"
  exit 1
fi

echo "install complete"
