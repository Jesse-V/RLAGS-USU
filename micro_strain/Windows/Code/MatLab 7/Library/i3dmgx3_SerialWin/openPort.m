function [SerialLink,Error] = openPort(ComNum,InBuff,OutBuff)
%Creates a serial link on a specified port.
%
%Arguments: ComNum - COM port number
%           InBuff - Size of input buffer
%           OutBuff - Size of output buffer
%
%Returns:   SerialLink - Handle of serial link
%           Error - Error number

Error = 0;
try
    ComNum = sprintf('COM%d',ComNum); %Convert COM # to string
    SerialLink = serial(ComNum); %Create handle for serial link
    fopen(SerialLink) %Open serial link
    set(SerialLink,'InputBufferSize',InBuff,'OutputBufferSize',OutBuff) %Set buffer sizes
catch SerialLink
    Error = 1; %Could not open serial link on specified port
end