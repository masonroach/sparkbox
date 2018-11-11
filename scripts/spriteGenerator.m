%%% PNG to 565 bitmap converter
clear variables
clear figures

%% Input parameters
inputPng = sprintf('testSprite3.png');
outputFile = strrep(inputPng, '.png', '.sparksprite');

%% Parse png data
[RGB, map, alpha] = imread(inputPng);
% Seperate into RGBA. When diving uint8 in MATLAB, automatically rounds.
% ./8 rounds to the third LSB.
R = RGB(:,:,1)./8;
G = RGB(:,:,2)./4;
B = RGB(:,:,3)./8;
A = bitsrl(alpha, 7);

height = size(A, 1);
width = size(A, 2);

%% Build the color palette
palette = zeros(15, 3);
pColors = 0;
for y = 1:height
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

% Convert the picture to an array
converted = zeros(width*height,1);
for y = 1:height
    for x = 1:width
        % Check to see if the pixel is transparent
        if (~A(y,x))
            converted((y-1)*width + x) = 0; % write 0 for transparent values
            continue;
        end
        
        for pnum = 1:pColors
            % Find the color in the palette
            if ( (R(y,x) == palette(pnum, 1)) && ...
                 (G(y,x) == palette(pnum, 2)) && ...
                 (B(y,x) == palette(pnum, 3)) )
                
                converted((y-1)*width + x) = pnum;
                break;
            end
        end
    end
end

%% Writing outputs
% Open output file
fout = fopen(outputFile, 'w');

% Write metadata
% uint16: Width
fwrite(fout, size(A, 1), 'uint16');

% uint16: Height
fwrite(fout, size(A, 2), 'uint16');

% uint16: number of colors in the palette
fwrite(fout, pnum, 'uint16');

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
fprintf(cout, '\t0x%04X,\t// Height = %d\n', height, height);
fprintf(cout, '\t0x%04X,\t// Colors = %d\n', pColors, pColors);
fprintf(cout, '\t0x%04X,\t// Reserved\n', 0);
fprintf(cout, '\t0x%04X,\t// Reserved\n', 0);
fprintf(cout, '\t0x%04X,\t// Reserved\n', 0);
fprintf(cout, '\t0x%04X,\t// Reserved\n', 0);

% Print palette
for i = 1:15
    fprintf(cout, '\t0x%04X,\t// Palette.%d\n', finalPalette(i), i);
end

% Print data as uint16 to mimic being in a file
for i = 1:4:size(converted)
    fprintf(cout, '\t0x%x%x%x%x,\t// Data[%d..%d]\n', converted(i+3), converted(i+2), converted(i+1), converted(i), i, i+3);
end

% Print footer
fprintf(cout, '};\n');

% Close file
fclose(cout);
