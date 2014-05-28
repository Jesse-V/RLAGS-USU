function Dec = convert2float32bin(Input)
%Converts matrix consisting of rows of 4 decimal bytes to 32-bit binary 
%floating point values in IEEE-754 format.
%
%Arguments: Input - 4-column matrix containing bytes to be converted
%
%Returns:   Decimal value

if size(Input)*[1;0] == 4 && size(Input)*[0;1] == 1
    Input = Input';
end
Coefficients = [ones(1,23)/2].^(1:23); %#ok<NBRAK>
Bin = strcat(dec2bin(Input(:,1),8),dec2bin(Input(:,2),8),dec2bin(Input(:,3),8),dec2bin(Input(:,4),8))-48;%The "-48" is just a shortcut to turn the character array into a matrix of ones and zeros - the ASCII codes for 0 and 1 are 48 and 49, respectively
Sign = (-Bin(:,1)+.5)*2; %Get signs
Exponent = bin2dec(reshape(sprintf('%d',Bin(:,2:9)),size(Bin)*[1;0],8))-127; %Get exponents
Mantissa = Bin(:,10:32)*Coefficients'+1; %Get mantissas
Dec = Sign.*Mantissa.*(2.^Exponent); %Compute decimal