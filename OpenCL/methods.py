import numpy as np
import pandas as pd
from shapely.geometry import Point
from shapely.geometry.polygon import Polygon
import math

def inpolygon(x, y, Xd, Yd):
    point = Point(x,y)
    n = len(Xd)
    polygon = []
    for i in range(n):
        polygon.append((Xd[i],Yd[i]))
    
    bool_val = False
    if polygon.contains(point):
        bool_val = True
    
    return bool_val
	
	
	
def Obj_Func(X, Xd, Yd):
    xA = X[0] 
    yA = X[1] 
    r3 = X[2] 
    beta = X[3]
    
    n = len(Xd)
    
    R = []
    for i in range(n):
        R.append(0.0)
    
    for i in range(n):
        R[i] = math.sqrt((Xd[i]-xA)**2 + (Yd[i]-yA)**2)
    
    Rmax = max(R)
    max_index = R.index(Rmax)
    Rmin = min(R)
    min_index = R.index(Rmin)
    
    if inpolygon(xA,yA,Xd,Yd):
        r2 = (Rmax + Rmin)/2;
        r5 = (Rmax-Rmin)/2;
    else:
        r2 = (Rmax-Rmin)/2; 
        r5 = (Rmax + Rmin)/2;
        
    xC1 = []
    yC1 = []
    xC2 = []
    yC2 = []
    for i in range(n):
        xC1.append(0.0)
        yC1.append(0.0)
        xC2.append(0.0)
        yC2.append(0.0)
        
    for i in range(n):
        if (i < max_index and i > min_index) or (i > max_index and i < min_index):
            # Trajectory 1
            theta2m1 = math.atan2(Yd[i]-yA,Xd[i]-xA) + math.acos((r2**2 + R[i]**2-r5**2)/(2 *r2 *R[i]))
            theta5m1 = math.atan2(Yd[i]-yA-r2 *math.sin(theta2m1),Xd[i]-xA-r2 * math.cos(theta2m1))
            theta3m1 = theta5m1-beta
            xC1[i] = xA + r2 *math.cos(theta2m1) + r3 *math.cos(theta3m1) 
            yC1[i] = yA + r2 *math.sin(theta2m1) + r3 *math.sin(theta3m1)
            
            # Trajectory 2
            theta2m2 = math.atan2(Yd[i]-yA,Xd[i]-xA) - math.acos((r2**2 + R[i]**2-r5**2)/(2 *r2 *R[i]))
            theta5m2 = math.atan2(Yd[i]-yA-r2 *math.sin(theta2m2),Xd[i]-xA-r2 * math.cos(theta2m2))
            theta3m2 = theta5m2-beta
            xC2[i] = xA + r2 *math.cos(theta2m2) + r3 *math.cos(theta3m2) 
            yC2[i] = yA + r2 *math.sin(theta2m2) + r3 *math.sin(theta3m2)
            
        elif (i == max_index):
            theta2 = math.atan2(Yd[i]-yA,Xd[i]-xA) 
            theta5 = theta2 
            theta3 = theta5-beta 
            xC1[i] = xA + r2 *math.cos(theta2) + r3 *math.cos(theta3) 
            yC1[i] = yA + r2 *math.sin(theta2) + r3 *math.sin(theta3) 
            xC2[i] = xC1[i] 
            yC2[i] = yC1[i]
            
        elif i == min_index:
            if inpolygon(xA,yA,Xd,Yd):
                theta2 = math.atan2(Yd[i]-yA,Xd[i]-xA) 
                theta5 = math.pi + theta2 
                theta3 = theta5-beta 
                xC1[i] = xA + r2 * math.cos(theta2) + r3 * math.cos(theta3) 
                yC1[i] = yA + r2 * math.sin(theta2) + r3 * math.sin(theta3) 
                xC2[i] = xC1[i] 
                yC2[i] = yC1[i]
            
            
            else:
                theta2 = math.pi + math.atan2(Yd[i]-yA,Xd[i]-xA) 
                theta5 = atan2(Yd[i]-yA,Xd[i]-xA) 
                theta3 = theta5-beta
                xC1[i] = xA + r2 * math.cos(theta2) + r3 * math.cos(theta3)
                yC1[i] = yA + r2 * math.sin(theta2) + r3 * math.sin(theta3)
                xC2[i] = xC1[i]
                yC2[i] = yC1[i]
                
        else:
             # Trajectory 2
            theta2m1 = math.atan2(Yd[i]-yA,Xd[i]-xA) + math.acos((r2**2 + R[i]**2-r5**2)/(2 *r2 *R[i])); 
            theta5m1 = math.atan2(Yd[i]-yA-r2 *math.sin(theta2m1),Xd[i]-xA-r2 *math.cos(theta2m1)); 
            theta3m1 = theta5m1-beta; 
            xC2[i] = xA + r2 *math.cos(theta2m1) + r3 *math.cos(theta3m1); 
            yC2[i] = yA + r2 *math.sin(theta2m1) + r3 *math.sin(theta3m1);
            
             # Trajectory 1
            theta2m2 = math.atan2(Yd[i]-yA,Xd[i]-xA)-math.acos((r2**2 + R[i]**2-r5**2)/(2 *r2 *R[i])); 
            theta5m2 = math.atan2(Yd[i]-yA-r2 *math.sin(theta2m2),Xd[i]-xA-r2 *math.cos(theta2m2)); 
            theta3m2 = theta5m2-beta; 
            xC1[i] = xA + r2 *math.cos(theta2m2) + r3 *math.cos(theta3m2);
            yC1[i] = yA + r2 *math.sin(theta2m2) + r3 *math.sin(theta3m2);
            
    
    CPF1 = CPF(xC1,yC1)
    CPF2 = CPF(xC2,yC2)
    err = 0
    
    if CPF1[0] < CPF2[0]:
        err = CPF1[0]
        xD = CPF1[1] 
        yD = CPF1[2] 
        r1 = math.sqrt((xA-xD)**2 + (yA-yD)**2) 
        r4 = CPF1[3] 
        alpha = math.atan2(yD-yA,xD-xA)
    
    else:
        err = CPF2[0]
        xD = CPF2[1] 
        yD = CPF2[2] 
        r1 = math.sqrt((xA-xD)**2 + (yA-yD)**2) 
        r4 = CPF2[3] 
        alpha = math.atan2(yD-yA,xD-xA)
        
    linkage = [r1,r2,r3,r4,r5,beta,xA,yA,alpha]
    l_4 = [r1,r2,r3,r4]
    s = min(l_4)
    l = max(l_4)
    pq = sum(l_4) - (s+l)
    
    if s + l <= pq and linkage[2] != s:
        err = err + 2000 
    elif s + l <= pq or linkage[2] != s:
        err = err + 1000
    
    else:
        err = err
    
    link = (xA, yA, r3, beta)
    
    return link
    