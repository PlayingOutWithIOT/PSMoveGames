//
//  testControllers.hpp
//  PSMoveOSX
//
//  Created by localadmin on 07/06/2018.
//

#pragma once
#include <stdio.h>
#include "movegame.hpp"

class Skipping : public MoveGame
{
public:
    BtU32 numControllers;
    PSMove *moveArr[16];

#ifdef MADGWICK
    Madgwick madgwick[16];
#endif
    
    void reset();
    void setup();
    void update();
};
