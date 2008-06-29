//////////////////////////////////////////////////////////////////////////////
//
// License Agreement:
//
// The following are Copyright � 2008, Daniel �nnerby
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
#pragma once

#include <core/config.h>
#include <expat/expat.h>
#include <sigslot/sigslot.h>
#include <string>
#include <vector>
#include <set>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <core/xml/Node.h>

//////////////////////////////////////////////////////////////////////////////

namespace musik{ namespace core{ namespace xml{

// Forward
class ParserNode;
//////////////////////////////////////////////////////////////////////////////

class Parser{
    public:
        Parser(boost::asio::ip::tcp::socket *socket);
        ~Parser();

    private:
		// XML Parser status
        XML_Parser xmlParser;
		int xmlParserStatus;

		// XML specific info
        int level;
        std::vector<Node::Ptr> currentNodeLevels;


        // Wait variables
        enum AWaitingTypes:int{
            NodeStart=1,
            NodeEnd=2,
            Content=3
        } awaitingType;

        std::list<std::string> awaitingNodeLevels;
        std::set<std::string> awaitingNodes;
        ParserNode *setNode;
        bool setNodeSuccess;

        bool continueParsing;

        std::list<std::string> NodeLevel();

		// Socket stuff
        boost::asio::ip::tcp::socket *socket;       

    private:
		void ContinueParsing();

        bool CheckExpected(int typeOfEvent);

        static void OnElementStart(void *thisobject,const char *name, const char **atts);
        void OnElementStartReal(const char *name, const char **atts);
        static void OnElementEnd(void *thisobject,const char *name);
        void OnElementEndReal(const char *name);
/*        static void OnContent(void *thisobject,const char *content,int length);
        void OnContentReal(const char *content,int length);
*/
    private:
        friend class ParserNode;

        bool WaitForNode(ParserNode *setNode);
        bool WaitForNode(ParserNode *setNode,std::string &expectedNode);

        void ReadFromSocket();

        boost::array<char, 4096> readBuffer;
        size_t readBufferLength;

};

//////////////////////////////////////////////////////////////////////////////
} } }
