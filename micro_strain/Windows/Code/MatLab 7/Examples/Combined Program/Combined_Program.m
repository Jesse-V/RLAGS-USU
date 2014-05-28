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
%* Combined Program
%*
%* This command line application can be used with the MicroStrain 3DM-GX3
%* sensors to execute a variety of commands, including all the actions
%* performed by the three smaller sample programs, plus two additional
%* graphing functions that show how the MatLab plot environment can be
%* utilized. When in active or idle mode, the options are to print the
%* acceleration and angular rate data, set the sample rate, and display a
%* 3-D representation of the sensor showing its orientation in relation to
%* the surface of the earth. When in continuous mode, all the options
%* available in active mode are available, with an additional option to
%* trace a plot of the acceleration and angular rate over time. Also, when
%* in continuous mode, the program asks for a period of time over which to
%* plot the orientation, and the plot of the model updates in real time
%* during this period. In sleep mode, the only option is to wake up the
%* sensor, as the sensor's processors shut down in this mode.
%*-------------------------------------------------------------------------
function Combined_Program
Run = 1;
while Run == 1
%     delete(instrfind);
%     clearvars SerialLink
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
                if Error == 5
                    [Packet,Error] = i3dmgx3_SetReadModeCM(SerialLink,0,SampleRate);
                end
                if Error == 0;
                    Mode = Packet(2);
                    if Mode == 1 || Mode || 3
                        [SampleRate,Error] = i3dmgx3_ReadDataRate(SerialLink);
                        Mode = 1;
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
            fprintf(sprintf('\nActive Mode\nSample rate is %.3f Hz\n',SampleRate));
            Input = input('\nD - Get data for acceleration and angular rate\nR - Set sample rate\nO - Display orientation\nC - Change to Continuous Mode\nI - Change to Idle Mode\nS - Change to Sleep Mode\nZ - Change to Deep Sleep Mode\nQ - Quit\n\n','s');
        elseif Mode == 2
            fprintf(sprintf('\nContinuous Mode\nSample rate is %.3f Hz\n',SampleRate));
            Input = input('\nD - Get data for acceleration and angular rate\nG - Graph acceleration and angular rate\nR - Set sample rate\nO - Display orientation\nA - Change to Active Mode\nI - Change to Idle Mode\nS - Change to Sleep Mode\nZ - Change to Deep Sleep Mode\nQ - Quit\n\n','s');
        elseif Mode == 3
            fprintf(sprintf('\nIdle Mode\nSample rate is %.3f Hz\n',SampleRate));
            Input = input('\nD - Get data for acceleration and angular rate\nR - Set sample rate\nO - Display orientation\nA - Change to Active Mode\nC - Change to Continuous Mode\nS - Change to Sleep Mode\nZ - Change to Deep Sleep Mode\nQ - Quit\n\n','s');
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
                    [Packet,Error] = i3dmgx3_ContinuousPreset(SerialLink,'CMD_ACCELERATION_ANGU');
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
            elseif Input == 'R' || Input == 'r'
                %Ask user for sample rate
                if Mode == 1 || Mode == 2 || Mode == 3
                    GetRate = 1;
                    while GetRate == 1
                        SampleRate1 = input('\nEnter desired sample rate in Hz (between 1 and 250): ','s');
                        if isstrprop(SampleRate1,'digit')
                            NewSampleRate = str2double(SampleRate1);
                            if NewSampleRate >= 1 && NewSampleRate <= 250
                                %Set sample rate
                                if Mode == 1 || Mode == 3
                                    [SampleRate,Error] = i3dmgx3_WriteDataRate(SerialLink,NewSampleRate);
                                    Mode = 1;
                                else
                                    [SampleRate,Error] = i3dmgx3_WriteDataRateCM(SerialLink,NewSampleRate,SampleRate);
                                end
                                GetRate = 0;
                            else
                                fprintf('\nInvalid response\n');
                            end
                        else
                            fprintf('\nInvalid response\n');
                        end
                    end
                else
                    fprintf('\nInvalid response\n');
                end
            elseif Input == 'O' || Input == 'o' || Input == '0'
                %Display 3-D plot of sensor
                if Mode == 1 || Mode == 3
                    [Packet,Error] = i3dmgx3_OrientMatrix(SerialLink);
                    if Error == 0
                        i3dmgx3_DrawSensor(Packet);
                    end
                elseif Mode == 2
                    %Get period from user
                    GetPeriod = 1;
                    while GetPeriod == 1
                        Period1 = input('\nEnter desired period of time (in seconds) to stream the orientation: ','s');
                        if isstrprop(Period1,'digit')
                            Period = str2double(Period1);
                            GetPeriod = 0;
                        else
                            fprintf('\n\nInvalid response\n')
                        end
                    end
                    StartTime = now;
                    while 86400*(now - StartTime) <= Period
                        [Packet,Error] = i3dmgx3_OrientMatrixCM(SerialLink,SampleRate);
                        if Error == 0
                            i3dmgx3_DrawSensor(Packet);
                        end
                        pause(0.001)
                    end
                else
                    fprintf('\nInvalid response\n');
                end
            elseif Input == 'D' || Input == 'd'
                %Get acceleration and angular rate, print in command window
                if Mode == 1 || Mode == 3
                    [Packet,Error] = i3dmgx3_AccelAndAngRate(SerialLink);
                elseif Mode == 2
                    [Packet,Error] = i3dmgx3_AccelAndAngRateCM(SerialLink,SampleRate);
                else
                    fprintf('\nInvalid response\n');
                end
                if Error == 0 && (Mode == 1 || Mode == 2 || Mode == 3)
                    i3dmgx3_PrintAccelAndAngRate(Packet);
                end
            elseif Input == 'g' || Input == 'G'
                %Graph acceleration and angular rate
                if Mode == 2
                    %Get period from user
                    GetPeriod = 1;
                    while GetPeriod == 1
                        Period1 = input('\nEnter desired time period of graph in seconds: ','s');
                        if isstrprop(Period1,'digit')
                            Period = str2double(Period1);
                            GetPeriod = 0;
                        else
                            fprintf('\n\nInvalid response\n')
                        end
                    end
                    Error = i3dmgx3_PlotAccelAndAngRate(SerialLink,Period,SampleRate);
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
            PortOpen = 0;
        end
    end
    if Error ~= 0
        closePort(ComNum);
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
catch SerialLink
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
function Error = i3dmgx3_SendBuffData(SerialLink,Command)
%Sends a command to the sensor
%
%Arguments: SerialLink - Handle of serial link
%           Command - A string from the command array in i3dmgx3_Cmd.m
%
%Returns:   Error number

