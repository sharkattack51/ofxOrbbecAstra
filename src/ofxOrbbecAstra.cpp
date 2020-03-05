//
//  ofxOrbbecAstra.h
//  ofxOrbbecAstra
//
//  Created by Matt Felsen on 10/24/15.
//
//

#include "ofxOrbbecAstra.h"

ofxOrbbecAstra::ofxOrbbecAstra() {
	width = 640;
	height = 480;
	nearClip = 300;
	farClip = 1800;

	cameraReqWidth = 1280;
	cameraReqHeight = 720;

	// Crop size confirmed by alignment of 1280x720 and 640x480
	registGrabberWidth = 1068;
	registGrabberHeight = 802;
	dstGrabberWidth = 1280;
	dstGrabberHeight = 960;

	bSetup = false;
	bIsFrameNew = false;
	bDepthImageEnabled = true;
}

ofxOrbbecAstra::~ofxOrbbecAstra() {
	astra::terminate();
}

void ofxOrbbecAstra::setup() {
	setup("device/default");
}

void ofxOrbbecAstra::exit() {
	astra::terminate();
}

void ofxOrbbecAstra::setup(const string& uri) {
	colorImage.allocate(width, height, OF_IMAGE_COLOR);
	depthImage.allocate(width, height, OF_IMAGE_GRAYSCALE);
	maskedColorImage.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
	depthPixels.allocate(width, height, OF_IMAGE_GRAYSCALE);
	
	grabberImage.allocate(dstGrabberWidth, dstGrabberHeight, OF_IMAGE_COLOR); // 1280x960
	grabberImageFbo.allocate(dstGrabberWidth, dstGrabberHeight); // 1280x960
	grabberImagePixels.allocate(dstGrabberWidth, dstGrabberHeight, OF_IMAGE_COLOR); // 1280x960
	grabberCroppedImage.allocate(registGrabberWidth, cameraReqHeight, OF_IMAGE_COLOR); // 1068x720
	grabberCroppedPixels.allocate(registGrabberWidth, cameraReqHeight, OF_IMAGE_COLOR); // 1068x720

	cachedCoords.resize(width * height);
	updateDepthLookupTable();

	astra::initialize();

	streamset = astra::StreamSet(uri.c_str());
	reader = astra::StreamReader(streamset.create_reader());

	bSetup = true;
	reader.add_listener(*this);
}

void ofxOrbbecAstra::enableDepthImage(bool enable) {
	bDepthImageEnabled = enable;
}

void ofxOrbbecAstra::enableRegistration(bool useRegistration) {
	if (!bSetup) {
		ofLogWarning("ofxOrbbecAstra") << "Must call setup() before setRegistration()";
		return;
	}

	reader.stream<astra::DepthStream>().enable_registration(useRegistration);
}

void ofxOrbbecAstra::setDepthClipping(unsigned short _near, unsigned short _far) {
	nearClip = _near;
	farClip = _far;
	updateDepthLookupTable();
}

void ofxOrbbecAstra::initColorStream() {
	if (!bSetup) {
		ofLogWarning("ofxOrbbecAstra") << "Must call setup() before initColorStream()";
		return;
	}

	astra::ImageStreamMode colorMode;
	auto colorStream = reader.stream<astra::ColorStream>();

	colorMode.set_width(width);
	colorMode.set_height(height);
	colorMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_RGB888);
	colorMode.set_fps(30);

	colorStream.set_mode(colorMode);
	colorStream.start();
}

void ofxOrbbecAstra::initDepthStream() {
	if (!bSetup) {
		ofLogWarning("ofxOrbbecAstra") << "Must call setup() before initDepthStream()";
		return;
	}

	astra::ImageStreamMode depthMode;
	auto depthStream = reader.stream<astra::DepthStream>();

	depthMode.set_width(width);
	depthMode.set_height(height);
	depthMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_DEPTH_MM);
	depthMode.set_fps(30);

	depthStream.set_mode(depthMode);
	depthStream.start();
}

