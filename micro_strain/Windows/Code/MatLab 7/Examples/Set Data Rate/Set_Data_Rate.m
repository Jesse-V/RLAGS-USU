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
%* Set Data Rate
%*
%* This command line application sets the rate at which the MicroStrain 
%* 3DM-GX3 sensors send data while in Continuous mode. The user is asked to
%* input the desired rate, which is written to the device and kept until
%* the user changes it or the device powers down. The default rate is 100
%* Hz. The device sends one packet of data each cycle, meaning that when
%* the sample rate is at 100 Hz, the device will send 1 packet every 100th
%* of a second. The only other input needed from the user is the number of 
%* the serial port at which the sensor is connected. The port number can be
%* obtained via the Windows Device Manager located at 
%* C:\WINDOWS\system32\devmgmt.msc. The device will appear under 
%* "Ports (COM & LPT)" as either "CP210x USB to UART Bridge Controller" or
%* "MicroStrain Virtual COM Port." The function will then return a text
%* confirmation that sample rate has been set to the desired amount.
%*-------------------------------------------------------------------------
function Set_Data_Rate
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
        fprintf(sprintf('\nSample rate is %.3f Hz\n',SampleRate));
        Input = input('\nR - Set data sample rate\nQ - Quit\n\n','s');
        %If device is idling or sleeping, put into active mode
        if Mode == 3 || Mode == 4 || Mode == 5
            [Packet,Error] = i3dmgx3_SetReadMode(SerialLink,1);
            if Error == 0
                Mode = 1;
            end
        end
        %Execute command
        if length(Input) == 1
            if Input == 'R' || Input == 'r'
                %Ask user for sample rate
                GetRate = 1;
                while GetRate == 1
                    SampleRate1 = input('\nEnter desired sample rate in Hz (between 1 and 250): ','s');
                    if isstrprop(SampleRate1,'digit')
                        NewSampleRate = str2double(SampleRate1);
                        if NewSampleRate >= 1 && NewSampleRate <= 250
                            %Set sample rate
                            if Mode == 1
                                [SampleRate,Error] = i3dmgx3_WriteDataRate(SerialLink,NewSampleRate);
                            elseif Mode == 2
                                [SampleRate,Error] = i3dmgx3_WriteDataRateCM(SerialLink,SampleRate,NewSampleRate);
                            end
                            GetRate = 0;
                        else
                            fprintf('\nInvalid response\n');
                        end
                    else
                        fprintf('\nInvalid response\n');
                    end
                end
            elseif Input == 'Q' || Input == 'q'
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
            PortOpen = 0;
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
function [SampleRate,Error] = i3dmgx3_WriteDataRate(SerialLink,SampleRate)
%Writes data rate to the sensor
%
%Arguments: SerialLink - Handle of serial link
%           SampleRate - Desired rate at which device sends out packets
%
%Returns:   Error - Error number
%           SampleRate - Rate at which device sends out packets

Error = setCommTimeouts(SerialLink,16); %Set timeouts to a large value to account for long gaps when data rate is low
if Error == 0
    Read = 1; %Activate Read == 1 loop
    StartTime = now; %Find starting time
    while Read == 1 && 86400*(now - StartTime) <= 16
        [Packet,Error] = i3dmgx3_SamplingSettings(SerialLink,1,1000/SampleRate,131,32,32);
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
function [SampleRate,Error] = i3dmgx3_WriteDataRateCM(SerialLink,NewSampleRate,OldSampleRate)
%Writes data rate to the sensor
%
%Arguments: SerialLink - Handle of serial link
%           NewSampleRate - Desired rate at which device sends out packets
%           OldSampleRate - Current rate at which device sends out packets
%
%Returns:   Error - Error number
%           SampleRate - Rate at which device sends out packets

Error = setCommTimeouts(SerialLink,16); %Set timeouts to a large value to account for long gaps when data rate is low
if Error == 0
    Read = 1; %Activate Read == 1 loop
    StartTime = now; %Find starting time
    while Read == 1 && 86400*(now - StartTime) <= 16
        [Packet,Error] = i3dmgx3_SamplingSettingsCM(SerialLink,1,1000/NewSampleRate,131,32,32,OldSampleRate);
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