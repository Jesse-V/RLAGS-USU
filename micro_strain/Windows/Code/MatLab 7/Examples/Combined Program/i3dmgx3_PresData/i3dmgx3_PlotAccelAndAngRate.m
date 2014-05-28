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