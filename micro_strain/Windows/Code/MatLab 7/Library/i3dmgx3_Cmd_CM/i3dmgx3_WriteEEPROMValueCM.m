function [Packet,Error] = i3dmgx3_WriteEEPROMValueCM(SerialLink,EEPROMAddress,EEPROMWord,SampleRate)
%Writes word to sensor's EEPROM
%
%Arguments: SerialLink - Handle of serial link
%           EEPROMAddress - Vector with address of EEPROM word
%           EEPROMWord - Vector with word to be written to EEPROM
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_WRITE_WORD_EEPROM'); %Write command to device
    if Error == 0
        fwrite(SerialLink,EEPROMAddress) %Write EEPROM address to device
        fwrite(SerialLink,EEPROMWord) %Write EEPROM word to device
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_WRITE_WORD_EEPROM',SampleRate); %Read EEPROM word from device
        if Error ~= 0
            Packet = [];
        end
    end
else
    Packet = [];
end