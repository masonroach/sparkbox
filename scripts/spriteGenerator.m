%%% PNG to 565 bitmap converter
clear variables
clear figures

%% Input parameters
inputPng = input('Input .png file to convert: ', 's');
numFrames = input('Input the number of frames in the sprite sheet: ');
outputFile = strrep(inputPng, '.png', '.sparksprite');

%% Parse png data
[RGB, map, alpha] = imread(inputPng);
% Seperate into RGBA. When diving uint8 in MATLAB, automatically rounds.
R = RGB(:,:,1).*(31/255);
G = RGB(:,:,2).*(63/255);
B = RGB(:,:,3).*(31/255);
A = bitsrl(alpha, 7);

% Get dimensions
width = size(A, 2);
sheetHeight = size(A, 1);
frameHeight = sheetHeight/numFrames;

%% Build the color palette
palette = zeros(15, 3);
pColors = 0;
for y = 1:sheetHeight
    for x = 1:width
    
        if (A(y,x)) % Check to see if the pixel is transparent
            % If no colors are in the palette yet, add the color
            if (pColors == 0)
                    palette(pColors+1, 1) = R(y,x);
                    palette(pColors+1, 2) = G(y,x);
                    palette(pColors+1, 3) = B(y,x);
                    pColors = pColors + 1;
                    
                    continue;
            end
            for pnum = 1:pColors
                
                % If the color is already on the palette, skip
                if ( (R(y,x) == palette(pnum, 1)) && ...
                     (G(y,x) == palette(pnum, 2)) && ...
                     (B(y,x) == palette(pnum, 3)))
                    break;
                end
                
                % If the color is not on the palette add it
                if (pnum == pColors)
                    palette(pColors+1, 1) = R(y,x);
                    palette(pColors+1, 2) = G(y,x);
                    palette(pColors+1, 3) = B(y,x);
                    pColors = pColors + 1;
                    
                    break;
                end
            end
        end
    end
end

% Build 565 palette
finalPalette = bitsll(palette(:, 1), 11) ...
    + bitsll(palette(:, 2), 5) + palette(:, 3);

if (size(finalPalette, 1) > 15)
    error('Error: Too many colors in the palette (%d). Reduce the number of colors to 15 or less and try again.', size(finalPalette, 1))
end

% Convert the picture into seperate frame arrays
converted = zeros(ceil(width*frameHeight/4)*4, numFrames);
for f = 1:numFrames
    for y = 1:frameHeight
        for x = 1:width
            
            % Check if pixel is transparent
            if (~A(y + (f-1)*frameHeight, x))
                converted((y-1)*width + x, f) = 0; % Write 0 for transparent values
                continue;
            end
            
            % Iterate through the palette to find the correct color index
            for pnum = 1:pColors
                % Check RGB values to find the color
                if ((R(y + (f-1)*frameHeight,x) == palette(pnum, 1)) && ...
                    (G(y + (f-1)*frameHeight,x) == palette(pnum, 2)) && ...
                    (B(y + (f-1)*frameHeight,x) == palette(pnum, 3)) )
                
                    converted((y-1)*width + x, f) = pnum;
                    break;
                end
            end
        end
    end
end

%% Writing outputs
% Open output file
fout = fopen(outputFile, 'w');

% Write metadata
% uint16: Width
fwrite(fout, width, 'uint16');

% uint16: Height
fwrite(fout, frameHeight, 'uint16');

% uint16: frames, palette
fwrite(fout, and(bitsrl(uint16(numFrames), 8), pColors), 'uint16');

% uint16: Reserved
fwrite(fout, 0, 'uint16');
fwrite(fout, 0, 'uint16');
fwrite(fout, 0, 'uint16');
fwrite(fout, 0, 'uint16');

% uint16[16]: Palette colors
fwrite(fout, finalPalette, 'uint16');

% ubit4[?]: bitmap
fwrite(fout, converted, 'ubit4');

% Close output file
fclose(fout);

%% DEBUGGING ONLY
% Output c file

% Open file
coutName = strrep(inputPng, '.png', '.c');
cout = fopen(coutName, 'w');

% Print Headers
fprintf(cout, 'const uint16_t fakeSpriteFile[] = {\n');
fprintf(cout, '\t0x%04X,\t// Width = %d\n', width, width);
fprintf(cout, '\t0x%04X,\t// Height = %d\n', frameHeight, frameHeight);
fprintf(cout, '\t0x%02X%01X%01X,\t// numFrames = %d, 4 reserved bits, Colors = %d\n', numFrames, 0, pColors, numFrames, pColors);
fprintf(cout, '\t0x%04X,\t// Reserved\n', 0);
fprintf(cout, '\t0x%04X,\t// Reserved\n', 0);
fprintf(cout, '\t0x%04X,\t// Reserved\n', 0);
fprintf(cout, '\t0x%04X,\t// Reserved\n', 0);

% Print palette
for i = 1:15
    fprintf(cout, '\t0x%04X,\t// Palette.%d\n', finalPalette(i), i);
end

% Print data as uint16 to mimic being in a file
for f = 1:size(converted, 2)
    for i = 1:4:size(converted, 1)
        fprintf(cout, '\t0x%x%x%x%x,\t// Frame[%d], Data[%d..%d]\n', ...
            converted(i+3, f), converted(i+2, f), ...
            converted(i+1, f), converted(i, f), f-1, i, i+3);
    end
end

% Print footer
fprintf(cout, '};\n');

% Close file
fclose(cout);
