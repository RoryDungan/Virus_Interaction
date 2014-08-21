#include "CellA.hpp"
#include "CellB.hpp"
#include "World.hpp"
#include "Random.hpp"
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreEntity.h>
#include <cmath>

// Cell mitosis times, based on real-world test results
// Initally I was planning on creating a formula to generate these mathematically but upon reflection,
// there are more than enough samples here for the demo and using the real test results will provide
// more accurate results.
const unsigned int CellA::MITOSIS_TIMES[] = {
    8500, 9500, 11900, 20400, 7100, 12200, 10900, 9400, 8900, 9200
};

const float CellA::NORMAL_TEMPERATURE = 35.0f;
const float CellA::MAX_TEMPERATURE = 45.0f; 
const float CellA::MIN_TEMPERATURE = 30.0f;
const float CellA::MITOSIS_TEMP_INCREASE = 3.0f; // Amount of heat generated during mitosis
const float CellA::TEMP_DIFF_PER_SECOND = 0.5f; // Rate at which the cell can warm or cool itself
const float CellA::CELL_INTERNAL_HEAT = 3.0f; // Total amount of heat the cell can generate
// Ex: Solution temperature is 27, cell generates 5 degrees extra heat meaning its 
// internal temperature is 32, just enough to survive. Splitting will increase this to 
// 35, although in 6 seconds this will drop back to 32.

const float CellA::MOVE_PER_SEC = 1.0f;

const float CellA::IMFECTED_MITOSIS_TIME_MULTIPLIER = 0.9f;
const float CellA::NORMAL_PH = 7.365f;
const float CellA::PH_MITOSIS_TIME_MULTIPLIER = 0.5f; // 10% per 0.2PH == 50% per 1PH
const Ogre::Real CellA::MAX_ABSORB_DIST = 15.0f; // Approxomately the size of CellB
const unsigned int CellA::ABSORB_TIME = 1000;
const unsigned int CellA::SPAWN_TIME = 3000;


CellA::CellA(World* world, const Ogre::Vector3 pos, const bool infected) : Cell(world) {
    // Create an entity
    Ogre::Entity* ent = mWorld->getSceneMgr()->createEntity(mName, "CellA.mesh");
    mNode = mWorld->getSceneMgr()->getRootSceneNode()->createChildSceneNode();
    mNode->attachObject(ent);
    mNode->setPosition(pos);

    // Set direction to move in
    mMoveDirection = getRandomDirection();
    mMoveSpeed = 20.0f; // Move faster at first to get away from parent and stop cells from clumping up

    // Default temperature to the normal cell temperature
    mTemperature = NORMAL_TEMPERATURE;
    mCurrentState = STATE_SPAWNING;
    mTargetB = NULL;

    if(infected) {
        infect();
	} else {
        bInfected = false;
	}

    resetMitosisTimer();
}


CellA::~CellA(void) {
    
}

void CellA::update(const int msDelta) {
    // Don't do anything if the cell is dead
    if(!bAlive) {
        return;
	}

    Cell::update(msDelta);

    calculateTemperature(msDelta);

    switch(mCurrentState) {
	case STATE_SPAWNING:
        mNode->translate(mMoveDirection * mMoveSpeed * (float)msDelta / 1000.0f);
        // Find a new direction to move in if we go outisde the bounds of the world
        if(mNode->getPosition().x > mWorld->getWorldSize().x || mNode->getPosition().y > mWorld->getWorldSize().y || mNode->getPosition().x < 0.0f || mNode->getPosition().y < 0.0f) {
            mMoveDirection = getRandomDirection();
        }

        if(Ogre::Root::getSingleton().getTimer()->getMilliseconds() > mInceptionTime + SPAWN_TIME) {
            mCurrentState = STATE_WANDERING;
            mMoveSpeed = MOVE_PER_SEC;
		}

        break;
	case STATE_WANDERING: {
        mNode->translate(mMoveDirection * mMoveSpeed * (float)msDelta / 1000.0f);
        if(mNode->getPosition().x > mWorld->getWorldSize().x || mNode->getPosition().y > mWorld->getWorldSize().y || mNode->getPosition().x < 0.0f || mNode->getPosition().y < 0.0f) {
            mMoveDirection = getRandomDirection();
        }

        if(Ogre::Root::getSingleton().getTimer()->getMilliseconds() > mLastSplitTime + mNextSplit) {
            split();
        }

        // Check to see if we're near enough to a dead CellB to absorb it
        CellB* b = mWorld->getClosestCell<CellB>(mNode->getPosition());
        if(b != NULL) {
            if(!b->isAlive() && mNode->getPosition().distance(b->getPosition()) <= MAX_ABSORB_DIST) {
                mAbsorbStartTime = Ogre::Root::getSingleton().getTimer()->getMilliseconds();
                mTargetB = b;
                mCurrentState = STATE_ABSORBING;

                // Catch the virus if it is present in the B Cell
                if(b->isInfected()) {
                    infect();
				}
			}
		}

        break;
	}
	case STATE_ABSORBING:
        if(Ogre::Root::getSingleton().getTimer()->getMilliseconds() > mAbsorbStartTime + ABSORB_TIME) {
            split();
            if(mTargetB) {
                mWorld->removeLifeForm(mTargetB);
			}
            mCurrentState = STATE_WANDERING;
		}

        break;
	default: 
		break;
	}
 //   if(mMoveSpeed > MOVE_PER_SEC) {
 //       mMoveSpeed -= 10.0f * (float)msDelta / 1000.0f;
	//}

}

void CellA::calculateTemperature(const int msDelta) {
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

// Reset timer and work out how long till next split
void CellA::resetMitosisTimer() {
    Cell::resetMitosisTimer();

    mNextSplit = MITOSIS_TIMES[Random::getSingleton().getRand(0, 9)];
    if(bInfected) {
        mNextSplit = (int)((float)mNextSplit * IMFECTED_MITOSIS_TIME_MULTIPLIER);
	}
    mNextSplit += mNextSplit * (int)(abs(mWorld->getPH() - NORMAL_PH) * PH_MITOSIS_TIME_MULTIPLIER);
}

void CellA::split() {
    split(bInfected);
}

void CellA::split(bool infected) {
    Cell::split();

    // Splitting genarates heat
    mTemperature += MITOSIS_TEMP_INCREASE;

    // Spawm new cell
    mWorld->addCellA(mNode->getPosition(), infected);
}