Error = 0;
CommandArray = i3dmgx3_Cmd; %Call command array
try
    CommandNum = strmatch(Command,CommandArray(:,1)); %Find command
    CommandBytes = CommandArray{CommandNum,2}; %Find command bytes
    CommandLength = length(CommandBytes); %Find number of command bytes
    for ByteNum = 1:CommandLength
        fwrite(SerialLink,CommandBytes(ByteNum)) %Write command to device
    end
catch %#ok<CTCH>
    Error = 4; %Could not write to device on specified port
end
function CommandArray = i3dmgx3_Cmd
%Creates the Command array in the MatLab workspace
CommandArray = {
    'CMD_RAW_ACCELEROMETER   0xC1',193,1,31;
    'CMD_ACCELERATION_ANGU   0xC2',194,1,31;
    'CMD_DELTA_ANGLE_VELOC   0xC3',195,1,31;
    'CMD_SET_CONTINIOUS      0xC4',[196,193,41],3,8;
    'CMD_ORRIENTATION_MAT    0xC5',197,1,43;
    'CMD_ATTITUDE_UP_MATRIX  0xC6',198,1,43;
    'CMD_MAGNETROMETER_VECT  0xC7',199,1,19;
    'CMD_ACCEL_ANG_ORIENT    0xC8',200,1,67;
    'CMD_WRITE_ACEL_BIAS_COR 0xC9',[201,183,68],3,19;
    'CMD_WRITE_GYRO_BIAS     0xCA',[202,18,165],3,19;
    'CMD_ACCEL_ANG_MAG_VECTO 0xCB',203,1,43;
    'CMD_ACEL_ANG_MAG_VEC_OR 0xCC',204,1,79;
    'CMD_CAPTURE_GYRO_BIAS   0xCD',[205,193,41],3,19;
    'CMD_EULER_ANGLES        0xCE',206,1,19;
    'CMD_EULER_ANGLES_ANG_RT 0xCF',207,1,31;
    'CMD_TRANSFER_NONV_MEM   0xD0',[208,193,41],3,9;
    'CMD_TEMPERATURES        0xD1',209,1,15;
    'CMD_GYRO_STAB_A_AR_MG   0xD2',210,1,43;
    'CMD_DELTA_ANGVEL_MAGV   0xD3',211,1,43;
    'CMD_SET_READ_MODE       0xD4',[212,163,71],3,4;
    'CMD_MODE_PRESET         0xD5',[213,186,137],3,4;
    'CMD_CONTINUOUS_PRESET   0xD6',[214,198,107],3,4;
    'CMD_SET_READ_TIMER      0xD7',[215,193,41],3,7;
    'CMD_COMM_SETTINGS       0xD9',[217,195,85],3,10;
    'CMD_STATIONARY_TEST     0xDA',218,1,7;
    'CMD_SAMPLING_SETTINGS   0xDB',[219,168,185],3,19;
    'CMD_WRITE_WORD_EEPROM   0xE4',[228,193,41,0],4,5;
    'CMD_READ_WORD_EEPROM    0xE5',[229,0],2,5;
    'CMD_FIRWARE_VERSION     0xE9',233,1,7;
    'CMD_GET_DEVICE_ID       0xEA',234,1,20;
    'CMD_STOP_CONTINIOUS     0xFA',[250,117,180],1,0;
    'CMD_FIRMWARE_UPDATE     0xFD',[253,115,74],3,0;
    'CMD_DEVICE_RESET        0xFE',[254,158,58],3,0};
