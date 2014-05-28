%*-------------------------------------------------------------------------
%* (c) 2009 Microstrain, Inc.
%*-------------------------------------------------------------------------
%* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING 
%* CUSTOMERS WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER 
%* FOR THEM TO SAVE TIME. AS A RESULT, MICROSTRAIN SHALL NOT BE HELD LIABLE 
%* FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY 
%* CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY 
%* CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH 
%* THEIR PRODUCTS.
%*-------------------------------------------------------------------------
%*
%*-------------------------------------------------------------------------
%* Change Data Mode
%*
%* This command line application can be used with the MicroStrain 3DM-GX3 
%* sensors to change the data mode of the sensor. There are five modes that
%* the sensor can be set to:
%*    1. Active Mode - Sensors & processor on; data output when requested 
%*    2. Continuous Mode - Sensors & processor on; data output continuously
%*    3. Idle Mode - Sensors off, processor on; data output when requested
%*    4. Sleep Mode - Sensors off, processor @ low power; wakes up at input
%*    5. Deep Sleep - Sensors & processor off; wakes up at input
%* To change to Continuous Mode, the user must issue one of the Hexadecimal 
%* Command Bytes listed in the Data Communications Protocol. The only other
%* input needed from the user is the number of the serial port to which the
%* sensor is connected. This port number can be obtained via the Windows 
%* Device Manager located at C:\WINDOWS\system32\devmgmt.msc. The device 
%* will appear under "Ports (COM & LPT)" as either "CP210x USB to UART 
%* Bridge Controller" or "MicroStrain Virtual COM Port." The script will
%* then return a text confirmation that the mode has been changed.
%*-------------------------------------------------------------------------
function Change_Data_Mode
Run = 1;
while Run == 1
    delete(instrfind);
    SampleRate = 1;
    PortOpen = 0;
    warning off MATLAB:serial:fread:unsuccessfulRead
    %Get COM port # and create serial link
    ComNum = input('\nEnter COM port # (1 for COM1, etc.): ','s');
    if isstrprop(ComNum,'digit')
        ComNum = str2double(ComNum);
        [SerialLink,Error] = i3dmgx3_OpenPort(ComNum);
        if Error == 0
            Error = setCommTimeouts(SerialLink,1/SampleRate+.1);
            if Error == 0
                %Determine mode status and get data rate
                [Packet,Error] = i3dmgx3_SetReadMode(SerialLink,0);
                if Error == 0;
                    Mode = Packet(2);
                    if Mode == 1 || Mode || 3
                        [SampleRate,Error] = i3dmgx3_ReadDataRate(SerialLink);
                    else
                        [SampleRate,Error] = i3dmgx3_ReadDataRateCM(SerialLink,SampleRate);
                    end
                    if Error == 0
                        PortOpen = 1;
                    end
                end
            end
        end
    else
        fprintf('\nInvalid response\n');
        Error = 0;
    end
    while PortOpen == 1
        %Get command from user
        if Mode == 1
            fprintf(sprintf('\nActive Mode\n'));
            Input = input('\nC - Change to Continuous Mode\nI - Change to Idle Mode\nS - Change to Sleep Mode\nZ - Change to Deep Sleep Mode\nQ - Quit\n\n','s');
        elseif Mode == 2
            fprintf(sprintf('\nContinuous Mode\n'));
            Input = input('\nA - Change to Active Mode\nI - Change to Idle Mode\nS - Change to Sleep Mode\nZ - Change to Deep Sleep Mode\nQ - Quit\n\n','s');
        elseif Mode == 3
            fprintf(sprintf('\nIdle Mode\n'));
            Input = input('\nA - Change to Active Mode\nC - Change to Continuous Mode\nS - Change to Sleep Mode\nZ - Change to Deep Sleep Mode\nQ - Quit\n\n','s');
        elseif Mode == 4
            fprintf(sprintf('\nSleep Mode\n'));
            Input = input('\nW - Wake up\nQ - Quit\n\n','s');
        elseif Mode == 5
            fprintf(sprintf('\nDeep Sleep Mode\n'));
            Input = input('\nW - Wake up\nQ - Quit\n\n','s');
        end
        %Execute command
        if length(Input) == 1
            if Input == 'C' || Input == 'c'
                %Turn continuous mode on
                if Mode == 1 || Mode == 3
                    %Get command byte from user
                    Hex = input('\nEnter hexadecimal command byte: ','s');
                    GetHex = 1;
                    while GetHex == 1 
                        if length(Hex) == 4
                            if strcmpi(Hex(1:2),'0x') == 1
                                Hex = Hex(3:4);
                            end
                        end
                        if length(Hex) == 2
                            if isstrprop(Hex,'xdigit') == [1 1] %#ok<BDSCA>
                                Command = i3dmgx3_Hex2Command(Hex);
                                if isempty(strmatch('Invalid',{Command}))
                                    GetHex = 0;
                                else
                                    Hex = input('\nInvalid response. Enter hexadecimal command byte: ','s');
                                end
                            else
                                Hex = input('\nInvalid response. Enter hexadecimal command byte: ','s');
                            end
                        else
                            Hex = input('\nInvalid response. Enter hexadecimal command byte: ','s');
                        end
                    end
                    [Packet,Error] = i3dmgx3_ContinuousPreset(SerialLink,Command);
                    if Error == 0
                        [Packet,Error] = i3dmgx3_SetReadMode(SerialLink,2);
                        if Error == 0
                            Mode = 2;
                        end
                    end
                else
                    fprintf('\nInvalid response\n');
                end
            elseif Input == 'A' || Input == 'a'
                %Turn active mode on
                if Mode == 3
                    [Packet,Error] = i3dmgx3_SetReadMode(SerialLink,1);
                    if Error == 0
                        Mode = 1;
                    end
                elseif Mode == 2
                    [Packet,Error] = i3dmgx3_SetReadModeCM(SerialLink,1,SampleRate);
                    if Error == 0
                        Mode = 1;
                    end
                else
                    fprintf('\nInvalid response\n');
                end
            elseif Input == 'I' || Input == 'i' || Input == '1'
                %Turn idle mode on
                if Mode == 1
                    [Packet,Error] = i3dmgx3_SetReadMode(SerialLink,3);
                    if Error == 0
                        Mode = 3;
                    end
                elseif Mode == 2
                    [Packet,Error] = i3dmgx3_SetReadModeCM(SerialLink,4,SampleRate);
                    if Error == 0
                        Mode = 3;
                    end    
                else
                    fprintf('\nInvalid response\n');
                end
            elseif Input == 'S' || Input == 's'
                %Turn sleep mode on
                if Mode == 2 || Mode == 1 || Mode == 3
                    [Packet,Error] = i3dmgx3_SetReadMode(SerialLink,4);
                    if Error == 0
                        Mode = 4;
                    end
                else
                    fprintf('\nInvalid response\n');
                end
            elseif Input == 'Z' || Input == 'z'
                %Turn deep sleep mode on
                if Mode == 2 || Mode == 1 || Mode == 3
                    [Packet,Error] = i3dmgx3_SetReadMode(SerialLink,5);
                    if Error == 0
                        Mode = 5;
                    end
                else
                    fprintf('\nInvalid response\n');
                end
            elseif Input == 'W' || Input == 'w'
                %Wake up from sleep
                if Mode == 4 || Mode == 5
                    fwrite(SerialLink,1)
                    if Error == 0
                        [Packet,Error] = i3dmgx3_SetReadMode(SerialLink,0);
                        if Error == 0;
                            Mode = Packet(2);
                            if Mode == 1 || Mode == 3
                                [SampleRate,Error] = i3dmgx3_ReadDataRate(SerialLink);
                            else
                                [SampleRate,Error] = i3dmgx3_ReadDataRateCM(SerialLink,SampleRate);
                            end
                        end
                    end
                else
                    fprintf('\nInvalid response\n');
                end
            elseif Input == 'q' || Input == 'Q'
                %Terminate the function
                PortOpen = 0;
                Run = 0;
            else
                fprintf('\nInvalid response\n');
            end
        else
            fprintf('\nInvalid response\n');
        end
        if Error ~= 0
            Run = 0;
        end
    end
    closePort(ComNum);
    if Error ~= 0
        %Report error, ask user what to do next
        i3dmgx3_ExplainError(Error)
        AskRestart = 1;
        while AskRestart == 1
            Input = input('\nP - Restart program\nQ - Quit\n\n','s');
            if length(Input) == 1
                if Input == 'P' || Input == 'p'
                    AskRestart = 0;
                    Run = 1;
                elseif Input == 'Q' || Input == 'q'
                    AskRestart = 0;
                    Run = 0;
                else
                    fprintf('\nInvalid response\n');
                end
            else
                fprintf('\nInvalid response\n');
            end
        end
    end
