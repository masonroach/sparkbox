% This script is used to generate a .WAV file
% Using 4 user defined frequencies

% Sampling frequency (Same as CDs)
Fs = 44.1e3;

% To meet spec, frequencies must be between .1-3 kHz
f1 = 110;
f2 = 220;
f3 = 440;
f4 = 880;

% Round frequency to nearest 5 Hz to limit array size
f1_rd = round(f1 / 5) * 5;
f2_rd = round(f2 / 5) * 5;
f3_rd = round(f3 / 5) * 5;
f4_rd = round(f4 / 5) * 5;

% Frequency of the combined waveform
f = gcd(gcd(gcd(f1_rd,f2_rd), f3_rd), f4_rd);

% Find number of samples to represent frequency f
numSamples = floor(Fs/f);

% Find data points
n = linspace(0,numSamples, numSamples);
y = sin(2*pi*n*f1/Fs)+sin(2*pi*n*f2/Fs)+sin(2*pi*n*f3/Fs)+sin(2*pi*n*f4/Fs);
% Scale data to int16 size
y = y + abs(min(y));
y = round(y / max(y) * (2^(16) - 1)) - 2^(15) - 1;

y_before = y;
% Remove any noise/clacking
diff = y(numSamples-1) - y(1);
while (diff > 0)
  numSamples = numSamples - 1;
  y = y(1:numSamples);
  diff = y(numSamples-1) - y(1);
end

% Plot simulated DAC output
nsim = linspace(0, numSamples * 10, numSamples * 10);
y = y(:).';
ysim = [y;y;y;y;y;y;y;y;y;y];
ysim = ysim(:);

plot (nsim / 10, ysim, 'LineWidth', 2);
grid on;

% Generate WAV file data here
% Find subchunksize and chunksize
subchunkSize = length(y) * 2 - 2;
chunkSize = 36 + subchunkSize;

% Always overwrite previously existing file
fid = fopen('sinewave.wav', 'w', 'b');

% Write RIFF specifier
fprintf(fid, 'RIFF');
% Write chunk size
fwrite(fid, chunkSize,'uint32', 'l');
% Write WAV format specifier
fprintf(fid, 'WAVEfmt');
fwrite(fid, 32,'uint8');
% Write subchunkSize1 = 16
fwrite(fid, 16,'uint32', 'l');
% Write AudioFormat = 1, (PCM)
fwrite(fid, 1,'uint16', 'l');
% Write numChannels = 1
fwrite(fid, 1,'uint16', 'l');
% Write sample rate
fwrite(fid, Fs,'uint32', 'l');
% Write byte rate = Fs * numChannels * bitsPerSample / 8
fwrite(fid, Fs*1*16/8,'uint32', 'l');
% Write blockAlign = numChannels * bitsPerSample / 8
fwrite(fid, 1*16/8,'uint16', 'l');
% Write bitsPerSample = 16
fwrite(fid, 16,'uint16', 'l');
% Write 'data' specifier
fprintf(fid, 'data');
% Write subchunkSize2
fwrite(fid, subchunkSize,'uint32', 'l');
% Write all data
fwrite(fid, y, 'int16', 'l');

% Close file
fclose(fid);
