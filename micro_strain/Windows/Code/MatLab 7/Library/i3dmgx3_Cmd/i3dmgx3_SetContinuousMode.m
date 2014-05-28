function Error = i3dmgx3_SetContinuousMode(SerialLink,DataType)
%Puts the sensor into continuous mode
%
%Arguments: SerialLink - Handle of serial link
%           DataType - Command for the type of data to be read continuously
%
%Returns:   Error number

Error = i3dmgx3_SendBuffData(SerialLink,'CMD_SET_CONTINIOUS'); %Write command to device
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,DataType); %Write data type to device
end