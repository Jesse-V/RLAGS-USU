function [Packet,Error] = i3dmgx3_CaptureGyroBiasCM(SerialLink,SamplingTime,SampleRate)
%Gets estimated gyroscope bias correction values from sensor
%
%Arguments: SerialLink - Handle of serial link
%           SamplingTime - Sampling time seconds
%           SampleRate - Rate at which device sends out packets
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
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_CAPTURE_GYRO_BIAS',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end