function [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,Command)
%Receives data from sensor
%
%Arguments: SerialLink - Handle of serial link
%           Command - A string from the command array in i3dmgx3_Cmd.m
%
%Returns:   Packet - Data packet, if successfully read (empty matrix if not)
%           Error - Error number

CommandArray = i3dmgx3_Cmd; %Call command array
CommandNum = strmatch(Command,CommandArray(:,1)); %Find command
CommandBytes = CommandArray{CommandNum,2}; %Find command bytes
ResponseLength = CommandArray{CommandNum,4}; %Find number of command bytes
Packet = fread(SerialLink,ResponseLength); %Read data packet from device
if isempty(Packet) == 0 %Check for response
    if length(Packet) == ResponseLength; %Check response length
        if Packet(1) == CommandBytes(1) %Check for proper command byte
            if i3dmgx3_CalcChecksum(Packet) == i3dmgx3_Checksum(Packet) %Evaluate checksum
                Error = 0; %No error
            else
                Error = 8; %Incorrect checksum
            end
        else
            Error = 5; %Output does not match command
        end
    else
        Error = 5; %Output does not match command
    end
else
    Error = 6; %Could not read device
end
function CalcChecksum = i3dmgx3_CalcChecksum(Packet)
%Calculate bytewise checksum on a received data packet.
%
%Note:      The last two bytes, which contain the received checksum,
%           are not included in the calculation.
%
%Arguments: Packet - The received data packet
%
%Returns:   The calculated checksum

CalcChecksum = sum(Packet(1:(length(Packet)-2)));
function Checksum = i3dmgx3_Checksum(Packet)
%Determine received checksum on a received data packet.
%
%Arguments: Packet - The received data packet
%
%Returns:   The received checksum

Checksum = convert2ushort([Packet(length(Packet)-1),Packet(length(Packet))]);
function [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,Command,SampleRate,varargin)
InputParser = inputParser;
addOptional(InputParser,'Record',{0})
%Receives data from sensor and finds correct packet
%
%Arguments: SerialLink - Handle of serial link
%           Command - A string from the command array in i3dmgx3_Cmd.m
%           SampleRate - Rate at which device sends out packets
%           Record(optional) - Handle of cell array on which to record data
%
%Returns:   Packet - Data packet, if successfully read, empty matrix if not
%           Record - Updated record with latest packet tacked on the end
%           Error - Error number

if isempty(varargin) %Check whether data record handle is given
    Record = {0};
else
    Record = varargin{1};
end
CommandArray = i3dmgx3_Cmd; %Call command array
CommandNum = strmatch(Command,CommandArray(:,1)); %Find command
CommandBytes = CommandArray{CommandNum,2}; %Find command bytes
Error = 5; %Output does not match command
Read = 1; %Activate Read == 1 loop
Packet = [];
StartTime = now; %Get starting time
while Read == 1 && 86400*(now - StartTime) <= 1/SampleRate+.1 %The Read == 1 loop reads the buffer until it finds a correct packet
    ByteNum = 0;
    while ByteNum == 0;
        PossibleRecord = i3dmgx3_ReadNextRecord(SerialLink,Record); %Find a data packet
        if length(PossibleRecord) > 1
            ByteNum = 1;
        end
        Record = PossibleRecord;
    end
    while ByteNum <= length(Record{length(Record)-1,1})
        PossiblePacket = Record{length(Record)-1,1}; 
        if isempty(PossiblePacket) == 0
            if PossiblePacket(1) == CommandBytes(1) %Check for proper command byte
                Packet = PossiblePacket;
                Error = 0;
                ByteNum = length(Record{length(Record)-1,1})+1;
                Read = 0;
            else
                Packet = [];
                ByteNum = ByteNum+1;
            end
        else
            Packet = [];
            ByteNum = ByteNum+1;
        end
    end
