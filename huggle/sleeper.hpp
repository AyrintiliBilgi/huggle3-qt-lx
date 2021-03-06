//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef SLEEPER_HPP
#define SLEEPER_HPP

#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QThread>

namespace Huggle
{
    /*!
     * \brief This is a workaround that allow us to use sleep
     */
    class Sleeper : public QThread
    {
        public:
            static void usleep(unsigned long usecs){QThread::usleep(usecs);}
            static void msleep(unsigned long msecs){QThread::msleep(msecs);}
            static void sleep(unsigned long secs){QThread::sleep(secs);}
    };
}

#endif // SLEEPER_HPP