end
warning on MATLAB:serial:fread:unsuccessfulRead
function [SerialLink,Error] = i3dmgx3_OpenPort(ComNum)
%Creates a serial link on a specified port using the default MicroStrain
%specifications for each property
%
%Arguments: ComNum - COM port number
%
%Returns:   SerialLink - Handle of serial link
%           Error - Error number

[SerialLink,Error] = openPort(ComNum,512,512); %Open serial link with maximum buffer sizes
if Error == 0
    Error = setCommParameters(SerialLink,115200,8,0,1); %Set parameters to default MicroStrain values
    if Error == 0
        Error = setCommTimeouts(SerialLink,16); %Set timeout to a large value - must be large to account for long gaps when data rate is set to a very low value
    end
end
function [SerialLink,Error] = openPort(ComNum,InBuff,OutBuff)
%Creates a serial link on a specified port.
%
%Arguments: ComNum - COM port number
%           InBuff - Size of input buffer
%           OutBuff - Size of output buffer
%
%Returns:   SerialLink - Handle of serial link
%           Error - Error number

Error = 0;
try
    ComNum = sprintf('COM%d',ComNum); %Convert COM # to string
    SerialLink = serial(ComNum); %Create handle for serial link
    fopen(SerialLink) %Open serial link
    set(SerialLink,'InputBufferSize',InBuff,'OutputBufferSize',OutBuff) %Set buffer sizes
