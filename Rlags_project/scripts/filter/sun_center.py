import cv2
import numpy as np
import math

img = cv2.imread('/home/linaro/Rlags_project/scripts/filter/sun_cam_0.jpg',0)
#gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
gray = img
for i in range(3):
    gray = cv2.GaussianBlur(gray,(5,5),1) 
    ret, gray = cv2.threshold(gray,240,255,cv2.THRESH_BINARY)

contours = 0
center = (-1,-1)
contours,hierarchy = cv2.findContours(gray, 1, 2)
if contours:
    cnt = contours[0]
    x,y,w,h = cv2.boundingRect(cnt)
    cv2.rectangle(img,(x,y),(x+w,y+h),(0,255,0),2)
    center = ((x+x+w)/2,(y+y+h)/2)
    print center
    cv2.circle(img,center,3,(0,0,255),-1)       

angle = 181
if(center[0]>0):
    hieght = 960
    width = 1280
    coordx = int(width)/2 - center[0]
    coordy = int(hieght) - center[1]
    angle = math.degrees(math.atan2(coordy,coordx))
    print angle
#cv2.imshow('frame',img)
#cv2.waitKey(0) 
#cv2.destroyAllWindows()


