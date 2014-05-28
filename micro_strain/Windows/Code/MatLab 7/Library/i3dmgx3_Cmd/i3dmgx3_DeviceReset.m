function [Error] = i3dmgx3_DeviceReset(SerialLink)
%Does a soft reset of the sensor
%
%Arguments: SerialLink - Handle of serial link
%
%Returns:   Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_DEVICE_RESET'); %Write command to device
end