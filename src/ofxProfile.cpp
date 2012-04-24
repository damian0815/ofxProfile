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

#include "ofxProfile.h"

#include <algorithm>
#include "ofThread.h"
#include "ofConstants.h"
#include "ofUtils.h"

ofxProfile::ofxProfileContexts ofxProfile::contexts;
ofMutex ofxProfile::lock;

int ofxProfileSection::EXEC_ORDER_ID = 0;

ofxProfileContext::~ofxProfileContext()
{
    if ( toplevel )
        delete toplevel;
}

ofxProfileContext* ofxProfile::getContext()
{
	ofThread* currentThread = ofThread::getCurrentThread();
	lock.lock();
	// try to get current thread context
	for ( ofxProfileContexts::const_iterator i = contexts.begin();
		  i!= contexts.end();
		  ++i )
	{
		if ( currentThread == ( *i )->thread )
		{
			lock.unlock();
			return *i;
		}
	}

	// no context found for this thread: must create a new one
	ofxProfileContext* context = new ofxProfileContext();
	// add it to the vector
	contexts.push_back( context );
	// fill in details
	context->thread = currentThread;
	context->toplevel = new ofxProfileSection();
	context->current = context->toplevel;

	// return
	lock.unlock();
	return context;
}



void ofxProfile::clear()
{
    // get lock
    lock.lock();
    // delete everything
    for ( int i=0; i<contexts.size(); i++ )
    {
        delete contexts[i];
    }
    contexts.clear();

    // done
    lock.unlock();

}

void ofxProfile::sectionPush(const std::string &name)
{
	ofxProfileContext* context = getContext();
	assert( context->current );

	// try to grab the section out of the db
	// we store by name so that we can accumulate results over multiple frames
	ofxProfileSection* s = context->current->children[name];
	if ( s == NULL )
	{
		s = new ofxProfileSection();
		s->parent = context->current;
		s->name = name;
		context->current->children[name] = s;
	}

	// shift current to us
	context->current = s;

	// store start time
	context->current->timer = ofGetElapsedTimeMicros();
//	QueryPerformanceCounter(&s->start_time);

}


void ofxProfile::sectionPop()
{
    unsigned long endTime = ofGetElapsedTimeMicros();
 
	// grab the section
	ofxProfileContext* context = getContext();
	ofxProfileSection* s = context->current;

    // check we're not popping up too far
	if ( context->current->parent == NULL )
        return;

	// get time for this run in ms
    unsigned long timeMicros = endTime - s->timer;

	// work out the new avg time and increment the call count
	s->totalTime += timeMicros;
	s->callCount++;

	// shift current up
	context->current = context->current->parent;
}

string ofxProfile::describe( ofxProfile::SORT_BY sort )
{
	string output = "";
	char buf[1024];
	output += "---------------------------------------------------------------------------------------\n";
    // re-use formatting from individual lines
	sprintf( buf, "ofxProfile output: sorted by %-15s", (sort==SORT_EXECUTION?"execution order":"total time") );
    sprintf( buf, "%-52s  %10s  %10s  %6s\n", buf, "total  ", "average  ", "execution" );
	output += buf;
    sprintf( buf, "%-52s  %10s  %10s  %6s\n", "name", "time (ms)", "time (ms) ", " count" );
	output += buf;
    output += "---------------------------------------------------------------------------------------\n";
	lock.lock();
	for ( ofxProfileContexts::iterator i = contexts.begin();
		  i != contexts.end();
		  ++i )
	{
		if ( (*i)->thread == NULL )
			sprintf( buf, "Main thread\n" );
		else
			sprintf(buf, "Thread %lx\n", (unsigned long)((*i)->thread) );
		output += buf;
		output += (*i)->toplevel->describe("| ", sort );
	}
	lock.unlock();
	output += "---------------------------------------------------------------------------------------\n";
	return output;
}


ofxProfileSection::ofxProfileSection()
{
	parent = NULL;
	totalTime = 0;
	callCount = 0;
	execOrderId = EXEC_ORDER_ID++;
}

ofxProfileSection::~ofxProfileSection()
{
    for ( ofxProfileSections::iterator i = children.begin();
        i != children.end();
        ++i )
    {
        delete (*i).second;
    }
    children.clear();
}

bool reverse_time_comparator( ofxProfileSection* a, ofxProfileSection* b )
{
    return a->totalTime > b->totalTime;
}

bool execution_order_comparator( ofxProfileSection* a, ofxProfileSection* b )
{
    return a->execOrderId < b->execOrderId;
}

string ofxProfileSection::describe( const std::string& prefix, ofxProfile::SORT_BY sort_by )
{
    std::vector<ofxProfileSection* > childrenVect;
	for ( ofxProfileSections::iterator i = children.begin();
		  i!= children.end();
		  ++i )
	{
        childrenVect.push_back( (*i).second );
	}

    // sort by ..
    if ( sort_by == ofxProfile::SORT_TIME )
    {
        std::sort( childrenVect.begin(), childrenVect.end(), reverse_time_comparator );
    }
    else if ( sort_by == ofxProfile::SORT_EXECUTION )
    {
        std::sort( childrenVect.begin(), childrenVect.end(), execution_order_comparator );
    }

	string output = "";
	char buf[1024];
    for ( int i=0; i<childrenVect.size(); i++ )
    {
        ofxProfileSection* sect = childrenVect[i];
	    // replace '+' with '|';
		std::string name;
		if ( prefix.size()>1 )
            name = prefix.substr( 0, prefix.size()-2 ) + std::string("+ ") + sect->name;
        else
            name = sect->name;
		sprintf( buf, "%-50s  %10.2f  %10.5f  %6d\n", name.c_str(),
				  0.001 * sect->totalTime,
				  0.001 * double(sect->totalTime)/sect->callCount, sect->callCount );
		output += buf;

        // if this is the last child,
        std::string nextPrefix = prefix;
        if ( prefix.size() > 1 && i==childrenVect.size()-1 )
        {
            // erase the previous "| " and replace with "  "
            nextPrefix = nextPrefix.substr(0, nextPrefix.size()-2 ) + std::string("  ");
        }
        // next deeper level
        output += sect->describe( nextPrefix + "| ", sort_by );

	}
	return output;
}
