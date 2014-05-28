function Error = setCommParameters(SerialLink,BaudRate,CharSize,Parity1,StopBits)
%Sets the parameters of a serial port.
%
%Arguments: SerialLink - Handle of serial link
%           BaudRate - Baudrate in hertz (MicroStrain default is 115200)
%           Parity - 0 for none, 1 for odd, 2 for even (default is none)
%           CharSize - Size of byte in bits (deafult is 8)
%           StopBits - 1 or 2 (default is 1)
%
%Returns:   Error number

Error = 0;
try
    ParityStr = {'none','odd','even'};
    Parity = char(ParityStr(Parity1+1)); %Assign parity input string
    set(SerialLink,'BaudRate',BaudRate,'Parity',Parity,'DataBits',CharSize,'StopBits',StopBits); %Set parameters
catch %#ok<CTCH>
    Error = 2; %Could not set communication parameters on specified port
end