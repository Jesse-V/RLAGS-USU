function [Packet,Error] = i3dmgx3_GetDeviceIdentiy(SerialLink,StringSelector)
%Reads the device identifier string from sensor
%
%Arguments: SerialLink - Handle of serial link
%           StringSelector - Hexadecimal byte selecting the string to read
%
%Returns:   Packet - The data packet
%           Error - Error number

StringSelector = num2str(StringSelector);
Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_GET_DEVICE_ID'); %Write command to device
    if Error == 0
        fwrite(SerialLink,hex2dec(StringSelector))
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_GET_DEVICE_ID'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end