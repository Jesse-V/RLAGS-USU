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
  IMU <<  atof(argv[11]),atof(argv[12]),atof(argv[13]),
          atof(argv[14]),atof(argv[15]),atof(argv[16]),
          atof(argv[17]),atof(argv[18]),atof(argv[19]);
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

double degMin2DecDeg(Degree val)
{
  double retFrac = val.min / 60;
  double retVal = val.degrees + retFrac;

  return retVal;
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

  // std::cout << "Lat Degrees: " << lat.degrees << std::endl;
  // std::cout << "Lat Min: " << lat.min << std::endl;
  // std::cout << "lon Degrees: " << lon.degrees << std::endl;
  // std::cout << "lon Min: " << lon.min << std::endl;
  // std::cout << "GMT Hr: " << gmt.hr << std::endl;
  // std::cout << "GMT Min: " << gmt.min << std::endl;
  // std::cout << "GMT Sec: " << gmt.sec << std::endl;
  // std::cout << "Date Year: " << day.year << std::endl;
  // std::cout << "Date Month: " << day.month << std::endl;
  // std::cout << "Date Day: " << day.day << std::endl;
  // std::cout << IMU << std::endl; 

  double Lon = degMin2DecDeg(lon);
  double Lat = degMin2DecDeg(lat);

  // SECTION 1: Finding the sun in Earth-Centered Inertial (ECI) coordinates

  // Converts the date to J (Julian Epoch - 2000, to within a minute or so)
  double julianDay = juliandate(day, gmt);

  double dDay = (juliandate(day, gmt) - 2456894.0000);
  double RAsun = 153.263 + dDay*0.922;                           // unit of degrees
  double DECsun = 10.991 - dDay*0.339;                           // unit of degrees
  double LST = 10.193 + dDay*0.0656 + (dDay-floor(dDay))*24 - Lon/15;  // units of hours
  double magDec = 7.72 + (Lon-104.246)*0.407;                 // units of degrees

  // convert RA, DEC, LST, and magDec to radians
  RAsun = RAsun * M_PI / 180;
  DECsun = DECsun * M_PI / 180;
  LST = LST * M_PI / 12;
  magDec = magDec * M_PI / 180;

  // construct sun vector, equatorial NED coordinate system
  Matrix_3x1d sunE;
  sunE << sin(DECsun), -cos(DECsun)*sin(LST-RAsun), -cos(DECsun)*cos(LST-RAsun);

  Matrix_3x3d Meg;
  Meg <<  cos(Lat * (M_PI / 180)),   0,  sin(Lat * (M_PI / 180)),
          0,          1,  0,
          -sin(Lat * (M_PI / 180)),  0,  cos(Lat * (M_PI / 180));

  Matrix_3x1d sunG = Meg * sunE;

  // convert the sun vector to the magnetic NED cooordinate system
  Matrix_3x3d Mgm;
  Mgm << cos(magDec), sin(magDec), 0,
        -sin(magDec), cos(magDec), 0,
         0,        0,              1;

  Matrix_3x1d sunM = Mgm * sunG;

  // Mimu is the 3x3 orientation matrix estimated by the IMU
  // (note:  this is NOT the update matrix)

  // convert the sun vector to the SEDIp sensor coordinate system
  Matrix_3x3d Mp;
  Mp << -0.9523, -0.1593, 0.2604,
         0.2963, -0.2764, 0.9143,
        -0.0737,  0.9478, 0.3104;

  Matrix_3x1d sunP = Mp * IMU * sunM;

  // calculate the polarizer rotation angle and the corresponding
  // actuator position code

  double phi = atan2(sunP[1],sunP[0])*180/M_PI;  //units of degrees
  double dPhi = fmod(167.5-phi,180);

  double CODE = 0;
  if (dPhi < 92.5)
  {
    CODE = dPhi - 2.5;
  }
  else
  {
    CODE = dPhi * 1.0588 - 7.939;
  }
  printf("%f", CODE);
}
