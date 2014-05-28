function [Packet,Error] = i3dmgx3_WriteEEPROMValue(SerialLink,EEPROMAddress,EEPROMWord)
%Writes word to sensor's EEPROM
%
%Arguments: SerialLink - Handle of serial link
%           EEPROMAddress - Vector with address of EEPROM word
%           EEPROMWord - Vector with word to be written to EEPROM
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_WRITE_WORD_EEPROM'); %Write command to device
    if Error == 0
        fwrite(SerialLink,EEPROMAddress)
        fwrite(SerialLink,EEPROMWord) %Write EEPROM word to device
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_WRITE_WORD_EEPROM'); %Read EEPROM word from device
        if Error ~= 0
            Packet = [];
        end
    end
else
    Packet = [];
end