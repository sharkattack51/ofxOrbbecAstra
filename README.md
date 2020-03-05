# ofxOrbbecAstra

openFrameworks addon for using the [Orbbec Astra SDK](https://orbbec3d.com/develop/). Mostly barebones support at the moment – PRs are welcome to help build this out!

## Setup

Clone this addon into your `openFrameworks/addons` folder.

```
cd path/to/openFrameworks/addons/
git clone https://github.com/mattfelsen/ofxOrbbecAstra.git
```

The Astra SDK is bundled in the `libs` folder and does not require a separate download or installation.

## Running the example project

Hopefully this should compile and run without any hiccups. Please open an issue if you run into trouble here.

## Using this addon in your project

If you're using the projectGenerator, either to create a new project or update an existing one, which are also outlined  `addon_config.mk`, are set up correctly.

Note: this assumes your project folder is at the standard "three levels down" from your openFrameworks folder, e.g. `openFrameworks/apps/myApps/projectFolder/`. If not, you may need to adjust the runpath setting.

Note: `copy_to_bin64_dir` is empty. It is necessary to include the dll required for runtime in this folder, so copy the contents of `AstraSDK-v2.0.19/bin` manually.

```
astra.dll
astra.toml
astra_core.dll
astra_core_api.dll
OniFile.dll
opencv_core248.dll
opencv_highgui248.dll
opencv_imgproc248.dll
opencv_video248.dll
OpenNI.ini
OpenNI2.dll
ORBBEC.dll
orbbec.ini
Plugins/openni_sensor.dll
Plugins/orbbec_hand.dll
Plugins/orbbec_hand.toml
Plugins/orbbec_xs.dll
Plugins/OrbbecBodyTracking.dll
```

## Support

This has been tested with the following setup:

- vs2017
- Windows 10 64bit
- openFrameworks 0.11.0
- Astra SDK 2.0.19 (which is included, no separate download/installation is required)
- Orbbec Astra/Astra Pro camera

## Hardware Note – Astra Pro & color data

There is a known issue with the SDK and Astra Pro cameras which does not make the color data available. Trying to start the color stream (i.e. calling `astra.initColorStream()`) will also cause other streams to fail, so you must comment-out this line in the example and/or your projects.

The color data is separately available as a standard webcam. Rather than using `initColorStream()`, you can use `initVideoGrabber()` which uses an `ofVideoGrabber` under the hood for you.

Note: grabberImage resizes and crops to depth data, not the full resolution of the camera :)

You can keep an eye on the [Orbbec forums](https://3dclub.orbbec3d.com/) to see the latest.