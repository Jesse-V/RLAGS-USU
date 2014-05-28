function Error = i3dmgx3_SendBuffData(SerialLink,Command)
%Sends a command to the sensor
%
%Arguments: SerialLink - Handle of serial link
%           Command - A string from the command array in i3dmgx3_Cmd.m
%
%Returns:   Error number

Error = 0;
CommandArray = i3dmgx3_Cmd; %Call command array
try
    CommandNum = strmatch(Command,CommandArray(:,1)); %Find command
    CommandBytes = CommandArray{CommandNum,2}; %Find command bytes
    CommandLength = length(CommandBytes); %Find number of command bytes
    for ByteNum = 1:CommandLength
        fwrite(SerialLink,CommandBytes(ByteNum)) %Write command to device
    end
catch %#ok<CTCH>
    Error = 4; %Could not write to device on specified port
end