function closePort(ComNum)
%Closes a serial link on a specified port.
%
%Arguments: ComNum - COM port number

ComNum = sprintf('COM%d',ComNum); %Converts COM # to string
SerialLink = instrfind('Port',ComNum);   %Find link on specified port
fclose(SerialLink) %Delete the serial link