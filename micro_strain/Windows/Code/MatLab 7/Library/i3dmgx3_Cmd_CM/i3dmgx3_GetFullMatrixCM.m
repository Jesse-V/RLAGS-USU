function [Packet,Error] = i3dmgx3_GetFullMatrixCM(SerialLink,SampleRate)
%Reads the acceleration, angular rate, magnetometer vector, and orientation
%matrix from sensor
%
%Arguments: SerialLink - Handle of serial link
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_ACEL_ANG_MAG_VEC_OR'); %Write command to device
    if Error == 0
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_ACEL_ANG_MAG_VEC_OR',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end