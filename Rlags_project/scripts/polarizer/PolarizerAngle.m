% lat = 41.7378;
% lon = 111.8308;
% day = [2014,8,1];
% GMT = [00,30,00];
% imu = [0,1,0;1,0,0;0,0,-1];
function [CODE] = PolarizerAngle(Lat,Lon,GMT,Day,IMU)
% RLAGS polarizer control algorithm
% A. Marchant   7/15/14

% julianDay = current Julian day
% updated as recently as possible from the GPS
% precision of dJ supports a value of 2456894.xxxx

% Long = payload longitude, units of degrees West (a positive value)
% Lat = payload latitude, units of degrees
% updated as recently as possible from the GPS
julianDay = juliandate([Day,GMT]);

dDay = julianDay - 2456894.0000;
RAsun = 153.263 + dDay*0.922;                           % unit of degrees
DECsun = 10.991 - dDay*0.339;                           % unit of degrees
LST = 10.193 + dDay*0.0656 + (dDay-floor(dDay))*24 - Lon/15;  % units of hours
magDec = 7.72 + (Lon-104.246)*0.407;                 % units of degrees

% convert RA, DEC, LST, and magDec to radians
RAsun = RAsun*pi/180;
DECsun = DECsun*pi/180;
LST = LST*pi/12;
magDec = magDec*pi/180;

% construct sun vector, equatorial NED coordinate system
sunE = [sin(DECsun); -cos(DECsun)*sin(LST-RAsun); -cos(DECsun)*cos(LST-RAsun)];

% convert the sun vector to the local geographic NED coordinate system
Meg = [cos(Lat), 0, sin(Lat); ...
       0,     1,           0; ...
      -sin(Lat), 0, cos(Lat)];
  
sunG = Meg * sunE;

% convert the sun vector to the magnetic NED cooordinate system
Mgm = [cos(magDec), sin(magDec), 0; ...
      -sin(magDec), cos(magDec), 0; ...
       0,        0,              1];
   
sunM = Mgm * sunG;

% Mimu is the 3x3 orientation matrix estimated by the IMU
% (note:  this is NOT the update matrix)

% convert the sun vector to the SEDIp sensor coordinate system
Mp = [-0.9523, -0.1593, 0.2604; ...
       0.2963, -0.2764, 0.9143; ...
      -0.0737,  0.9478, 0.3104];
sunP = Mp * IMU * sunM;

% calculate the polarizer rotation angle and the corresponding
% actuator position code
phi = atan2(sunP(1),sunP(2))*180/pi;  %units of degrees
dPhi = mod(167.5-phi,180);
if dPhi<92.5
    CODE = max(dPhi-2.5,0);
else
    CODE = min(dPhi*1.0588-7.939,180);
end
