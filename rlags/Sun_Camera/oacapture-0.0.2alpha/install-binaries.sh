#!/bin/bash
#
# install.sh -- install script for the binary distribution
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

arch=i686
if [ ! -d ./$arch ];
then
  echo "$arch is not yet supported"
  exit 1
fi

rsync -a $arch/lib/ /usr/local/openastro/lib
cp $arch/bin/oacapture /usr/local/openastro/bin

mkdir -p /usr/local/bin
ln -sf /usr/local/openastro/bin/oacapture /usr/local/bin

cp etc/70-asi-cameras.rules /etc/udev/rules.d

echo "install complete"
