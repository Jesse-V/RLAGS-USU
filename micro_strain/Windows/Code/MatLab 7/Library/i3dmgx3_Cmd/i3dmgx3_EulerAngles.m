function [Packet,Error] = i3dmgx3_EulerAngles(SerialLink)
%Reads the euler angles from sensor
%
%Arguments: SerialLink - Handle of serial link
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_EULER_ANGLES'); %Write command to device
    if Error == 0
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_EULER_ANGLES'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end