#pragma once
#include "Cell.hpp"

class CellB;

class CellA :
	public Cell
{
public:
	CellA(World* world, const Ogre::Vector3 pos = Ogre::Vector3::ZERO, const bool infected = false);
	virtual ~CellA(void);

    virtual void update(const int msDelta);

protected:
    virtual void resetMitosisTimer();
    virtual void split();
    virtual void split(bool infected);

private:
    virtual void calculateTemperature(const int msDelta);

    enum CellState { // CellA splits and instantly starts wandering again, hence why there is no need for STATE_SPLITTING as in CellB
        STATE_SPAWNING, STATE_WANDERING, STATE_ABSORBING
	} mCurrentState;

    unsigned int mAbsorbStartTime;
    CellB* mTargetB;

    // Temperature constants.
    // Note that these were not specified in the design document, only that the cells must
    // react to heat and cold
    static const float NORMAL_TEMPERATURE;
	static const float MAX_TEMPERATURE;
    static const float MIN_TEMPERATURE;
    static const float MITOSIS_TEMP_INCREASE; // Amount of heat generated during mitosis
    static const float TEMP_DIFF_PER_SECOND; // Rate at which the cell can warm or cool itself
    static const float CELL_INTERNAL_HEAT; // Total amount of heat the cell can generate
    // Ex: Solution temperature is 27, cell generates 5 degrees extra heat meaning its 
    // internal temperature is 32, just enough to survive. Splitting will increase this to 
    // 35, although in 6 seconds this will drop back to 32.

    static const unsigned int MITOSIS_TIMES[];
    static const float IMFECTED_MITOSIS_TIME_MULTIPLIER;
    static const float NORMAL_PH;
    static const float PH_MITOSIS_TIME_MULTIPLIER; // 10% per 0.2PH == 50% per 1PH
    static const float MOVE_PER_SEC;
    static const Ogre::Real MAX_ABSORB_DIST;
    static const unsigned int ABSORB_TIME;
    static const unsigned int SPAWN_TIME;
};

