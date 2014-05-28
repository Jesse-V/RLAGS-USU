function [Packet,Error] = i3dmgx3_CommSettingsCM(SerialLink,PortSelector,FunctionSelector,BaudRate,PortConfig,SampleRate)
%Sets or reads the various communication settings on the sensor
%
%Arguments: SerialLink - Handle of serial link
%           PortSelector - 1 for UART1, 2 for UART2, 3 for UART3
%           FunctionSelector - 0 to read timer, 1 to restart timer at
%                            specified amount, 2 to restart the PPS counter
%           BaudRate - Rate at which data is sent to and from sensor -
%                    32-bit unsigned integer
%           PortConfig - 0 to disable UART, 1 to enable (UART1 cannot be
%                      disabled)
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_COMM_SETTINGS'); %Write command to device
    if Error == 0
        fwrite(SerialLink,PortSelector)
        fwrite(SerialLink,FunctionSelector)
        fwrite(SerialLink,BaudRate)
        fwrite(SerialLink,PortConfig)
        fwrite(SerialLink,0)
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_COMM_SETTINGS',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end