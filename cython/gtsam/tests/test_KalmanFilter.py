import unittest
from gtsam import *
from math import *
import numpy as np

class TestKalmanFilter(unittest.TestCase):

    def test_KalmanFilter(self):
        F = np.eye(2)
        B = np.eye(2)
        u = np.array([1.0, 0.0])
        modelQ = noiseModel_Diagonal.Sigmas(np.array([0.1, 0.1]))
        Q = 0.01 * np.eye(2)
        H = np.eye(2)
        z1 = np.array([1.0, 0.0])
        z2 = np.array([2.0, 0.0])
        z3 = np.array([3.0, 0.0])
        modelR = noiseModel_Diagonal.Sigmas(np.array([0.1, 0.1]))
        R = 0.01 * np.eye(2)

        # Create the set of expected output TestValues
        expected0 = np.array([0.0, 0.0])
        P00 = 0.01 * np.eye(2)

        expected1 = np.array([1.0, 0.0])
        P01 = P00 + Q
        I11 = np.linalg.inv(P01) + np.linalg.inv(R)

        expected2 = np.array([2.0, 0.0])
        P12 = np.linalg.inv(I11) + Q
        I22 = np.linalg.inv(P12) + np.linalg.inv(R)

        expected3 = np.array([3.0, 0.0])
        P23 = np.linalg.inv(I22) + Q
        I33 = np.linalg.inv(P23) + np.linalg.inv(R)

        # Create an KalmanFilter object
        KF = KalmanFilter(n=2)

        # Create the Kalman Filter initialization point
        x_initial = np.array([0.0, 0.0])
        P_initial = 0.01 * np.eye(2)

        # Create an KF object
        state = KF.init(x_initial, P_initial)
        self.assertTrue(np.allclose(expected0, state.mean()))
        self.assertTrue(np.allclose(P00, state.covariance()))

        # Run iteration 1
        state = KF.predict(state, F, B, u, modelQ)
        self.assertTrue(np.allclose(expected1, state.mean()))
        self.assertTrue(np.allclose(P01, state.covariance()))
        state = KF.update(state, H, z1, modelR)
        self.assertTrue(np.allclose(expected1, state.mean()))
        self.assertTrue(np.allclose(I11, state.information()))

        # Run iteration 2
        state = KF.predict(state, F, B, u, modelQ)
        self.assertTrue(np.allclose(expected2, state.mean()))
        state = KF.update(state, H, z2, modelR)
        self.assertTrue(np.allclose(expected2, state.mean()))

        # Run iteration 3
        state = KF.predict(state, F, B, u, modelQ)
        self.assertTrue(np.allclose(expected3, state.mean()))
        state = KF.update(state, H, z3, modelR)
        self.assertTrue(np.allclose(expected3, state.mean()))

if __name__ == "__main__":
    unittest.main()