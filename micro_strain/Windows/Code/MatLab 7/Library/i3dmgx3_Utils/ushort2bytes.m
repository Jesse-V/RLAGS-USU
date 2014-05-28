function Bytes = ushort2bytes(Input)
%Converts a short unsigned integer into 2 bytes.
%
%Arguments: Input - Vector of short unsigned integers to be converted
%
%Returns:   2-column matrix containing bytes

InputSize = length(Input);
Bin = dec2bin(Input,16); %Convert bytes to binary
Bytes = reshape(bin2dec(reshape(sprintf('%d',Bin'-48),8,InputSize*2)'),2,InputSize)'; %Convert binary to decimal