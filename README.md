# dicom-viewer-3D
<img width="796" height="596" alt="example" src="https://github.com/user-attachments/assets/b075c69e-2f31-4ed0-900f-2c38c133a7ed" />

The program is designed to create 3D models of certain organs from DICOM images.
Main development tools Qt, VTK, C++.
# Functionality
- Build 3D Model from DICOM
- Set volume extraction parameters for model
- Save 3D model to stl
# Model types
- Lungs
- Bones
- Skin
# Implementation
The application uses the vtkMarchingCubes surface construction algorithm
and some simple computational geometry algorithms to generate 3D models of certain organs.
# Build requirements
- Visual Studio 2017 msvc compiler
- Qt 5
- Windows 10 / 11
# Release
- [Release msvc 2017](https://drive.google.com/file/d/1CP8jiLco1_gryoz_mbCdJC_O6Jq8JcPM/view?usp=sharing)
- [VTK binarys msvc 2017](https://drive.google.com/file/d/14xWSCmUyoiDUJTjGzdh0Buf1E0NwSy7R/view?usp=sharing)
