#!/bin/bash
#
# BUILD-PREREQ.sh -- Build script for prerequisites
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

mkdir -p ./thirdparty/ffmpeg
cd ./thirdparty/ffmpeg
if [ ! -f ffmpeg-1.2.4.tar.bz2 ]
then
  wget http://ffmpeg.org/releases/ffmpeg-1.2.4.tar.bz2
fi
if [ ! -d ffmpeg-1.2.4 ]
then
  tar jxf ffmpeg-1.2.4.tar.bz2
fi
cd ffmpeg-1.2.4
./configure --prefix=/usr/local/openastro --enable-memalign-hack --enable-static --enable-shared --disable-debug --disable-programs --disable-network --disable-encoders --enable-encoder=utvideo --enable-encoder=rawvideo --disable-doc --disable-muxers --enable-muxer=avi --disable-bsfs --disable-indevs --disable-protocols --enable-protocol=file
make all
cd ../../..

mkdir -p ./thirdparty/asi
cd ./thirdparty/asi
if [ ! -f 'asicamera linux sdk 0914.zip' ]
then
  wget 'http://www.zwoptical.com/software/asicamera%20linux%20sdk%200914.zip'
fi
unzip -q -u -o 'asicamera linux sdk 0914.zip'

echo "now run bin/INSTALL-PREREQ.sh as root"
