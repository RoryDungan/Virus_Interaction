#include "Cell.hpp"
#include "World.hpp"
#include "Random.hpp"
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreEntity.h>

const float Cell::CELL_TRANSMITTED_HEAT = 0.5f;
const float Cell::CELL_ROTATE_SPEED = 0.001f;

Cell::Cell(World* world) : LifeForm(world) {
    mRotation.x = (float)Random::getSingleton().getRand(0, 360);
    mRotation.y = (float)Random::getSingleton().getRand(0, 360);
    mRotation.z = (float)Random::getSingleton().getRand(0, 360);
}


Cell::~Cell(void) {

}

void Cell::update(const int msDelta) {
    // Make living cells roll around
    if(bAlive) {
        mNode->pitch(Ogre::Radian(mRotation.x * CELL_ROTATE_SPEED * (float)msDelta / 1000.0f));
        mNode->yaw(Ogre::Radian(mRotation.y * CELL_ROTATE_SPEED * (float)msDelta / 1000.0f));
        mNode->roll(Ogre::Radian(mRotation.z * CELL_ROTATE_SPEED * (float)msDelta / 1000.0f));
	}
}

float Cell::getTemperature() const {
    return mTemperature;
}

Ogre::Vector3 Cell::getPosition() const {
    return mNode->getPosition();
}

bool Cell::isInfected() const {
    return bInfected;
}

void Cell::split() {
    resetMitosisTimer();
}

void Cell::resetMitosisTimer() {
    mLastSplitTime = Ogre::Root::getSingleton().getTimer()->getMilliseconds();
}

void Cell::denature() {
    bAlive = false;
}

void Cell::infect() {
    // Create some viruses
    for(int i = 0; i < 5; i++) {
        Ogre::SceneNode* node = mNode->createChildSceneNode(getRandomDirection() * 7.0f);
        node->attachObject(mWorld->getSceneMgr()->createEntity("Virus.mesh"));
	}
    bInfected = true;
}