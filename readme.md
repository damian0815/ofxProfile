**ofxProfile**

by Damian Stewart <http://damianstewart.com>

A thread-safe profiler. Stores and manages execution count, total execution time and average time per execution of each labelled section, separated by thread.

To use in a particular cpp file, you need to #define PROFILE either globally or in that file, and #include the ofxProfile.h header. 

	#define PROFILE
	#include "ofxProfile.h"

Then use the following macros to profile parts of code:

* `ofxProfileSectionPush(label)` and `ofxProfileSectionPop()`

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

* `ofxProfileThisFunction()`

  Automatically wraps the current function in a pair of `ofxProfileSectionPush(functionName)` and `ofxProfileSectionPop()` calls.

* `ofxProfileThisBlock(label)`

  Wraps the current block (the code between the current level `{` and `}`) in a pair of `ofxProfileSectionPush(label)` and `ofxProfileSectionPop()` calls.  

  eg:

            if ( test_condition )
            {
                ofxProfileThisBlock( "test passed" );
                // code to run on test condition
            }


To display profile results, call `ofxProfile::display()`. 

To fetch results as a string, call `ofxProfile::describe()`.

To clear current results, call `ofxProfile::clear()`.
 
**Notes**
 
* If PROFILE is not #defined, the profiling code is not executed. This means you can leave profiling code in place for release builds and not have it impact on performance.
 
* Profiling is invasive, this means it will have an impact on execution times. Output from ofxProfile is therefore not 100% accurate but is usually good enough to help nail down performance bottlenecks.
 
* OpenGL may do all sorts of interesting things behind your back, that ofxProfiler doesn't know about. If you want to profile OpenGL calls please use a proper OpenGL profiler instead.
 

original GitHub url: <http://github.com/damiannz/ofxProfile>
