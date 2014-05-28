function Error = i3dmgx3_StopContinuousMode(SerialLink)
%Takes the sensor out of continuous mode
%
%Arguments: SerialLink - Handle of serial link
%
%Returns:   Error number

Error = i3dmgx3_SendBuffData(SerialLink,'CMD_STOP_CONTINIOUS'); %Write command to device