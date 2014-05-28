/*****************************************************************************
 *
 * state.h -- global application state datastructures
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

#include "mainWindow.h"
#include "controlWidget.h"
#include "previewWidget.h"
#include "captureWidget.h"
#include "imageWidget.h"
#include "zoomWidget.h"
#include "histogramWidget.h"
#include "settingsWidget.h"
#include "camera.h"

typedef struct
{
  MainWindow*		mainWindow;
  ControlWidget*	controlWidget;
  PreviewWidget*	previewWidget;
  CaptureWidget*	captureWidget;
  ImageWidget*		imageWidget;
  ZoomWidget*		zoomWidget;
  CameraWidget*		cameraWidget;
  int			libavStarted; // has libav* been initialised?
  int			histogramOn;
  HistogramWidget*	histogramWidget;
  SettingsWidget*	settingsWidget;

  Camera*		camera;

  int			generalSettingsIndex;
  int			captureSettingsIndex;
  int			profileSettingsIndex;
  int			filterSettingsIndex;
  int			autorunSettingsIndex;
  int			histogramSettingsIndex;

  int			autorunEnabled;
  int			autorunRemaining;
  unsigned long		autorunStartNext;

  QString		lastRecordedFile;
  QString		currentDirectory;

} STATE;

extern STATE		state;
