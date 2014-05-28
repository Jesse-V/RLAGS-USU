/*****************************************************************************
 *
 * config.h -- declaration of data structures for configuration data
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

#pragma once

typedef struct {
  QString       profileName;
  int           sixteenBit;
  int           binning2x2;
  int           useROI;
  int           imageSizeX;
  int           imageSizeY;
  int           gainValue;
  int           gammaValue;
  int           brightnessValue;
  int           exposureValue;
  int           exposureAbsoluteValue;
  int           frameRateNumerator;
  int           frameRateDenominator;
  int           fileTypeOption;
  int           filterOption;
  int           limitEnabled;
  int           limitValue;
  QString       fileNameTemplate;
} PROFILE;


// overkill, but i may want to expand this later
typedef struct {
  QString	filterName;
} FILTER;


typedef struct
{
  // general
  int			saveSettings;

  // settings from device menu
  int			cameraDevice;

  // options menu
  int			showHistogram;
  int			autoAlign;
  int			alignBox;
  int			autoGuide;
  int			showReticle;
  int			cutout;
  int			focusAid;
  int			darkFrame;
  int			derotate;
  int			flipX;
  int			flipY;
  int			debayer;

  // camera config
  int			sixteenBit;
  int			binning2x2;

  // image config
  int			useROI;
  int			imageSizeX;
  int			imageSizeY;

  // zoom config
  int			zoomButton1Option;
  int			zoomButton2Option;
  int			zoomButton3Option;
  int			zoomValue;

  // control config
  int			gainValue;
  int			exposureValue;
  int			exposureAbsoluteValue;
  int			gammaValue;
  int			brightnessValue;
  int			exposureMenuOption;
  int			frameRateNumerator;
  int			frameRateDenominator;

  // capture config
  int			profileOption;
  int			filterOption;
  int			fileTypeOption;
  int			limitEnabled;
  int			limitValue;
  QString		fileNameTemplate;
  QString		captureDirectory;
  int			autorunCount;
  int			autorunDelay;

  // display config
  int			preview;
  int			nightMode;
  int			displayFPS;

  // histogram config
  int			splitHistogram;

  // saved profiles
  int			numProfiles;
  QList<PROFILE>	profiles;

  // saved profiles
  int			numFilters;
  QList<FILTER>		filters;
} CONFIG;

extern CONFIG		config;
