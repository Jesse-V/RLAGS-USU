#include <iostream>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <valarray>

#include <Eigen/Dense>

#define _USE_MATH_DEFINES

using namespace Eigen;

typedef Matrix<float, 1, 3> Matrix_1x3f;

struct GMT
{
  int hrs;
  int min;
  int sec;
};

struct Degree
{
  float degrees;
  float min;
};

struct IMU
{
  float matrix[3][3];
};

int juliandate(int day, GMT gmt)
{
return 2456826;
}


void polarizer(/*Degree lat, Degree lon, float alt, */GMT gmt, int day/*, IMU imu*/)
{

  // SECTION 1: Finding the sun in Earth-Centered Inertial (ECI) coordinates

  // Converts the date to J (Julian Epoch - 2000, to within a minute or so)
  int J = (juliandate(day, gmt) - 2451545) / 365.25;

  // Mean longitude of Sun (L), corrected for aberration: 
  float L_raw = 4.894961213 + 628.3319706889 * J;

  // Mean anomaly (A)
  float A_raw = (6.240035939 + 628.301956 * J);

  // Put L and A in the range [0,2 M_PI) by adding multiples of 2 M_PI
  int int_L= floor(L_raw / 2 / M_PI);
  float L = (L_raw - (int_L * 2 * M_PI));

  int int_A = floor(A_raw / 2 / M_PI);
  float A = (A_raw - (int_A * 2 * M_PI));

  // Ecliptic longitude (lambda):
  float lambda_raw = L + 0.033417234 * sin(A) + 0.00034897235 * sin(2 * A);
  int int_lambda = floor(lambda_raw / (2 * M_PI));
  float lambda = lambda_raw - 2 * M_PI * int_lambda;

  // Obliquity of ecliptic (epsilon);
  float epsilon = (0.4090928 - 0.000226966 * J);

  // Equatorial rectangular coordinates  of the Sun, proportional:
  float x = cos(lambda);    
  float y = cos(epsilon) * sin(lambda);   
  float z = sin(epsilon) * sin(lambda);

  // Vector format
  Matrix_1x3f ECI;
  ECI << x, y, z;
  std::cout << ECI << std::endl;
  // Normalization
  ECI.normalize();

  std::cout << ECI << std::endl;


// // Section 2: Conversion from ECI to East-North-Up (ENU) coordinates

// // Generate Local Standard Time in radians (LRT)
// //       Assumption: GMT is in [hh,mm,ss] format.
//   GRT = sum([pi / 12, pi / 720, pi / 43200].*GMT);

// // Convert lat and lon into radians

//   latR = sum([pi/180,pi/10800].*lat);
//   lonR = sum([pi/180,pi/10800].*lon);

// // Convert GRT to LRT (takes values from 0 to 4pi)
// //       Assumption: it's okay that this goes up to 4pi
//   LRT = GRT + lonR;

// // Define Rotation Matrix
//   ECI2ENU = [           -sin(LRT) ,            cos(LRT) ,       0       ;
//     -sin(latR)*cos(LRT) , -sin(latR)*sin(LRT) , cos(latR)     ;
//     cos(latR)*cos(LRT) ,  cos(latR)*sin(LRT) , sin(latR)   ] ;

// // Coordinate change
//   ENU = ECI2ENU * ECI;



// // Section 3: Conversion from ENU to the IMU's local coordinate system
// // (Body) and from Body to "sun from SEDI" (SFS) coordinates.

// // The IMU provides this rotation matrix. Plug it in, plug it in.
//   Body = IMU*ENU;

// // We've calculated the rotation matrix from body to SFS.

//   Body2SFS = [ -.9380 ,  .0814 ,  .3370 ;
//     -.0865 , -.9963 ,   0    ;
//     -.3357 , -.0291 ,  .9415 ] ;

// // Method, for debugging, if necessary:
// // FOV2 is the azimuth, inclination of field of view 2 in the local
// // coordinate system: FOV2 = [.0866,.3437];
// // 
// // The rotation matrix for azimuth, inclination changes is 
// // Body2SFS = [ cos(FOV2(2))*cos(FOV2(1)), cos(FOV2(2))*sin(FOV2(1)), sin(FOV2(2)) ;
// //                          -sin(FOV2(1)),              cos(FOV2(1)),      0       ;
// //             -sin(FOV2(2))*cos(FOV2(1)),-sin(FOV2(2))*sin(FOV2(1)), cos(FOV2(2))];

// // Coordinate change
//   SFS = Body2SFS*Body;



// // Section 4: Generating the command for the filter Servo (DOT)

// // Projection of SFS into xz plane, switch to polar coordinates, getting
// // filter angle in degrees
// //       Assumption: The Field of View vector is parallel with the y axis.

//   theta1 = atan(SFS(3)/SFS(1));
//   if theta1 >= 0
//     theta2 = theta1;
//   else
//     theta2 = theta1 + pi;
//   end
//   DOT = 180/pi*(pi-theta2);

}


int main(){

GMT tmp;

polarizer(tmp, 1);

  return 0;
}