function Int = convert2long(Input)
%Converts 4 bytes into a long signed integer.
%
%Arguments: Input - four-column matrix containing bytes to be converted 
%           (one integer per row)
%
%Returns:   Long signed integer

if size(Input)*[1;0] == 4 && size(Input)*[0;1] == 1
    Input = Input';
end
Bin = [dec2bin(Input(:,1),8),dec2bin(Input(:,2),8),dec2bin(Input(:,3),8),dec2bin(Input(:,4),8)]; %Convert bytes to binary
Int = bin2dec(Bin)-2147483648; %Convert binary to decimal