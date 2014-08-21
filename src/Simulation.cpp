#include "Simulation.hpp"
#include "CellA.hpp"
#include "CellB.hpp"
#include <CEGUI/PropertyHelper.h>

const float Simulation::CAMERA_MOVEMENT_SPEED = 50.0f;
const OIS::KeyCode Simulation::KEY_CAM_UP = OIS::KC_W;
const OIS::KeyCode Simulation::KEY_CAM_LEFT = OIS::KC_A;
const OIS::KeyCode Simulation::KEY_CAM_DOWN = OIS::KC_S;
const OIS::KeyCode Simulation::KEY_CAM_RIGHT = OIS::KC_D;

Simulation::Simulation(void) {
    mCameraMovement = Ogre::Vector3::ZERO;
    mSceneMgr = NULL;
    mSelectedLifeForm = NULL;
}


Simulation::~Simulation(void) {
    if(mSceneMgr) {
        mSceneMgr->destroyQuery(mRaySceneQuery);
	}
}

void Simulation::createScene() {
    // Set the scene's ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));

    // Set up camera
    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(100, 100, 80));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(5);

    // Set up world
    mWorld = new World(mSceneMgr);

	// Create a Light and set its position
    Ogre::Light* light = mSceneMgr->createLight("MainLight");
    light->setPosition(20.0f, 80.0f, 50.0f);

    // Set up GUI
    mHud = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("hud.layout");
    mRootWindow->addChild(mHud);

    mRootWindow->getChild("Sidebar/QuitButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&Simulation::quitEvent, this));
    mRootWindow->getChild("Sidebar/ResetButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&Simulation::resetEvent, this));
    mRootWindow->getChild("Sidebar/EnvGroupBox/TempSpinner")->subscribeEvent(CEGUI::Spinner::EventValueChanged, CEGUI::SubscriberSlot(&Simulation::tempChangedEvent, this));
    mRootWindow->getChild("Sidebar/EnvGroupBox/PhSpinner")->subscribeEvent(CEGUI::Spinner::EventValueChanged, CEGUI::SubscriberSlot(&Simulation::phChangedEvent, this));
    mRootWindow->getChild("Sidebar/EnvGroupBox/ACellsSpinner")->subscribeEvent(CEGUI::Spinner::EventValueChanged, CEGUI::SubscriberSlot(&Simulation::numCellsChangedEvent, this));
    mRootWindow->getChild("Sidebar/EnvGroupBox/ACellsSpinner")->subscribeEvent(CEGUI::Spinner::EventValueChanged, CEGUI::SubscriberSlot(&Simulation::numCellsChangedEvent, this));
    mRootWindow->getChild("Sidebar/EnvGroupBox/ACellsSpinner")->subscribeEvent(CEGUI::Spinner::EventValueChanged, CEGUI::SubscriberSlot(&Simulation::numCellsChangedEvent, this));

    static_cast<CEGUI::Spinner*>(mRootWindow->getChild("Sidebar/EnvGroupBox/TempSpinner"))->setCurrentValue((double)mWorld->getTemperature());
    static_cast<CEGUI::Spinner*>(mRootWindow->getChild("Sidebar/EnvGroupBox/PhSpinner"))->setCurrentValue((double)mWorld->getPH());
}

void Simulation::createFrameListener(void) {
    BaseApplication::createFrameListener();

    mRaySceneQuery = mSceneMgr->createRayQuery(Ogre::Ray());
}

void Simulation::calculateCameraMovement() {
    float x = 0.0f;
    float y = 0.0f;
    if(mKeyboard->isKeyDown(KEY_CAM_UP) && mCamera->getPosition().y < mWorld->getWorldSize().y - 20.0f) {
		y += CAMERA_MOVEMENT_SPEED;
	} else if(mKeyboard->isKeyDown(KEY_CAM_LEFT) && mCamera->getPosition().x > 50.0f) {
		x -= CAMERA_MOVEMENT_SPEED;
	} else if(mKeyboard->isKeyDown(KEY_CAM_DOWN) && mCamera->getPosition().y > 50.0f) {
		y -= CAMERA_MOVEMENT_SPEED;
	} else if(mKeyboard->isKeyDown(KEY_CAM_RIGHT) && mCamera->getPosition().x < mWorld->getWorldSize().x - 50.0f) {
		x += CAMERA_MOVEMENT_SPEED;
	}

    mCameraMovement = Ogre::Vector3(x, y, 0.0f);
}

bool Simulation::frameRenderingQueued(const Ogre::FrameEvent& evt) {
    if(!BaseApplication::frameRenderingQueued(evt)) {
        return false;
	}

    mWorld->update(int(evt.timeSinceLastFrame * 1000));

    // Make sure mSelectedLifeForm hasn't been deleted
    if(!mWorld->hasLifeForm(mSelectedLifeForm)) {
        mSelectedLifeForm = NULL;
        selectLifeForm(NULL);
	}

    calculateCameraMovement();
    mCamera->setPosition(mCamera->getPosition() + mCameraMovement * evt.timeSinceLastFrame);

    updateInfoBox();

    return true;
}

bool Simulation::keyPressed(const OIS::KeyEvent& keyEventRef) {
    if(!BaseApplication::keyPressed(keyEventRef)) {
        return false;
	}

    return true;
}

bool Simulation::keyReleased(const OIS::KeyEvent& keyEventRef) {
    if(!BaseApplication::keyReleased(keyEventRef)) {
        return false;
	}

	switch (keyEventRef.key)
	{
	case OIS::KC_ESCAPE:
        bShutdown = true;
        break;
	default:
		break;
	}

    return true;
}

bool Simulation::mouseMoved(const OIS::MouseEvent& evt) {
    if(!BaseApplication::mouseMoved(evt)) {
        return false;
	}

    return true;
}

bool Simulation::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id) {
    if(!BaseApplication::mousePressed(evt, id)) {
        return false;
	}

    // Select LifeForm under cursor
    CEGUI::GUIContext &context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Vector2f mousePos = context.getMouseCursor().getPosition();

    Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.d_x/float(evt.state.width), mousePos.d_y/float(evt.state.height));
    mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setSortByDistance(true);

    Ogre::RaySceneQueryResult& result = mRaySceneQuery->execute();
    Ogre::RaySceneQueryResult::iterator iter = result.begin();

    if(iter != result.end() && iter->movable) {
        LifeForm* lifeForm = mWorld->getLifeFormByName(iter->movable->getName());
        if(lifeForm != NULL) {
            selectLifeForm(lifeForm);
		} else {
            selectLifeForm(NULL);
		}
	} else if(mSelectedLifeForm != NULL) {
        //selectLifeForm(NULL);
	}

    return true;
}

