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