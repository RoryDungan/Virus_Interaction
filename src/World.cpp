#include "World.hpp"
#include "CellA.hpp"
#include "CellB.hpp"
#include "Random.hpp"

const Ogre::Vector3 World::WORLD_SIZE = Ogre::Vector3(200.0f, 200.0f, 0.0f);
const float World::DEFAULT_TEMPERATURE = 25.0f;
const float World::DEFAULT_PH = 7.365f;

World::World(Ogre::SceneManager* scene) {
    mSceneMgr = scene;

    reset(8, 4, 2);

    // Reset variables
    mTemperature = DEFAULT_TEMPERATURE;
    mPh = DEFAULT_PH;
}


World::~World(void)
{
}

void World::reset(unsigned int aCells, unsigned int bCells, unsigned int infectedACells) {
    // Clear all lifeforms
    for(unsigned int i = 0; i < vLifeForms.size(); i++) {
        //vLifeForms.erase(vLifeForms.begin() + i);
        delete vLifeForms[i];
	} 
    vLifeForms.clear();

    // Populate the world
    for(unsigned int i = 0; i < aCells; i++) {
        addCellA(getRandom2dPosition());
	}
    for(unsigned int i = 0; i < infectedACells; i++) {
        addCellA(getRandom2dPosition(), true);
	}
    for(unsigned int i = 0; i < bCells; i++) {
        addCellB(getRandom2dPosition());
	}
}

void World::update(int msDelta) {
    for(unsigned int i = 0; i < vLifeForms.size(); i++) {
        vLifeForms[i]->update(msDelta);
	}
}

void World::addCellA(const Ogre::Vector3 pos, const bool infected) {
    vLifeForms.push_back(new CellA(this, pos, infected));
}

void World::addCellB(const Ogre::Vector3 pos) {
    vLifeForms.push_back(new CellB(this, pos));
}

float World::getTemperature() const {
    return mTemperature;
}

float World::getPH() const {
    return mPh;
}

Ogre::SceneManager* World::getSceneMgr() {
    return mSceneMgr;
}

void World::setTemperature(float newTemp) {
    mTemperature = newTemp;
}

void World::setPh(float newPh) {
    mPh = newPh;
}

Ogre::Vector3 World::getRandom2dPosition() {
    Ogre::Real x = Ogre::Math::RangeRandom(0.0f, WORLD_SIZE.x);
    Ogre::Real y = Ogre::Math::RangeRandom(0.0f, WORLD_SIZE.y);
    return Ogre::Vector3(x, y, 0);
}

LifeForm* World::getLifeFormByName(const Ogre::String name) {
    for(unsigned int i = 0; i < vLifeForms.size(); i++) {
        if(vLifeForms[i]->getName() == name) {
            return vLifeForms[i];
		}
	}
    return NULL;
}

Ogre::Vector3 World::getWorldSize() const {
    return WORLD_SIZE;
}

unsigned int World::getNumLifeForms() const {
    return vLifeForms.size();
}

unsigned int World::getNumLivingCells() const {
    unsigned int tally = 0;
    for(unsigned int i = 0; i < vLifeForms.size(); i++) {
        if(dynamic_cast<Cell*>(vLifeForms[i]) != NULL) {
            if(static_cast<Cell*>(vLifeForms[i])->isAlive()) {
                tally++;
			}
		}
	}
    return tally;
}

bool World::removeLifeForm(LifeForm* lifeForm) {
    for(unsigned int i = 0; i < vLifeForms.size(); i++) {
        if(vLifeForms[i] == lifeForm) {
            delete vLifeForms[i];
            vLifeForms.erase(vLifeForms.begin() + i);
            return true;
		}
	}
    return false; // Could not find LifeForm at specified address
}

bool World::hasLifeForm(LifeForm* lifeForm) {
    for(unsigned int i = 0; i < vLifeForms.size(); i++) {
        if(vLifeForms[i] == lifeForm) {
            return true;
		}
	}
    return false;
}
