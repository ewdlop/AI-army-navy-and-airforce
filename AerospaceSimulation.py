import numpy as np
from scipy.optimize import minimize
from scipy.integrate import odeint
import matplotlib.pyplot as plt

class AerospaceSimulation:
    def __init__(self):
        # Physical constants
        self.g = 9.81  # gravitational acceleration (m/s^2)
        self.rho = 1.225  # air density at sea level (kg/m^3)
        self.Cd = 0.5  # drag coefficient
        self.area = 1.0  # reference area (m^2)
        
    def drag_force(self, velocity):
        """Calculate drag force given velocity"""
        return 0.5 * self.rho * self.Cd * self.area * velocity**2
        
    def equations_of_motion(self, state, t):
        """
        Define the differential equations for flight path
        state = [x, y, vx, vy]
        """
        x, y, vx, vy = state
        
        # Calculate total velocity and drag
        v = np.sqrt(vx**2 + vy**2)
        drag = self.drag_force(v)
        
        # Drag force components
        drag_x = -drag * vx/v if v > 0 else 0
        drag_y = -drag * vy/v if v > 0 else 0
        
        # Equations of motion
        dx_dt = vx
        dy_dt = vy
        dvx_dt = drag_x
        dvy_dt = drag_y - self.g
        
        return [dx_dt, dy_dt, dvx_dt, dvy_dt]
    
    def optimize_trajectory(self, initial_state, target_position, t_span, n_points=1000):
        """
        Optimize flight path to reach target position
        initial_state = [x0, y0, vx0, vy0]
        target_position = [x_target, y_target]
        """
        t = np.linspace(0, t_span, n_points)
        
        def objective(v_initial):
            """Objective function to minimize"""
            state0 = [initial_state[0], initial_state[1], v_initial[0], v_initial[1]]
            solution = odeint(self.equations_of_motion, state0, t)
            
            # Calculate final position error
            x_error = solution[-1, 0] - target_position[0]
            y_error = solution[-1, 1] - target_position[1]
            
            return np.sqrt(x_error**2 + y_error**2)
        
        # Initial guess for velocity
        v_guess = [50.0, 50.0]
        
        # Optimize
        result = minimize(objective, v_guess, method='Nelder-Mead')
        
        # Get optimal trajectory
        optimal_state0 = [initial_state[0], initial_state[1], result.x[0], result.x[1]]
        trajectory = odeint(self.equations_of_motion, optimal_state0, t)
        
        return t, trajectory
    
    def plot_trajectory(self, t, trajectory, target_position):
        """Plot the optimized trajectory"""
        plt.figure(figsize=(10, 6))
        
        # Plot trajectory
        plt.plot(trajectory[:, 0], trajectory[:, 1], 'b-', label='Flight Path')
        
        # Plot start and end points
        plt.plot(trajectory[0, 0], trajectory[0, 1], 'go', label='Start')
        plt.plot(target_position[0], target_position[1], 'ro', label='Target')
        
        plt.grid(True)
        plt.xlabel('X Position (m)')
        plt.ylabel('Y Position (m)')
        plt.title('Optimized Flight Trajectory')
        plt.legend()
        plt.axis('equal')
        plt.show()

# Example usage
sim = AerospaceSimulation()

# Define initial conditions and target
initial_state = [0, 0, 0, 0]  # [x0, y0, vx0, vy0]
target_position = [1000, 500]  # [x_target, y_target]
t_span = 20  # simulation time span

# Optimize and plot trajectory
t, trajectory = sim.optimize_trajectory(initial_state, target_position, t_span)
sim.plot_trajectory(t, trajectory, target_position)

# Calculate and print flight statistics
final_position = trajectory[-1, :2]
final_velocity = trajectory[-1, 2:]
print(f"Final position: {final_position}")
print(f"Final velocity: {final_velocity}")
print(f"Flight time: {t[-1]:.2f} seconds")
