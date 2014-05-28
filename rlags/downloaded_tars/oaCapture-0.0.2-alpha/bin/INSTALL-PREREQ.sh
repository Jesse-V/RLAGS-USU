#!/bin/bash
#
# INSTALL-PREREQ.sh -- install script for prerequisites
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

id=`id -u`
if [ $id -ne 0 ]
then
  echo "This script should be runs as root"
  exit 1
fi

if [ ! -d ./thirdparty/ffmpeg/ffmpeg-1.2.4 ]
then
  echo "ffmpeg directory doesn't appear to exist"
  exit 1
fi
if [ ! -d ./thirdparty/asi/lib ]
then
  echo "ASI SDK directory doesn't appear to exist"
  exit 1
fi

cd ./thirdparty/ffmpeg/ffmpeg-1.2.4
make install
cd ../../..

cd ./thirdparty/asi
arch=`uname -p`
archdir=$arch
if [ $arch == 'x86_64' ]
then
  archdir=x64
fi
if [ ! -d ./lib/$archdir ];
then
  echo "$arch is not yet supported"
  exit 1
fi

mkdir -p /usr/local/openastro/bin
mkdir -p /usr/local/openastro/lib

cp ./lib/$archdir/* /usr/local/openastro/lib
chmod a+r /usr/local/openastro/lib/*

echo "Now build the binaries and install them"