void ofxOrbbecAstra::initBodyStream() {
	if (!bSetup) {
		ofLogWarning("ofxOrbbecAstra") << "Must call setup() before initBodyStream()";
		return;
	}

	reader.stream<astra::BodyStream>().start();
}

void ofxOrbbecAstra::initMaskedColorStream() {
	if (!bSetup) {
		ofLogWarning("ofxOrbbecAstra") << "Must call setup() before initMaskedColorStream()";
		return;
	}

	reader.stream<astra::MaskedColorStream>().start();
}

void ofxOrbbecAstra::initPointStream() {
	if (!bSetup) {
		ofLogWarning("ofxOrbbecAstra") << "Must call setup() before initPointStream()";
		return;
	}

	reader.stream<astra::PointStream>().start();
}

void ofxOrbbecAstra::initHandStream() {
	if (!bSetup) {
		ofLogWarning("ofxOrbbecAstra") << "Must call setup() before initHandStream()";
		return;
	}

	reader.stream<astra::HandStream>().start();
}

void ofxOrbbecAstra::initVideoGrabber(int deviceID) {
	bUseVideoGrabber = true;

	grabber = make_shared<ofVideoGrabber>();
	grabber->setDeviceID(deviceID);
	grabber->setup(cameraReqWidth, cameraReqHeight);
}

void ofxOrbbecAstra::update() {
	// See on_frame_ready() for more processing
	bIsFrameNew = false;
	astra_update();

	if (bUseVideoGrabber && grabber) {
		grabber->update();
		
		if (grabber->isFrameNew()) {
			// 1280x720 -> 1068x720 crop & mirror
			grabber->getPixels().cropTo(
				grabberCroppedPixels,
				(cameraReqWidth - registGrabberWidth) / 2, 0, registGrabberWidth, cameraReqHeight);
			grabberCroppedImage.setFromPixels(grabberCroppedPixels);
			grabberCroppedImage.mirror(false, true);
			grabberCroppedImage.update();

			// 1068x720 -> 1280x960
			float scale = (float)dstGrabberWidth / (float)registGrabberWidth;
			grabberImageFbo.begin();
			{
				ofClear(0);
				grabberCroppedImage.draw(
					0, (dstGrabberHeight - (registGrabberHeight * scale)) / 2,
					dstGrabberWidth, registGrabberHeight * scale);
			}
			grabberImageFbo.end();
			grabberImageFbo.readToPixels(grabberImagePixels);
			grabberImage.setFromPixels(grabberImagePixels);
		}
	}
}

void ofxOrbbecAstra::draw(float x, float y, float w, float h) {
	if (!w) w = colorImage.getWidth();
	if (!h) h = colorImage.getHeight();
	colorImage.draw(x, y, w, h);
}

void ofxOrbbecAstra::drawDepth(float x, float y, float w, float h) {
	if (!w) w = depthImage.getWidth();
	if (!h) h = depthImage.getHeight();
	depthImage.draw(x, y, w, h);
}

void ofxOrbbecAstra::drawMaskedColor(float x, float y, float w, float h) {
	if (!w) w = maskedColorImage.getWidth();
	if (!h) h = maskedColorImage.getHeight();
	maskedColorImage.draw(x, y, w, h);
}

void ofxOrbbecAstra::drawGrabber(float x, float y, float w, float h) {
	if (!w) w = grabberImage.getWidth();
	if (!h) h = grabberImage.getHeight();
	grabberImage.draw(x, y, w, h);
}

bool ofxOrbbecAstra::isFrameNew() {
	return bIsFrameNew;
}

