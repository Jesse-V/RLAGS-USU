function [Packet,Error] = i3dmgx3_SetReadMode(SerialLink,ModeSelector)
%Sets or reads the data mode
%
%Arguments: SerialLink - Handle of serial link
%           ModeSelector - 0 to read mode, 1 for Active, 2 for Continuous,
%                          3 for Idle, 4 for Sleep, 5 for Deep Sleep
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_SET_READ_MODE'); %Write command to device
    if Error == 0
        fwrite(SerialLink,ModeSelector)
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_SET_READ_MODE'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end