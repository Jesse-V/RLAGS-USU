function Error = purgePort(SerialLink)
%Clears the input and output buffers
%
%Arguments: SerialLink - Handle of serial link
%
%Returns:   Error number

Error = 0;
try
    ByteNum = (get(SerialLink,'BytesAvailable')); %Find number of bytes available to read on input buffer
    if ByteNum > 0
        fread(SerialLink,ByteNum); %Read all available bytes to clear buffer
    end
catch %#ok<CTCH>
    Error = 7; %Could not clear buffer
end