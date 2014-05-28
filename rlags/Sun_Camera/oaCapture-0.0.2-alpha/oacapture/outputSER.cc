/*****************************************************************************
 *
 * outputSER.cc -- SER output class
 *
 * Copyright 2013 James Fidell (james@openastroproject.org)
 *
 * License:
 *
 * This file is part of the Open Astro Project.
 *
 * The Open Astro Project is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Open Astro Project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Open Astro Project.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <unistd.h>
#include <errno.h>

#include "outputHandler.h"
#include "outputSER.h"
#include "config.h"
#include "state.h"

OutputSER::OutputSER ( int x, int y, int n, int d ) :
    OutputHandler ( x, y, n, d )
{
}


OutputSER::~OutputSER()
{
}


int
OutputSER::outputExists ( void )
{
  return 0;
}


int
OutputSER::outputWritable ( void )
{
  return 0;
}


int
OutputSER::openOutput ( void )
{
  return 0;
}


int
OutputSER::addFrame ( void* frame )
{
  return 0;
}


void
OutputSER::closeOutput ( void )
{
}
