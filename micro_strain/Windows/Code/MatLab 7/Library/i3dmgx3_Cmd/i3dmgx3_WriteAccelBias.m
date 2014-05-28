function [Packet,Error] = i3dmgx3_WriteAccelBias(SerialLink,AccelBias)
%Writes the accelerometer bias correction to sensor
%
%Arguments: SerialLink - Handle of serial link
%           AccelBias - Vector of accelerometer bias correction values
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_WRITE_ACEL_BIAS_COR'); %Write command to device
    if Error == 0
        fwrite(SerialLink,AccelBias)
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_WRITE_ACEL_BIAS_COR'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end