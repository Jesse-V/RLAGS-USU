function Bytes = long2bytes(Input)
%Converts a long signed integer into 4 bytes.
%
%Arguments: Input - Vector of long signed integers to be converted
%
%Returns:   4-column matrix containing bytes

InputSize = length(Input);
Bin = dec2bin(Input+2147483648,32); %Convert bytes to binary
Bytes = reshape(bin2dec(reshape(sprintf('%d',Bin'-48),8,InputSize*4)'),4,InputSize)'; %Convert binary to decimal