function [Packet,Error] = i3dmgx3_TransferNonVolatileCM(SerialLink,TransferQuantity,SampleRate)
%Transfers current bias correction values to non-volatile memory
%
%Arguments: SerialLink - Handle of serial link
%           TransferQuantity - Transfer quantity value
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_TRANSFER_NONV_MEM'); %Write command to device
    if Error == 0
        fwrite(SerialLink,0)
        fwrite(SerialLink,TransferQuantity)
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_TRANSFER_NONV_MEM',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end