function [Error] = i3dmgx3_FirmwareUpdate(SerialLink)
%Puts the sensor in Firmware Update mode
%
%Arguments: SerialLink - Handle of serial link
%
%Returns:   Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_FIRMWARE_UPDATE'); %Write command to device
end