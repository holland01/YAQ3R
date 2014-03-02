#pragma once

#include "../common.h"

enum
{
    KEYOBJ_UP = 0,
    KEYOBJ_DOWN = 1,
    KEYOBJ_LEFT = 2,
    KEYOBJ_RIGHT = 3,
    KEYOBJ_FORWARD = 4,
    KEYOBJ_BACKWARD = 5
};

class KeyMover
{
public:

    static const int NUM_KEYS = 6;

    KeyMover( glm::vec3& positionRef, float moveStep );

    void EvalKeyPress( int key );

    void EvalKeyRelease( int key );

    void Update( void );

private:

    bool keyStates[ NUM_KEYS ];

    glm::vec3& position;

    float step;
};
