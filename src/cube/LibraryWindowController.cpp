//////////////////////////////////////////////////////////////////////////////
//
// License Agreement:
//
// The following are Copyright � 2007, Casey Langen
//
// Sources and Binaries of: mC2, win32cpp
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright 
//      notice, this list of conditions and the following disclaimer in the 
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the author nor the names of other contributors may 
//      be used to endorse or promote products derived from this software 
//      without specific prior written permission. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE. 
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.hpp"
#include <cube/LibraryWindowController.hpp>
#include <cube/LibraryWindowView.hpp>
#include <cube/SourcesView.hpp>
#include <cube/SourcesController.hpp>
#include <cube/TracklistController.hpp>
#include <cube/TracklistView.hpp>

#include <core/LibraryFactory.h>
#include <core/Pluginfactory.h>
#include <core/PlaybackQueue.h>
#include <core/Preferences.h>

#include <win32cpp/Types.hpp>    // uichar, uistring
#include <win32cpp/TopLevelWindow.hpp>
#include <win32cpp/Splitter.hpp>
#include <win32cpp/TabView.hpp>
#include <win32cpp/RedrawLock.hpp>
 
//////////////////////////////////////////////////////////////////////////////

using namespace musik::cube;

//////////////////////////////////////////////////////////////////////////////

/*ctor*/    LibraryWindowController::LibraryWindowController(LibraryWindowView& view)
: view(view)
, nowPlayingController(NULL)
{
    musik::core::PluginFactory::Instance();

    this->view.Handle()
        ? this->OnViewCreated(&view)
        : this->view.Created.connect(this, &LibraryWindowController::OnViewCreated);

    this->view.Destroyed.connect(this,&LibraryWindowController::OnDestroyed);
}

LibraryWindowController::~LibraryWindowController()
{
    delete this->nowPlayingController;
}

void        LibraryWindowController::OnViewCreated(Window* window)
{

    using namespace musik::core;

    // Start by adding the "now playing" tab
    TracklistView *nowPlayingView   = new TracklistView();
    this->nowPlayingController = new TracklistController(*nowPlayingView,NULL,musik::core::PlaybackQueue::Instance().NowPlayingTracklist(),TracklistController::Deletable|TracklistController::HighlightActive);
    this->view.AddTab( uistring(UTF("Now Playing")) ,nowPlayingView);


	// Get libraries from LibraryFactory
    this->UpdateLibraryTabs();
    LibraryFactory::Instance().LibrariesUpdated.connect(this,&LibraryWindowController::UpdateLibraryTabs);

	// Connect to the MessageQueue
	musik::core::MessageQueue::EventController().connect(this,&LibraryWindowController::OnLibraryMessage);
}

void LibraryWindowController::UpdateLibraryTabs(){
    using namespace musik::core;

    musik::core::Preferences windowPrefs("MainWindow");
    int activeLibrary   = windowPrefs.GetInt("activeLibrary",1);

    RedrawLock redrawLock(&this->view);

    LibraryFactory::LibraryVector& libraries	= LibraryFactory::Libraries();

    // Loop through the libraries
    for(LibraryFactory::LibraryVector::iterator library=libraries.begin();library!=libraries.end();++library){

        const utfstring libraryId( (*library)->Identifier() );

        // check if library already exist
        bool found(false);
        for(SourcesMap::iterator libraryWindow=this->libraries.begin();libraryWindow!=this->libraries.end() && !found;++libraryWindow){
            if( libraryWindow->second->library->Identifier()==libraryId ){
                found   = true;
            }
        }

        if(!found){
            SourcesView* sourcesView = new SourcesView();
            this->libraries[sourcesView]	= SourcesControllerPtr(new SourcesController(*sourcesView,*library));
            this->view.AddTab( (*library)->Name() ,sourcesView);

            // Active this library
            if((*library)->Id()==activeLibrary){
                this->view.SetActiveTab(sourcesView);
            }
        }

    }
}

SourcesController* LibraryWindowController::CurrentSourceController(){
	SourcesView *sourcesView	= (SourcesView*)this->view.ActiveWindow();

	SourcesMap::iterator foundLibrary	= this->libraries.find(sourcesView);
	if(foundLibrary!=this->libraries.end()){
		return foundLibrary->second.get();
	}
	return NULL;
}

void LibraryWindowController::OnLibraryMessage(const char* identifier,void* data){
	if(!data){
		SourcesController* sourcesController	= this->CurrentSourceController();
		if(sourcesController){
			musik::core::MessageQueue::SendMessage(identifier,sourcesController->library.get());
		}
	}
}

void LibraryWindowController::OnDestroyed(Window* window){
    musik::core::Preferences windowPrefs("MainWindow");
    SourcesView *activeWindow    = (SourcesView*)this->view.VisibleWindow();

    SourcesMap::iterator sourceController = this->libraries.find(activeWindow);
    if(sourceController!=this->libraries.end()){
        windowPrefs.SetInt("activeLibrary",sourceController->second->library->Id());
    }else{
        windowPrefs.SetInt("activeLibrary",0);
    }
}


