function ByteMat = float32bin2bytes(Dec)
%Converts decimal number into matrix consisting of rows of 4 decimal bytes
%in 32-bit binary floating point IEEE-754 format.
%
%Arguments: Dec - Vector of decimal values to be converted
%
%Returns:   4-column matrix of decimal bytes

InputSize = length(Dec);
Bin = zeros(InputSize,32);
ByteMat = zeros(InputSize,4);
for DecNum = 1:InputSize
    %Determine sign
    if Dec(DecNum) < 0
        Bin(DecNum,1) = 1;
    end
    %If decimal is smaller than precision will allow, treat it as zero
    if abs(Dec(DecNum)) < 5.877471754111438e-039
        if Dec(DecNum) >= 0
            Dec(DecNum) = 5.877471754111438e-039;
        else
            Dec(DecNum) = -5.877471754111438e-039;
        end
    end
    %Determine exponent
    Exponent = 0;
    while abs(Dec(DecNum)) < 1
        Dec(DecNum) = Dec(DecNum)*2;
        Exponent = Exponent-1;
    end
    while abs(Dec(DecNum)) >= 2
        Dec(DecNum) = Dec(DecNum)/2;
        Exponent = Exponent+1;
    end
    Bin(DecNum,2:9) = str2mat(dec2bin(Exponent+127,8))-48;
    %Determine mantissa
    Mantissa = 1;
    for Power = 1:23
        if Mantissa+.5^Power <= abs(Dec(DecNum))
            Bin(DecNum,Power+9) = 1;
            Mantissa = Mantissa+.5^Power;
        end
    end
    %Convert binary to bytes
    BinString = sprintf('%d',Bin(DecNum,:));
    Bytes = [sprintf('%d',BinString(1:8)-48);sprintf('%d',BinString(9:16)-48);sprintf('%d',BinString(17:24)-48);sprintf('%d',BinString(25:32)-48)];
    ByteMat(DecNum,:) = bin2dec(Bytes)';
end