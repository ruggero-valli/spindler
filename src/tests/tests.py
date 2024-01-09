import unittest
import numpy as np

from spindler.solver import Solver_DD21, Solver_Siwek23, Solver_Zrake21
from spindler.interpolator import RegularGridInterpolatorNNextrapol

class TestRegularGridInterpolatorNNextrapol(unittest.TestCase):

    def setUp(self):
        # Sample data for testing
        x = np.linspace(0,1,5)
        y = np.linspace(2,3,5)
        x_grid,y_grid = np.meshgrid(x,y)
        values = x_grid+y_grid
        self.interp = RegularGridInterpolatorNNextrapol((x,y), values)

    def test_within_interpolation_domain(self):
        # Test when given a point within the interpolation domain, it provides an interpolation
        result = self.interp(np.array([0.15, 2.35]))
        self.assertIsInstance(result, np.ndarray)
        self.assertTrue(np.all(result >= 1.0) and np.all(result <= 4.0))

    def test_outside_interpolation_domain(self):
        # Test when given a point outside of the interpolation domain,
        # it returns the value of the closest known point
        result = self.interp(np.array([3, 0]))
        self.assertIsInstance(result, np.ndarray)
        self.assertTrue(np.all(result == 3))
        
class TestSolverSiwek23(unittest.TestCase):

    def setUp(self):
        self.solver = Solver_Siwek23()

    def test_get_De_vs_Dq(self):
        q = np.array([0.2, 0.4, 1])
        e = np.array([0.8, 0.3, 0.2])
        De = self.solver.get_De(q, e)
        Dq = self.solver.get_Dq(q, e)
        self.assertTrue(np.all(np.abs(De) > Dq))

    def test_get_Dq_positive(self):
        q = np.array([0.2, 0.4, 0.6])
        e = np.array([0.1, 0.3, 0.5])
        Dq = self.solver.get_Dq(q, e)
        self.assertTrue(np.all(Dq > 0))

    def test_get_Da_q1_e0_5(self):
        q = np.array([1.0])
        e = np.array([0.48])
        Da = self.solver.get_Da(q, e)
        self.assertTrue(np.all(Da < 0))

    def test_get_Da_q1_e0_05(self):
        q = np.array([1.0])
        e = np.array([0.05])
        Da = self.solver.get_Da(q, e)
        self.assertTrue(np.all(Da > 0))

    def test_evolve_accreted_mass_10(self):
        accreted_mass = 10.0
        q_values = [0.3, 0.5, 1.0]
        e_values = [0.1, 0.5, 0.7]

        for q0 in q_values:
            for e0 in e_values:
                a, q, e, m = self.solver.evolve(accreted_mass, q0, e0)
                self.assertTrue(np.isclose(q[-1], 1.0, rtol=0.05))
                self.assertTrue(np.isclose(e[-1], 0.5, rtol=0.05))
                self.assertTrue(np.isclose(m[-1], 11.0, rtol=0.05))
                
