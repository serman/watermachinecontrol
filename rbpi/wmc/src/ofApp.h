// =============================================================================
//
// Copyright (c) 2009-2013 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#pragma once


#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "IPVideoGrabber.h"
#include "ofxCv.h"
#include "ofxOsc.h"


#define HOST "localhost"
#define PORT 12345
#define TIME_BETWEEN_DETECTIONS 1000


#if defined(TARGET_OF_IPHONE) || defined(TARGET_ANDROID) || defined(TARGET_LINUX_ARM)
    #define NUM_CAMERAS 2
    #define NUM_ROWS 1
    #define NUM_COLS 1
#else
    #define NUM_CAMERAS 2
    #define NUM_ROWS 1
    #define NUM_COLS 1
#endif


class detectionData
{
public:
    detectionData()
    {
    }
    ofRectangle position;
    int moustache;
    int beard;
    int age;
    int glasses;
    int gender;
    int smile;
    
    void set(int x, int y, int w, int h, int _beard, int _age, int _glasses, int _gender, int _smile){
        position.set(x,y,w,h);
        beard=_beard;
        age=_age;
        glasses=_glasses;
        gender=_gender;
        smile=_smile;
    }
    
};

class IPCameraDef
{
public:
    enum AuthType {
        NONE,
        BASIC,
        COOKIE
    };
    
    IPCameraDef()
    {
    }

    IPCameraDef(const std::string& url): _url(url)
    {
    }
    
    IPCameraDef(const std::string& name,
                const std::string& url,
                const std::string& username,
                const std::string& password,
                const AuthType authType
                ):
        _name(name),
        _url(url),
        _username(username),
        _password(password),
        _authType(authType)
    {
    }

    void setName(const std::string& name) { _name = name; }
    std::string getName() const { return _name; }

    void setURL(const std::string& url) { _url = url; }
    std::string getURL() const { return _url; }

    void setUsername(const std::string& username) { _username = username; }
    std::string getUsername() const { return _username; }

    void setPassword(const std::string& password) { _password = password; }
    std::string getPassword() const { return _password; }
    
    void setAuthType(AuthType authType) { _authType = authType; }
    AuthType getAuthType() const { return _authType; }


private:
    std::string _name;
    std::string _url;
    std::string _username;
    std::string _password;
    AuthType _authType = NONE;
};


using ofx::Video::IPVideoGrabber;
//using ofx::Video::IPVideoGrabber;

using namespace ofx;


class ofApp: public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();
    void drawDetection();    
    void saveFrameAndNotify();
    void sendImage();
    void getContour();
    ofxOscSender sender;
    long lastTimeFaceDetected;
    int timeHibernStarted=0;
    int timeLastDetection=0;
    
    ofxCv::ObjectFinder finder;
    ofxCv::ObjectFinder faceFinder;
    ofImage grabFrame;
    ofImage grabFrameEnvio;
    std::string fileName;
    int faceTrackingGrabber=0;
    int microsoftGrabber=1;
    
    
    void keyPressed(int key);

    std::vector<std::shared_ptr<Video::IPVideoGrabber>> grabbers;

    void loadCameras();
    IPCameraDef& getNextCamera();
    std::vector<IPCameraDef> ipcams; // a list of IPCameras
    std::size_t nextCamera;

    ofRectangle facesRectangle;
    // This message occurs when the incoming video stream image size changes. 
    // This can happen if the IPCamera has a single broadcast state (some cheaper IPCams do this)
    // and that broadcast size is changed by another user. 
    void videoResized(const void* sender, ofResizeEventArgs& arg);
    
    
    bool useLocalVideo=true;
    //solo en el caso de usar videograbber
    ofVideoPlayer 		videoPlayer;
    bool                frameByframe;
    std::string videoName;
    int videoWidth = 704;
    int videoHeight = 576;

    void takeShootingDecision();
    void killThatOne();
    
    ofxOscReceiver myosc;
 	void oscRcvUpdate();
    std::vector<detectionData> detections;
    cv::Mat imgMat2;
    enum modes {
        HIBERN,
        SHOOTING,
        FINDING
    };
    
    int status=HIBERN;
    
    /****arduino  ***/
     
    ofSerial serial;
    int serialPort;
    unsigned char bangMsg[3] = {'b',0, '\n'};
    unsigned char servoMsg[3] = {'s', 0, '\n'};
    
};


