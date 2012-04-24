/*
 Copyright 2008, 2009, 2010 Damian Stewart <damian@frey.co.nz>.

 This file is part of The Artvertiser.

 The Artvertiser is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The Artvertiser is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with The Artvertiser.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ofxProfiler.h"

#include <algorithm>
#include "ofThread.h"
#include "ofConstants.h"
#include "ofUtils.h"

ofxProfiler::ofxProfileContexts ofxProfiler::contexts;
ofMutex ofxProfiler::lock;

int ofxProfileSection::EXEC_ORDER_ID = 0;

ofxProfileContext::~ofxProfileContext()
{
    if ( toplevel )
        delete toplevel;
}

ofxProfileContext* ofxProfiler::GetContext()
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



void ofxProfiler::Clear()
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

void ofxProfiler::SectionPush(const std::string &name)
{
	ofxProfileContext* context = GetContext();
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


void ofxProfiler::SectionPop()
{
    unsigned long endTime = ofGetElapsedTimeMicros();
 
	// grab the section
	ofxProfileContext* context = GetContext();
	ofxProfileSection* s = context->current;

    // check we're not popping up too far
	if ( context->current->parent == NULL )
        return;

	// get time for this run in ms
    unsigned long timeMicros = endTime - s->timer;

	// work out the new avg time and increment the call count
	double totalTimeMicros = timeMicros + s->avgTime * s->callCount;
	s->callCount++;
	s->avgTime = totalTimeMicros/s->callCount;

	// shift current up
	context->current = context->current->parent;
}

string ofxProfiler::Describe( ofxProfiler::SORT_BY sort )
{
	string output = "";
	char buf[1024];
	output += "---------------------------------------------------------------------------------------\n";
    // re-use formatting from individual lines
    sprintf( buf, "ofxProfiler output: sorted by %-15s            total    average   execution\n", (sort==SORT_EXECUTION?"execution order":"total time"));
	output += buf;
    sprintf( buf, "%-50s  %10s  %10s  %6s\n", "name                                values in ms ->", "time  ", "time    ", "count" );
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
		output += (*i)->toplevel->Describe("| ", sort );
	}
	lock.unlock();
	output += "---------------------------------------------------------------------------------------\n";
	return output;
}


ofxProfileSection::ofxProfileSection()
{
	parent = NULL;
	avgTime = 0;
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
    return a->avgTime*a->callCount > b->avgTime*b->callCount;
}

bool execution_order_comparator( ofxProfileSection* a, ofxProfileSection* b )
{
    return a->execOrderId < b->execOrderId;
}

string ofxProfileSection::Describe( const std::string& prefix, ofxProfiler::SORT_BY sort_by )
{
    std::vector<ofxProfileSection* > childrenVect;
	for ( ofxProfileSections::iterator i = children.begin();
		  i!= children.end();
		  ++i )
	{
        childrenVect.push_back( (*i).second );
	}

    // sort by ..
    if ( sort_by == ofxProfiler::SORT_TIME )
    {
        std::sort( childrenVect.begin(), childrenVect.end(), reverse_time_comparator );
    }
    else if ( sort_by == ofxProfiler::SORT_EXECUTION )
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
				  0.001 * sect->avgTime * sect->callCount,
				  0.001 * sect->avgTime, sect->callCount );
		output += buf;

        // if this is the last child,
        std::string nextPrefix = prefix;
        if ( prefix.size() > 1 && i==childrenVect.size()-1 )
        {
            // erase the previous "| " and replace with "  "
            nextPrefix = nextPrefix.substr(0, nextPrefix.size()-2 ) + std::string("  ");
        }
        // next deeper level
        output += sect->Describe( nextPrefix + "| ", sort_by );

	}
	return output;
}
