#include "testApp.h"
#define PROFILE
#include "ofxProfiler.h"

//--------------------------------------------------------------
void testApp::setup(){

	ofSetFrameRate( 60 );
	
	lastDump = "";
	
}

//--------------------------------------------------------------
void testApp::update(){

	PROFILE_THIS_FUNCTION();
	
	PROFILE_SECTION_PUSH("if and sleep");
	// sleep 1 ms every second frame
	if ( ofGetFrameNum() % 2 == 0 )
	{
		PROFILE_THIS_BLOCK("sleep");
		ofSleepMillis(1);
	}
	PROFILE_SECTION_POP();
	
}

//--------------------------------------------------------------
void testApp::draw(){
	PROFILE_THIS_FUNCTION();
	
	PROFILE_SECTION_PUSH("draw circle");
	ofCircle( ofGetFrameNum()%ofGetWidth(), 50, 10 );
	PROFILE_SECTION_POP();
	
	PROFILE_SECTION_PUSH("draw dump");
	ofDrawBitmapString( lastDump, ofPoint( 10, 100 ) );
	PROFILE_SECTION_POP();
	
	PROFILE_SECTION_PUSH("draw instruction text");
	ofDrawBitmapString( "press 'd' to update profile dump, 'c' to clear profile data", ofPoint( 10, ofGetHeight()-20 ) );
	PROFILE_SECTION_POP();
	
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	if ( key == 'd' )
		lastDump = ofxProfiler::Describe();
	else if ( key == 'c' )
		ofxProfiler::Clear();
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