#include "CellB.hpp"
#include "CellA.hpp"
#include "World.hpp"
#include "Random.hpp"
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreEntity.h>

const unsigned int CellB::ABSORPTION_TIMES[] = {
    1900, 1700, 3600, 2900, 2500, 1200, 1900, 2400, 2100, 3000
};
const unsigned int CellB::DEATH_TIMES[] = {
    11100, 11000, 11300, 11200, 11400, 11600, 11700, 11300, 11100, 11500
};

const float CellB::NORMAL_TEMPERATURE = 35.0f;
const float CellB::MAX_TEMPERATURE = 110.0f;
const float CellB::MIN_TEMPERATURE = 0.0f;
const float CellB::MIN_MOVEMENT_TEMPERATURE = 20.0f;
const float CellB::MITOSIS_TEMP_INCREASE = 3.0f;
const float CellB::TEMP_DIFF_PER_SECOND = 0.5f;
const float CellB::CELL_INTERNAL_HEAT = 3.0f;
const float CellB::INFECTION_CHANCE = 0.28f;
const float CellB::INFECTED_MITOSIS_TIME_MULTIPLIER = 0.2f; // 80% faster = 0.2x original speed
const float CellB::NORMAL_PH = 7.365f;
const float CellB::MOVE_PER_SEC = 10.0f;
const Ogre::Real CellB::MAX_ABSORPTION_DISTANCE = 3.0f;
const unsigned int CellB::SPAWN_TIME = 5000;

CellB::CellB(World* world, const Ogre::Vector3 pos) : Cell(world) {
    // Create entity
    Ogre::Entity* ent = mWorld->getSceneMgr()->createEntity(mName, "CellB.mesh");
    mNode = mWorld->getSceneMgr()->getRootSceneNode()->createChildSceneNode();
    mNode->attachObject(ent);
    mNode->setPosition(pos);

    mTarget = NULL;

    mTemperature = NORMAL_TEMPERATURE;
    mMoveSpeed = MOVE_PER_SEC;
    mMoveDirection = getRandomDirection();
    mCurrentState = STATE_SPAWNING;
    bInfected = false;

    resetMitosisTimer();
}


CellB::~CellB(void)
{
}

void CellB::update(const int msDelta) {
    // Don't do anything if the cell is dead
    if(!bAlive) {
        return;
	}

    Cell::update(msDelta);

    calculateTemperature(msDelta);

    // Calculate movement
    switch(mCurrentState) {
	case STATE_SPAWNING:
        // Move in a random direction for a bit so that the B cells don't all just target the same A cell
        mNode->translate(mMoveDirection * mMoveSpeed * (float)msDelta / 1000.0f);
        // Find a new direction to move in if we go outisde the bounds of the world
        if(mNode->getPosition().x > mWorld->getWorldSize().x || mNode->getPosition().y > mWorld->getWorldSize().y || mNode->getPosition().x < 0.0f || mNode->getPosition().y < 0.0f) {
            mMoveDirection = getRandomDirection();
        }

        if(Ogre::Root::getSingleton().getTimer()->getMilliseconds() > mInceptionTime + SPAWN_TIME) {
            mCurrentState = STATE_SEEKING;
		}

        break;
	case STATE_SEEKING:
        // Look for new target to absorb
        // It is necessary to do this every frame because otherwise, if two CellBs are targetting the 
		// same CellA and it gets absorbed, the second CellB would continue trying to target the non-existant 
		// cell, resulting in a segfault.
        mTarget = mWorld->getClosestCell<CellA>(mNode->getPosition());

        if(mTarget != NULL) {

            mMoveDirection = get2dDirectionFromVector(mTarget->getPosition() - mNode->getPosition());
		} else { // If there are no A cells, move in a random direction
            mMoveDirection = getRandomDirection();
            mCurrentState = STATE_WANDERING;
		}

        // Handle movement
        if(mTemperature >= MIN_MOVEMENT_TEMPERATURE) {
            mNode->translate(mMoveDirection * mMoveSpeed * (float)msDelta / 1000.0f);

            // If we're within range, begin absorbing the cell
            if(mTarget != NULL && (mTarget->getPosition() - mNode->getPosition()).normalise() < MAX_ABSORPTION_DISTANCE) {
                mCurrentState = STATE_ABSORBING;
                resetMitosisTimer();
			}
        }
        break;
	case STATE_WANDERING:
        // Active if there are no more A Cells left.
        mNode->translate(mMoveDirection * mMoveSpeed * (float)msDelta / 1000.0f);

        // If a new A cell is found, seek it
        if(!bInfected && mWorld->getClosestCell<CellA>() != NULL) {
            mCurrentState = STATE_SEEKING;
		}

        // If the cell is infected, see if it's time for it to die
        if(bInfected) {
            if(Ogre::Root::getSingleton().getTimer()->getMilliseconds() > mInfectionTime + mDeathTime) {
                denature();
			}
		}

        // Find a new direction to move in if we go outisde the bounds of the world
        if(mNode->getPosition().x > mWorld->getWorldSize().x || mNode->getPosition().y > mWorld->getWorldSize().y || mNode->getPosition().x < 0.0f || mNode->getPosition().y < 0.0f) {
            mMoveDirection = getRandomDirection();
        }
        break;
	case STATE_ABSORBING:
        // If time is up, absorb the cell
        if(Ogre::Root::getSingleton().getTimer()->getMilliseconds() > mLastSplitTime + mNextSplit) {
            // The target has already been absorbed by another cell
            if(!mWorld->hasLifeForm(mTarget)) {
                mTarget = NULL;
                mCurrentState = STATE_SEEKING;
                break;
			}

            // If the target is infected, catch the virus
            if(mTarget->isInfected()) {
                infect();
			} else {
                // Wait the same amount of time again before splitting
                mLastSplitTime = Ogre::Root::getSingleton().getTimer()->getMilliseconds();
                mCurrentState = STATE_SPLITTING;
			}

            // Absorb mTarget
            mWorld->removeLifeForm(mTarget);
            mTarget = NULL;
		}
        break;
	case STATE_SPLITTING:
        // If time is up, split()
        if(Ogre::Root::getSingleton().getTimer()->getMilliseconds() > mLastSplitTime + mNextSplit) {
            split();
		}
        break;
	default:
        break;
	}

}

