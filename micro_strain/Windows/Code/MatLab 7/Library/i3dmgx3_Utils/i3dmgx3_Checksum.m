function Checksum = i3dmgx3_Checksum(Packet)
%Determine received checksum on a received data packet.
%
%Arguments: Packet - The received data packet
%
%Returns:   The received checksum

Checksum = convert2ushort([Packet(length(Packet)-1),Packet(length(Packet))]);