# Mesh Thickness 

This visual studio project calculates **normalized thickness of mesh** and return **depth map** for **matlab**. It builds **.mex file**  which can be used as function on matlab.  This project supports **2 different mode for depth map**. The idea is inspired by [Philip Rideout](http://prideout.net/blog/?p=51) and the code is based on his Glass project.
## Depth map mode
|Normal depth|Z-axis depth map  | 
|--|--|
|The thickness of mesh like the X-ray |The thickness of rolled out mesh along the z-axis|
|![the normal thickness](./img/normal.png)|![the z-axis thickness](./img/z-axis.png)|

## Input 
 1. (char)Full path to model(only .ctm format is supported)
(ex:`D:\\dev\\MESH-THK\\model\\venus.ctm','D:/dev/MESH-THK/model/bunny.ctm', and 'D:\dev\MESH-THK\model\venus.ctm'` are acceptable)
 2. (matrix)Euler rotation matrix 
(ex:` eye(3)`)
 3. (int)Thickness mode (0: nomal thickness, 1: z-axis depth map)

## Output

 1. (matrix)2048x2048 normalized depth map 

## Remarks on configuration

 1. Re-locate path to mex.h in the mesh_thk_mex project for your computer (in my case, `C:\Program Files\MATLAB\R2017a\extern\include`)
 2. Locate .glsl file the same folder of .mex64 file. This app looks for .glsl up to 4 upper folder.
 3. Use `x64` matlab to build `x64` application

