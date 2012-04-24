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

    To use in a particular cpp file, you need to #define PROFILE either globally or in that 
	file, and #include the ofxProfile.h header. 

	#define PROFILE
	#include "ofxProfile.h"

    Then use the following macros to profile parts of code:

    * ofxProfileSectionPush(label) and ofxProfileSectionPop()

        Profile code between these calls, storing results in a tree.
        For example:

            .. unprofiled preprocessing
            ofxProfileSectionPush("partial section");
            .. code to be profiled
            ofxProfileSectionPop();
            .. unprofiled postprocessing

        Or for nested output:

            ofxProfileSectionPush("two-step process");
                .. preprocessing code
                ofxProfileSectionPush("step 1");
                .. step 1 code
                ofxProfileSectionPop();
                ofxProfileSectionPush("step 2");
                .. step 2 code
                ofxProfileSectionPop();
            ofxProfileSectionPop();

        will produce output like this:
 
            + two-step process      <total time for steps 1 + 2 + preprocessing> ...
			  + step 1              <time for step 1> ...
			  + step 2              <time for step 2> ...

        NOTE: all labels at a given level in the tree must be unique.

    * ofxProfileThisFunction()

        Wraps the current function in a pair of
        ofxProfileSectionPush( function_name ) and ofxProfileSectionPop calls.

    * ofxProfileThisBlock(label)

        Wraps the current block (the code between the current level { and })
        in a pair of ofxProfileSectionPush( label ) and ofxProfileSectionPop
        calls.

        eg:

            if ( test_condition )
            {
                ofxProfileThisBlock( "test passed" );
                // code to run on test condition
            }


    To display profile results, call ofxProfile::Display(). 
	To fetch results as a string, call ofxProfile::Describe().
 
	NOTES
 
	If PROFILE is not #defined, the profiling code is not executed. This means you can leave
	profiling code in place for release builds and not have it impact on performance.
 
	Profiling is invasive, this means it will have an impact on execution times. Output from 
	ofxProfile is therefore not 100% accurate but is usually good enough to help nail down
	performance bottlenecks.
 
	OpenGL may do all sorts of interesting things behind your back, that ofxProfiler doesn't 
	kno about. If you want to profile OpenGL calls please use a proper OpenGL profiler instead.
 


@author Damian

*/


/// macros
#ifdef PROFILE
#define ofxProfileSectionPush( label ) ofxProfile::SectionPush( label );
#define ofxProfileSectionPop() ofxProfile::SectionPop();
#define ofxProfileThisFunction() volatile ofxProfileFunctionProfiler __function_profiler_object__( __FUNCTION__ );
#define ofxProfileThisBlock( label ) volatile ofxProfileFunctionProfiler __section_profiler_object__##__LINE__( label );
#warning Profiling with ofxProfile enabled
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
    static void Clear();

	/// start a section
	static void SectionPush( const std::string& name = "unlabelled section" );
	/// end a section
	static void SectionPop();

	/// return a pointer to the context for the current thread
	static ofxProfileContext* GetContext();

	/// show profiles recorded. SORT_BY defines sort order.
	typedef enum _SORT_BY { SORT_EXECUTION, SORT_TIME } SORT_BY;
	static string Describe( SORT_BY sort = SORT_TIME );
	static void Display( SORT_BY sort = SORT_TIME ) { ofLogNotice( "ofxProfile", "\n" + Describe( sort ) ); }

private:

	typedef std::vector<ofxProfileContext*> ofxProfileContexts;
	static ofxProfileContexts contexts;

	static ofMutex lock;
};

// one section
class ofxProfileSection {
public:
	ofxProfileSection();
	~ofxProfileSection();

	string Describe( const std::string& prefix, ofxProfile::SORT_BY sortBy );

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
	{	ofxProfile::SectionPush(function_name);	}
	~ofxProfileFunctionProfiler()
	{	ofxProfile::SectionPop(); }
};



#endif
