function [Packet,Error] = i3dmgx3_ModePreset(SerialLink,ModeSelector)
%Sets or reads the mode that the sensor will enter upon powering up
%
%Arguments: SerialLink - Handle of serial link
%           ModeSelector - 0 to read mode, 1 for Active, 2 for Continuous,
%                          3 for Idle
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_MODE_PRESET'); %Write command to device
    if Error == 0
        fwrite(SerialLink,ModeSelector)
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_MODE_PRESET'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end