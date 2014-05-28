function Int = convert2ulong(Input)
%Converts 4 bytes into a long unsigned integer.
%
%Arguments: Input - four-column matrix containing bytes to be converted 
%           (one integer per row)
%
%Returns:   Long unsigned integer

if size(Input)*[1;0] == 4 && size(Input)*[0;1] == 1
    Input = Input';
end
Bin = [dec2bin(Input(:,1),8),dec2bin(Input(:,2),8),dec2bin(Input(:,3),8),dec2bin(Input(:,4),8)]; %Convert bytes to binary
Int = bin2dec(Bin); %Convert binary to decimal