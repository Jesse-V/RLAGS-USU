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