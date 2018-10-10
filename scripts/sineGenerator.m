% This script is used to generate a .WAV file
% Using 4 user defined frequencies

% Sampling frequency
Fs = 100e3;

% To meet spec, frequencies must be between .1-3 kHz
f1 = 3000;
f2 = 3000;
f3 = 3000;
f4 = 3000;


% Frequency of the combined waveform
f = gcd(gcd(gcd(f1,f2), f3), f4);

% Find number of samples to represent frequency f
numSamples = ceil(Fs/f);

% Find data points
n = linspace(0,numSamples, numSamples);
y = sin(2*pi*n*f1/Fs)+sin(2*pi*n*f2/Fs)+sin(2*pi*n*f3/Fs)+sin(2*pi*n*f4/Fs);
y = y + abs(min(y));
y = y / max(y) * 2^(12);

% Plot simulated DAC output
nsim = linspace(0, numSamples * 10, numSamples * 10);
y = y(:).';
ysim = [y;y;y;y;y;y;y;y;y;y];
ysim = ysim(:);

plot (nsim / 10, ysim, 'LineWidth', 2);
grid on;