end
function [Packet,Record,Error] = i3dmgx3_ReceiveBufferContinuous(SerialLink,Command,SampleRate,varargin)
%Receives all data on input buffer and finds the correct packets
%
%Arguments: SerialLink - Handle of serial link
%           Command - A string from the command array in i3dmgx3_Cmd.m
%           SampleRate - Rate at which device sends out packets
%           Record(optional) - Handle of cell array on which to record data
%
%Returns:   Packet - Last data packet, if successfully read
%           Record - Updated record with latest buffer tacked on the end
%           Error - Error number

InputParser = inputParser;
addOptional(InputParser,'Record',{0})
if isempty(varargin) %Check whether data record handle is given
    Record = {0};
else
    Record = varargin{1};
end
CommandArray = i3dmgx3_Cmd; %Call command array
CommandNum = strmatch(Command,CommandArray(:,1)); %Find command
CommandBytes = CommandArray{CommandNum,2}; %Find command bytes
Error = 5; %Output does not match command
Read = 1; %Activate Read == 1 loop
Packet = [];
StartTime = now; %Get starting time
while Read == 1 && 86400*(now - StartTime) <= 1/SampleRate+.1 %The Read == 1 loop reads the buffer until it finds a correct packet
    ByteNum = 0;
    while ByteNum == 0;
        PossibleRecord = i3dmgx3_ReadNextBuffer(SerialLink,Record); %Find a data packet
        if length(PossibleRecord) > 1
            ByteNum = 1;
        end
        Record = PossibleRecord;
    end
    while ByteNum <= length(Record{length(Record)-1,1})
        PossiblePacket = Record{length(Record)-1,1}; 
        if isempty(PossiblePacket) == 0
            if PossiblePacket(1) == CommandBytes(1) %Check for proper command byte
                Packet = PossiblePacket;
                Error = 0;
                ByteNum = length(Record{length(Record)-1,1})+1;
                Read = 0;
            else
                Packet = [];
                ByteNum = ByteNum+1;
            end
        else
            Packet = [];
            ByteNum = ByteNum+1;
        end
    end
end
function Record = i3dmgx3_ReadNextRecord(SerialLink,Record)
%Reads the next packet in continuous mode
%
%Arguments: SerialLink - Handle of serial link
%           Record - Name of cell array where data packets are to be stored
%
%Returns:   Cell array of data records with latest record tacked on the end

CommandArray = i3dmgx3_Cmd; %Call command array
RecordSize = size(Record);
CommandArraySize = size(CommandArray);
Read = 1; %Activate Read == 1 loop
ByteNum = 1;
Remainder = Record{RecordSize(1)}; %Get remainder from record
BufferSize = get(SerialLink,'BytesAvailable'); %Get number of bytes availble on input buffer
if isempty(Remainder) == 0
    Buffer = Remainder;
