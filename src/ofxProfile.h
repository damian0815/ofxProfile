/***********************************************************************
 * Copyright (c) 2006-2012 Damian Stewart http://damianstewart.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of MSA Visuals nor the names of its contributors 
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 * ***********************************************************************/ 


#ifndef __PROFILER_H
#define __PROFILER_H


/** ofxProfile

 A thread-safe profiler. Stores and manages execution count, total execution time and 
 average time per execution of each labelled section, separated by thread.

 see readme.md for details.

*/


/// macros
#ifdef PROFILE
#define ofxProfileSectionPush( label ) ofxProfile::sectionPush( label );
#define ofxProfileSectionPop() ofxProfile::sectionPop();
#define ofxProfileThisFunction() volatile ofxProfileFunctionProfiler __function_profiler_object__( __FUNCTION__ );
#define ofxProfileThisBlock( label ) volatile ofxProfileFunctionProfiler __section_profiler_object__##__LINE__( label );

#ifdef _MSC_VER
#pragma message ("Profiling with ofxProfile enabled")
#else
#warning Profiling with ofxProfile enabled
#endif

#else
#define ofxProfileSectionPush( label ) ;
#define ofxProfileSectionPop() ;
#define ofxProfileThisFunction() ;
#define ofxProfileThisBlock( label );
//#warning Profiling with ofxProfile disabled
#endif


#include "ofConstants.h"
#include "ofThread.h"
#include "ofUtils.h"
#include <map>
#include <string>
#include <vector>


class ofxProfileSection;

class ofxProfileContext
{
public:
	ofThread* thread;
	ofxProfileSection* toplevel;
	ofxProfileSection* current;

    ofxProfileContext() { toplevel = NULL; }
    ~ofxProfileContext();
};


class ofxProfile
{
public:
    /// clear the database and restart profiling
    static void clear();

	/// start a section
	static void sectionPush( const std::string& name = "unlabelled section" );
	/// end a section
	static void sectionPop();

	/// show profiles recorded. SORT_BY defines sort order.
	typedef enum _SORT_BY { SORT_EXECUTION, SORT_TIME } SORT_BY;
	static string describe( SORT_BY sort = SORT_TIME );
	static void display( SORT_BY sort = SORT_TIME ) { ofLogNotice( "ofxProfile", "\n" + describe( sort ) ); }

private:
	/// return a pointer to the context for the current thread
	static ofxProfileContext* getContext();
	

	typedef std::vector<ofxProfileContext*> ofxProfileContexts;
	static ofxProfileContexts contexts;

	static ofMutex lock;
};

// one section
class ofxProfileSection {
public:
	ofxProfileSection();
	~ofxProfileSection();

	string describe( const std::string& prefix, ofxProfile::SORT_BY sortBy );

	int callCount;
	unsigned long totalTime;
	int execOrderId;
	static int EXEC_ORDER_ID;

	double timer;

	ofxProfileSection* parent;
	std::string name;

	// map of sections
	typedef std::map<const std::string, ofxProfileSection* > ofxProfileSections;

	// don't try this at home
	/*struct less_than_comparator : public std::binary_function<const ProfileSection*,const ProfileSection*,bool>
	{
		result_type operator() ( first_argument_type a, second_argument_type b )
	{
			return ( a->avg_time * a->call_count < b->avg_time*b->call_count );
	}
	};*/
	ofxProfileSections children;


};




/** ofxProfileFunctionProfiler

  convenience class. designed to be used as a volatile instance, within a function/block.

*/

class ofxProfileFunctionProfiler
{
public:
	ofxProfileFunctionProfiler( const char* function_name )
	{	ofxProfile::sectionPush(function_name);	}
	~ofxProfileFunctionProfiler()
	{	ofxProfile::sectionPop(); }
};



#endif
