function Command = i3dmgx3_Hex2Command(Hex)
%Takes a hexadecimal byte and finds the proper command name
%
%Arguments: Hex - Hexadecimal command byte
%
%Returns:   Command name

CommandArray = i3dmgx3_Cmd; %Call command array
CommandNames = cell2mat({CommandArray{:,1}}'); %Turn command names into a matrix
CommandHex = (CommandNames(:,27:28));  %Create matrix of just command bytes
CommandNum = strmatch(upper(Hex),CommandHex); %Match given byte to command
if isempty(CommandNum)
    Command = 'Invalid';
else
    Command = CommandArray{CommandNum,1}; %Return command name
end