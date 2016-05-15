/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    testPreintegrationBase.cpp
 * @brief   Unit test for the InertialNavFactor
 * @author  Frank Dellaert
 */

#include <gtsam/navigation/PreintegrationBase.h>
#include <gtsam/base/numericalDerivative.h>
#include <gtsam/nonlinear/expressions.h>
#include <gtsam/nonlinear/ExpressionFactor.h>
#include <gtsam/nonlinear/expressionTesting.h>

#include <CppUnitLite/TestHarness.h>
#include <boost/bind.hpp>

#include "imuFactorTesting.h"

static const double kDt = 0.1;

#ifdef GTSAM_IMU_MANIFOLD_INTEGRATION
Vector9 f(const Vector9& zeta, const Vector3& a, const Vector3& w) {
  return PreintegrationBase::UpdatePreintegrated(a, w, kDt, zeta);
}

namespace testing {
// Create default parameters with Z-down and above noise parameters
static boost::shared_ptr<PreintegrationParams> Params() {
  auto p = PreintegrationParams::MakeSharedD(kGravity);
  p->gyroscopeCovariance = kGyroSigma * kGyroSigma * I_3x3;
  p->accelerometerCovariance = kAccelSigma * kAccelSigma * I_3x3;
  p->integrationCovariance = 0.0001 * I_3x3;
  return p;
}
}

/* ************************************************************************* */
TEST(PreintegrationBase, UpdateEstimate1) {
  PreintegrationBase pim(testing::Params());
  const Vector3 acc(0.1, 0.2, 10), omega(0.1, 0.2, 0.3);
  Vector9 zeta;
  zeta.setZero();
  Matrix9 aH1;
  Matrix93 aH2, aH3;
  pim.UpdatePreintegrated(acc, omega, kDt, zeta, aH1, aH2, aH3);
  EXPECT(assert_equal(numericalDerivative31(f, zeta, acc, omega), aH1, 1e-9));
  EXPECT(assert_equal(numericalDerivative32(f, zeta, acc, omega), aH2, 1e-9));
  EXPECT(assert_equal(numericalDerivative33(f, zeta, acc, omega), aH3, 1e-9));
}

/* ************************************************************************* */
TEST(PreintegrationBase, UpdateEstimate2) {
  PreintegrationBase pim(testing::Params());
  const Vector3 acc(0.1, 0.2, 10), omega(0.1, 0.2, 0.3);
  Vector9 zeta;
  zeta << 0.01, 0.02, 0.03, 100, 200, 300, 10, 5, 3;
  Matrix9 aH1;
  Matrix93 aH2, aH3;
  pim.UpdatePreintegrated(acc, omega, kDt, zeta, aH1, aH2, aH3);
  // NOTE(frank): tolerance of 1e-3 on H1 because approximate away from 0
  EXPECT(assert_equal(numericalDerivative31(f, zeta, acc, omega), aH1, 1e-3));
  EXPECT(assert_equal(numericalDerivative32(f, zeta, acc, omega), aH2, 1e-8));
  EXPECT(assert_equal(numericalDerivative33(f, zeta, acc, omega), aH3, 1e-9));
}

/* ************************************************************************* */
TEST(PreintegrationBase, computeError) {
  PreintegrationBase pim(testing::Params());
  NavState x1, x2;
  imuBias::ConstantBias bias;
  Matrix9 aH1, aH2;
  Matrix96 aH3;
  pim.computeError(x1, x2, bias, aH1, aH2, aH3);
  boost::function<Vector9(const NavState&, const NavState&,
                          const imuBias::ConstantBias&)> f =
      boost::bind(&PreintegrationBase::computeError, pim, _1, _2, _3,
                  boost::none, boost::none, boost::none);
  // NOTE(frank): tolerance of 1e-3 on H1 because approximate away from 0
  EXPECT(assert_equal(numericalDerivative31(f, x1, x2, bias), aH1, 1e-9));
  EXPECT(assert_equal(numericalDerivative32(f, x1, x2, bias), aH2, 1e-9));
  EXPECT(assert_equal(numericalDerivative33(f, x1, x2, bias), aH3, 1e-9));
}

/* ************************************************************************* */
TEST(PreintegrationBase, Compose) {
  testing::SomeMeasurements measurements;
  PreintegrationBase pim(testing::Params());
  testing::integrateMeasurements(measurements, &pim);

  boost::function<Vector9(const Vector9&, const Vector9&)> f =
      [pim](const Vector9& zeta01, const Vector9& zeta12) {
        return PreintegrationBase::Compose(zeta01, zeta12, pim.deltaTij());
      };

  // Expected merge result
  PreintegrationBase expected_pim02(testing::Params());
  testing::integrateMeasurements(measurements, &expected_pim02);
  testing::integrateMeasurements(measurements, &expected_pim02);

  // Actual result
  Matrix9 H1, H2;
  PreintegrationBase actual_pim02 = pim;
  actual_pim02.mergeWith(pim, &H1, &H2);

  const Vector9 zeta = pim.preintegrated();
  const Vector9 actual_zeta =
      PreintegrationBase::Compose(zeta, zeta, pim.deltaTij());
  EXPECT(assert_equal(expected_pim02.preintegrated(), actual_zeta, 1e-7));
  EXPECT(assert_equal(numericalDerivative21(f, zeta, zeta), H1, 1e-7));
  EXPECT(assert_equal(numericalDerivative22(f, zeta, zeta), H2, 1e-7));
}

/* ************************************************************************* */
TEST(PreintegrationBase, MergedBiasDerivatives) {
  testing::SomeMeasurements measurements;

  auto f = [=](const Vector3& a, const Vector3& w) {
    PreintegrationBase pim02(testing::Params(), Bias(a, w));
    testing::integrateMeasurements(measurements, &pim02);
    testing::integrateMeasurements(measurements, &pim02);
    return pim02.preintegrated();
  };

  // Expected merge result
  PreintegrationBase expected_pim02(testing::Params());
  testing::integrateMeasurements(measurements, &expected_pim02);
  testing::integrateMeasurements(measurements, &expected_pim02);

  EXPECT(assert_equal(numericalDerivative21<Vector9, Vector3, Vector3>(f, Z_3x1, Z_3x1),
                      expected_pim02.preintegrated_H_biasAcc()));
  EXPECT(assert_equal(numericalDerivative22<Vector9, Vector3, Vector3>(f, Z_3x1, Z_3x1),
                      expected_pim02.preintegrated_H_biasOmega(), 1e-7));
}
#endif

/* ************************************************************************* */
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */