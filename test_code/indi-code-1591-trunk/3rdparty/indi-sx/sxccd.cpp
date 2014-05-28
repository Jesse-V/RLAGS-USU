/*
 Starlight Xpress CCD INDI Driver

 Copyright (c) 2012-2013 Cloudmakers, s. r. o.
 All Rights Reserved.

 Code is based on SX INDI Driver by Gerry Rozema and Jasem Mutlaq
 Copyright(c) 2010 Gerry Rozema.
 Copyright(c) 2012 Jasem Mutlaq.
 All rights reserved.

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 more details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59
 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 The full GNU General Public License is included in this distribution in the
 file called LICENSE.
 */

#include <stdio.h>
#include <unistd.h>
#include <memory.h>

#include "sxconfig.h"
#include "sxccd.h"

#define SX_GUIDE_EAST             0x08     /* RA+ */
#define SX_GUIDE_NORTH            0x04     /* DEC+ */
#define SX_GUIDE_SOUTH            0x02     /* DEC- */
#define SX_GUIDE_WEST             0x01     /* RA- */
#define SX_CLEAR_NS               0x09
#define SX_CLEAR_WE               0x06

static int count;
static SXCCD *cameras[20];

#define TIMER 1000

void cleanup() {
  for (int i = 0; i < count; i++) {
    delete cameras[i];
  }
}

void ISInit() {
  static bool isInit = false;
  if (!isInit) {
    DEVICE devices[20];
    const char* names[20];
    count = sxList(devices, names, 20);
    for (int i = 0; i < count; i++) {
      cameras[i] = new SXCCD(devices[i], names[i]);
    }
    atexit(cleanup);
    isInit = true;
  }
}

void ISGetProperties(const char *dev) {
  ISInit();
  for (int i = 0; i < count; i++) {
    SXCCD *camera = cameras[i];
    if (dev == NULL || !strcmp(dev, camera->name)) {
      camera->ISGetProperties(camera->name);
      if (dev != NULL)
        break;
    }
  }
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int num) {
  ISInit();
  for (int i = 0; i < count; i++) {
    SXCCD *camera = cameras[i];
    if (dev == NULL || !strcmp(dev, camera->name)) {
      camera->ISNewSwitch(camera->name, name, states, names, num);
      if (dev != NULL)
        break;
    }
  }
}

void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int num) {
  ISInit();
  for (int i = 0; i < count; i++) {
    SXCCD *camera = cameras[i];
    if (dev == NULL || !strcmp(dev, camera->name)) {
      camera->ISNewText(camera->name, name, texts, names, num);
      if (dev != NULL)
        break;
    }
  }
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int num) {
  ISInit();
  for (int i = 0; i < count; i++) {
    SXCCD *camera = cameras[i];
    if (dev == NULL || !strcmp(dev, camera->name)) {
      camera->ISNewNumber(camera->name, name, values, names, num);
      if (dev != NULL)
        break;
    }
  }
}

void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n) {
  INDI_UNUSED(dev);
  INDI_UNUSED(name);
  INDI_UNUSED(sizes);
  INDI_UNUSED(blobsizes);
  INDI_UNUSED(blobs);
  INDI_UNUSED(formats);
  INDI_UNUSED(names);
  INDI_UNUSED(n);
}

void ISSnoopDevice(XMLEle *root) {
  INDI_UNUSED(root);
}

void ExposureTimerCallback(void *p) {
  ((SXCCD *) p)->ExposureTimerHit();
}

void GuideExposureTimerCallback(void *p) {
  ((SXCCD *) p)->GuideExposureTimerHit();
}

void WEGuiderTimerCallback(void *p) {
  ((SXCCD *) p)->WEGuiderTimerHit();
}

void NSGuiderTimerCallback(void *p) {
  ((SXCCD *) p)->NSGuiderTimerHit();
}

SXCCD::SXCCD(DEVICE device, const char *name) {
  this->device = device;
  handle = NULL;
  model = 0;
  oddBuf = NULL;
  evenBuf = NULL;
  GuideStatus = 0;
  TemperatureRequest = 0;
  TemperatureReported = 0;
  ExposureTimeLeft = 0.0;
  GuideExposureTimeLeft = 0.0;
  HasShutter = false;
  HasCooler = false;
  HasST4Port = false;
  HasGuideHead = false;
  ExposureTimerID = 0;
  DidFlush = false;
  DidLatch = false;
  GuideExposureTimerID = 0;
  InGuideExposure = false;
  DidGuideLatch = false;
  NSGuiderTimerID = 0;
  WEGuiderTimerID = 0;
  snprintf(this->name, 32, "SX CCD %s", name);
  setDeviceName(this->name);
  setVersion(VERSION_MAJOR, VERSION_MINOR);
}

