

#include "ofApp.h"
using namespace ofxCv;
using namespace cv;

//------------------------------------------------------------------------------
void ofApp::setup()
{
    fileName="";
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(30);
    sender.setup(HOST, PORT);
    loadCameras();
    
    
    int serverRecvPort = 12000;
    string dst="localhost";
    myosc.setup(serverRecvPort);
    
    if(useLocalVideo==false){
        // initialize connection
        for(std::size_t i = 0; i < NUM_CAMERAS; i++)
        {
            IPCameraDef& cam = getNextCamera();
            
            auto grabber = std::make_shared<Video::IPVideoGrabber>();

            // if your camera uses standard web-based authentication, use this
            // c->setUsername(cam.username);
            // c->setPassword(cam.password);
            
            // if your camera uses cookies for authentication, use something like this:
            // c->setCookie("user", cam.username);
            // c->setCookie("password", cam.password);
            
            grabber->setCameraName(cam.getName());
            grabber->setURI(cam.getURL());
            grabber->connect(); // connect immediately

            // if desired, set up a video resize listener
            ofAddListener(grabber->videoResized, this, &ofApp::videoResized);
            grabbers.push_back(grabber);
        }
        
/*        for(std::size_t i = 0; i < grabbers.size(); i++){
            grabbers[i]->update();
            if(grabbers[i]->getWidth()<1000){ //TODO niapa de elegir fuente
                faceTrackingGrabber=i;
            }
            else{
                microsoftGrabber=i;
            }
        }*/
        faceTrackingGrabber=0;
        microsoftGrabber=1;
        
        
    }else{
        videoPlayer.load(videoName);
        videoPlayer.setLoopState(OF_LOOP_NORMAL);
        videoPlayer.play();
        videoPlayer.setLoopState(OF_LOOP_NORMAL);
    }
    
    grabFrame.allocate(videoWidth,videoHeight,OF_IMAGE_COLOR);
    
    
    ofSetVerticalSync(true);
    finder.setup("haarcascade_upperbody.xml");
    finder.setPreset(ObjectFinder::Fast); //accurate sensitive fast
                //info de los presets: https://github.com/kylemcdonald/ofxCv/blob/master/libs/ofxCv/src/ObjectFinder.cpp
    faceFinder.setup("haarcascade_frontalface_alt2.xml");
    faceFinder.setPreset(ObjectFinder::Sensitive);
    
    //finder.setMinNeighbors(3);
    //finder.setMultiScaleFactor(1.07);
    //cam.initGrabber(640, 480);
    
/***
 arduino
 ***/
    
    serial.listDevices();
/*    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();*/
    if(serialPort>=0){
        serial.setup(serialPort, 9600);
    }
}

//------------------------------------------------------------------------------
IPCameraDef& ofApp::getNextCamera()
{
    nextCamera = (nextCamera + 1) % ipcams.size();
    return ipcams[nextCamera];
}

