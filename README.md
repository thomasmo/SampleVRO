# SampleVRO
Sample D3D11/D2D app as OpenVR Overlay in SteamVR

This project is a simple project to test using a Win32 app with a D3D11 texture as the source for an OpenVR VROverlay.
For initial user interaction, this app draws text on the screen that is typed from the keyboard.

As of c9c0af6, mouse input is handled (LButton click) by the Win32 app and causes line to be drawn between subsequent clicks. As of 2d949d6, this same behavior is maintained with controller input, which is forwarded as a mouse message.

VS solution has been testing in v2017. The openvr_api.dll is needed in the same directory as the built SampleVRO.exe to run.
Note that (for debugging purposes) the texture is written to disk to verify what is sent to the VROverlay APIs.

**Note**
~~This app must run on the same GPU as the OpenVR host for the texture to present.~~ As of 13fee8c, this app now runs on the same GPU that the OpenVR instance is running on.
