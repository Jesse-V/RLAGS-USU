function Int = convert2short(Input)
%Converts 2 bytes into a short signed integer.
%
%Arguments: Input - two-column matrix containing bytes to be converted 
%           (one integer per row)
%
%Returns:   Short signed integer

if size(Input)*[1;0] == 2 && size(Input)*[0;1] == 1
    Input = Input';
end
Bin = [dec2bin(Input(:,1),8),dec2bin(Input(:,2),8)]; %Convert bytes to binary
Int = bin2dec(Bin)-32768; %Convert binary to decimal