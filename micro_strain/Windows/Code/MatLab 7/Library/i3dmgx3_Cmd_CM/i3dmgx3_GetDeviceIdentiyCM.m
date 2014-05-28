function [Packet,Error] = i3dmgx3_GetDeviceIdentiyCM(SerialLink,StringSelector,SampleRate)
%Reads the device identifier string from sensor
%
%Arguments: SerialLink - Handle of serial link
%           StringSelector - Hexadecimal byte selecting the string to read
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

StringSelector = num2str(StringSelector);
Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_GET_DEVICE_ID'); %Write command to device
    if Error == 0
        fwrite(SerialLink,hex2dec(StringSelector))
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_GET_DEVICE_ID',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end