//------------------------------------------------------------------------------
void ofApp::loadCameras()
{
    
    // all of these cameras were found using this google query
    // http://www.google.com/search?q=inurl%3A%22axis-cgi%2Fmjpg%22
    // some of the cameras below may no longer be valid.
    
    // to define a camera with a username / password
    //ipcams.push_back(IPCameraDef("http://148.61.142.228/axis-cgi/mjpg/video.cgi", "username", "password"));

	ofLog(OF_LOG_NOTICE, "---------------Loading Streams---------------");

	ofxXmlSettings XML;
    
	if(XML.loadFile("streams.xml"))
    {
        XML.pushTag("streams");
        std::string tag = "stream";
        
        std::size_t nCams = static_cast<std::size_t>(XML.getNumTags(tag));
        
        for (std::size_t n = 0; n < nCams; ++n)
        {
            std::string username = XML.getAttribute(tag, "username", "", n);
            std::string password = XML.getAttribute(tag, "password", "", n);
            
            std::string auth = XML.getAttribute(tag, "auth-type", "NONE", n);
            
            IPCameraDef::AuthType authType = IPCameraDef::AuthType::NONE;
            
            if (auth.compare("NONE") == 0)
            {
                authType = IPCameraDef::AuthType::NONE;
            }
            else if (auth.compare("BASIC") == 0)
            {
                authType = IPCameraDef::AuthType::BASIC;
            }
            else if (auth.compare("COOKIE") == 0)
            {
                authType = IPCameraDef::AuthType::COOKIE;
            }
            
            IPCameraDef def(XML.getAttribute(tag, "name", "", n),
                            XML.getAttribute(tag, "url", "", n),
                            username,
                            password,
                            authType);
            
            
            std::string logMessage = "STREAM LOADED: " + def.getName() +
            " url: " +  def.getURL() +
            " username: " + def.getUsername() +
            " password: " + def.getPassword() +
            " auth: " + std::to_string(static_cast<int>((def.getAuthType())));
            
            ofLogNotice() << logMessage;
            
            ipcams.push_back(def);
		}
		
		XML.popTag();
        
        fileName = XML.getValue("file","");
        useLocalVideo= (bool)XML.getValue("useLocalVideo",1);
        useLocalVideo= (bool)XML.getValue("useLocalVideo",1);
        videoName= XML.getValue("videoName","abierto.mp4");
        serialPort= XML.getValue("serialPort",-1);
        
        ofLog(OF_LOG_NOTICE, "filename: " + fileName);

	}
    else
    {
		ofLog(OF_LOG_ERROR, "Unable to load streams.xml.");
	}
    
	ofLog(OF_LOG_NOTICE, "-----------Loading Streams Complete----------");
    
    nextCamera = ipcams.size();
}

//------------------------------------------------------------------------------
void ofApp::videoResized(const void* sender, ofResizeEventArgs& arg)
{
    // find the camera that sent the resize event changed
    for(std::size_t i = 0; i < NUM_CAMERAS; i++)
    {
        if(sender == &grabbers[i])
        {
            std::stringstream ss;
            ss << "videoResized: ";
            ss << "Camera connected to: " << grabbers[i]->getURI() + " ";
            ss << "New DIM = " << arg.width << "/" << arg.height;
            ofLogVerbose("ofApp") << ss.str();
        }
    }
}



//------------------------------------------------------------------------------
void ofApp::update()
{
    // update the cameras
    if(status==HIBERN &&  ( (ofGetElapsedTimeMillis()/1000) - timeHibernStarted > 5 ) ){
        status=FINDING;
    }
    
    if(useLocalVideo==false){
        grabbers[faceTrackingGrabber]->update();
        //grabFrame.resize(640,480);
        grabFrame.setFromPixels(grabbers[faceTrackingGrabber]->getPixels(),
                                grabbers[faceTrackingGrabber]->getWidth(),
                                grabbers[faceTrackingGrabber]->getHeight(),
                                OF_IMAGE_COLOR);
        grabFrame.update();
        
        
    }else{
        videoPlayer.update();
        if(videoPlayer.isFrameNew() ){
            unsigned char * pixels ;
            pixels = videoPlayer.getPixels();
            grabFrame.setFromPixels(pixels, videoWidth,videoHeight, OF_IMAGE_COLOR);
            grabFrame.update();
        }
    }
    
    if(grabFrame.getWidth()>100){
        //Mat imgMat = toCv(grabFrame);
        finder.update( grabFrame );
            if(status==FINDING && finder.size()>0){
                cv::Rect mroi=cv::Rect(finder.getObject(0).x,finder.getObject(0).y,finder.getObject(0).width, finder.getObject(0).height);
                
                Mat imgMat = toCv(grabFrame);
                Mat imgMat3=imgMat(mroi); //TODO hacerlo funcioanr para varias caras
                //imgMat2=toCv(grabFrameEnvio);
                //Mat imgMat3=toCv(grabFrameEnvio);
                faceFinder.update(imgMat3);
                if(faceFinder.size()>0){
                    if( (ofGetElapsedTimeMillis()-lastTimeFaceDetected) > TIME_BETWEEN_DETECTIONS){
                        lastTimeFaceDetected=ofGetElapsedTimeMillis();

                        saveFrameAndNotify();
                    }
                }
            }
    }
    oscRcvUpdate();
}

