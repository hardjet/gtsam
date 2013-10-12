/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    timePinholeCamera.cpp
 * @brief   time PinholeCamera derivatives
 * @author  Frank Dellaert
 */

#include <time.h>
#include <iostream>

#include <gtsam/geometry/PinholeCamera.h>
#include <gtsam/geometry/Cal3_S2.h>

using namespace std;
using namespace gtsam;

int main()
{
  int n = 100000;

  const Pose3 pose1(Matrix_(3,3,
      1., 0., 0.,
      0.,-1., 0.,
      0., 0.,-1.
  ),
  Point3(0,0,0.5));

  static const Cal3_S2 K(625, 625, 0, 0, 0);
  const PinholeCamera<Cal3_S2> camera(pose1,K);
  const Point3 point1(-0.08,-0.08, 0.0);

  /**
   * NOTE: because we only have combined derivative functions now,
   * parts of this test are no longer useful.
   */

  // Oct 12 2013, iMac 3.06GHz Core i3
  //  6.78564e+06 calls/second
  //  0.14737 musecs/call
  // And after collapse:
  //  8.71916e+06 calls/second
  //  0.11469 musecs/call
  {
    long timeLog = clock();
    for(int i = 0; i < n; i++)
      camera.project(point1);
    long timeLog2 = clock();
    double seconds = (double)(timeLog2-timeLog)/CLOCKS_PER_SEC;
    cout << ((double)n/seconds) << " calls/second" << endl;
    cout << ((double)seconds*1000000/n) << " musecs/call" << endl;
  }

  // Oct 12 2013, iMac 3.06GHz Core i3
  //  258265 calls/second
  //  3.87199 musecs/call
  // And after collapse:
  //  380686 calls/second
  //  2.62684 musecs/call
  {
    Matrix Dpose, Dpoint;
    long timeLog = clock();
    for(int i = 0; i < n; i++)
      camera.project(point1, Dpose, Dpoint);
    long timeLog2 = clock();
    double seconds = (double)(timeLog2-timeLog)/CLOCKS_PER_SEC;
    cout << ((double)n/seconds) << " calls/second" << endl;
    cout << ((double)seconds*1000000/n) << " musecs/call" << endl;
  }

  // Oct 12 2013, iMac 3.06GHz Core i3
  //  249258 calls/second
  //  4.0119 musecs/call
  // And after collapse:
  //  389135 calls/second
  //  2.5698 musecs/call
  {
    Matrix Dpose, Dpoint, Dcal;
    long timeLog = clock();
    for(int i = 0; i < n; i++)
      camera.project(point1, Dpose, Dpoint, Dcal);
    long timeLog2 = clock();
    double seconds = (double)(timeLog2-timeLog)/CLOCKS_PER_SEC;
    cout << ((double)n/seconds) << " calls/second" << endl;
    cout << ((double)seconds*1000000/n) << " musecs/call" << endl;
  }

  return 0;
}
