function Record = i3dmgx3_ReadNextBuffer(SerialLink,Record)
%Reads the next record in continuous mode, and when used many times in
%succession, adds all the packets to a cell array.
%
%Arguments: SerialLink - Handle of serial link
%           Record - Name of cell array where data packets are to be stored
%
%Returns:   Cell array of data records with latest record tacked on the end

CommandArray = i3dmgx3_Cmd; %Call command array
RecordSize = size(Record);
CommandArraySize = size(CommandArray);
Read = 1; %Activate Read == 1 loop
ByteNum = 1;
Head = Record{RecordSize(1)}; %Get remainder from previous buffer
BufferSize = get(SerialLink,'BytesAvailable'); %Get number of bytes availble on input buffer
if BufferSize > 0
    Tail = fread(SerialLink,BufferSize); %Read entire buffer
    Buffer = [Head,Tail']; %Combine remainder with newly read buffer
elseif isempty(Head) == 0
    Buffer = Head(1);
else
    Read = 0; %Skips over Read == 1 loop if there are no bytes available, leaving the record unchanged
end
while Read == 1 && ByteNum < length(Buffer)
    CommandNum = 2;
    while CommandNum <= CommandArraySize(1)-1 %Try all possible commands
        CommandBytes = CommandArray{CommandNum,2}; %Find command bytes
        ResponseLength = CommandArray{CommandNum,4}; %Find number of bytes in data packet
        if Buffer(ByteNum) == CommandBytes(1) %See if first byte is potential command byte
            if length(Buffer)-ByteNum+1 >= ResponseLength %See if enough bytes remain to make an entire packet
                Packet = Buffer(ByteNum:ByteNum+ResponseLength-1);
                if i3dmgx3_CalcChecksum(Packet) == i3dmgx3_Checksum(Packet) %Evaluate checksum
                    CommandNum = CommandArraySize(1)+1;
                    ByteNum = ByteNum+length(Packet)-1;
                    Record{RecordSize(1),:} = Packet; %Add new packet to record
                    if length(Buffer) >= ByteNum
                        Record{RecordSize(1)+1,1} = Buffer(ByteNum:length(Buffer)); %Add remainder to end of record
                    else
                        Record{RecordSize(1)+1,1} = 0;
                    end
                    RecordSize(1) = RecordSize(1)+1;
                end
            end
        end
        CommandNum = CommandNum+1;
    end
    ByteNum = ByteNum+1;
end