elseif BufferSize > 0
    Tail = fread(SerialLink,1); %Read one byte
    Buffer = [Remainder,Tail']; %Combine remainder with newly read byte
else
    Read = 0; %Skips over Read == 1 loop if there are no bytes available, leaving the record unchanged
end
while Read == 1 && ByteNum < BufferSize
    CommandNum = 2;
    while CommandNum <= CommandArraySize(1)-1 %Try all possible commands
        CommandBytes = CommandArray{CommandNum,2}; %Find command bytes
        ResponseLength = CommandArray{CommandNum,4}; %Find number of bytes in data packet
        BufferLength = length(Buffer);
        if Buffer(ByteNum) == CommandBytes(1) %See if first byte is potential command byte
            if ResponseLength-BufferLength+ByteNum-1 > 0
                Buffer = [Buffer,fread(SerialLink,ResponseLength-BufferLength+ByteNum-1)']; %#ok<AGROW> Read enough bytes to complete potential data packet
                BufferLength = length(Buffer);
            end
            PossiblePacket = Buffer(ByteNum:ByteNum+ResponseLength-1)';
            if i3dmgx3_CalcChecksum(PossiblePacket) == i3dmgx3_Checksum(PossiblePacket) %Evaluate checksum
                Record{RecordSize(1)} = PossiblePacket'; %Add new packet to record
                CommandNum = CommandArraySize(1)+1; %Exit CommandNum <= CommandArraySize(1)-1 loop
                ByteNum = ByteNum+length(PossiblePacket);
                if BufferLength >= ByteNum
                    Record{RecordSize(1)+1,1} = Buffer(ByteNum:BufferLength); %Add remainder to end of record
                else
                    Record{RecordSize(1)+1,1} = 0;
                end
                Read = 0;
            end
        end
        CommandNum = CommandNum+1;
    end
    if BufferLength == ByteNum
        Buffer = [Buffer,fread(SerialLink,1)]; %#ok<AGROW> %Read next byte if none of the bytes read so far are command bytes
    end
    ByteNum = ByteNum+1;
end
function Record = i3dmgx3_ReadNextBuffer(SerialLink,Record)
%Reads the next record in continuous mode, and when used many times in
%succession, adds all the packets to a cell array.
%
%Arguments: SerialLink - Handle of serial link
%           Record - Name of cell array where data packets are to be stored
%
%Returns:   Cell array of data records with latest record tacked on the end

CommandArray = i3dmgx3_Cmd; %Call command array
RecordSize = size(Record);
CommandArraySize = size(CommandArray);
Read = 1; %Activate Read == 1 loop
ByteNum = 1;
Head = Record{RecordSize(1)}; %Get remainder from previous buffer
BufferSize = get(SerialLink,'BytesAvailable'); %Get number of bytes availble on input buffer
if BufferSize > 0
    Tail = fread(SerialLink,BufferSize); %Read entire buffer
    Buffer = [Head,Tail']; %Combine remainder with newly read buffer
elseif isempty(Head) == 0
    Buffer = Head(1);
else
    Read = 0; %Skips over Read == 1 loop if there are no bytes available, leaving the record unchanged
end
while Read == 1 && ByteNum < length(Buffer)
    CommandNum = 2;
    while CommandNum <= CommandArraySize(1)-1 %Try all possible commands
        CommandBytes = CommandArray{CommandNum,2}; %Find command bytes
        ResponseLength = CommandArray{CommandNum,4}; %Find number of bytes in data packet
        if Buffer(ByteNum) == CommandBytes(1) %See if first byte is potential command byte
            if length(Buffer)-ByteNum+1 >= ResponseLength %See if enough bytes remain to make an entire packet
                Packet = Buffer(ByteNum:ByteNum+ResponseLength-1);
                if i3dmgx3_CalcChecksum(Packet) == i3dmgx3_Checksum(Packet) %Evaluate checksum
                    CommandNum = CommandArraySize(1)+1;
                    ByteNum = ByteNum+length(Packet)-1;
                    Record{RecordSize(1),:} = Packet; %Add new packet to record
                    if length(Buffer) >= ByteNum
                        Record{RecordSize(1)+1,1} = Buffer(ByteNum:length(Buffer)); %Add remainder to end of record
                    else
                        Record{RecordSize(1)+1,1} = 0;
                    end
                    RecordSize(1) = RecordSize(1)+1;
                end
            end
        end
        CommandNum = CommandNum+1;
    end
    ByteNum = ByteNum+1;
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
function [Packet,Error] = i3dmgx3_OrientMatrix(SerialLink)
%Reads the orientation matrix from sensor
%
%Arguments: SerialLink - Handle of serial link
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_ORRIENTATION_MAT'); %Write command to device
    if Error == 0
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_ORRIENTATION_MAT'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end
function [Packet,Error] = i3dmgx3_OrientMatrixCM(SerialLink,SampleRate)
%Reads the orientation matrix from sensor
%
%Arguments: SerialLink - Handle of serial link
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_ORRIENTATION_MAT'); %Write command to device
    if Error == 0
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_ORRIENTATION_MAT',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end
function i3dmgx3_DrawSensor(Packet)
%Makes a 3D plot of the sensor relative to the earth
%
%Arguments: Packet - Data packet from sensor

OrientationMatrix = [convert2float32bin(Packet(2:5)) convert2float32bin(Packet(6:9)) convert2float32bin(Packet(10:13));
                     convert2float32bin(Packet(14:17)) convert2float32bin(Packet(18:21)) convert2float32bin(Packet(22:25));
                     convert2float32bin(Packet(26:29)) convert2float32bin(Packet(30:33)) convert2float32bin(Packet(34:37))]; %Construct orientation matrix
Vertices = [-2,-1.2,0.5;
            2,-1.2,0.5;
            2,1.2,0.5;
            -2,1.2,0.5;
            -2,-1.2,-0.2;
            2,-1.2,-0.2;
            2,1.2,-0.2;
            -2,1.2,-0.2;
            -2,-1.2,-0.5;
            2,-1.2,-0.5;
            2,1.2,-0.5;
            -2,1.2,-0.5;
            -0.5,-1.8,-0.2;
            0.5,-1.8,-0.2;
            0.5,-1.2,-0.2;
            -0.5,-1.2,-0.2;
            -0.5,-1.8,-0.5;
            0.5,-1.8,-0.5;
            0.5,-1.2,-0.5;
            -0.5,-1.2,-0.5;
            -0.5,1.2,-0.2;
            0.5,1.2,-0.2;
            0.5,1.8,-0.2;
            -0.5,1.8,-0.2;
            -0.5,1.2,-0.5;
            0.5,1.2,-0.5;
            0.5,1.8,-0.5;
            -0.5,1.8,-0.5;
            -0.5,-1.2,0.5;
            0.5,-1.2,0.5;
            0.5,1.2,0.5;
            -0.5,1.2,0.5;]; %Define vertices
Faces = [1 2 3 4;
         1 5 16 29;
         2 3 7 6;
         3 7 22 31;
         1 4 8 5;
         5 9 20 16;
         6 10 19 15;
         7 11 26 22;
         8 12 25 21;
         9 10 11 12;
         13 14 15 16;
         13 16 20 17;
         13 14 18 17;
         14 15 19 18;
         17 18 19 20;
         21 22 23 24;
         22 23 27 26;
         23 24 28 27;
         21 24 28 25;
         25 26 27 28;
         15 16 29 30;
         2 6 15 30;
         21 22 31 32;
         4 8 21 32;
         6 7 11 10;
         5 8 12 9]; %Define faces
TransformedVerticies = zeros(32,3);
for VertexNum = 1:32
    Vertex = (Vertices(VertexNum,:));
    TransformedVertex = inv(OrientationMatrix)*Vertex'; %Transform using orientation matrix
    TransformedVerticies(VertexNum,1)=TransformedVertex(2);
    TransformedVerticies(VertexNum,2)=TransformedVertex(1);
    TransformedVerticies(VertexNum,3)=TransformedVertex(3);
end
clf
patch('Vertices',TransformedVerticies,'Faces',Faces,'EdgeColor','k','FaceColor',[.5,.5,.5]) %Draw sensor
axis equal
axis([-4 4 -4 4 -4 4])
grid on
function [Packet,Error] = i3dmgx3_AccelAndAngRate(SerialLink)
%Reads scaled Acceleration and Angular Rate from sensor
%
%Arguments: SerialLink - Handle of serial link
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_ACCELERATION_ANGU'); %Write command to device
    if Error == 0
        [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_ACCELERATION_ANGU'); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end
function [Packet,Error] = i3dmgx3_AccelAndAngRateCM(SerialLink,SampleRate)
%Reads scaled Acceleration and Angular Rate from sensor
%
%Arguments: SerialLink - Handle of serial link
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_ACCELERATION_ANGU'); %Write command to device
    if Error == 0
        [Packet,Record,Error] = i3dmgx3_ReceiveDataContinuous(SerialLink,'CMD_ACCELERATION_ANGU',SampleRate); %Read data from device
        if Error ~= 0
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end
function i3dmgx3_PrintAccelAndAngRate(Packet)
%Prints acceleration and angular rate components and time
%
%Arguments: Packet - Data packet from sensor

Data = [convert2float32bin(Packet(2:5));
          convert2float32bin(Packet(6:9));
          convert2float32bin(Packet(10:13));
          convert2float32bin(Packet(14:17));
          convert2float32bin(Packet(18:21));
          convert2float32bin(Packet(22:25));
          convert2ulong(Packet(26:29))/62500]; %Convert data to decimals
Data = [Data(1);
        Data(2);
        Data(3);
        norm(Data(1:3));
        Data(4);
        Data(5);
        Data(6);
        norm(Data(4:6));
        Data(7)];
PositivePrint = {sprintf('\nAcceleration:\nX:     %f g',Data(1));
                 sprintf('Y:     %f g',Data(2));
                 sprintf('Z:     %f g',Data(3));
                 sprintf('Total: %f g\n',Data(4));
                 sprintf('Angular Rate:\nX:     %f rad/s',Data(5));
                 sprintf('Y:     %f rad/s',Data(6));
                 sprintf('Z:     %f rad/s',Data(7));
                 sprintf('Total: %f rad/s\n',Data(8));
                 sprintf('Time: %f seconds from powerup',Data(9))}; %Define format to print data in
NegativePrint = {sprintf('\nAcceleration:\nX:    %f g',Data(1));
                 sprintf('Y:    %f g',Data(2));
                 sprintf('Z:    %f g',Data(3));
                 sprintf('Total:%f g\n',Data(4));
                 sprintf('Angular Rate:\nX:    %f rad/s',Data(5));
                 sprintf('Y:    %f rad/s',Data(6));
                 sprintf('Z:    %f rad/s',Data(7));
                 sprintf('Total:%f rad/s\n',Data(8));
                 sprintf('Time:%f seconds from powerup',Data(9))}; %Remove a space for negative values to make decimal points align
for DataNum = 1:9
    if Data(DataNum) < 0
        disp(char(NegativePrint(DataNum))); %Print formatted data
    else
        disp(char(PositivePrint(DataNum)));
    end
end
function Error = i3dmgx3_PlotAccelAndAngRate(SerialLink,Period,SampleRate)
%Plots acceleration and angular rate of sensor over a given period of time
%
%Arguments: SerialLink - Handle of serial link
%           Period - Time over which to plot the data
%           SampleRate - Rate at which device sends out packets
%
%Returns:   Error number

clf
Rollover = zeros(1);
RolloverCount = 0;
Time = 0;
PacketNum = 0;
Record = zeros(9,1);
output1 = zeros(16,7);
purgePort(SerialLink); %Clear buffer
while max(Time) < Period
    [Packet,Record2,Error] = i3dmgx3_ReceiveBufferContinuous(SerialLink,'CMD_ACCELERATION_ANGU',SampleRate); %Get AAR packets from input buffer
    for x = 1:size(Record2)*[1;0]-1
        PacketNum = PacketNum+1;
        Record3 = Record2{x};
        output1(x,:) = [convert2float32bin([Record3(2:5);Record3(6:9);Record3(10:13);Record3(14:17);Record3(18:21);Record3(22:25)]);convert2ulong(Record3(26:29))/62500]; %Convert AAR values
        Record(:,PacketNum) = [output1(x,1:3),norm(output1(x,1:3)),output1(x,4:6),norm(output1(x,4:6)),output1(x,7)]; %Find magnitudes, add to record
        if PacketNum > 1
            if Record(9,PacketNum)<Record(9,PacketNum-1)
                RolloverCount = RolloverCount+1; %Compensate for timer rollover in sensor
            end
        else
            StartTime = Record(9,1);
        end
        Rollover(PacketNum) = RolloverCount;
    end
    Time = Record(9,:)+Rollover*2^32/62500-StartTime; %Set time relative to starting time
    if PacketNum <= 1500
        subplot(2,1,1)
        plot(Time(1:PacketNum),Record(4,1:PacketNum),'b-',Time(1:PacketNum),Record(1,1:PacketNum),'r-',Time(1:PacketNum),Record(2,1:PacketNum),'m-',Time(1:PacketNum),Record(3,1:PacketNum),'y-') %Plot acceleration
        if max(Time) <= 5
            xlim([0 10])
        else
            xlim([max(Time-5) max(Time+5)])
        end
        grid on
        subplot(2,1,2)
        plot(Time(1:PacketNum),Record(8,1:PacketNum),'b-',Time(1:PacketNum),Record(5,1:PacketNum),'r-',Time(1:PacketNum),Record(6,1:PacketNum),'m-',Time(1:PacketNum),Record(7,1:PacketNum),'y-') %Plot angular rate
        if max(Time) <= 5
            xlim([0 10])
        else
            xlim([max(Time-5) max(Time+5)])
        end
        grid on
        pause(0.00001);
    else
        clf
        Record = Record(1:9,PacketNum-1500:PacketNum); %When record gets too long, shift the entries backwards, getting rid of some old ones, to save memory and time
        Time = Time(PacketNum-1500:PacketNum);
        Rollover = Rollover(PacketNum-1500:PacketNum);
        PacketNum = 1501;
        subplot(2,1,1)
        plot(Time,Record(4,:),'b-',Time,Record(1,:),'r-',Time,Record(2,:),'m-',Time,Record(3,:),'y-') %Plot acceleration
        if max(Time) <= 5
            xlim([0 10])
        else
            xlim([max(Time-5) max(Time+5)])
        end
        grid on
        subplot(2,1,2)
        plot(Time,Record(8,:),'b-',Time,Record(5,:),'r-',Time,Record(6,:),'m-',Time,Record(7,:),'y-') %Plot acceleration
        xlim([max(Time-5) max(Time+5)])
        grid on
        pause(.00001);
    end
end
if Error == 0
    %Label graphs
    subplot(2,1,1)
    xlabel('Elapsed time (sec)')
    ylabel('Acceleration (m/s^2)')
    legend('Total Magnitude','X-Component','Y-Component','Z-Component','Location','Best')
    subplot(2,1,2)
    xlabel('Elapsed time (sec)')
    ylabel('Angular rate (rad/s)')
    legend('Total Magnitude','X-Component','Y-Component','Z-Component','Location','Best')
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
function Int = convert2ulong(Input)
%Converts 4 bytes into a long unsigned integer.
%
%Arguments: Input - four-column matrix containing bytes to be converted 
%           (one integer per row)
%
%Returns:   Long unsigned integer

if size(Input)*[1;0] == 4 && size(Input)*[0;1] == 1
    Input = Input';
end
Bin = [dec2bin(Input(:,1),8),dec2bin(Input(:,2),8),dec2bin(Input(:,3),8),dec2bin(Input(:,4),8)]; %Convert bytes to binary
Int = bin2dec(Bin); %Convert binary to decimal
function Dec = convert2float32bin(Input)
%Converts matrix consisting of rows of 4 decimal bytes to 32-bit binary 
%floating point values in IEEE-754 format.
%
%Arguments: Input - 4-column matrix containing bytes to be converted
%
%Returns:   Decimal value

if size(Input)*[1;0] == 4 && size(Input)*[0;1] == 1
    Input = Input';
end
Coefficients = [ones(1,23)/2].^(1:23); %#ok<NBRAK>
Bin = strcat(dec2bin(Input(:,1),8),dec2bin(Input(:,2),8),dec2bin(Input(:,3),8),dec2bin(Input(:,4),8))-48;%The "-48" is just a shortcut to turn the character array into a matrix of ones and zeros - the ASCII codes for 0 and 1 are 48 and 49, respectively
Sign = (-Bin(:,1)+.5)*2; %Get signs
Exponent = bin2dec(reshape(sprintf('%d',Bin(:,2:9)),size(Bin)*[1;0],8))-127; %Get exponents
Mantissa = Bin(:,10:32)*Coefficients'+1; %Get mantissas
Dec = Sign.*Mantissa.*(2.^Exponent); %Compute decimal
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
function Bytes = ushort2bytes(Input)
%Converts a short unsigned integer into 2 bytes.
%
%Arguments: Input - Vector of short unsigned integers to be converted
%
%Returns:   2-column matrix containing bytes

InputSize = length(Input);
Bin = dec2bin(Input,16); %Convert bytes to binary
Bytes = reshape(bin2dec(reshape(sprintf('%d',Bin'-48),8,InputSize*2)'),2,InputSize)'; %Convert binary to decimal
function [Packet,Error] = i3dmgx3_ContinuousPreset(SerialLink,Command)
%Sets or reads the data type that the sensor will output in continous mode
%
%Arguments: SerialLink - Handle of serial link
%           Command - Command for data type to output in continuous mode
%
%Returns:   Packet - The data packet
%           Error - Error number

Error = purgePort(SerialLink); %Clear buffer
if Error == 0
    Error = i3dmgx3_SendBuffData(SerialLink,'CMD_CONTINUOUS_PRESET'); %Write command to device
    if Error == 0
        Error = i3dmgx3_SendBuffData(SerialLink,Command); %Write data type to device
        if Error == 0
            [Packet,Error] = i3dmgx3_ReceiveData(SerialLink,'CMD_CONTINUOUS_PRESET'); %Read data from device
            if Error ~= 0
                Packet = [];
            end
        else
            Packet = [];
        end
    else
        Packet = [];
    end
else
    Packet = [];
end