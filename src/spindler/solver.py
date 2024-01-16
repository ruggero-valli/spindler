from __future__ import annotations

import numpy as np
import pandas as pd
from numpy.typing import ArrayLike
from scipy.interpolate import interp1d
from scipy.integrate import solve_ivp
from importlib.resources import files as importfiles
from spindler.interpolator import RegularGridInterpolatorNNextrapol
    
class Solver():
    """Abstract class defining the common methods to access the derivatives of
    the binary parameters and means to compute their evolution. 
    """
    
    def evolve(self,
                accreted_mass:float,
                q0:float,
                e0:float,
                method:str='LSODA',
                rtol:float=1e-2,
                atol:float=1e-5,
                **kwargs) -> tuple[
                    ArrayLike,
                    ArrayLike,
                    ArrayLike,
                    ArrayLike]:
        """Compute the evolution of the binary system

        Args:
            accreted_mass (float): mass accreted to the binary, in units of initial
                binary mass m0.
            q0 (float): initial mass ratio
            e0 (float): initial eccentricity
            method (str, optional): integration method. See scipy solve_ivp documentation.
                Defaults to 'LSODA'.
            rtol (float, optional): relative tolerance. Defaults to 1e-2.
            atol (float, optional): absolute tolerane. Defaults to 1e-5.
            **kwargs: keywords arguments passed to solve_ivp

        Returns:
            a (ArrayLike): semimajor axis
            q (ArrayLike): mass ratio
            e (ArrayLike): eccentricity
            m (ArrayLike): mass of the binary
        """
        
        # Integration domain
        m_span = [1,1+accreted_mass]
        # Initial conditions
        U0 = [1,q0,e0]
        
        # Define the differential equation's right hand side
        def f(m,U):
            a,q,e = U
            Da = self.get_Da(q,e)
            De = self.get_De(q,e)
            Dq = self.get_Dq(q,e)
            dadm = Da*a/m
            dqdm = Dq*q/m
            dedm = De*e/m
            dUdm = np.array([dadm,dqdm,dedm])
            return dUdm
        
        # Solve the differential equation
        res = solve_ivp(f, m_span, U0, method=method, rtol=rtol, atol=atol, **kwargs)
        # Return the solution
        m = res.t
        a, q, e = res.y
        return a,q,e,m
    
    def get_Da(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the semimajor axis a with respect to the
        mass of the binary m.
        This function is implemented in the child classes.

        Args:
            q (ArrayLike): mass ratio
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dloga/dlogm
        """
        raise NotImplementedError
    
    def get_De(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the eccentricity e with respect to the
        mass of the binary m.
        This function is implemented in the child classes.

        Args:
            q (ArrayLike): mass ratio
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dloge/dlogm
        """
        raise NotImplementedError
    
    def get_Dq(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the mass ratio e with respect to the
        mass of the binary m.
        This function is implemented in the child classes.

        Args:
            q (ArrayLike): mass ratio
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dlogq/dlogm
        """
        raise NotImplementedError

    def get_DE(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the orbital energy E with respect to the
        mass of the binary m.

        Args:
            q (ArrayLike): mass ratio
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dlogE/dlogm
        """
        
        Da = self.get_Da(q,e)
        Dq = self.get_Dq(q,e)
        
        # By differentiating the orbital energy
        # E = - (GM \mu)/(2a),
        # where \mu=M q/(1+q)^2 is the reduced mass:
        DE = 2 - Da + (1-q)/(q**2+q)*Dq
        return DE
    
    def get_DJ(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes derivative of the orbital angular momentum J with
        respect to the mass of the binary m.

        Args:
            q (ArrayLike): mass ratio
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dlogJ/dlogm
        """
        
        Da = self.get_Da(q,e)
        De = self.get_De(q,e)
        Dq = self.get_Dq(q,e)

        # By differentiating the orbital angular momentum 
        # J = \mu \sqrt{GMa(1-e^2)}
        DJ = 3/2 + 1/2*Da + (1-q)/(q**2+q)*Dq - e/(1-e**2)*De
        return DJ

class Solver_Siwek23(Solver):
    """Circumbinary disk model based on the simulations in
    Siwek et al. 2023 (2023MNRAS.518.5059S)
    Siwek et al. 2023 (2023MNRAS.522.2707S)
    
    It is defined for eccentricity between 0 and 0.8, and mass ratio between 0.1
    and 1.
    """
    def __init__(self):
        # Read the tables from Siwek et al 2023
        adota_table = pd.read_csv(importfiles("spindler.tables")/"adota_siwek23.csv")
        adota_table.columns = [float(x) for x in adota_table.columns]
        edot_table = pd.read_csv(importfiles("spindler.tables")/"edot_siwek23.csv")
        edot_table.columns = [float(x) for x in edot_table.columns]

        q_list = np.array(adota_table.index)
        e_list = np.array(adota_table.columns)

        self.adota = RegularGridInterpolatorNNextrapol((q_list, e_list), np.array(adota_table.values))
        self.edot = RegularGridInterpolatorNNextrapol((q_list, e_list), np.array(edot_table.values))

        lambda_table = pd.read_csv(importfiles("spindler.tables")/"lambda_siwek23.csv")
        lambda_table.columns = [float(x) for x in lambda_table.columns]

        q_list = lambda_table.index
        e_list = lambda_table.columns
        e_grid, q_grid = np.meshgrid(e_list, q_list)
        # equation 9 of Siwek et al. 2023 
        qdot_table = (1+q_grid)*(lambda_table-q_grid)/(1+lambda_table)
        self.qdot = RegularGridInterpolatorNNextrapol((q_list, e_list), qdot_table.values)
        
    def get_Da(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the semimajor axis a with respect to the
        mass of the binary m.

        Args:
            q (ArrayLike): mass ratio
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dloga/dlogm
        """
        points = np.dstack((q,e))
        Da = self.adota(points)
        return Da
        
    def get_De(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the eccentricity e with respect to the
        mass of the binary m.

        Args:
            q (ArrayLike): mass ratio
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dloge/dlogm
        """
        points = np.dstack((q,e))
        De = self.edot(points)
        with np.errstate(divide="ignore", invalid="ignore"):
            return np.where(e>0, De/e, 0)
    
    def get_Dq(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the mass ratio e with respect to the
        mass of the binary m.

        Args:
            q (ArrayLike): mass ratio
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dlogq/dlogm
        """
        points = np.dstack((q,e))
        Dq = self.qdot(points)
        return Dq/q
        

class Solver_DD21(Solver):
    """Circumbinary disk model based on the simulations in
    D'Orazio and Duffell 2021 (2021ApJ...914L..21D)
    
    It is defined for eccentricity between 0 and 0.8, and mass ratio equal to 1.
    """
    def __init__(self, print_warnings=False):
        self.print_warnings = print_warnings
        
        # Read the tables from D'Orazio and Duffell 2021
        adota_table = pd.read_csv(importfiles("spindler.tables")/"adota_DD21.csv")
        edot_table = pd.read_csv(importfiles("spindler.tables")/"edot_DD21.csv")

        self.adota = interp1d(adota_table.e, adota_table.adota, fill_value="extrapolate")
        self.edot = interp1d(edot_table.e, edot_table.edot, fill_value="extrapolate")
        
    def get_Da(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the semimajor axis a with respect to the
        mass of the binary m.

        Args:
            q (ArrayLike): mass ratio. Ignored, but if different from 1 generates
                a warning.
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dloga/dlogm
        """
        if self.print_warnings and np.any(q != 1):
            print(f"warning: mass ratio different from 1")
            
        Da = self.adota(e)
        return Da
    
    def get_De(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the eccentricity e with respect to the
        mass of the binary m.

        Args:
            q (ArrayLike): mass ratio. Ignored, but if different from 1 generates
                a warning.
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dloge/dlogm
        """
        if self.print_warnings and np.any(q != 1):
            print(f"warning: mass ratio different from 1")
            
        De = self.edot(e)
        with np.errstate(divide="ignore", invalid="ignore"):
            return np.where(e>0, De/e, 0)
                
    def get_Dq(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the mass ratio e with respect to the
        mass of the binary m.

        Args:
            q (ArrayLike): mass ratio. Ignored, but if different from 1 generates
                a warning.
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dlogq/dlogm
        """
        if self.print_warnings and np.any(q != 1):
            print(f"warning: mass ratio different from 1")
        
        return np.zeros(e.shape)
    
        
class Solver_Zrake21(Solver):
    """Circumbinary disk model based on the simulations in
    Zrake et al. 2021 (2021ApJ...909L..13Z)
    
    It is defined for eccentricity between 0 and 0.8, and mass ratio equal to 1.
    """
    def __init__(self, print_warnings=False):
        self.print_warnings = print_warnings
        
        # Read the tables from Zrake+2021
        adota_table = pd.read_csv(importfiles("spindler.tables")/"adota_zrake21.csv")
        self.adota = interp1d(adota_table.e, adota_table.adota, fill_value="extrapolate")

        edot_table = pd.read_csv(importfiles("spindler.tables")/"edot_zrake21.csv")
        self.edot = interp1d(edot_table.e, edot_table.edot, fill_value="extrapolate")
        
    def get_Da(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the semimajor axis a with respect to the
        mass of the binary m.

        Args:
            q (ArrayLike): mass ratio. Ignored, but if different from 1 generates
                a warning.
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dloga/dlogm
        """
        if self.print_warnings and np.any(q != 1):
            print(f"warning: mass ratio different from 1")
            
        Da = self.adota(e)
        return Da
    
    def get_De(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the eccentricity e with respect to the
        mass of the binary m.

        Args:
            q (ArrayLike): mass ratio. Ignored, but if different from 1 generates
                a warning.
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dloge/dlogm
        """
        if self.print_warnings and np.any(q != 1):
            print(f"warning: mass ratio different from 1")
        
        De = self.edot(e)
        with np.errstate(divide="ignore", invalid="ignore"):
            return np.where(e>0, De/e, 0)
        
    
    def get_Dq(self, q:ArrayLike, e:ArrayLike) -> ArrayLike:
        """Computes the derivative of the mass ratio e with respect to the
        mass of the binary m.

        Args:
            q (ArrayLike): mass ratio. Ignored, but if different from 1 generates
                a warning.
            e (ArrayLike): eccentricity

        Returns:
            ArrayLike: dlogq/dlogm
        """
        if self.print_warnings and np.any(q != 1):
            print(f"warning: mass ratio different from 1")
        return np.zeros(e.shape)