//------------------------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0,0,0);
    
    // keep track of some totals
   // float avgFPS = totalFPS / NUM_CAMERAS;
   // float avgKbps = totalKbps / NUM_CAMERAS;

    ofEnableAlphaBlending();
    ofSetColor(0,80);
    ofDrawRectangle(5,5, 150, 40);
    
    ofSetColor(255);
    
    ofDisableAlphaBlending();
    
    grabFrame.draw(0,0,grabFrame.getWidth(),grabFrame.getHeight());
    finder.draw();
    ofSetColor(255,0,0);
    ofPushMatrix();
    if(status==FINDING && finder.size()>0){
        ofTranslate(facesRectangle.x, facesRectangle.y);
        faceFinder.draw();
    }
    ofPopMatrix();
    ofSetColor(255,255,255);
    //drawMat(imgMat2, 1000, 0);

    ofDrawBitmapStringHighlight(ofToString(finder.size()), 10, 20);
    //ofLog(OF_LOG_NOTICE, "file name file " + fileName);
    std::stringstream ss;

    if(useLocalVideo==false){
        
        ofSetHexColor(0xffffff);
        float kbps = grabbers[faceTrackingGrabber]->getBitRate() / 1000.0f; // kilobits / second, not kibibits / second
        float fps = grabbers[faceTrackingGrabber]->getFrameRate();
        
        // ofToString formatting available in 0072+
        ss << "          NAME: " << grabbers[faceTrackingGrabber]->getCameraName() << endl;
        ss << "          HOST: " << grabbers[faceTrackingGrabber]->getHost() << endl;
        ss << "           FPS: " << ofToString(fps,  2) << endl;
        ss << "          Kb/S: " << ofToString(kbps, 2) << endl;
        ss << " #Bytes Recv'd: " << ofToString(grabbers[faceTrackingGrabber]->getNumBytesReceived(),  0) << endl;
        ss << "#Frames Recv'd: " << ofToString(grabbers[faceTrackingGrabber]->getNumFramesReceived(), 0) << endl;
        ss << "Width: " << ofToString(grabbers[faceTrackingGrabber]->getWidth()) << endl;
        ss << "Height: " << ofToString(grabbers[faceTrackingGrabber]->getHeight()) << endl;
        ss << "Auto Reconnect: " << (grabbers[faceTrackingGrabber]->getAutoReconnect() ? "YES" : "NO") << endl;
        ss << " Needs Connect: " << (grabbers[faceTrackingGrabber]->getNeedsReconnect() ? "YES" : "NO") << endl;
        ss << "Time Till Next: " << grabbers[faceTrackingGrabber]->getTimeTillNextAutoRetry() << " ms" << endl;
        ss << "Num Reconnects: " << ofToString(grabbers[faceTrackingGrabber]->getReconnectCount()) << endl;
        ss << "Max Reconnects: " << ofToString(grabbers[faceTrackingGrabber]->getMaxReconnects()) << endl;
        ss << "  Connect Fail: " << (grabbers[faceTrackingGrabber]->hasConnectionFailed() ? "YES" : "NO");
        ss << "  frameRAte: " << ofToString(ofGetFrameRate() ) << endl;
        
    }
    else{
        ss << "  frameRAte: " << ofToString(ofGetFrameRate() ) << endl;
    }
    
    ss << "  Status: " << ofToString(status ) << endl;
    ofDrawBitmapString(ss.str(), 10, 10+12);
    drawDetection();
    
    
    /******* POST DRAW UPDATES ****/
    if(status==SHOOTING){
        status=HIBERN; //TODO mover esto al update
        timeHibernStarted=ofGetElapsedTimeMillis()/1000;
    }

}

