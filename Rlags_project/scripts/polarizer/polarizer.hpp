#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>

#include <Eigen/Dense>

#define _USE_MATH_DEFINES

using namespace Eigen;

typedef Matrix<double, 3, 1> Matrix_3x1d;
typedef Matrix<double, 3, 3> Matrix_3x3d;

struct GMT
{
  GMT(int iHr, int iMin, int iSec);

  int hr;
  int min;
  int sec;
};

struct Degree
{
  Degree(double iDeg, double iMin);

  double degrees;
  double min;
};

struct Day
{
  Day(int iYear, int iMonth, int iDay);

  int year;
  int month;
  int day;
};

double juliandate(Day day, GMT gmt);
void polarizer(Degree lat, Degree lon, /*double alt, */GMT gmt, Day day, Matrix_3x3d imu);

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}
