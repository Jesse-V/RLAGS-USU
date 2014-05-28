function [Packet,Error] = i3dmgx3_AccelAngMagRate(SerialLink)
%Reads acceleration, angular Rate, and magnetometer vector from sensor
%
%Arguments: SerialLink - Handle of serial link
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_ACCEL_ANG_MAG_VECTO'); %Write command to device
    if Error == 0
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_ACCEL_ANG_MAG_VECTO'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end