void ofApp::drawDetection(){
    ofSetColor(0, 0, 200);
    ofNoFill();
    ofPushMatrix();
    ofTranslate(facesRectangle.x, facesRectangle.y);
    for(std::size_t i = 0; i < detections.size(); i++){

            std::stringstream ss;
        if(status==SHOOTING){
            
            
            ofFill();
            ofSetColor(0, 200, 0);
           
        }
        if( (ofGetElapsedTimeMillis()/1000) - timeLastDetection < 2 ){
        
            ofDrawRectangle(detections[i].position);
            ss << "  beard " << ofToString(detections[i].beard ) << endl;
            ss << "  age "  << ofToString(detections[i].age ) << endl;
            ss << "  glasses " << ofToString(detections[i].glasses ) << endl;
            ss << "  gender " << ofToString(detections[i].gender ) << endl;
            ss << "  smile " << ofToString(detections[i].smile ) << endl;
            ofDrawBitmapString(ss.str(), detections[i].position.x-90,detections[i].position.y );
        }
    }
    ofPopMatrix();
}

//------------------------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if(key == ' ')
    {
        // initialize connection
        for(std::size_t i = 0; i < NUM_CAMERAS; i++)
        {
            ofRemoveListener(grabbers[i]->videoResized, this, &ofApp::videoResized);
            auto c = std::make_shared<Video::IPVideoGrabber>();
            IPCameraDef& cam = getNextCamera();
            c->setUsername(cam.getUsername());
            c->setPassword(cam.getPassword());
            Poco::URI uri(cam.getURL());
            c->setURI(uri);
            c->connect();
            
            grabbers[i] = c;
        }
    }
    if(key == 'a' || key == 'A'){
        ofxOscMessage m;
        m.setAddress("/video");
        m.addIntArg(1);
       // m.addFloatArg(3.5f);
       // m.addStringArg("hello");
       // m.addFloatArg(ofGetElapsedTimef());
        sender.sendMessage(m);
    }
    if(key == 's' ){
        //string fileName = "/Users/sergiogalan/temporalborrar/snapshot.png";
        /*grabFrameEnvio.setFromPixels(grabbers[microsoftGrabber]->getPixels(), grabbers[microsoftGrabber]->getWidth(), grabbers[microsoftGrabber]->getHeight(), OF_IMAGE_COLOR);
        grabFrameEnvio.update();
        grabFrameEnvio.saveImage(fileName);
        ofLog(OF_LOG_NOTICE, "saved file " + fileName);*/
        saveFrameAndNotify();
    }
    if(key == 'b' ){//bang
        killThatOne();
    }
    
}

void ofApp::getContour(){
    //facesRectangle.set(0,0,0,0);
    for(int i=0; i<finder.size()>0; i++){
        if(i==0){
            facesRectangle=finder.getObject(i);
        }
        else{
            facesRectangle.growToInclude(finder.getObject(i));
        }
    }
}



void ofApp::saveFrameAndNotify(){
    
    //string fileName = "/Users/sergiogalan/temporalborrar/snapshot.png";
    if(useLocalVideo==false){
        grabbers[microsoftGrabber]->update();
        grabFrameEnvio.setFromPixels(
                                     grabbers[microsoftGrabber]->getPixels(),
                                     grabbers[microsoftGrabber]->getWidth(),
                                     grabbers[microsoftGrabber]->getHeight(),
                                     OF_IMAGE_COLOR);
        //grabFrameEnvio.update();
        //getContour();
        //grabFrameEnvio.crop(facesRectangle.x, facesRectangle.y, facesRectangle.width, facesRectangle.height);
        grabFrameEnvio.save(fileName);
        grabFrame.save("/Users/sergiogalan/temporalborrar/snapshot.jpg");
        
    }
    else{
        unsigned char * pixels ;
        pixels = videoPlayer.getPixels();
        grabFrameEnvio.setFromPixels(pixels, videoWidth,videoHeight, OF_IMAGE_COLOR);
        grabFrameEnvio.update();
        getContour();
        grabFrameEnvio.crop(facesRectangle.x, facesRectangle.y, facesRectangle.width, facesRectangle.height);
        grabFrameEnvio.save(fileName);
    }
    ofLog(OF_LOG_NOTICE, ofToString( ofGetElapsedTimeMillis()/1000) + ": saved file " + fileName);
    ofxOscMessage m;
    m.setAddress("/video");
    m.addIntArg(1);
    m.addFloatArg(3.5f);
    m.addStringArg("hello");
    m.addFloatArg(ofGetElapsedTimef());
    sender.sendMessage(m);
}
