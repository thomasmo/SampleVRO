# SampleVRO
Sample D3D11/D2D app as OpenVR Overlay in SteamVR

This project is a simple project to test using a Win32 app with a D3D11 texture as the source for an OpenVR VROverlay.
For initial user interaction, this app draws text on the screen that is typed from the keyboard.

VS solution has been testing in v2017. The openvr_api.dll is needed in the same directory as the built SampleVRO.exe to run.
Note that (for debugging purposes) the texture is written to disk to verify what is sent to the VROverlay APIs.
