(* SAFER Analog System Stability Analysis *)

(* Clear any previous definitions *)
ClearAll["Global`*"]

(* System Parameters *)
gainNominal = 100;
gbw = 10^6;  (* 1MHz GBW *)
fn = 1000;   (* 1kHz cutoff *)
tempCoeff = -0.5*10^-4;  (* %/°C *)
dampingRatio = 0.707;    (* Butterworth response *)

(* Define Transfer Functions *)
(* Input Stage - AD620 *)
inputStage = TransferFunctionModel[
  gainNominal * gbw/(s + 2π*10), 
  s
];

(* Active Filter Stage *)
wn = 2π*fn;
filterStage = TransferFunctionModel[
  wn^2/(s^2 + 2*dampingRatio*wn*s + wn^2),
  s
];

(* Complete System Transfer Function *)
systemTF = TransferFunctionModel[
  SystemsModelMerge[inputStage, filterStage],
  s
];

(* Temperature Analysis *)
tempRange = Range[-40, 85, 5];
gains = Table[
  gainNominal*(1 + tempCoeff*(t - 25)), 
  {t, tempRange}
];

tempPlot = ListLinePlot[
  Transpose[{tempRange, gains}],
  PlotLabel -> "Gain vs Temperature",
  AxesLabel -> {"Temperature (°C)", "Gain"},
  PlotTheme -> "Detailed",
  GridLines -> Automatic
];

(* PSRR Analysis *)
psrrDC = 85;  (* dB *)
psrrPole = 1000;  (* Hz *)
freqPSRR = Table[10^x, {x, 0, 5, 0.01}];
psrrResponse = psrrDC - 20*Log10[Sqrt[1 + (freqPSRR/psrrPole)^2]];

psrrPlot = ListLogLinearPlot[
  Transpose[{freqPSRR, psrrResponse}],
  PlotLabel -> "Power Supply Rejection Ratio",
  AxesLabel -> {"Frequency (Hz)", "PSRR (dB)"},
  PlotTheme -> "Detailed",
  GridLines -> Automatic
];

(* Noise Analysis *)
(* Constants *)
kb = 1.380649*10^-23;  (* Boltzmann constant *)
temp = 298.15;         (* Temperature in Kelvin *)
rsource = 10*10^3;     (* Source resistance *)

(* Noise sources *)
enPreamp = 10*10^-9;   (* V/√Hz @ 1kHz *)
inPreamp = 0.1*10^-12; (* A/√Hz *)

(* Thermal noise *)
enThermal = Sqrt[4*kb*temp*rsource];

(* Total input noise *)
enTotal = Sqrt[enPreamp^2 + (inPreamp*rsource)^2 + enThermal^2];

(* Frequency-dependent noise *)
freqNoise = Table[10^x, {x, 0, 4, 0.01}];
noiseDensity = Table[
  enTotal/Sqrt[1 + (f/fn)^2],
  {f, freqNoise}
];

noisePlot = ListLogLogPlot[
  Transpose[{freqNoise, noiseDensity*10^9}],
  PlotLabel -> "Input-Referred Noise Density",
  AxesLabel -> {"Frequency (Hz)", "Noise (nV/√Hz)"},
  PlotTheme -> "Detailed",
  GridLines -> Automatic
];

(* Stability Analysis *)
(* Bode Plot *)
bodePlot = BodePlot[systemTF,
  {10^0, 10^6},
  PlotTheme -> "Detailed",
  GridLines -> Automatic,
  PlotLabel -> "System Frequency Response"
];

(* Calculate Stability Margins *)
{gainMargin, phaseMargin, crossoverFreqs} = 
  MarginCalculate[systemTF];

Print["Stability Margins:"];
Print["Gain Margin: ", N[20*Log10[gainMargin], 3], " dB"];
Print["Phase Margin: ", N[phaseMargin*180/π, 3], " degrees"];

(* Monte Carlo Analysis *)
SeedRandom[1234];
nIterations = 1000;
rTol = 0.01;  (* 1% resistor tolerance *)
cTol = 0.05;  (* 5% capacitor tolerance *)

(* Component variations *)
rVar = Table[
  10*10^3*(1 + rTol*RandomReal[NormalDistribution[0, 1]]),
  {nIterations}
];
cVar = Table[
  15*10^-9*(1 + cTol*RandomReal[NormalDistribution[0, 1]]),
  {nIterations}
];

(* Calculate distributions *)
cutoffFreqs = 1/(2π) * 1/(rVar*cVar);
gainsMC = Table[
  gainNominal*(1 + rTol*RandomReal[NormalDistribution[0, 1]]),
  {nIterations}
];

(* Plot Monte Carlo results *)
monteCarloPlot = GraphicsGrid[{
  {Histogram[cutoffFreqs,
    PlotLabel -> "Cutoff Frequency Distribution",
    AxesLabel -> {"Frequency (Hz)", "Count"}]},
  {Histogram[gainsMC,
    PlotLabel -> "Gain Distribution",
    AxesLabel -> {"Gain", "Count"}]}
}];

(* Statistical Analysis *)
Print["\nMonte Carlo Analysis Results:"];
Print["Cutoff Frequency: ", 
  Mean[cutoffFreqs], " ± ", StandardDeviation[cutoffFreqs], " Hz"];
Print["Gain: ", 
  Mean[gainsMC], " ± ", StandardDeviation[gainsMC]];

(* Root Locus Analysis *)
rootLocusPlot = RootLocusPlot[systemTF,
  PlotRange -> {{-2000, 200}, {-2000, 2000}},
  PlotTheme -> "Detailed",
  GridLines -> Automatic,
  PlotLabel -> "Root Locus"
];

(* Step Response Analysis *)
stepResponse = Plot[
  Evaluate[OutputResponse[systemTF, UnitStep[t], t]],
  {t, 0, 0.01},
  PlotRange -> All,
  PlotLabel -> "Step Response",
  AxesLabel -> {"Time (s)", "Amplitude"},
  PlotTheme -> "Detailed",
  GridLines -> Automatic
];

(* Nyquist Plot *)
nyquistPlot = NyquistPlot[systemTF,
  PlotRange -> {{-2, 2}, {-2, 2}},
  PlotTheme -> "Detailed",
  GridLines -> Automatic,
  PlotLabel -> "Nyquist Plot"
];

(* Export Results *)
Export["stability_analysis.pdf", {
  tempPlot, psrrPlot, noisePlot, bodePlot,
  monteCarloPlot, rootLocusPlot, stepResponse, nyquistPlot
}];

(* System Performance Metrics *)
(* Calculate key performance indicators *)
bandWidth = N[crossoverFreqs[[1]]/(2π), 3];
settling = N[4/(dampingRatio*wn), 5];
peakOvershoot = N[100*Exp[-π*dampingRatio/Sqrt[1 - dampingRatio^2]], 3];

Print["\nSystem Performance Metrics:"];
Print["Bandwidth: ", bandWidth, " Hz"];
Print["Settling Time: ", settling*1000, " ms"];
Print["Peak Overshoot: ", peakOvershoot, " %"];

(* Noise Analysis Results *)
noiseBW = fn*π/2;
rmsNoise = enTotal*Sqrt[noiseBW];
snr = 20*Log10[1/rmsNoise];  (* Assuming 1V full scale *)

Print["\nNoise Performance:"];
Print["Total RMS Noise: ", N[rmsNoise*10^6, 3], " µVrms"];
Print["Signal-to-Noise Ratio: ", N[snr, 3], " dB"];
