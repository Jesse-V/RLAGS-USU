function [Packet,Error] = i3dmgx3_CaptureGyroBias(SerialLink,SamplingTime)
%Gets estimated gyroscope bias correction values from sensor
%
%Arguments: SerialLink - Handle of serial link
%           SamplingTime - Vector with sampling time values
%
%Returns:   Packet - The data packet
%           Error - Error number

SamplingTime = ushort2bytes(SamplingTime*1000);
Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_CAPTURE_GYRO_BIAS'); %Write command to device
    if Error == 0
        fwrite(SerialLink,SamplingTime)
        pause(convert2ushort(SamplingTime)/1000+.1);
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_CAPTURE_GYRO_BIAS'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end