function Bytes = ulong2bytes(Input)
%Converts a long unsigned integer into 4 bytes.
%
%Arguments: Input - Vector of long unsigned integers to be converted
%
%Returns:   4-column matrix containing bytes

InputSize = length(Input);
Bin = dec2bin(Input,32); %Convert bytes to binary
Bytes = reshape(bin2dec(reshape(sprintf('%d',Bin'-48),8,InputSize*4)'),4,InputSize)'; %Convert binary to decimal