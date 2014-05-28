function [SampleRate,Error] = i3dmgx3_WriteDataRate(SerialLink,SampleRate)
%Writes data rate to the sensor
%
%Arguments: SerialLink - Handle of serial link
%           SampleRate - Desired rate at which device sends out packets
%
%Returns:   Error - Error number
%           SampleRate - Rate at which device sends out packets

Error = setCommTimeouts(SerialLink,16); %Set timeouts to a large value to account for long gaps when data rate is low
if Error == 0
    Read = 1; %Activate Read == 1 loop
    StartTime = now; %Find starting time
    while Read == 1 && 86400*(now - StartTime) <= 16
        [Packet,Error] = i3dmgx3_SamplingSettings(SerialLink,1,1000/SampleRate,131,32,32);
        if Error == 0
            SampleRate = 1000/convert2ushort(Packet(2:3)); %Convert bytes to decimal sample rate
            Read = 0; %Exit Read == 1 loop
            setCommTimeouts(SerialLink,1/SampleRate+.1);
            pause(1/SampleRate) %Wait for next data packet to be sent
            purgePort(SerialLink); %Clear buffer
            fread(SerialLink,1);
        else
            SampleRate = 1;
        end
    end
else
    SampleRate = 1;
end