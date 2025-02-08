import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from scipy.fft import fft, fftfreq
import control
import seaborn as sns

# Set plotting style
plt.style.use('seaborn')
sns.set_palette("husl")

class AnalogSystemAnalysis:
    def __init__(self):
        # System parameters
        self.gain_nominal = 100
        self.GBW = 1e6  # 1MHz gain-bandwidth
        self.fn = 1000  # 1kHz cutoff
        self.temp_coeff = -0.5e-4  # %/°C
        
    def create_transfer_functions(self):
        """Create system transfer functions"""
        # Input stage (AD620)
        pole1 = 2 * np.pi * 10  # Low frequency pole
        num_in = [self.GBW]
        den_in = [1, pole1, 0]
        self.H_in = signal.TransferFunction(num_in, den_in)
        
        # Filter stage (Butterworth)
        wn = 2 * np.pi * self.fn
        zeta = 0.707
        num_f = [wn**2]
        den_f = [1, 2*zeta*wn, wn**2]
        self.H_f = signal.TransferFunction(num_f, den_f)
        
        return self.H_in, self.H_f
    
    def temperature_analysis(self, temp_range=np.arange(-40, 86, 5)):
        """Analyze temperature stability"""
        gains = self.gain_nominal * (1 + self.temp_coeff * (temp_range - 25))
        gain_var = (np.max(gains) - np.min(gains))/self.gain_nominal * 100
        
        plt.figure(figsize=(10, 6))
        plt.plot(temp_range, gains, linewidth=2)
        plt.grid(True)
        plt.xlabel('Temperature (°C)')
        plt.ylabel('Gain')
        plt.title('Gain vs Temperature')
        plt.show()
        
        return gains, gain_var
    
    def psrr_analysis(self):
        """Analyze Power Supply Rejection Ratio"""
        PSRR_dc = 85  # dB
        PSRR_pole = 1000  # Hz
        f_psrr = np.logspace(0, 5, 1000)
        PSRR = PSRR_dc - 20 * np.log10(np.sqrt(1 + (f_psrr/PSRR_pole)**2))
        
        plt.figure(figsize=(10, 6))
        plt.semilogx(f_psrr, PSRR, linewidth=2)
        plt.grid(True)
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('PSRR (dB)')
        plt.title('Power Supply Rejection Ratio')
        plt.show()
        
        return f_psrr, PSRR
    
    def noise_analysis(self):
        """Perform detailed noise analysis"""
        # Noise parameters
        en_preamp = 10e-9  # V/√Hz @ 1kHz
        in_preamp = 0.1e-12  # A/√Hz
        R_source = 10e3  # Source resistance
        k = 1.38e-23  # Boltzmann constant
        T = 298.15  # Temperature (K)
        
        # Calculate noise contributions
        en_thermal = np.sqrt(4 * k * T * R_source)
        en_total = np.sqrt(en_preamp**2 + (in_preamp * R_source)**2 + en_thermal**2)
        
        # Frequency-dependent noise
        f_noise = np.logspace(0, 4, 1000)
        noise_density = en_total / np.sqrt(1 + (f_noise/self.fn)**2)
        
        plt.figure(figsize=(10, 6))
        plt.loglog(f_noise, noise_density * 1e9, linewidth=2)
        plt.grid(True)
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Noise Density (nV/√Hz)')
        plt.title('Input-Referred Noise Density')
        plt.show()
        
        # Calculate total RMS noise
        noise_bw = self.fn * np.pi/2
        rms_noise = en_total * np.sqrt(noise_bw)
        
        return f_noise, noise_density, rms_noise
    
    def monte_carlo_analysis(self, n_iterations=1000):
        """Perform Monte Carlo analysis for component variations"""
        # Component tolerances
        R_tol = 0.01  # 1% resistor tolerance
        C_tol = 0.05  # 5% capacitor tolerance
        
        # Arrays for results
        cutoff_freqs = np.zeros(n_iterations)
        gains_mc = np.zeros(n_iterations)
        
        for i in range(n_iterations):
            # Vary component values
            R_var = 10e3 * (1 + R_tol * np.random.randn())
            C_var = 15e-9 * (1 + C_tol * np.random.randn())
            
            # Calculate variations
            cutoff_freqs[i] = 1/(2*np.pi*R_var*C_var)
            gains_mc[i] = self.gain_nominal * (1 + R_tol * np.random.randn())
        
        # Plot results
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10))
        
        ax1.hist(cutoff_freqs, bins=50)
        ax1.set_xlabel('Cutoff Frequency (Hz)')
        ax1.set_ylabel('Count')
        ax1.set_title('Monte Carlo: Cutoff Frequency Distribution')
        
        ax2.hist(gains_mc, bins=50)
        ax2.set_xlabel('Gain')
        ax2.set_ylabel('Count')
        ax2.set_title('Monte Carlo: Gain Distribution')
        
        plt.tight_layout()
        plt.show()
        
        return cutoff_freqs, gains_mc
    
    def stability_margins(self):
        """Calculate stability margins"""
        # Combine transfer functions
        H_total = control.TransferFunction(
            self.H_in.num[0] * self.H_f.num[0],
            control.convolve(self.H_in.den[0], self.H_f.den[0])
        )
        
        # Calculate margins
        gm, pm, wg, wp = control.margin(H_total)
        
        return gm, pm, wg, wp

def main():
    # Create analyzer instance
    analyzer = AnalogSystemAnalysis()
    
    # Create transfer functions
    H_in, H_f = analyzer.create_transfer_functions()
    print("Transfer functions created")
    
    # Temperature analysis
    gains, gain_var = analyzer.temperature_analysis()
    print(f"\nGain variation over temperature: {gain_var:.2f}%")
    
    # PSRR analysis
    f_psrr, PSRR = analyzer.psrr_analysis()
    print(f"PSRR at 100Hz: {PSRR[np.abs(f_psrr-100).argmin()]:.1f} dB")
    
    # Noise analysis
    f_noise, noise_density, rms_noise = analyzer.noise_analysis()
    print(f"\nTotal RMS noise: {rms_noise*1e6:.2f} µVrms")
    
    # Stability margins
    gm, pm, wg, wp = analyzer.stability_margins()
    print(f"\nGain Margin: {20*np.log10(gm):.1f} dB")
    print(f"Phase Margin: {pm:.1f} degrees")
    
    # Monte Carlo analysis
    cutoff_freqs, gains_mc = analyzer.monte_carlo_analysis()
    print(f"\nMonte Carlo Results:")
    print(f"Cutoff Frequency: {np.mean(cutoff_freqs):.1f} ± {np.std(cutoff_freqs):.1f} Hz")
    print(f"Gain: {np.mean(gains_mc):.1f} ± {np.std(gains_mc):.1f}")

if __name__ == "__main__":
    main()
