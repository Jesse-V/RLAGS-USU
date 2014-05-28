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