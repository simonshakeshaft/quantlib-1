/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/patterns/singleton.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

namespace QuantLib {

    namespace {

        boost::mutex mutex;

        Integer singleSessionFunc() {
            return 0;
        }
    
        #if defined(QL_ENABLE_SESSIONS)
        // backwards compatibility to use the "sessionId" function
        sessionIdFunc getSessionIdFunc = &sessionId;
        #else
        // use the default function which provides only a single session
        sessionIdFunc getSessionIdFunc = &singleSessionFunc;
        #endif
    }

    void setSessionIdFunc(sessionIdFunc func) {
        // lock to ensure full data visibility possibly used in the function
        boost::unique_lock<boost::mutex> lock( mutex );
        getSessionIdFunc = func;
    }

    void* SingletonInstanceLoader(
                       std::map<Integer,boost::shared_ptr<void> > & instances,
                       boost::shared_ptr<void> (*newFunc)() ) {
        //multiple threads may access this, even without sessions the
        //first 0 session could be requested by more than one thread
        //at once.
        boost::unique_lock<boost::mutex> lock( mutex );
    
        Integer id = getSessionIdFunc();
        boost::shared_ptr<void>& instance = instances[id];
        if (!instance)
            instance = newFunc();
        return instance.get();
    }

}
