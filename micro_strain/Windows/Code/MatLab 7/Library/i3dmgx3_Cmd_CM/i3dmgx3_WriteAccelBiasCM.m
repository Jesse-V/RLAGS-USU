function [Packet,Error] = i3dmgx3_WriteAccelBiasCM(SerialLink,AccelBias,SampleRate)
%Writes the accelerometer bias correction to sensor
%
%Arguments: SerialLink - Handle of serial link
%           AccelBias - Vector of accelerometer bias correction values
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_WRITE_ACEL_BIAS_COR'); %Write command to device
    if Error == 0
        fwrite(SerialLink,AccelBias)
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_WRITE_ACEL_BIAS_COR',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end