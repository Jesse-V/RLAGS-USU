function Record = i3dmgx3_ReadNextRecord(SerialLink,Record)
%Reads the next packet in continuous mode
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
Remainder = Record{RecordSize(1)}; %Get remainder from record
BufferSize = get(SerialLink,'BytesAvailable'); %Get number of bytes availble on input buffer
if isempty(Remainder) == 0
    Buffer = Remainder;
elseif BufferSize > 0
    Tail = fread(SerialLink,1); %Read one byte
    Buffer = [Remainder,Tail']; %Combine remainder with newly read byte
else
    Read = 0; %Skips over Read == 1 loop if there are no bytes available, leaving the record unchanged
end
while Read == 1 && ByteNum < BufferSize
    CommandNum = 2;
    while CommandNum <= CommandArraySize(1)-1 %Try all possible commands
        CommandBytes = CommandArray{CommandNum,2}; %Find command bytes
        ResponseLength = CommandArray{CommandNum,4}; %Find number of bytes in data packet
        BufferLength = length(Buffer);
        if Buffer(ByteNum) == CommandBytes(1) %See if first byte is potential command byte
            if ResponseLength-BufferLength+ByteNum-1 > 0
                Buffer = [Buffer,fread(SerialLink,ResponseLength-BufferLength+ByteNum-1)']; %#ok<AGROW> Read enough bytes to complete potential data packet
                BufferLength = length(Buffer);
            end
            PossiblePacket = Buffer(ByteNum:ByteNum+ResponseLength-1)';
            if i3dmgx3_CalcChecksum(PossiblePacket) == i3dmgx3_Checksum(PossiblePacket) %Evaluate checksum
                Record{RecordSize(1)} = PossiblePacket'; %Add new packet to record
                CommandNum = CommandArraySize(1)+1; %Exit CommandNum <= CommandArraySize(1)-1 loop
                ByteNum = ByteNum+length(PossiblePacket);
                if BufferLength >= ByteNum
                    Record{RecordSize(1)+1,1} = Buffer(ByteNum:BufferLength); %Add remainder to end of record
                else
                    Record{RecordSize(1)+1,1} = 0;
                end
                Read = 0;
            end
        end
        CommandNum = CommandNum+1;
    end
    if BufferLength == ByteNum
        Buffer = [Buffer,fread(SerialLink,1)]; %#ok<AGROW> %Read next byte if none of the bytes read so far are command bytes
    end
    ByteNum = ByteNum+1;
end