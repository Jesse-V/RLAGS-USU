function Endian = TestByteOrder
%Determine Endian Format of local host.
%
%Returns:   'L' for Little-Endian format, 'B' for Big-Endian

[C Maxsize Endian] = computer; %Get Endian format