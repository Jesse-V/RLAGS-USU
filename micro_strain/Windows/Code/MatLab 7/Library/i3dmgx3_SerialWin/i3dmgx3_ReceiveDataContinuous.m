function [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,Command,SampleRate,varargin)
InputParser = inputParser;
addOptional(InputParser,'Record',{0})
%Receives data from sensor and finds correct packet
%
%Arguments: SerialLink - Handle of serial link
%           Command - A string from the command array in i3dmgx3_Cmd.m
%           SampleRate - Rate at which device sends out packets
%           Record(optional) - Handle of cell array on which to record data
%
%Returns:   Packet - Data packet, if successfully read, empty matrix if not
%           Record - Updated record with latest packet tacked on the end
%           Error - Error number

if isempty(varargin) %Check whether data record handle is given
    Record = {0};
else
    Record = varargin{1};
end
CommandArray = i3dmgx3_Cmd; %Call command array
CommandNum = strmatch(Command,CommandArray(:,1)); %Find command
CommandBytes = CommandArray{CommandNum,2}; %Find command bytes
Error = 5; %Output does not match command
Read = 1; %Activate Read == 1 loop
Packet = [];
StartTime = now; %Get starting time
while Read == 1 && 86400*(now - StartTime) <= 1/SampleRate+.1 %The Read == 1 loop reads the buffer until it finds a correct packet
    ByteNum = 0;
    while ByteNum == 0;
        PossibleRecord = i3dmgx3_ReadNextRecord(SerialLink,Record); %Find a data packet
        if length(PossibleRecord) > 1
            ByteNum = 1;
        end
        Record = PossibleRecord;
    end
    while ByteNum <= length(Record{length(Record)-1,1})
        PossiblePacket = Record{length(Record)-1,1}; 
        if isempty(PossiblePacket) == 0
            if PossiblePacket(1) == CommandBytes(1) %Check for proper command byte
                Packet = PossiblePacket;
                Error = 0;
                ByteNum = length(Record{length(Record)-1,1})+1;
                Read = 0;
            else
                Packet = [];
                ByteNum = ByteNum+1;
            end
        else
            Packet = [];
            ByteNum = ByteNum+1;
        end
    end
end