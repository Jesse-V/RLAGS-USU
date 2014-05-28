function [Packet,Error] = i3dmgx3_WriteGyroBiasCM(SerialLink,GyroBias,SampleRate)
%Writes the gyroscope bias correction to sensor
%
%Arguments: SerialLink - Handle of serial link
%           GyroBias - Vector of gyroscope bias correction values
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_WRITE_GYRO_BIAS'); %Write command to device
    if Error == 0
        fwrite(SerialLink,GyroBias)
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_WRITE_GYRO_BIAS',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end