catch %#ok<CTCH>
    Error = 1; %Could not open serial link on specified port
end
function closePort(ComNum)
%Closes a serial link on a specified port.
%
%Arguments: ComNum - COM port number

ComNum = sprintf('COM%d',ComNum); %Converts COM # to string
SerialLink = instrfind('Port',ComNum);   %Find link on specified port
fclose(SerialLink) %Delete the serial link
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
function [SampleRate,Error] = i3dmgx3_ReadDataRate(SerialLink)
%Reads the data rate from the sensor
%
%Arguments: SerialLink - Handle of serial link
%
%Returns:   Error - Error number
%           SampleRate - Rate at which device sends out packets

Error = setCommTimeouts(SerialLink,16); %Set timeouts to a large value to account for long gaps when data rate is low
if Error == 0
    Read = 1; %Activate Read == 1 loop
    StartTime = now; %Find starting time
    while Read == 1 && 86400*(now - StartTime) <= 16
        [Packet,Error] = i3dmgx3_SamplingSettings(SerialLink,0,0,0,0,0);
        if Error == 0
            SampleRate = 1000/convert2ushort(Packet(2:3)); %Convert bytes to decimal sample rate
            Read = 0; %Exit Read == 1 loop
            setCommTimeouts(SerialLink,1/SampleRate+.1);
            pause(1/SampleRate) %Wait for next data packet to be sent
            purgePort(SerialLink); %Clear buffer
            fread(SerialLink,1);
        else
            SampleRate = 1;
        end
    end
else
    SampleRate = 1;
end
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
function [SampleRate,Error] = i3dmgx3_ReadDataRateCM(SerialLink,SampleRate)
%Reads the data rate from the sensor
%
%Arguments: SerialLink - Handle of serial link
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Error - Error number
%           SampleRate - Rate at which device sends out packets

Error = setCommTimeouts(SerialLink,16); %Set timeouts to a large value to account for long gaps when data rate is low
if Error == 0
    Read = 1; %Activate Read == 1 loop
    StartTime = now; %Find starting time
    while Read == 1 && 86400*(now - StartTime) <= 16
        [Packet,Error] = i3dmgx3_SamplingSettingsCM(SerialLink,0,0,0,0,0,SampleRate);
        if Error == 0
            SampleRate = 1000/convert2ushort(Packet(2:3)); %Convert bytes to decimal sample rate
            Read = 0; %Exit Read == 1 loop
            setCommTimeouts(SerialLink,1/SampleRate+.1);
            pause(1/SampleRate) %Wait for next data packet to be sent
            purgePort(SerialLink); %Clear buffer
            fread(SerialLink,1);
        else
            SampleRate = 1;
        end
    end
else
    SampleRate = 1;
end
function i3dmgx3_ExplainError(Error)
%Display an appropriate error message when an error occurs
%
%Arguments: Error - Error number

ErrorText = i3dmgx3_Errors;
fprintf(['\nError: ' char(ErrorText(Error)) '\n']);
function ErrorMessages = i3dmgx3_Errors
%Creates the Command array in the MatLab workspace
ErrorMessages = {
    'Could not open serial link on specified port';
    'Could not set communication parameters on specified port';
    'Could not set timeouts on specified port';
    'Could not write to device on specified port';
    'Output does not match command';
    'Could not read device';
    'Could not clear buffer';
    'Incorrect checksum'};
function Int = convert2ushort(Input)
%Converts 2 bytes into a short unsigned integer.
%
%Arguments: Input - two-column matrix containing bytes to be converted 
%           (one integer per row)
%
%Returns:   Short unsigned integer

if size(Input)*[1;0] == 2 && size(Input)*[0;1] == 1
    Input = Input';
