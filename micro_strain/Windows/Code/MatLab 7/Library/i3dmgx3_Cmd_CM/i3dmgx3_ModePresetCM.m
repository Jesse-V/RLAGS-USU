function [Packet,Error] = i3dmgx3_ModePresetCM(SerialLink,ModeSelector,SampleRate)
%Sets or reads the mode that the sensor will enter upon powering up
%
%Arguments: SerialLink - Handle of serial link
%           ModeSelector - 0 to read mode, 1 for Active, 2 for Continuous,
%                          3 for Idle
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_MODE_PRESET'); %Write command to device
    if Error == 0
        fwrite(SerialLink,ModeSelector)
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_MODE_PRESET',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end