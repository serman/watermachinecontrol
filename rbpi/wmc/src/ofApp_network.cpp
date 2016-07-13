//
//  ofApp_network.cpp
//  ipcameraWMC
//
//  Created by Sergio Galan on 08/07/16.
//
//

#include <stdio.h>
#include "ofApp.h"
using namespace ofxCv;
using namespace cv;
//https://gist.github.com/darrenmothersele/7597016

// The main openFrameworks include
#include "ofMain.h"

void ofApp::sendImage(){
   

}

void ofApp::oscRcvUpdate(){
    while(myosc.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;

        myosc.getNextMessage(&m);
        detections.clear();
        // check for mouse moved message
        if(m.getAddress() == "/face"){
            cout << ofToString(m.getNumArgs());
            detectionData d;
            d.set(m.getArgAsInt32(0),//x
                  m.getArgAsInt32(1),//y
                  m.getArgAsInt32(2),//w
                  m.getArgAsInt32(3),//h
                  m.getArgAsInt32(4),//bear
                  m.getArgAsInt32(5),//age
                  m.getArgAsInt32(6),//glasses
                  m.getArgAsInt32(7),//gender
                  m.getArgAsInt32(8)//smile
                  );
            detections.push_back(d);
            timeLastDetection=ofGetElapsedTimeMillis()/1000;
        }
        takeShootingDecision();
    }
    
    
}


void ofApp::takeShootingDecision(){
    //detections
    for(std::size_t i = 0; i < detections.size(); i++){
       if(detections[i].age>30 /*&& detections[i].beard>0.5*/ )
           killThatOne();
    }
    
}
void ofApp::killThatOne(){
    // send arduino Orde
    if(status==FINDING){
        status=SHOOTING;
        ofLog(OF_LOG_NOTICE, ofToString(detections.size()) + "DISPARO " + ofToString(ofGetElapsedTimeMillis()/1000));
        if(serial.isInitialized()){
            bool byteWasWritten = serial.writeBytes(&bangMsg[0],3);
            if ( !byteWasWritten )
                printf("byte was not written to serial port");
        }
        
    }
}