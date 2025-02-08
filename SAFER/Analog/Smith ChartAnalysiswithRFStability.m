% SAFER System Smith Chart Analysis with RF Stability
clear all;
close all;

% System Parameters
Z0 = 50;          % Reference impedance
f = linspace(1e6, 1e9, 1000); % Frequency range 1MHz to 1GHz
w = 2*pi*f;

% Input Stage Parameters (AD620)
Zin = 10e6;       % Input impedance
Cin = 2e-12;      % Input capacitance
Rin_cm = 10e9;    % Common mode input resistance
Cin_cm = 2e-12;   % Common mode input capacitance

% Output Stage Parameters
Rout = 100;       % Output resistance
Cout = 100e-12;   % Output capacitance

% Calculate S-parameters
function [S11, S12, S21, S22] = calc_s_params(f, Zin, Cin, Rout, Cout, Z0)
    w = 2*pi*f;
    Zin_comp = 1./(1./(Zin + 1./(1i*w*Cin)));
    Zout_comp = 1./(1./(Rout + 1./(1i*w*Cout)));
    
    % S-parameter calculations
    S11 = (Zin_comp - Z0)./(Zin_comp + Z0);
    S22 = (Zout_comp - Z0)./(Zout_comp + Z0);
    S21 = 2*sqrt(real(Zin_comp)*real(Zout_comp))./((Zin_comp + Z0).*(Zout_comp + Z0));
    S12 = S21/100; % Assuming reverse isolation
end

% Calculate K-factor for stability
function K = calc_k_factor(S11, S12, S21, S22)
    delta = S11.*S22 - S12.*S21;
    K = (1 - abs(S11).^2 - abs(S22).^2 + abs(delta).^2)./(2*abs(S12.*S21));
end

% Calculate μ-factor (mu) for stability
function mu = calc_mu_factor(S11, S12, S21, S22)
    delta = S11.*S22 - S12.*S21;
    mu = (1 - abs(S11).^2)./(abs(S22 - conj(S11).*delta) + abs(S12.*S21));
end

% Calculate stability circles
function [Cs, rs] = source_stability_circles(S11, S12, S21, S22)
    delta = S11.*S22 - S12.*S21;
    Cs = conj(S11 - delta.*conj(S22))./(abs(S11).^2 - abs(delta).^2);
    rs = abs(S12.*S21)./(abs(S11).^2 - abs(delta).^2);
end

function [Cl, rl] = load_stability_circles(S11, S12, S21, S22)
    delta = S11.*S22 - S12.*S21;
    Cl = conj(S22 - delta.*conj(S11))./(abs(S22).^2 - abs(delta).^2);
    rl = abs(S12.*S21)./(abs(S22).^2 - abs(delta).^2);
end

% Calculate S-parameters across frequency
[S11, S12, S21, S22] = calc_s_params(f, Zin, Cin, Rout, Cout, Z0);

% Calculate stability factors
K = calc_k_factor(S11, S12, S21, S22);
mu = calc_mu_factor(S11, S12, S21, S22);

% Calculate stability circles
[Cs, rs] = source_stability_circles(S11, S12, S21, S22);
[Cl, rl] = load_stability_circles(S11, S12, S21, S22);

% Plot Smith Chart with Stability Circles
figure('Position', [100 100 800 800]);
smithplot = smithchart;
hold on;

% Plot S11 trace
plot(smithplot, S11, 'b-', 'LineWidth', 2);

% Plot S22 trace
plot(smithplot, S22, 'r--', 'LineWidth', 2);

% Plot stability circles
theta = linspace(0, 2*pi, 100);
for i = 1:10:length(f)
    if abs(Cs(i)) < 10 && rs(i) > 0 && rs(i) < 10
        circle_s = Cs(i) + rs(i)*exp(1i*theta);
        plot(smithplot, circle_s, 'g:', 'LineWidth', 1);
    end
    if abs(Cl(i)) < 10 && rl(i) > 0 && rl(i) < 10
        circle_l = Cl(i) + rl(i)*exp(1i*theta);
        plot(smithplot, circle_l, 'm:', 'LineWidth', 1);
    end
