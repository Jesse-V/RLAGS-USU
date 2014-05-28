function Bytes = short2bytes(Input)
%Converts a short signed integer into 2 bytes.
%
%Arguments: Input - Vector of short signed integers to be converted
%
%Returns:   2-column matrix containing bytes

InputSize = length(Input);
Bin = dec2bin(Input+32768,16); %Convert bytes to binary
Bytes = reshape(bin2dec(reshape(sprintf('%d',Bin'-48),8,InputSize*2)'),2,InputSize)'; %Convert binary to decimal