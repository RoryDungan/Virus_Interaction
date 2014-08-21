#pragma once
#include "LifeForm.hpp"

class Cell :
	public LifeForm
{
public:
	Cell(World* world);
	virtual ~Cell(void);

    virtual float getTemperature() const;
    virtual Ogre::Vector3 getPosition() const;
    virtual bool isInfected() const;
    virtual void update(const int msDelta);

protected:
    virtual void resetMitosisTimer();
    virtual void split();
    virtual void denature();
    virtual void infect();

    float mMoveSpeed; // Cells move faster when they're first genarated and stop when dead
    Ogre::Vector3 mMoveDirection;
    Ogre::Vector3 mRotation;

    unsigned int mLastSplitTime;
    unsigned int mNextSplit;
    float mTemperature;
    bool bInfected;

    static const float CELL_TRANSMITTED_HEAT; // Amount of heat transferred to other cells.
    static const float CELL_ROTATE_SPEED;
};