end

% Add maximum stable gain circles
MSG = abs(S21./S12);
r_msg = linspace(0, 1, 5);
for i = 1:length(r_msg)
    msg_circle = r_msg(i)*exp(1i*theta);
    plot(smithplot, msg_circle, 'k:', 'LineWidth', 0.5);
end

title('Smith Chart with Stability Analysis');
legend('S11', 'S22', 'Source Stability', 'Load Stability', 'MSG Circles');
grid on;

% Plot Stability Factors
figure('Position', [100 100 800 400]);
subplot(1,2,1);
semilogx(f, K, 'LineWidth', 2);
grid on;
title('Rollett Stability Factor (K)');
xlabel('Frequency (Hz)');
ylabel('K');
yline(1, 'r--');

subplot(1,2,2);
semilogx(f, mu, 'LineWidth', 2);
grid on;
title('μ-Factor Stability');
xlabel('Frequency (Hz)');
ylabel('μ');
yline(1, 'r--');

% Calculate and display stability metrics
fprintf('Stability Analysis Results:\n');
fprintf('Minimum K factor: %.2f\n', min(K));
fprintf('Minimum μ factor: %.2f\n', min(mu));
fprintf('Maximum MSG: %.2f dB\n', 20*log10(max(MSG)));

% Noise Analysis with Smith Chart
% Calculate noise figure circles
NFmin = 1.5; % Minimum noise figure in dB
Rn = 50;    % Equivalent noise resistance
Gamma_opt = 0.2*exp(1i*pi/4); % Optimal source reflection coefficient

% Plot noise circles
figure('Position', [100 100 800 800]);
smithplot2 = smithchart;
hold on;

% Add noise circles
NF_circles = [NFmin, NFmin+1, NFmin+2, NFmin+4];
for nf = NF_circles
    r = sqrt(1 - 10^(-(nf-NFmin)/10));
    noise_circle = Gamma_opt + r*exp(1i*theta);
    plot(smithplot2, noise_circle, 'b:', 'LineWidth', 1);
end

% Plot Gamma_opt point
plot(smithplot2, Gamma_opt, 'r*', 'MarkerSize', 10);

title('Noise Performance Analysis');
legend('NF Circles', 'Optimal Γ_s');

% Calculate available gain circles
Ga_circles = [0 -1 -3 -6];
for ga = Ga_circles
    k = abs(S21)/abs(S12);
    r = sqrt(1 - 10^(ga/10)/k);
    gain_circle = r*exp(1i*theta);
    plot(smithplot2, gain_circle, 'g:', 'LineWidth', 1);
end

% Print noise performance metrics
fprintf('\nNoise Performance:\n');
fprintf('Minimum Noise Figure: %.2f dB\n', NFmin);
fprintf('Optimal Source Reflection: %.2f∠%.1f°\n', abs(Gamma_opt), angle(Gamma_opt)*180/pi);

% Add matching network analysis
ZL = Z0*(1 + S22)./(1 - S22);
ZS = Z0*(1 + S11)./(1 - S11);

% Calculate matching network components
function [L, C] = calculate_matching(Z, Z0, f)
    if real(Z) > Z0
        % Use L-network high-to-low
        Q = sqrt(real(Z)/Z0 - 1);
        L = Q*Z0/(2*pi*f);
        C = Q/(2*pi*f*Z0);
    else
        % Use L-network low-to-high
        Q = sqrt(Z0/real(Z) - 1);
        L = Z0/(Q*2*pi*f);
        C = 1/(Q*2*pi*f*Z0);
    end
end

% Calculate matching components at center frequency
fc = f(end/2);
[L_in, C_in] = calculate_matching(ZS(end/2), Z0, fc);
[L_out, C_out] = calculate_matching(ZL(end/2), Z0, fc);

fprintf('\nMatching Network Components:\n');
fprintf('Input Match: L = %.2f nH, C = %.2f pF\n', L_in*1e9, C_in*1e12);
fprintf('Output Match: L = %.2f nH, C = %.2f pF\n', L_out*1e9, C_out*1e12);