SXCCD::~SXCCD() {
  if (handle)
    sxClose(&handle);
}

void SXCCD::debugTriggered(bool enable) {
  sxDebug(enable);
}

void SXCCD::simulationTriggered(bool enable) {
  INDI_UNUSED(enable);
}

const char *SXCCD::getDefaultName() {
  return name;
}

bool SXCCD::initProperties() {
  INDI::CCD::initProperties();
  addDebugControl();
  IUFillSwitch(&CoolerS[0], "DISCONNECT_COOLER", "Off", ISS_ON);
  IUFillSwitch(&CoolerS[1], "CONNECT_COOLER", "On", ISS_OFF);
  IUFillSwitchVector(&CoolerSP, CoolerS, 2, getDeviceName(), "COOLER_CONNECTION", "Cooler", OPTIONS_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
  IUFillSwitch(&ShutterS[0], "SHUTTER_ON", "Manual open", ISS_OFF);
  IUFillSwitch(&ShutterS[1], "SHUTTER_OFF", "Manual close", ISS_ON);
  IUFillSwitchVector(&ShutterSP, ShutterS, 2, getDeviceName(), "SHUTTER_CONNECTION", "Shutter", OPTIONS_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
  return true;
}

bool SXCCD::updateProperties() {
  struct t_sxccd_params params;
  INDI::CCD::updateProperties();
  if (isConnected()) {
    if (HasCooler)
      defineSwitch(&CoolerSP);
    if (HasShutter)
      defineSwitch(&ShutterSP);
  } else {
    if (HasCooler)
      deleteProperty(CoolerSP.name);
    if (HasShutter)
      deleteProperty(ShutterSP.name);
  }
  return true;
}

bool SXCCD::UpdateCCDBin(int hor, int ver) {
  if (hor == 3 || ver == 3) {
    IDMessage(getDeviceName(), "3x3 binning is not supported.");
    return false;
  }
  PrimaryCCD.setBin(hor, ver);
  return true;
}

bool SXCCD::Connect() {
  if (handle == NULL) {
    int rc = sxOpen(device, &handle);
    if (rc >= 0) {
      getCameraParams();
      return true;
    }
  }
  return false;
}

bool SXCCD::Disconnect() {
  if (handle != NULL) {
    sxClose(&handle);
  }
  return true;
}

void SXCCD::getCameraParams() {
  struct t_sxccd_params params;
  model = sxGetCameraModel(handle);
  bool isInterlaced = sxIsInterlaced(model);
  PrimaryCCD.setInterlaced(isInterlaced);
  sxGetCameraParams(handle, 0, &params);
  if (isInterlaced) {
    params.pix_height /= 2;
    params.height *= 2;
  }
  SetCCDParams(params.width, params.height, params.bits_per_pixel, params.pix_width, params.pix_height);
  int nbuf = params.width * params.height;
  if (params.bits_per_pixel == 16)
    nbuf *= 2;
  nbuf += 512;
  PrimaryCCD.setFrameBufferSize(nbuf);
  if (evenBuf != NULL)
    delete evenBuf;
  if (oddBuf != NULL)
    delete oddBuf;
  evenBuf = new char[nbuf / 2];
  oddBuf = new char[nbuf / 2];
  HasGuideHead = params.extra_caps & SXCCD_CAPS_GUIDER;
  HasCooler = params.extra_caps & SXUSB_CAPS_COOLER;
  HasShutter = params.extra_caps & SXUSB_CAPS_SHUTTER;
  HasST4Port = params.extra_caps & SXCCD_CAPS_STAR2K;

  if (HasGuideHead) {
    GuideCCD.setInterlaced(false);
    sxGetCameraParams(handle, 1, &params);
    nbuf = params.width * params.height;
    nbuf += 512;
    GuideCCD.setFrameBufferSize(nbuf);
    SetGuiderParams(params.width, params.height, params.bits_per_pixel, params.pix_width, params.pix_height);
  }

  Capability cap;
  cap.canAbort = true;
  cap.canBin = true;
  cap.canSubFrame = true;
  cap.hasCooler = HasCooler;
  cap.hasGuideHead = HasGuideHead;
  cap.hasShutter = HasShutter;
  cap.hasST4Port = HasST4Port;
  SetCapability(&cap);

  SetTimer(TIMER);
}

void SXCCD::TimerHit() {
  if (isConnected() && HasCooler) {
    if (!DidLatch && !DidGuideLatch) {
      unsigned char status;
      unsigned short temperature;
      sxSetCooler(handle, (unsigned char) (CoolerS[1].s == ISS_ON), (unsigned short) (TemperatureRequest * 10 + 2730), &status, &temperature);
      TemperatureN[0].value = (temperature - 2730) / 10.0;
      if (TemperatureReported != TemperatureN[0].value) {
        TemperatureReported = TemperatureN[0].value;
        if (abs(TemperatureRequest - TemperatureReported) < 1)
          TemperatureNP.s = IPS_OK;
        else
          TemperatureNP.s = IPS_BUSY;
        IDSetNumber(&TemperatureNP, NULL);
      }
    }
  }
  if (InExposure && ExposureTimeLeft >= 0)
    PrimaryCCD.setExposureLeft(ExposureTimeLeft--);
  if (InGuideExposure && GuideExposureTimeLeft >= 0)
    GuideCCD.setExposureLeft(GuideExposureTimeLeft--);
  if (isConnected())
    SetTimer(TIMER);
}

int SXCCD::SetTemperature(double temperature) {
    int result=0;
    TemperatureRequest = temperature;
    unsigned char status;
    unsigned short sx_temperature;
    sxSetCooler(handle, (unsigned char) (CoolerS[1].s == ISS_ON), (unsigned short) (TemperatureRequest * 10 + 2730), &status, &sx_temperature);
    TemperatureReported = TemperatureN[0].value = (sx_temperature - 2730) / 10.0;
    if (abs(TemperatureRequest - TemperatureReported) < 1)
      result = 1;
    else
      result = 0;

    CoolerSP.s = IPS_OK;
    CoolerS[0].s = ISS_OFF;
    CoolerS[1].s = ISS_ON;
    IDSetSwitch(&CoolerSP, NULL);

    return result;
}

bool SXCCD::StartExposure(float n) {
  InExposure = true;
  PrimaryCCD.setExposureDuration(n);
  if (PrimaryCCD.isInterlaced() && PrimaryCCD.getBinY() == 1) {
    sxClearPixels(handle, CCD_EXP_FLAGS_FIELD_EVEN, 0);
    usleep(100);
    sxClearPixels(handle, CCD_EXP_FLAGS_FIELD_ODD, 0);
  } else
    sxClearPixels(handle, CCD_EXP_FLAGS_FIELD_BOTH, 0);
  if (HasShutter)
    sxSetShutter(handle, 0);
  int time = (int) (1000 * n);
  if (time < 1)
    time = 1;
  if (time > 3000) {
    DidFlush = false;
    time -= 3000;
  } else
    DidFlush = true;
  DidLatch = false;
  ExposureTimeLeft = n;
  ExposureTimerID = IEAddTimer(time, ExposureTimerCallback, this);
  return true;
}

bool SXCCD::AbortExposure() {
  if (InExposure) {
    if (ExposureTimerID)
      IERmTimer(ExposureTimerID);
    if (HasShutter)
      sxSetShutter(handle, 1);
    ExposureTimerID = 0;
    PrimaryCCD.setExposureLeft(ExposureTimeLeft = 0);
    DidLatch = false;
    DidFlush = false;
    return true;
  }
  return false;
}

void SXCCD::ExposureTimerHit() {
  if (InExposure) {
    if (!DidFlush) {
      ExposureTimerID = IEAddTimer(3000, ExposureTimerCallback, this);
      sxClearPixels(handle, CCD_EXP_FLAGS_NOWIPE_FRAME, 0);
      DidFlush = true;
    } else {
      int rc;
      ExposureTimerID = 0;
      bool isInterlaced = PrimaryCCD.isInterlaced();
      int subX = PrimaryCCD.getSubX();
      int subY = PrimaryCCD.getSubY();
      int subW = PrimaryCCD.getSubW();
      int subH = PrimaryCCD.getSubH();
      int binX = PrimaryCCD.getBinX();
      int binY = PrimaryCCD.getBinY();
      int subWW = subW * 2;
      char *buf = PrimaryCCD.getFrameBuffer();
      int size;
      if (isInterlaced && binY > 1)
        size = subW * subH / 2 / binX / (binY / 2);
      else
        size = subW * subH / binX / binY;
      if (HasShutter)
        sxSetShutter(handle, 1);
      DidLatch = true;
      if (isInterlaced) {
        if (binY > 1) {
          rc = sxLatchPixels(handle, CCD_EXP_FLAGS_FIELD_BOTH, 0, subX, subY, subW, subH / 2, binX, binY / 2);
          if (rc)
            rc = sxReadPixels(handle, buf, size * 2);
        } else {
          rc = sxLatchPixels(handle, CCD_EXP_FLAGS_FIELD_EVEN | CCD_EXP_FLAGS_SPARE2, 0, subX, subY / 2, subW, subH / 2, binX, 1);
          if (rc)
            rc = sxReadPixels(handle, evenBuf, size);
          if (rc)
            rc = sxLatchPixels(handle, CCD_EXP_FLAGS_FIELD_ODD | CCD_EXP_FLAGS_SPARE2, 0, subX, subY / 2, subW, subH / 2, binX, 1);
          if (rc)
            rc = sxReadPixels(handle, oddBuf, size);
          if (rc)
            for (int i = 0, j = 0; i < subH; i += 2, j++) {
              memcpy(buf + i * subWW, evenBuf + (j * subWW), subWW);
              memcpy(buf + ((i + 1) * subWW), oddBuf + (j * subWW), subWW);
            }
        }
      } else {
        rc = sxLatchPixels(handle, CCD_EXP_FLAGS_FIELD_BOTH, 0, subX, subY, subW, subH, binX, binY);
        if (rc)
          rc = sxReadPixels(handle, buf, size * 2);
      }
      DidLatch = false;
      InExposure = false;
      PrimaryCCD.setExposureLeft(ExposureTimeLeft = 0);
      if (rc)
        ExposureComplete (&PrimaryCCD);
    }
  }
}

bool SXCCD::StartGuideExposure(float n) {
  InGuideExposure = true;
  GuideCCD.setExposureDuration(n);
  sxClearPixels(handle, CCD_EXP_FLAGS_FIELD_BOTH, 1);
  int time = (int) (1000 * n);
  if (time < 1)
    time = 1;
  ExposureTimeLeft = n;
  GuideExposureTimerID = IEAddTimer(time, GuideExposureTimerCallback, this);
  return true;
}

bool SXCCD::AbortGuideExposure() {
  if (InGuideExposure) {
    if (GuideExposureTimerID)
      IERmTimer(GuideExposureTimerID);
    GuideCCD.setExposureLeft(GuideExposureTimeLeft = 0);
    GuideExposureTimerID = 0;
    DidGuideLatch = false;
    return true;
  }
  return false;
}

void SXCCD::GuideExposureTimerHit() {
  if (InGuideExposure) {
    int rc;
    GuideExposureTimerID = 0;
    int subX = GuideCCD.getSubX();
    int subY = GuideCCD.getSubY();
    int subW = GuideCCD.getSubW();
    int subH = GuideCCD.getSubH();
    int binX = GuideCCD.getBinX();
    int binY = GuideCCD.getBinY();
    int size = subW * subH / binX / binY;
    char *buf = GuideCCD.getFrameBuffer();
    DidGuideLatch = true;
    rc = sxLatchPixels(handle, CCD_EXP_FLAGS_FIELD_BOTH, 1, subX, subY, subW, subH, binX, binY);
    if (rc)
      rc = sxReadPixels(handle, buf, size);
    DidGuideLatch = false;
    InGuideExposure = false;
    GuideCCD.setExposureLeft(GuideExposureTimeLeft = 0);
    if (rc)
      ExposureComplete (&GuideCCD);
  }
}

bool SXCCD::GuideWest(float time) {
  if (!HasST4Port || time < 1) {
    return false;
  }
  if (WEGuiderTimerID) {
    IERmTimer(WEGuiderTimerID);
    WEGuiderTimerID = 0;
  }
  GuideStatus &= SX_CLEAR_WE;
  GuideStatus |= SX_GUIDE_WEST;
  sxSetSTAR2000(handle, GuideStatus);
  if (time < 100) {
    usleep(time * 1000);
    GuideStatus &= SX_CLEAR_WE;
    sxSetSTAR2000(handle, GuideStatus);
  } else
    WEGuiderTimerID = IEAddTimer(time, WEGuiderTimerCallback, this);
  return true;
}

bool SXCCD::GuideEast(float time) {
  if (!HasST4Port || time < 1) {
    return false;
  }
  if (WEGuiderTimerID) {
    IERmTimer(WEGuiderTimerID);
    WEGuiderTimerID = 0;
  }
  GuideStatus &= SX_CLEAR_WE;
  GuideStatus |= SX_GUIDE_EAST;
  sxSetSTAR2000(handle, GuideStatus);
  if (time < 100) {
    usleep(time * 1000);
    GuideStatus &= SX_CLEAR_WE;
    sxSetSTAR2000(handle, GuideStatus);
  } else
    WEGuiderTimerID = IEAddTimer(time, WEGuiderTimerCallback, this);
  return true;
}

void SXCCD::WEGuiderTimerHit() {
  GuideStatus &= SX_CLEAR_WE;
  sxSetSTAR2000(handle, GuideStatus);
  WEGuiderTimerID = 0;
}

bool SXCCD::GuideNorth(float time) {
  if (!HasST4Port || time < 1) {
    return false;
  }
  if (NSGuiderTimerID) {
    IERmTimer(NSGuiderTimerID);
    NSGuiderTimerID = 0;
  }
  GuideStatus &= SX_CLEAR_NS;
  GuideStatus |= SX_GUIDE_NORTH;
  sxSetSTAR2000(handle, GuideStatus);
  if (time < 100) {
    usleep(time * 1000);
    GuideStatus &= SX_CLEAR_NS;
    sxSetSTAR2000(handle, GuideStatus);
  } else
    NSGuiderTimerID = IEAddTimer(time, NSGuiderTimerCallback, this);
  return true;
}

bool SXCCD::GuideSouth(float time) {
  if (!HasST4Port || time < 1) {
    return false;
  }
  if (NSGuiderTimerID) {
    IERmTimer(NSGuiderTimerID);
    NSGuiderTimerID = 0;
  }
  GuideStatus &= SX_CLEAR_NS;
  GuideStatus |= SX_GUIDE_SOUTH;
  sxSetSTAR2000(handle, GuideStatus);
  if (time < 100) {
    usleep(time * 1000);
    GuideStatus &= SX_CLEAR_NS;
    sxSetSTAR2000(handle, GuideStatus);
  } else
    NSGuiderTimerID = IEAddTimer(time, NSGuiderTimerCallback, this);
  return true;
}

void SXCCD::NSGuiderTimerHit() {
  GuideStatus &= SX_CLEAR_NS;
  sxSetSTAR2000(handle, GuideStatus);
  NSGuiderTimerID = 0;
}

void SXCCD::ISGetProperties(const char *dev) {
  INDI::CCD::ISGetProperties(name);
  addDebugControl();
}

bool SXCCD::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) {
  bool result = false;
  if (strcmp(name, ShutterSP.name) == 0) {
    IUUpdateSwitch(&ShutterSP, states, names, n);
    ShutterSP.s = IPS_OK;
    IDSetSwitch(&ShutterSP, NULL);
    sxSetShutter(handle, ShutterS[0].s != ISS_ON);
    result = true;
  } else if (strcmp(name, CoolerSP.name) == 0) {
    IUUpdateSwitch(&CoolerSP, states, names, n);
    CoolerSP.s = IPS_OK;
    IDSetSwitch(&CoolerSP, NULL);
    unsigned char status;
    unsigned short temperature;
    sxSetCooler(handle, (unsigned char) (CoolerS[1].s == ISS_ON), (unsigned short) (TemperatureRequest * 10 + 2730), &status, &temperature);
    TemperatureReported = TemperatureN[0].value = (temperature - 2730) / 10.0;
    TemperatureNP.s = IPS_OK;
    IDSetNumber(&TemperatureNP, NULL);
    result = true;
  } else
    result = INDI::CCD::ISNewSwitch(dev, name, states, names, n);
  return result;
}

