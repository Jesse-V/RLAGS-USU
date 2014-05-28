function [Packet,Error] = i3dmgx3_DeltaAngVelMag(SerialLink)
%Reads the change in acceleration, angular rate, and magnetometer vector
%from sensor
%
%Arguments: SerialLink - Handle of serial link
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_DELTA_ANGVEL_MAGV'); %Write command to device
    if Error == 0
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_DELTA_ANGVEL_MAGV'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end