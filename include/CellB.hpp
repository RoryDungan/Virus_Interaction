#pragma once
#include "Cell.hpp"

class CellA;

class CellB :
	public Cell
{
public:
	CellB(World* world, const Ogre::Vector3 pos = Ogre::Vector3::ZERO);
	virtual ~CellB(void);

    virtual void update(const int msDelta);

protected:
    virtual void resetMitosisTimer();
    virtual void split();
    virtual void infect();

private:
    virtual void calculateTemperature(const int msDelta);
    virtual Ogre::Vector3 get2dDirectionFromVector(const Ogre::Vector3 vIn) const;

    CellA* mTarget;
    unsigned int mInfectionTime;
    unsigned int mDeathTime;

    // CellB is slightly more complicated than CellA as there are several stages to its life cycle
    enum CellState {
        STATE_SEEKING, STATE_ABSORBING, STATE_SPLITTING, STATE_SPAWNING, STATE_WANDERING
	} mCurrentState;

    static const float NORMAL_TEMPERATURE;
	static const float MAX_TEMPERATURE;
    static const float MIN_TEMPERATURE;
    static const float MIN_MOVEMENT_TEMPERATURE; // Below this temperature the cell will not die but cannot move
    static const float MITOSIS_TEMP_INCREASE; // Amount of heat generated during mitosis
    static const float TEMP_DIFF_PER_SECOND; // Rate at which the cell can warm or cool itself
    static const float CELL_INTERNAL_HEAT; // Total amount of heat the cell can generate

    static const unsigned int ABSORPTION_TIMES[];
    static const unsigned int DEATH_TIMES[];
    static const Ogre::Real MAX_ABSORPTION_DISTANCE; // Max distance from CellB to CellA before A can be absorbed
    static const float INFECTION_CHANCE;
    static const float INFECTED_MITOSIS_TIME_MULTIPLIER;
    static const float NORMAL_PH;
    static const float PH_MITOSIS_TIME_MULTIPLIER; // 10% per 0.2PH == 50% per 1PH
    static const float MOVE_PER_SEC;
    static const unsigned int SPAWN_TIME;
};

