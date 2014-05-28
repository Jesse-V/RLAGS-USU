function [SerialLink,Error] = i3dmgx3_OpenPort(ComNum)
%Creates a serial link on a specified port using the default MicroStrain
%specifications for each property
%
%Arguments: ComNum - COM port number
%
%Returns:   SerialLink - Handle of serial link
%           Error - Error number

[SerialLink,Error] = openPort(ComNum,512,512); %Open serial link with maximum buffer sizes
if Error == 0
    Error = setCommParameters(SerialLink,115200,8,0,1); %Set parameters to default MicroStrain values
    if Error == 0
        Error = setCommTimeouts(SerialLink,16); %Set timeout to a large value - must be large to account for long gaps when data rate is set to a very low value
    end
end