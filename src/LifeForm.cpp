#include "LifeForm.hpp"
#include "World.hpp"
#include "sha1.h"
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreMath.h>

LifeForm::LifeForm(World* world) {
    mWorld = world;
    bAlive = true;
    mInceptionTime = Ogre::Root::getSingleton().getTimer()->getMilliseconds();

    // Create a unique name
    unsigned char hash[20];
    unsigned long time = Ogre::Root::getSingleton().getTimer()->getMicroseconds();
    sha1::calc(&time, sizeof(unsigned long), hash);
    char hexString[41];
    sha1::toHexString(hash, hexString);
    mName = Ogre::String(hexString);
}


LifeForm::~LifeForm(void) {
    mNode->removeAndDestroyAllChildren();
    mWorld->getSceneMgr()->destroySceneNode(mNode);
}

bool LifeForm::isAlive() const {
    return bAlive;
}

Ogre::Vector3 LifeForm::getRandomDirection() {
    return Ogre::Vector3(Ogre::Math::SymmetricRandom(), Ogre::Math::SymmetricRandom(), 0.0f);
}

Ogre::String LifeForm::getName() const {
    return mName;
}

void LifeForm::showSelected(const bool selected) {
    mNode->showBoundingBox(selected);
}