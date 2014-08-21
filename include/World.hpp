#pragma once

#include <vector>
#include <OGRE/OgreSceneManager.h>

#include "LifeForm.hpp"
#include "CellA.hpp"

class World
{
public:
	World(Ogre::SceneManager* scene);
	virtual ~World(void);

    void reset(unsigned int aCells, unsigned int bCells, unsigned int intectedACells);
    void update(int msDelta);
    void addCellA(const Ogre::Vector3 pos = Ogre::Vector3::ZERO, const bool infected = false);
    void addCellB(const Ogre::Vector3 pos = Ogre::Vector3::ZERO);

    // Mutators
    void setTemperature(float newTemp);
    void setPh(float newPh);
    // Accessors
    float getTemperature() const;
    float getPH() const;
    Ogre::SceneManager* getSceneMgr();
    Ogre::Vector3 getWorldSize() const;
    unsigned int getNumLifeForms() const;   // Total
    template <typename T> unsigned int getNumLifeForms() const; // Specific type
    unsigned int getNumLivingCells() const; // Gets the number of LifeForms that are cells and are alive.
    bool removeLifeForm(LifeForm* lifeForm); // Called when CellB absorbs a CellA
    bool hasLifeForm(LifeForm* lifeForm); // Used to verify that lifeforms still exist
    template <typename T> T* getClosestCell(const Ogre::Vector3 pos = Ogre::Vector3::ZERO);// Used by CellB to find the nearest CellA to track and consume

    LifeForm* getLifeFormByName(const Ogre::String name);

private:
    Ogre::Vector3 getRandom2dPosition();

    static const float DEFAULT_TEMPERATURE;
    static const float DEFAULT_PH;
    float mTemperature;
    float mPh;
    unsigned int mACells, mBCells, mInfectedACells;
    static const Ogre::Vector3 WORLD_SIZE;

    std::vector<LifeForm*> vLifeForms;

    Ogre::SceneManager* mSceneMgr;
};

template <typename T> T* World::getClosestCell(const Ogre::Vector3 pos) {
    Ogre::Real bestDist = 10000.0f;
    T* bestCell = NULL;
    for(unsigned int i = 0; i < vLifeForms.size(); i++) {
        if(dynamic_cast<T*>(vLifeForms[i]) != NULL) {
            T* currentCell = static_cast<T*>(vLifeForms[i]);
            Ogre::Real dist = currentCell->getPosition().distance(pos);
            if(dist < bestDist) {
                bestCell = currentCell;
                bestDist = dist;
			}
		}
	}
    return bestCell;
}

template <typename T> unsigned int World::getNumLifeForms() const {
    unsigned int tally;
    for(unsigned int i = 0; i < vLifeForms.size(); i++) {
        if(dynamic_cast<T*>(vLifeForms[i]) != NULL) {
            tally++;
		}
	}
    return tally;
}