void CellB::calculateTemperature(const int msDelta) {
    // Calculate temperature
    float targetTemp = mWorld->getTemperature() + CELL_INTERNAL_HEAT + mWorld->getNumLivingCells() * CELL_TRANSMITTED_HEAT;
    if(targetTemp > mTemperature) {
        mTemperature += (targetTemp - mTemperature) / TEMP_DIFF_PER_SECOND * ((float)msDelta / 1000.0f);
	} else if(targetTemp < mTemperature) {
        mTemperature -= (mTemperature - targetTemp) / TEMP_DIFF_PER_SECOND * ((float)msDelta / 1000.0f);
	}

    // Kill the cell if temperature out of range
    if(mTemperature > MAX_TEMPERATURE || mTemperature < MIN_TEMPERATURE) {
        denature();
	}
}

void CellB::resetMitosisTimer() {
    Cell::resetMitosisTimer();

    mNextSplit = ABSORPTION_TIMES[Random::getSingleton().getRand(0, 9)];

    // Infected cells are consumed 80% faster
    if(mTarget != NULL) {
        if(mTarget->isInfected()) {
            mNextSplit = (unsigned int)((float)mNextSplit * INFECTED_MITOSIS_TIME_MULTIPLIER);
		}
	}
}

void CellB::split() {
    mTemperature += MITOSIS_TEMP_INCREASE;

    // Spawn new cell
    mWorld->addCellB(mNode->getPosition());

    mCurrentState = STATE_SEEKING;
}

void CellB::infect() {
    Cell::infect();

    mCurrentState = STATE_WANDERING; // Cell won't absorb A cells or duplicate

    mInfectionTime = Ogre::Root::getSingleton().getTimer()->getMilliseconds();

    // Work out death time
    mDeathTime = DEATH_TIMES[Random::getSingleton().getRand(0, 9)];
}

Ogre::Vector3 CellB::get2dDirectionFromVector(const Ogre::Vector3 vIn) const {
    // Special case to prevent dividing by zero
    if(vIn.y == 0.0f) {
        if(vIn.x > 0.0f) {
            return Ogre::Vector3(1.0f, 0.0f, 0.0f);
		} else if(vIn.x < 0.0f) {
            return Ogre::Vector3(-1.0f, 0.0f, 0.0f);
		} else {
            return Ogre::Vector3::ZERO;
		}
	}	
    if(vIn.x == 0.0f) {
        if(vIn.y > 0.0f) {
            return Ogre::Vector3(0.0f, 1.0f, 0.0f);
		} else if(vIn.y < 0.0f) {
            return Ogre::Vector3(0.0f, -1.0f, 0.0f);
		} else {
            return Ogre::Vector3::ZERO;
		}
	}	
    Ogre::Radian theta = Ogre::Math::ATan(vIn.x / vIn.y);
    Ogre::Vector3 result = Ogre::Vector3(Ogre::Math::Sin(theta), Ogre::Math::Cos(theta), 0.0f);
    if(vIn.x < 0.0f) {
        result.x = -result.x;
	}
    if(vIn.y < 0.0f) {
        result.y = -result.y;
	}
    return result;
}
