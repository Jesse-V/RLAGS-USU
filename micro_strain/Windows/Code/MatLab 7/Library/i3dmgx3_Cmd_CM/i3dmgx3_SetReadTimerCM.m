function [Packet,Error] = i3dmgx3_SetReadTimerCM(SerialLink,FunctionSelector,NewTimerValue,SampleRate)
%Sets or reads the time stamp
%
%Arguments: SerialLink - Handle of serial link
%           New Timer Value - Value at which to restart timer - 32-bit
%                           unsigned integer
%           FunctionSelector - 0 to read timer, 1 to restart timer at
%                            specified amount, 2 to restart the PPS counter
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_SET_READ_TIMER'); %Write command to device
    if Error == 0
        fwrite(SerialLink,FunctionSelector)
        fwrite(SerialLink,NewTimerValue)
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_SET_READ_TIMER',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end