class TestSolverZrake21(unittest.TestCase):

    def setUp(self):
        self.solver = Solver_Zrake21()

    def test_get_Da_independence_on_q(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        Da_ref = self.solver.get_Da(q_values, e_values)

        for q0 in q_values:
            Da = self.solver.get_Da(q0, e_values)
            self.assertTrue(np.allclose(Da, Da_ref))

    def test_get_De_independence_on_q(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        De_ref = self.solver.get_De(q_values, e_values)

        for q0 in q_values:
            De = self.solver.get_De(q0, e_values)
            self.assertTrue(np.allclose(De, De_ref))

    def test_get_Dq_independence_on_q(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        Dq_ref = self.solver.get_Dq(q_values, e_values)

        for q0 in q_values:
            Dq = self.solver.get_Dq(q0, e_values)
            self.assertTrue(np.allclose(Dq, Dq_ref))

    def test_get_DJ_independence_on_q(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        DJ_ref = self.solver.get_DJ(q_values, e_values)

        for q0 in q_values:
            DJ = self.solver.get_DJ(q0, e_values)
            self.assertTrue(np.allclose(DJ, DJ_ref))

    def test_get_DE_independence_on_q(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        DE_ref = self.solver.get_DE(q_values, e_values)

        for q0 in q_values:
            DE = self.solver.get_DE(q0, e_values)
            self.assertTrue(np.allclose(DE, DE_ref))

    def test_get_Dq_always_zero(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        Dq = self.solver.get_Dq(q_values, e_values)
        self.assertTrue(np.allclose(Dq, 0))

    def test_get_De_e0(self):
        e = np.array([0.0])
        De = self.solver.get_De(1, e)
        self.assertTrue(np.allclose(De, 0))

    def test_get_De_e0_05(self):
        e = np.array([0.05])
        De = self.solver.get_De(1, e)
        self.assertTrue(np.all(De < 0))

    def test_get_De_e0_3(self):
        e = np.array([0.3])
        De = self.solver.get_De(1, e)
        self.assertTrue(np.all(De > 3))

    def test_get_De_e0_45(self):
        e = np.array([0.442])
        De = self.solver.get_De(1, e)
        self.assertTrue(np.allclose(De, 0, atol=0.5))

    def test_get_De_egreaterthan0_5(self):
        e = np.array([0.5])
        De = self.solver.get_De(1, e)
        self.assertTrue(np.all(De < 0))

    def test_get_Da_e0(self):
        e = np.array([0.0])
        Da = self.solver.get_Da(1, e)
        self.assertTrue(np.all(Da > 0))

    def test_get_Da_e0_4(self):
        e = np.array([0.4])
        Da = self.solver.get_Da(1, e)
        self.assertTrue(np.all(Da < 0))

    def test_get_Da_e0_7(self):
        e = np.array([0.7])
        Da = self.solver.get_Da(1, e)
        self.assertTrue(np.all(Da < 0))
        
    def test_evolve_accreted_mass_10(self):
        accreted_mass = 10.0
        q_values = [1,1,1]
        e_values = [0.1, 0.45, 0.7]

        for q0 in q_values:
            for e0 in e_values:
                a, q, e, m = self.solver.evolve(accreted_mass, q0, e0)
                self.assertTrue(np.isclose(q[-1], 1.0, rtol=1e-5))
                self.assertTrue(np.isclose(e[-1], 0.44, rtol=0.05))
                self.assertTrue(np.isclose(m[-1], 11.0, rtol=1e-5))
                

class TestSolverDD21(unittest.TestCase):

    def setUp(self):
        self.solver = Solver_DD21()

    def test_get_Da_independence_on_q(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        Da_ref = self.solver.get_Da(q_values, e_values)

        for q0 in q_values:
            Da = self.solver.get_Da(q0, e_values)
            self.assertTrue(np.allclose(Da, Da_ref))

    def test_get_De_independence_on_q(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        De_ref = self.solver.get_De(q_values, e_values)

        for q0 in q_values:
            De = self.solver.get_De(q0, e_values)
            self.assertTrue(np.allclose(De, De_ref))

    def test_get_Dq_independence_on_q(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        Dq_ref = self.solver.get_Dq(q_values, e_values)

        for q0 in q_values:
            Dq = self.solver.get_Dq(q0, e_values)
            self.assertTrue(np.allclose(Dq, Dq_ref))

    def test_get_DJ_independence_on_q(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        DJ_ref = self.solver.get_DJ(q_values, e_values)

        for q0 in q_values:
            DJ = self.solver.get_DJ(q0, e_values)
            self.assertTrue(np.allclose(DJ, DJ_ref))

    def test_get_DE_independence_on_q(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        DE_ref = self.solver.get_DE(q_values, e_values)

        for q0 in q_values:
            DE = self.solver.get_DE(q0, e_values)
            self.assertTrue(np.allclose(DE, DE_ref))

    def test_get_Dq_always_zero(self):
        q_values = np.array([0.1, 0.2, 0.3])
        e_values = np.array([0.1, 0.2, 0.3])
        Dq = self.solver.get_Dq(q_values, e_values)
        self.assertTrue(np.allclose(Dq, 0))

    def test_get_De_e0(self):
        e = np.array([0.0])
        De = self.solver.get_De(1, e)
        self.assertTrue(np.allclose(De, 0))

    def test_get_De_e0_05(self):
        e = np.array([0.05])
        De = self.solver.get_De(1, e)
        self.assertTrue(np.all(De < 0))

    def test_get_De_e0_3(self):
        e = np.array([0.3])
        De = self.solver.get_De(1, e)
        self.assertTrue(np.all(De > 3))

    def test_get_De_e0_38(self):
        e = np.array([0.387])
        De = self.solver.get_De(1, e)
        self.assertTrue(np.allclose(De, 0, atol=0.5))

    def test_get_De_egreaterthan0_5(self):
        e = np.array([0.5])
        De = self.solver.get_De(1, e)
        self.assertTrue(np.all(De < 0))

    def test_get_Da_e0(self):
        e = np.array([0.0])
        Da = self.solver.get_Da(1, e)
        self.assertTrue(np.all(Da > 0))

    def test_get_Da_e0_0_36(self):
        e = np.array([0.36])
        Da = self.solver.get_Da(1, e)
        self.assertTrue(np.all(Da < 0))

    def test_get_Da_e0_7(self):
        e = np.array([0.7])
        Da = self.solver.get_Da(1, e)
        self.assertTrue(np.all(Da < 0))
        
    def test_evolve_accreted_mass_10(self):
        accreted_mass = 10.0
        q_values = [1,1,1]
        e_values = [0.1, 0.45, 0.7]

        for q0 in q_values:
            for e0 in e_values:
                a, q, e, m = self.solver.evolve(accreted_mass, q0, e0)
                self.assertTrue(np.isclose(q[-1], 1.0, rtol=1e-5))
                self.assertTrue(np.isclose(e[-1], 0.39, rtol=0.05))
                self.assertTrue(np.isclose(m[-1], 11.0, rtol=1e-5))


        
if __name__ == '__main__':
    unittest.main()