end
Bin = [dec2bin(Input(:,1),8),dec2bin(Input(:,2),8)]; %Convert bytes to binary
Int = bin2dec(Bin); %Convert binary to decimal
function Command = i3dmgx3_Hex2Command(Hex)
%Takes a hexadecimal byte and finds the proper command name
%
%Arguments: Hex - Hexadecimal command byte
%
%Returns:   Command name

CommandArray = i3dmgx3_Cmd; %Call command array
CommandNames = cell2mat({CommandArray{:,1}}'); %Turn command names into a matrix
CommandHex = (CommandNames(:,27:28));  %Create matrix of just command bytes
CommandNum = strmatch(upper(Hex),CommandHex); %Match given byte to command
if isempty(CommandNum)
    Command = 'Invalid';
else
    Command = CommandArray{CommandNum,1}; %Return command name
end
function [Packet,Error] = i3dmgx3_SetReadMode(SerialLink,ModeSelector)
%Sets or reads the data mode
%
%Arguments: SerialLink - Handle of serial link
%           ModeSelector - 0 to read mode, 1 for Active, 2 for Continuous,
%                          3 for Idle, 4 for Sleep, 5 for Deep Sleep
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_SET_READ_MODE'); %Write command to device
    if Error == 0
        fwrite(SerialLink,ModeSelector)
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_SET_READ_MODE'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end
function [Packet,Error] = i3dmgx3_SetReadModeCM(SerialLink,ModeSelector,SampleRate)
%Sets or reads the data mode
%
%Arguments: SerialLink - Handle of serial link
%           ModeSelector - 0 to read mode, 1 for Active, 2 for Continuous,
%                          3 for Idle, 4 for Sleep, 5 for Deep Sleep
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_SET_READ_MODE'); %Write command to device
    if Error == 0
        fwrite(SerialLink,ModeSelector)
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_SET_READ_MODE',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end
function [Packet,Error] = i3dmgx3_SamplingSettings(SerialLink,FunctionSelector,DataRateDecimation,DataConditioning,GyroAccelFilter,MagFilter)
%Sets or reads the various data sampling settings on the sensor
%
%Arguments: SerialLink - Handle of serial link
%           FunctionSelector - 0 to read settings, 1 to change settings
%           DataRateDecimation - Value that 1000 is divided by to get
%                                sample rate - 16 bit unsigned integer
%           DataConditioning - Data conditioning funciton selector:
%                              Bit 0: If set - Calculate orientation
%                              Bit 1: If set - Enable Coning&Sculling
%                              Bit 4: If set - Set to Little Endian
%                              Bit 5: If set - NaN data is suppressed
%                              Bit 6: If set - Enable finite size corection
%                              Bit 2-3,7-15: If set - Calculate orientation
%                              16-bit unsigned integer
%           GyroAccelFilter - Window size of Gyro and Accel digital filter
%           MagFilter - Magnetometer digital filter window size
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_SAMPLING_SETTINGS'); %Write command to device
    if Error == 0
        fwrite(SerialLink,[FunctionSelector,ushort2bytes(DataRateDecimation),ushort2bytes(DataConditioning),GyroAccelFilter,MagFilter,zeros(1,10)])
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_SAMPLING_SETTINGS'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end
function [Packet,Error] = i3dmgx3_SamplingSettingsCM(SerialLink,FunctionSelector,DataRateDecimation,DataConditioning,GyroAccelFilter,MagFilter,SampleRate)
%Sets or reads the various data sampling settings on the sensor
%
%Arguments: SerialLink - Handle of serial link
%           FunctionSelector - 0 to read settings, 1 to change settings
%           DataRateDecimation - Value that 1000 is divided by to get
%                                sample rate - 16 bit unsigned integer
%           DataConditioning - Data conditioning funciton selector:
%                              Bit 0: If set - Calculate orientation
%                              Bit 1: If set - Enable Coning&Sculling
%                              Bit 4: If set - Set to Little Endian
%                              Bit 5: If set - NaN data is suppressed
%                              Bit 6: If set - Enable finite size corection
%                              Bit 2-3,7-15: If set - Calculate orientation
%                              16-bit unsigned integer
%           GyroAccelFilter - Window size of Gyro and Accel digital filter
%           MagFilter - Magnetometer digital filter window size
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_SAMPLING_SETTINGS'); %Write command to device
    if Error == 0
        fwrite(SerialLink,[FunctionSelector,ushort2bytes(DataRateDecimation),ushort2bytes(DataConditioning),GyroAccelFilter,MagFilter,zeros(1,10)])
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_SAMPLING_SETTINGS',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end