//
// Copyright (C) 2019 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <iostream>
#include <vector>
#include <filesystem>

#include "Traiettoria.h"

#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"

using namespace matlab::engine;
using namespace matlab::data;
using namespace std::filesystem;

namespace plexe {

Define_Module(Traiettoria);

std::unique_ptr<MATLABEngine> matlabEngine;

void Traiettoria::initialize(int stage)
{
    if(stage == 0){
        try {
            // start the engine
            matlabEngine = startMATLAB();
            
            // add script path
            std::string relPath = "../../matlab";
            std::u16string absPath = std::filesystem::absolute(relPath).u16string();
            std::u16string addPathCmd = u"addpath('" + absPath + u"');";
            matlabEngine->eval(addPathCmd);


        } catch (const EngineException& e) {
            EV_ERROR << "Failed to start MATLAB: " << e.what() << std::endl;
        }
    }
}

void Traiettoria::finish()
{
    EV_INFO << "finishing" << std::endl;
    matlabEngine.reset();
}

void Traiettoria::handleMessage(cMessage* msg)
{
    delete msg;
}

MatlabResult Traiettoria::traiettoria(double t, double xA2, double xA1, double xA, double yA)
{   
    /*
    ArrayFactory factory;
    TypedArray<double> arg01 = factory.createArray({1,1}, {xA2});
    TypedArray<double> arg02 = factory.createArray({1,1}, {xA1});
    TypedArray<double> arg03 = factory.createArray({1,1}, {xA});
    TypedArray<double> arg04 = factory.createArray({1,1}, {yA});
    */


    ArrayFactory factory;
    std::vector<Array> args;
    args.push_back(factory.createArray({1, 1}, {t}));
    args.push_back(factory.createArray({1, 1}, {xA2}));
    args.push_back(factory.createArray({1, 1}, {xA1}));
    args.push_back(factory.createArray({1, 1}, {xA}));
    args.push_back(factory.createArray({1, 1}, {yA}));
    
    std::vector<Array> result = matlabEngine->feval(u"traiettoria", 6, args);
    if (result.size() != 6) {
        throw cRuntimeError("MATLAB returned a wrong number of elements");
    }

    MatlabResult out {result[0][0], result[1][0], result[2][0], result[3][0], result[4][0], result[5][0]};
    return out;
}

Traiettoria::~Traiettoria(){
}

}
