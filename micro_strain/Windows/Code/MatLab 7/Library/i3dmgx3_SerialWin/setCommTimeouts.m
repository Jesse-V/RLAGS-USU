function Error = setCommTimeouts(SerialLink,Timeout)
%Sets the parameters of a serial port.
%
%Arguments: SerialLink - Handle of serial link
%           Timeout - Maximum time allowed between writing or reading two
%           values before an error message is returned
%
%Returns:   Error number
 
Error = 0;
try
    set(SerialLink,'Timeout',Timeout); %Set timeouts
catch %#ok<CTCH>
    Error = 3; %Could not set timeouts on specified port
end