void ofxOrbbecAstra::on_frame_ready(astra::StreamReader& reader,
									astra::Frame& frame)
{
	bIsFrameNew = true;

	auto colorFrame = frame.get<astra::ColorFrame>();
	auto depthFrame = frame.get<astra::DepthFrame>();
	auto bodyFrame = frame.get<astra::BodyFrame>();
	auto maskedColorFrame = frame.get<astra::MaskedColorFrame>();
	auto pointFrame = frame.get<astra::PointFrame>();
	auto handFrame = frame.get<astra::HandFrame>();

	if (colorFrame.is_valid() && !bUseVideoGrabber) {
		colorFrame.copy_to((astra::RgbPixel*) colorImage.getPixels().getData());
		colorImage.update();
	}

	if (depthFrame.is_valid()) {
		depthFrame.copy_to((short*) depthPixels.getData());

		if (bDepthImageEnabled) {
			// TODO do this with a shader so it's fast?
			for (int i = 0; i < depthPixels.size(); i++) {
				short depth = depthPixels.getColor(i).r;
				float val = depthLookupTable[depth];
				depthImage.setColor(i, ofColor(val));
			}
			depthImage.update();
		}
	}

	if (maskedColorFrame.is_valid()) {
		/*
			ref:[ samples\sfml\MaskedColorViewer-SFML\main.cpp ]
		*/
		auto maskedColorData = maskedColorFrame.data();
		for (int y = 0; y < maskedColorFrame.height(); y++)
		{
			for (int x = 0; x < maskedColorFrame.width(); x++)
			{ 
				int idx = y * maskedColorFrame.width() + x;
				//int r = maskedColorData[idx].r; // color is 0 ?
				//int g = maskedColorData[idx].g;
				//int b = maskedColorData[idx].b;
				int a = maskedColorData[idx].alpha;
				maskedColorImage.setColor(x, y, ofColor(255, 255, 255, a));
			}
		}
		maskedColorImage.update();
	}

	if (pointFrame.is_valid()) {
		pointFrame.copy_to((astra::Vector3f*) cachedCoords.data());
	}

	if (handFrame.is_valid()) {
		handMapDepth.clear();
		handMapWorld.clear();
		auto& list = handFrame.handpoints();

		for (auto& handPoint : list) {
			const auto& id = handPoint.tracking_id();

			if (handPoint.status() == HAND_STATUS_TRACKING) {
				const auto& depthPos = handPoint.depth_position();
				const auto& worldPos = handPoint.world_position();

				handMapDepth[id] = ofVec2f(depthPos.x, depthPos.y);
				handMapWorld[id] = ofVec3f(worldPos.x, worldPos.y, worldPos.z);
			} else {
				handMapDepth.erase(id);
				handMapWorld.erase(id);
			}
		}
	}
}

void ofxOrbbecAstra::updateDepthLookupTable() {
	// From product specs, range is 8m
	int maxDepth = 8000;
	depthLookupTable.resize(maxDepth);

	// Depth values of 0 should be discarded, so set the LUT value to 0 as well
	depthLookupTable[0] = 0;

	// Set the rest
	for (int i = 1; i < maxDepth; i++) {
		depthLookupTable[i] = ofMap(i, nearClip, farClip, 255, 0, true);
	}
}

ofVec3f ofxOrbbecAstra::getWorldCoordinateAt(int x, int y) {
	return cachedCoords[int(y) * width + int(x)];
}

unsigned short ofxOrbbecAstra::getNearClip() {
	return nearClip;
}

unsigned short ofxOrbbecAstra::getFarClip() {
	return farClip;
}

ofShortPixels& ofxOrbbecAstra::getRawDepth() {
	return depthPixels;
}

ofImage& ofxOrbbecAstra::getDepthImage() {
	return depthImage;
}

ofImage& ofxOrbbecAstra::getColorImage() {
	return colorImage;
}

ofImage& ofxOrbbecAstra::getMaskedColorImage() {
	return maskedColorImage;
}

ofImage& ofxOrbbecAstra::getGrabberImage() {
	return grabberImage;
}

unordered_map<int32_t,ofVec2f>& ofxOrbbecAstra::getHandsDepth() {
	return handMapDepth;
}

unordered_map<int32_t,ofVec3f>& ofxOrbbecAstra::getHandsWorld() {
	return handMapWorld;
}

void ofxOrbbecAstra::SetBodyTrackingSdkLicense(const char* lic) {
	orbbec_body_tracking_set_license(lic);
}
