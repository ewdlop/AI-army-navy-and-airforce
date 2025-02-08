% SAFER Analog System Stability Analysis
% Define system parameters
clear all;
close all;

% Temperature range for analysis
temp_range = -40:5:85;  % °C

% Frequency range for analysis
f = logspace(-1, 6, 1000);
s = 1i * 2 * pi * f;

%% Input Stage (AD620) Transfer Function
% Temperature-dependent gain variation
Gain_nominal = 100;
temp_coeff = -0.5e-4;  % %/°C
gains = Gain_nominal * (1 + temp_coeff * (temp_range - 25));

% AD620 frequency response
GBW = 1e6;  % 1MHz GBW
pole1 = 2*pi*10;  % Low frequency pole
Hin = tf([GBW], [1 pole1 0]);

%% Active Filter Stage
% Second-order Butterworth filter
fn = 1000;  % 1kHz cutoff
wn = 2*pi*fn;
zeta = 0.707;  % Damping ratio
Hf = tf(wn^2, [1 2*zeta*wn wn^2]);

%% Loop Stability Analysis
figure(1);
bode(Hin * Hf);
grid on;
title('Loop Frequency Response');

% Calculate stability margins
[Gm,Pm,Wcg,Wcp] = margin(Hin * Hf);
fprintf('Gain Margin: %.2f dB\n', 20*log10(Gm));
fprintf('Phase Margin: %.2f degrees\n', Pm);

%% Temperature Stability Analysis
figure(2);
T = tiledlayout(2,1);
nexttile
plot(temp_range, gains, 'LineWidth', 2);
grid on;
xlabel('Temperature (°C)');
ylabel('Gain');
title('Gain vs Temperature');

% Calculate temp coefficient of entire system
sys_gain_var = (max(gains) - min(gains))/Gain_nominal * 100;
fprintf('System gain variation: %.2f%% over temp range\n', sys_gain_var);

%% PSRR Analysis
% PSRR model with frequency dependence
PSRR_dc = 85;  % dB
PSRR_pole = 1000;  % Hz
f_psrr = logspace(0, 5, 1000);
PSRR = PSRR_dc - 20*log10(sqrt(1 + (f_psrr/PSRR_pole).^2));

nexttile
semilogx(f_psrr, PSRR, 'LineWidth', 2);
grid on;
xlabel('Frequency (Hz)');
ylabel('PSRR (dB)');
title('Power Supply Rejection Ratio');

%% Noise Analysis
% Input noise sources
en_preamp = 10e-9;  % V/√Hz @ 1kHz
in_preamp = 0.1e-12;  % A/√Hz
Rsource = 10e3;  % Source resistance

% Calculate total input noise
en_thermal = sqrt(4 * 1.38e-23 * (273.15 + 25) * Rsource);  % Johnson noise
en_total = sqrt(en_preamp^2 + (in_preamp * Rsource)^2 + en_thermal^2);

% Noise vs frequency
f_noise = logspace(0, 4, 1000);
noise_density = en_total ./ sqrt(1 + (f_noise/fn).^2);

figure(3);
loglog(f_noise, noise_density * 1e9, 'LineWidth', 2);
grid on;
xlabel('Frequency (Hz)');
ylabel('Noise Density (nV/√Hz)');
title('Input-Referred Noise Density');

% Calculate total RMS noise
noise_bw = fn * pi/2;  % Equivalent noise bandwidth
rms_noise = en_total * sqrt(noise_bw);
fprintf('Total RMS noise: %.2f µVrms\n', rms_noise*1e6);

%% SNR Calculation
Vin_fs = 1;  % Full-scale input voltage
SNR = 20 * log10(Vin_fs / rms_noise);
fprintf('Signal-to-Noise Ratio: %.2f dB\n', SNR);

%% Monte Carlo Analysis for Component Variations
n_iterations = 1000;
% Component tolerances
R_tol = 0.01;  % 1% resistor tolerance
C_tol = 0.05;  % 5% capacitor tolerance

% Arrays to store results
cutoff_freqs = zeros(n_iterations, 1);
gains_mc = zeros(n_iterations, 1);

for i = 1:n_iterations
    % Vary component values
    R_var = 10e3 * (1 + R_tol * randn);
    C_var = 15e-9 * (1 + C_tol * randn);
    
    % Calculate resulting cutoff frequency
    fc_var = 1/(2*pi*R_var*C_var);
    cutoff_freqs(i) = fc_var;
    
    % Calculate gain variation
    gains_mc(i) = Gain_nominal * (1 + R_tol * randn);
end

figure(4);
T2 = tiledlayout(2,1);
nexttile
histogram(cutoff_freqs, 50);
xlabel('Cutoff Frequency (Hz)');
ylabel('Count');
title('Monte Carlo: Cutoff Frequency Distribution');

nexttile
histogram(gains_mc, 50);
xlabel('Gain');
ylabel('Count');
title('Monte Carlo: Gain Distribution');

% Calculate statistics
fprintf('\nMonte Carlo Analysis Results:\n');
fprintf('Cutoff Frequency: %.2f Hz ± %.2f Hz\n', mean(cutoff_freqs), std(cutoff_freqs));
fprintf('Gain: %.2f ± %.2f\n', mean(gains_mc), std(gains_mc));
