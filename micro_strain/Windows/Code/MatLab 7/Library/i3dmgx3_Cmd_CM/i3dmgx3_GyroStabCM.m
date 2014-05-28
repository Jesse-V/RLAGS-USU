function [Packet,Error] = i3dmgx3_GyroStabCM(SerialLink,SampleRate)
%Reads the gyro-stabilized acceleration, angular rate, and magnetometer
%vector from sensor
%
%Arguments: SerialLink - Handle of serial link
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_GYRO_STAB_A_AR_MG'); %Write command to device
    if Error == 0
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_GYRO_STAB_A_AR_MG',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end