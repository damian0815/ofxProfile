#include "testApp.h"
#define PROFILE
#include "ofxProfile.h"

//--------------------------------------------------------------
void testApp::setup(){

	ofSetFrameRate( 60 );
	
	lastDump = "";
	
}

//--------------------------------------------------------------
void testApp::update(){

	ofxProfileThisBlock();
	
	ofxProfileSectionPush("if and sleep");
	// sleep 1 ms every second frame
	if ( ofGetFrameNum() % 2 == 0 )
	{
		ofxProfileThisBlock("sleep 1ms every 2nd frame");
		ofSleepMillis(1);
	}
	ofxProfileSectionPop();
	
}

//--------------------------------------------------------------
void testApp::draw(){
	ofxProfileThisFunction();
	
	ofxProfileSectionPush("draw circle");
	ofxProfileSectionPush("calculate radius");
	ofxProfileSectionPush("sin");
	float sinus = sin( ofGetElapsedTimef() );
	ofxProfileSectionPop(); // pop "sin"
	float radius = 0.5f*(sinus+1.0f) * 30 + 10;
	ofxProfileSectionPop(); // pop "calculate radius"
	ofCircle( ofGetFrameNum()%ofGetWidth(), 50, radius );
	ofxProfileSectionPop(); // pop "draw circle"
	
	ofxProfileSectionPush("draw profiler information");
	ofDrawBitmapString( lastDump, ofPoint( 10, 120 ) );
	ofxProfileSectionPop();
	
	ofxProfileSectionPush("draw instruction text");
	ofDrawBitmapString( "press 'd' to update profile information, 'c' to clear profile data", ofPoint( 10, ofGetHeight()-20 ) );
	ofxProfileSectionPop();
	
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	if ( key == 'd' )
		lastDump = ofxProfile::Describe();
	else if ( key == 'c' )
	{
		ofxProfile::Clear();
		lastDump = "";
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}