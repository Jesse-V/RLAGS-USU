#include "polarizer.hpp"

using namespace Eigen;

int main(int argc, char const *argv[])
{
  if(argc != 20)
  {
    printf("polarizer [Year][Month][Day][Hour][Min][Sec][LatDeg][LatMin][LonDeg][LonMin][IMU00][IMU01][IMU02][IMU10][IMU11][IMU12][IMU20][IMU21][IMU22]\n");
    return 1;
  }

  GMT gmt(atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
  Day date(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
  Degree lat(atoi(argv[7]), atof(argv[8]));
  Degree lon(atoi(argv[9]), atof(argv[10]));
  Matrix_3x3d IMU;
  IMU <<  atoi(argv[11]),atoi(argv[12]),atoi(argv[13]),
          atoi(argv[14]),atoi(argv[15]),atoi(argv[16]),
          atoi(argv[17]),atoi(argv[18]),atoi(argv[19]);
  // GMT gmt(10, 10, 10);
  // Day date(2014, 06, 18);
  // Degree lat(50, 30.5);
  // Degree lon(30, 22.5);
  // Matrix_3x3d IMU;
  // IMU << 1,0,0,0,1,0,0,0,1;

  polarizer(lat, lon, gmt, date, IMU);

  return 0;
}

GMT::GMT(int iHr, int iMin, int iSec)
: hr(iHr),
  min(iMin),
  sec(iSec)
  {
    //Empty
  }

Degree::Degree(double iDeg, double iMin)
: degrees(iDeg),
  min(iMin)
  {
    //Empty
  }

Day::Day(int iYear, int iMonth, int iDay)
: year(iYear),
  month(iMonth),
  day(iDay)
{
  //Empty
}

double juliandate(Day date, GMT gmt)
{
  double tm = gmt.hr + (((double)gmt.min + ((double)gmt.sec) / 60) / 60);
  int sign = sgn(100 * date.year + date.month - 190002.5);
  double JD = (367 * date.year) - trunc((7 * ((double)date.year + (((double)date.month + 9) / 12))) / 4)
    + trunc((275 * (double)date.month) / 9) + date.day + 1721013.5 + tm / 24 - 0.5 * sign + 0.5;

  return JD;
}

// double juliandate(Day date, GMT gmt)
// {
//    long int jd12h;

//    double tjd;

//    int year = date.year;
//    int month = date.month;
//    int day = date.day;
//    double hour = gmt.hr + (((double)gmt.min + ((double)gmt.sec) / 60) / 60);
//    int minute = gmt.min;
//    int sec = gmt.sec;

//    jd12h = (long) day - 32075L + 1461L * ((long) year + 4800L
//       + ((long) month - 14L) / 12L) / 4L
//       + 367L * ((long) month - 2L - ((long) month - 14L) / 12L * 12L)
//       / 12L - 3L * (((long) year + 4900L + ((long) month - 14L) / 12L)
//       / 100L) / 4L;
//    tjd = (double) jd12h - 0.5 + hour / 24.0;

//    std::cout << tjd << std::endl;

//    return (tjd);
// }


void polarizer(Degree lat, Degree lon, /*double alt, */GMT gmt, Day day, Matrix_3x3d IMU)
{

  // SECTION 1: Finding the sun in Earth-Centered Inertial (ECI) coordinates

  // Converts the date to J (Julian Epoch - 2000, to within a minute or so)
  double J = ((juliandate(day, gmt) - 2451545) / 365.25);

  // Mean longitude of Sun (L), corrected for aberration:
  double L_raw = 4.894961213 + 628.3319706889 * J;

  // Mean anomaly (A)
  double A_raw = (6.240035939 + 628.301956 * J);

  // Put L and A in the range [0,2 M_PI) by adding multiples of 2 M_PI
  int int_L= floor(L_raw / 2 / M_PI);
  double L = (L_raw - (int_L * 2 * M_PI));

  int int_A = floor(A_raw / 2 / M_PI);
  double A = (A_raw - (int_A * 2 * M_PI));

  // Ecliptic longitude (lambda):
  double lambda_raw = L + 0.033417234 * sin(A) + 0.00034897235 * sin(2 * A);
  int int_lambda = floor(lambda_raw / (2 * M_PI));
  double lambda = lambda_raw - 2 * M_PI * int_lambda;

  // Obliquity of ecliptic (epsilon);
  double epsilon = (0.4090928 - 0.000226966 * J);

  // Equatorial rectangular coordinates  of the Sun, proportional:
  double x = cos(lambda);
  double y = cos(epsilon) * sin(lambda);
  double z = sin(epsilon) * sin(lambda);

  // Vector format
  Matrix_3x1d ECI;
  ECI << x, y, z;

// Section 2: Conversion from ECI to East-North-Up (ENU) coordinates

// Generate Local Standard Time in radians (LRT)
//       Assumption: GMT is in [hh,mm,ss] format.
  // double GRT = (((double)M_PI / 12) * 12) + (((double)M_PI / 720) * 0) + (((double)M_PI / 43200) * 0);
  double GRT = (((double)M_PI / 12) * gmt.hr) + (((double)M_PI / 720) * gmt.min) + (((double)M_PI / 43200) * gmt.sec);

// Convert lat and lon into radians

  double latR = ((M_PI/180) * lat.degrees) + ((M_PI/10800) * lat.min);
  double lonR = ((M_PI/180) * lon.degrees) + ((M_PI/10800) * lon.min);

// Convert GRT to LRT (takes values from 0 to 4pi)
//       Assumption: it's okay that this goes up to 4pi
  double LRT = GRT + lonR;

// Define Rotation Matrix
  Matrix_3x3d ECI2ENU;
  ECI2ENU <<  -sin(LRT),           cos(LRT),            0,
              -sin(latR)*cos(LRT), -sin(latR)*sin(LRT), cos(latR),
              cos(latR)*cos(LRT),  cos(latR)*sin(LRT),  sin(latR);


// Coordinate change
  Matrix_3x1d ENU = ECI2ENU * ECI;



// Section 3: Conversion from ENU to the IMU's local coordinate system
// (Body) and from Body to "sun from SEDI" (SFS) coordinates.

// The IMU provides this rotation matrix. Plug it in, plug it in.
  Matrix_3x1d Body = IMU*ENU;

// We've calculated the rotation matrix from body to SFS.

  Matrix_3x3d Body2SFS;

  Body2SFS << -.9380 ,  .0814 ,  .3370,
              -.0865 , -.9963 ,   0,
              -.3357 , -.0291 ,  .9415;

// Method, for debugging, if necessary:
// FOV2 is the azimuth, inclination of field of view 2 in the local
// coordinate system: FOV2 = [.0866,.3437];
//
// The rotation matrix for azimuth, inclination changes is
// Body2SFS = [ cos(FOV2(2))*cos(FOV2(1)), cos(FOV2(2))*sin(FOV2(1)), sin(FOV2(2)) ;
//                          -sin(FOV2(1)),              cos(FOV2(1)),      0       ;
//             -sin(FOV2(2))*cos(FOV2(1)),-sin(FOV2(2))*sin(FOV2(1)), cos(FOV2(2))];

// Coordinate change
  Matrix_3x1d SFS = Body2SFS*Body;

// Section 4: Generating the command for the filter Servo (DOT)

// Projection of SFS into xz plane, switch to polar coordinates, getting
// filter angle in degrees
//       Assumption: The Field of View vector is parallel with the y axis.

  double theta1 = atan(SFS[2]/SFS[0]);
  double theta2 = 0;
  if (theta1 >= 0)
    theta2 = theta1;
  else
    theta2 = theta1 + M_PI;
  double DOT = 180/M_PI*(M_PI-theta2);

  printf("%f", DOT);
}
