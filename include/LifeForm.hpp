#pragma once

#include <OGRE/OgreSceneNode.h>

class World;

class LifeForm
{
public:
	LifeForm(World* world);
	virtual ~LifeForm(void);

    virtual void update(const int msDelta) = 0;
    virtual Ogre::String getName() const;
    virtual bool isAlive() const;
    virtual void showSelected(const bool selected);

protected:
    Ogre::Vector3 getRandomDirection();
    World* mWorld;
    Ogre::SceneNode* mNode;
    Ogre::String mName;

    bool bAlive;
    unsigned long mInceptionTime;
};

