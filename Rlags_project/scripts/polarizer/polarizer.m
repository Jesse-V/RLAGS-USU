function [SFS,DOT] = polarizer (lat,lon,GMT,day,IMU)
% Input is
%                     -----FROM GPS-----
%          GMT is Greenwich Mean Time, an output of the GPS.
%              Format for algorithm: [hh,mm,ss]
%          lat is latitude, an output of the GPS.
%              Format for algorithm: [Degrees,Minutes(with decimals)]
%          lon is longitude, an output of the GPS.
%              Format for algorithm: [Degrees,Minutes(with decimals)]
%          alt is altitude, an output (probably unused) of the GPS.
%              Format for algorithm: Speed of an unladen African Swallow
%              times the acreage of the Vatican divided by the duration of
%              an elepant's sneeze with units furlongs.
%     Format from GPS: HH:MM:SSLLLLLmmm.mLLLLLmmm.maaaaaaaa and then stuff
%     we're not interested in. See p10 of "Consolidated Instrument Package
%     (CIP) Interface User Handbook" in the white folder. (The order is
%     Time, Latitude, Longitude, altitude, stuff.)
%                  -----FROM SOMEWHERE-----
%          day is the day the majority of the flight will take place, to be
%              manually input immediately pre-flight.
%              Format for algorithm: [yyyy,mm,dd]
%     Format from wherever: It'd be cool if we could get the date (and
%     the time) somehow. The Odroid has been mentioned as a possibility if
%     it can handle a hard reset.
%                     -----FROM IMU-----
%          IMU is M, an output of the IMU, a 9 component coordinate
%              transformation matrix which describes the orientation of the
%              IMU with respect to the fixed earth coordinate system. 
%              M*S(ENU) = S(Body) with S(ENU) a vector in East-North-Up and
%              S(Body) in the IMU's local coordinate system
%              Format for algorithm: 3 by 3 matrix, in the way it comes in.
%     Format from IMU (quoted from its manual in dropbox): 9 element array
%     of 32bit floating point values in IEEE-754 format. The elements are
%     arranged in Row major order: (M11,M12,M13,M21,...,M32,M33).

% SECTION 1: Finding the sun in Earth-Centered Inertial (ECI) coordinates

% Converts the date to J (Julian Epoch - 2000, to within a minute or so)
J = (juliandate([day,GMT]) - 2451545)/365.25;
% vpa(J)

% Mean longitude of Sun (L), corrected for aberration: 
L_raw=4.894961213+628.3319706889*J;

% Mean anomaly (A)
A_raw= (6.240035939 + 628.301956*J);

% Put L and A in the range [0,2pi) by adding multiples of 2pi
int_L= floor(L_raw/2/pi);
L = (L_raw-(int_L*2*pi));

int_A= floor(A_raw/2/pi);
A = (A_raw-(int_A*2*pi));

% Ecliptic longitude (lambda):
lambda_raw = L + 0.033417234*sin(A) + 0.00034897235*sin(2*A);
int_lambda= floor(lambda_raw/(2*pi));
lambda=lambda_raw-2*pi*int_lambda;

% Obliquity of ecliptic (epsilon);
epsilon = (0.4090928 - 0.000226966*J);

% Equatorial rectangular coordinates  of the Sun, proportional:
x = cos(lambda);		
y = cos(epsilon)*sin(lambda);		
z = sin(epsilon)*sin(lambda);

% Vector format
ECI = [x;y;z];

% Section 2: Conversion from ECI to East-North-Up (ENU) coordinates

% Generate Local Standard Time in radians (LRT)
%       Assumption: GMT is in [hh,mm,ss] format.
GRT = sum([pi/12, pi/720, pi/43200].*GMT);

% Convert lat and lon into radians

latR = sum([pi/180,pi/10800].*lat);
lonR = sum([pi/180,pi/10800].*lon);
  
% Convert GRT to LRT (takes values from 0 to 4pi)
%       Assumption: it's okay that this goes up to 4pi
LRT = GRT + lonR;

% Define Rotation Matrix
ECI2ENU = [           -sin(LRT) ,            cos(LRT) ,       0       ;
            -sin(latR)*cos(LRT) , -sin(latR)*sin(LRT) , cos(latR)     ;
             cos(latR)*cos(LRT) ,  cos(latR)*sin(LRT) , sin(latR)   ] ;
         
% Coordinate change
ENU = ECI2ENU * ECI;



% Section 3: Conversion from ENU to the IMU's local coordinate system
% (Body) and from Body to "sun from SEDI" (SFS) coordinates.

% The IMU provides this rotation matrix. Plug it in, plug it in.
Body = IMU*ENU;

% We've calculated the rotation matrix from body to SFS.

Body2SFS = [ -.9380 ,  .0814 ,  .3370 ;
             -.0865 , -.9963 ,   0    ;
             -.3357 , -.0291 ,  .9415 ] ;

% Method, for debugging, if necessary:
% FOV2 is the azimuth, inclination of field of view 2 in the local
% coordinate system: FOV2 = [.0866,.3437];
% 
% The rotation matrix for azimuth, inclination changes is 
% Body2SFS = [ cos(FOV2(2))*cos(FOV2(1)), cos(FOV2(2))*sin(FOV2(1)), sin(FOV2(2)) ;
%                          -sin(FOV2(1)),              cos(FOV2(1)),      0       ;
%             -sin(FOV2(2))*cos(FOV2(1)),-sin(FOV2(2))*sin(FOV2(1)), cos(FOV2(2))];

% Coordinate change
SFS = Body2SFS*Body

% Section 4: Generating the command for the filter Servo (DOT)

% Projection of SFS into xz plane, switch to polar coordinates, getting
% filter angle in degrees
%       Assumption: The Field of View vector is parallel with the y axis.

theta1 = atan(SFS(3)/SFS(1));
if theta1 >= 0
    theta2 = theta1;
else
    theta2 = theta1 + pi;
end
DOT = 180/pi*(pi-theta2)