void Simulation::selectLifeForm(LifeForm* lifeForm) {
    // Update bounding box
    if(mSelectedLifeForm) {
        mSelectedLifeForm->showSelected(false);
	}
    if(lifeForm) {
        lifeForm->showSelected(true);
	} 

    // Update mSelectedLifeForm
    mSelectedLifeForm = lifeForm;
}

void Simulation::updateInfoBox() {
    CEGUI::String message;
    if(!mSelectedLifeForm) {
        message = "WASD to move the viewport.\n\nClick on a life form to learn more about it.\n\n";
	} else {
        // Work out what type of LifeForm was selected using RTTI
        if(dynamic_cast<CellA*>(mSelectedLifeForm) != NULL) {
            Cell* cell = dynamic_cast<Cell*>(mSelectedLifeForm);
            message = "Type A cell selected.\n\nCell temperature: " + CEGUI::PropertyHelper<float>::toString(cell->getTemperature()) + "\n\n";
            message += "Cell status: ";
			if(cell->isAlive()) {
				if(cell->isInfected()) {
					message += "Infected.\n\n";
				} else {
					message += "Normal.\n\n"; 
				}
			} else {
				message += "Dead.\n\n";
			}
		} else if(dynamic_cast<CellB*>(mSelectedLifeForm) != NULL) {
            Cell* cell = dynamic_cast<Cell*>(mSelectedLifeForm);
            message = "Type B cell selected.\n\nCell temperature: " + CEGUI::PropertyHelper<float>::toString(dynamic_cast<Cell*>(mSelectedLifeForm)->getTemperature()) + "\n\n";
            message += "Cell status: ";
			if(cell->isAlive()) {
				if(cell->isInfected()) {
					message += "Infected\n\n";
				} else {
					message += "Normal\n\n"; 
				}
			} else {
				message += "Dead\n\n";
			}
		} else {
            message = "Unknown life form selected.";
		}
	}

    message += "Total number of life forms: " + CEGUI::PropertyHelper<unsigned int>::toString(mWorld->getNumLifeForms()) + ".\n";
    message += "Total number of living cells: " + CEGUI::PropertyHelper<unsigned int>::toString(mWorld->getNumLivingCells()) + ".\n\n";

    // Update the actual info box
    mRootWindow->getChild("Sidebar/InfoGroupBox/Label")->setText(message);
}


bool Simulation::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id) {
    if(!BaseApplication::mouseReleased(evt, id)) {
        return false;
	}

    return true;
}

bool Simulation::quitEvent(const CEGUI::EventArgs& evt) {
    bShutdown = true;
    return true;
}

bool Simulation::resetEvent(const CEGUI::EventArgs& evt) {
    unsigned int aCells = (unsigned int) static_cast<CEGUI::Spinner*>(mRootWindow->getChild("Sidebar/EnvGroupBox/ACellsSpinner"))->getCurrentValue();
    unsigned int bCells = (unsigned int) static_cast<CEGUI::Spinner*>(mRootWindow->getChild("Sidebar/EnvGroupBox/BCellsSpinner"))->getCurrentValue();
    unsigned int iCells = (unsigned int) static_cast<CEGUI::Spinner*>(mRootWindow->getChild("Sidebar/EnvGroupBox/ICellsSpinner"))->getCurrentValue();
    mWorld->reset(aCells, bCells, iCells);

    mSelectedLifeForm = NULL;

    mRootWindow->getChild("Sidebar/InfoLabel")->setText("");

    return true;
}

bool Simulation::tempChangedEvent(const CEGUI::EventArgs& evt) {
    mWorld->setTemperature((float)static_cast<CEGUI::Spinner*>(mRootWindow->getChild("Sidebar/EnvGroupBox/TempSpinner"))->getCurrentValue());
    return true;
}

bool Simulation::phChangedEvent(const CEGUI::EventArgs& evt) {
    mWorld->setPh((float)static_cast<CEGUI::Spinner*>(mRootWindow->getChild("Sidebar/EnvGroupBox/PhSpinner"))->getCurrentValue());
    return true;
}

bool Simulation::numCellsChangedEvent(const CEGUI::EventArgs& evt) {
    mRootWindow->getChild("Sidebar/InfoLabel")->setText("Reset to see changes.");
    return true;
}